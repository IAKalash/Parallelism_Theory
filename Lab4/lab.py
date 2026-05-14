import time
import queue
import logging
import threading
import argparse
import cv2
from pathlib import Path


log_dir = Path("log")
log_dir.mkdir(exist_ok=True)
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    handlers=[
        logging.FileHandler(log_dir / "lab4.log"),
        logging.StreamHandler()
    ]
)
logger = logging.getLogger(__name__)


class Sensor:
    def get(self):
        raise NotImplementedError("Subclasses must implement method get()")


class SensorX(Sensor):
    '''Sensor X'''
    def __init__(self, delay):
        self._delay = delay
        self._data = 0

    def get(self):
        time.sleep(self._delay)
        self._data += 1
        return self._data


class SensorCam(Sensor):
    def __init__(self, camera_name, resolution):
        self.logger = logging.getLogger("SensorCam")
        self.camera_name = camera_name
        self.resolution_str = resolution
        self.cap = None
        
        try:
            camera_id = int(camera_name) if camera_name.isdigit() else camera_name
            self.cap = cv2.VideoCapture(camera_id)
            
            if not self.cap.isOpened():
                raise RuntimeError(f"Failed to open camera: {camera_name}")
            
            if resolution:
                width, height = map(int, resolution.split('x'))
                self.cap.set(cv2.CAP_PROP_FRAME_WIDTH, width)
                self.cap.set(cv2.CAP_PROP_FRAME_HEIGHT, height)
            
            self.logger.info(f"Camera initialized: {camera_name}, resolution: {resolution}")
        except Exception as e:
            self.logger.error(f"Failed to initialize camera: {e}")
            raise

    def get(self):
        if self.cap is None:
            self.logger.error("Camera is not initialized")
            return None
        
        ret, frame = self.cap.read()
        if not ret:
            self.logger.error("Failed to read frame from camera")
            return None
        return frame

    def __del__(self):
        if self.cap is not None:
            self.cap.release()
            self.logger.info("Camera released")


class WindowImage:
    def __init__(self, display_freq):
        self.logger = logging.getLogger("WindowImage")
        self.display_freq = display_freq
        self.frame_time = 1.0 / display_freq if display_freq > 0 else 0.033
        self.window_name = "Sensor Data Visualization"
        
        try:            
            cv2.namedWindow(self.window_name, cv2.WINDOW_NORMAL)
            self.logger.info(f"Window created with display frequency: {display_freq} Hz")
        except Exception as e:
            self.logger.error(f"Failed to initialize window: {e}")
            raise

    def show(self, img):
        try:
            if img is None:
                self.logger.warning("Received None image, skipping display")
                return
            
            cv2.imshow(self.window_name, img)
            key = cv2.waitKey(int(self.frame_time * 1000)) & 0xFF
            if key == ord('q'):
                return False
            return True
        except Exception as e:
            self.logger.error(f"Error displaying image: {e}")
            return True

    def __del__(self):
        try:
            cv2.destroyWindow(self.window_name)
            self.logger.info("Window destroyed")
        except Exception as e:
            self.logger.error(f"Error destroying window: {e}")


def sensor_thread_worker(sensor, sensor_id, data_queue, stop_event):
    logger = logging.getLogger(f"SensorThread-{sensor_id}")
    logger.info(f"Sensor thread started: {sensor_id}")
    
    while not stop_event.is_set():
        try:
            data = sensor.get()
            data_queue.put_nowait((sensor_id, data))
        except queue.Full:
            logger.debug(f"Queue full for {sensor_id}, dropping oldest sample")
            try:
                data_queue.get_nowait()
            except queue.Empty:
                pass
            try:
                data_queue.put_nowait((sensor_id, data))
            except queue.Full:
                logger.debug(f"Skipping sample for {sensor_id}, queue still full")
        except Exception as e:
            logger.error(f"Error reading from sensor: {e}")
    
    logger.info(f"Sensor thread stopped: {sensor_id}")


