import matplotlib.pyplot as plt


THREADS = [1, 2, 4, 7, 8, 16, 20, 40]

TIME_20000 = [1.221274, 0.646584, 0.306653, 0.228191, 0.153956, 0.089559, 0.069623, 0.073889]
TIME_40000 = [5.034208, 2.568334, 1.284437, 0.718840, 0.627904, 0.344710, 0.297461, 0.255635]
TIME_20000_PINNED = [1.234209, 0.654540, 0.292870, 0.179817, 0.162010, 0.138518, 0.110020, 0.060793]


def calc_speedup(times):
    t1 = times[0]
    return [t1 / t for t in times]


def main():
    speedup_20000 = calc_speedup(TIME_20000)
    speedup_40000 = calc_speedup(TIME_40000)
    speedup_20000_pinned = calc_speedup(TIME_20000_PINNED)
    ideal = THREADS

    plt.figure(figsize=(10, 6))
    plt.plot(THREADS, speedup_20000, marker="o", linewidth=2.5, color="#1f77b4", label="20000x20000")
    plt.plot(THREADS, speedup_40000, marker="s", linewidth=2.5, color="#ff7f0e", label="40000x40000")
    plt.plot(THREADS, ideal, linestyle="--", linewidth=2, color="gray", label="Линейное S(p)=p")

    plt.title("Ускорение матрично-векторного умножения", fontsize=14)
    plt.xlabel("Число потоков", fontsize=12)
    plt.ylabel("Ускорение S(p)", fontsize=12)
    plt.grid(True, linestyle="--", alpha=0.5)
    plt.xticks(THREADS)
    plt.legend(fontsize=11)
    plt.tight_layout()
    plt.savefig("speedup_graph.png", dpi=160)
    plt.close()

    plt.figure(figsize=(10, 6))
    plt.plot(THREADS, speedup_20000, marker="o", linewidth=2.5, color="#1f77b4", label="20000x20000 без закрепления")
    plt.plot(THREADS, speedup_20000_pinned, marker="^", linewidth=2.5, color="#2ca02c", label="20000x20000 с закреплением")
    plt.plot(THREADS, ideal, linestyle="--", linewidth=2, color="gray", label="Линейное S(p)=p")

    plt.title("Сравнение ускорения: без закрепления vs с закреплением", fontsize=14)
    plt.xlabel("Число потоков", fontsize=12)
    plt.ylabel("Ускорение S(p)", fontsize=12)
    plt.grid(True, linestyle="--", alpha=0.5)
    plt.xticks(THREADS)
    plt.legend(fontsize=10)
    plt.tight_layout()
    plt.savefig("speedup_pin_compare.png", dpi=160)
    plt.close()


if __name__ == "__main__":
    main()