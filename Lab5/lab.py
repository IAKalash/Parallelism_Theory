import argparse
import logging
import queue
import threading
import time
from dataclasses import dataclass
from pathlib import Path
import cv2
import numpy as np
from ultralytics import YOLO


INPUT_SENTINEL = object()
OUTPUT_SENTINEL = object()
DEFAULT_MODEL = "yolov8s-pose.pt"


@dataclass
class FrameItem:
    index: int
    frame: np.ndarray


def setup_logging(verbose: bool = False):
    level = logging.DEBUG if verbose else logging.INFO
    logging.basicConfig(level=level, format="%(asctime)s - %(levelname)s - %(message)s")


def open_capture(source: str, width: int | None, height: int | None):
    try:
        src = int(source) if str(source).isdigit() else source
    except Exception:
        src = source
    cap = cv2.VideoCapture(src)
    if not cap.isOpened():
        raise RuntimeError(f"Failed to open source: {source}")
    if width:
        cap.set(cv2.CAP_PROP_FRAME_WIDTH, int(width))
    if height:
        cap.set(cv2.CAP_PROP_FRAME_HEIGHT, int(height))
    return cap


def create_writer(path: str, fps: float, w: int, h: int):
    Path(path).parent.mkdir(parents=True, exist_ok=True)
    fourcc = cv2.VideoWriter_fourcc(*"mp4v")
    writer = cv2.VideoWriter(path, fourcc, fps, (w, h))
    if not writer.isOpened():
        raise RuntimeError("Failed to open VideoWriter")
    return writer


def worker_func(worker_id, model_path, imgsz, conf, in_q: queue.Queue, out_q: queue.Queue):
    logging.debug(f"Worker {worker_id} starting...")
    model = YOLO(model_path)
    while True:
        item = in_q.get()
        if item is INPUT_SENTINEL:
            logging.debug(f"Worker {worker_id} received shutdown signal")
            out_q.put(OUTPUT_SENTINEL)
            return
        idx, frame = item.index, item.frame
        logging.debug(f"Worker {worker_id} processing frame {idx}")
        res = model.predict(source=frame, device="cpu", imgsz=imgsz, conf=conf, verbose=False)[0]
        out = res.plot()
        out_q.put((idx, out))


def run_pipeline(cap, model_path, imgsz, conf, threads, max_frames, writer=None, display=False):
    in_q: queue.Queue = queue.Queue(maxsize=max(4, threads * 2))
    out_q: queue.Queue = queue.Queue()

    stop_event = threading.Event()

    def reader():
        i = 0
        while not stop_event.is_set():
            if max_frames and i >= max_frames:
                break
            ok, frame = cap.read()
            if not ok:
                break
            in_q.put(FrameItem(i, frame))
            i += 1
        for _ in range(threads):
            in_q.put(INPUT_SENTINEL)

    reader_t = threading.Thread(target=reader)
    workers = [threading.Thread(target=worker_func, args=(i, model_path, imgsz, conf, in_q, out_q)) for i in range(threads)]

    for w in workers:
        w.start()
    reader_t.start()
    logging.info(f"Pipeline started with {threads} workers")

    start = time.perf_counter()
    pending = {}
    next_idx = 0
    written = 0
    finished_workers = 0

    try:
        while True:
            item = out_q.get()
            if item is OUTPUT_SENTINEL:
                finished_workers += 1
                if finished_workers >= threads:
                    break
                continue

            idx, out = item

            if display:
                cv2.imshow("realtime", out)
                if cv2.waitKey(1) & 0xFF == ord('q'):
                    break

            if writer:
                pending[idx] = out
                while next_idx in pending:
                    frame_to_write = pending.pop(next_idx)
                    writer.write(frame_to_write)
                    next_idx += 1
                    written += 1
            else:
                written += 1
    finally:
        stop_event.set()
        reader_t.join(timeout=1.0)
        for worker in workers:
            worker.join(timeout=1.0)
        cap.release()
        if display:
            cv2.destroyAllWindows()

    elapsed = time.perf_counter() - start
    return elapsed, written


def parse_args():
    p = argparse.ArgumentParser(description="Lab5: multithreaded pose processing")
    p.add_argument("--source", type=str, default="0", help="Video file or camera id (default 0)")
    p.add_argument("--output", type=str, default=None, help="Output video path (optional)")
    p.add_argument("--display", action="store_true", help="Show annotated frames in realtime")
    p.add_argument("--threads", type=int, default=2, help="Worker threads (use 1 for single-thread behavior)")
    p.add_argument("--model", type=str, default=DEFAULT_MODEL)
    p.add_argument("--imgsz", type=int, default=640)
    p.add_argument("--conf", type=float, default=0.25)
    p.add_argument("--width", type=int, default=640)
    p.add_argument("--height", type=int, default=480)
    p.add_argument("--max-frames", type=int, default=0, help="Optional: limit frames for camera/stream")
    p.add_argument("--verbose", action="store_true")
    return p.parse_args()


def main():
    args = parse_args()
    setup_logging(args.verbose)

    cap = open_capture(args.source, args.width, args.height)
    fps = cap.get(cv2.CAP_PROP_FPS) or 30.0
    w = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
    h = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
    max_frames = args.max_frames if args.max_frames > 0 else None

    writer = None
    if args.output:
        writer = create_writer(args.output, fps, w, h)

    elapsed, total = run_pipeline(cap, args.model, args.imgsz, args.conf, max(1, args.threads), max_frames, writer=writer, display=args.display)

    if writer:
        writer.release()

    print(f"Processed {total} frames in {elapsed:.3f} s with {args.threads} threads")


if __name__ == "__main__":
    main()