def main():
    parser = argparse.ArgumentParser(description="Lab4: Sensor data collection and visualization")
    parser.add_argument('--camera', type=str, default='0', help='Camera device name (default: 0)')
    parser.add_argument('--resolution', type=str, default='640x480', help='Camera resolution (default: 640x480)')
    parser.add_argument('--display-freq', type=float, default=10.0, help='Display frequency in Hz (default: 10.0)')
    
    args = parser.parse_args()
    
    logger.info("Lab4 starting...")
    logger.info(f"Args: camera={args.camera}, resolution={args.resolution}, display_freq={args.display_freq}")
    
    try:
        sensor_cam = SensorCam(args.camera, args.resolution)
    except Exception as e:
        logger.error(f"Failed to create camera sensor: {e}")
        return
    
    sensor0 = SensorX(0.01)   # 100 Hz
    sensor1 = SensorX(0.1)    # 10 Hz
    sensor2 = SensorX(1.0)    # 1 Hz
    
    sensor_queues = {
        'cam': queue.Queue(maxsize=5),
        'sensor0': queue.Queue(maxsize=5),
        'sensor1': queue.Queue(maxsize=5),
        'sensor2': queue.Queue(maxsize=5),
    }
    
    try:
        window = WindowImage(args.display_freq)
    except Exception as e:
        logger.error(f"Failed to create window: {e}")
        return
    
    stop_event = threading.Event()
    
    threads = []
    t = threading.Thread(target=sensor_thread_worker, args=(sensor_cam, 'cam', sensor_queues['cam'], stop_event))
    t.start()
    threads.append(t)
    
    t0 = threading.Thread(target=sensor_thread_worker, args=(sensor0, 'sensor0', sensor_queues['sensor0'], stop_event))
    t0.start()
    threads.append(t0)
    
    t1 = threading.Thread(target=sensor_thread_worker, args=(sensor1, 'sensor1', sensor_queues['sensor1'], stop_event))
    t1.start()
    threads.append(t1)
    
    t2 = threading.Thread(target=sensor_thread_worker, args=(sensor2, 'sensor2', sensor_queues['sensor2'], stop_event))
    t2.start()
    threads.append(t2)
    
    logger.info(f"Started {len(threads)} sensor threads")
    
    sensor_data = {
        'cam': None,
        'sensor0': None,
        'sensor1': None,
        'sensor2': None,
    }
    
    frame_count = 0
    display_continue = True
    
    while display_continue:
        for sensor_id, q in sensor_queues.items():
            try:
                while True:
                    try:
                        _, data = q.get_nowait()
                        sensor_data[sensor_id] = data
                    except queue.Empty:
                        break
            except Exception as e:
                logger.error(f"Error getting data from {sensor_id}: {e}")
        
        if sensor_data['cam'] is not None:
            img = sensor_data['cam'].copy()
            
            cv2.putText(img, f"Sensor0: {sensor_data['sensor0']}", (10, 30), 
                       cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
            cv2.putText(img, f"Sensor1: {sensor_data['sensor1']}", (10, 70),
                       cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
            cv2.putText(img, f"Sensor2: {sensor_data['sensor2']}", (10, 110),
                       cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
            cv2.putText(img, f"Frame: {frame_count}", (10, 150),
                       cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
            
            result = window.show(img)
            if result is False:
                logger.info("Quit signal received from window")
                display_continue = False
            frame_count += 1
        else:
            print(f"\rFrame {frame_count} - Sensor0: {sensor_data['sensor0']}, "
                  f"Sensor1: {sensor_data['sensor1']}, Sensor2: {sensor_data['sensor2']}", end='', flush=True)
            frame_count += 1
            time.sleep(1.0 / args.display_freq)
    
    logger.info("Main loop exiting, stopping sensor threads...")
    
    stop_event.set()
    
    for t in threads:
        t.join(timeout=5.0)


if __name__ == '__main__':
    main()
