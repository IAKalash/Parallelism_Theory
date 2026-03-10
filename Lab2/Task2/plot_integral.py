import matplotlib.pyplot as plt

threads = [1, 2, 4, 7, 8, 16, 20, 40]
time_vals = [4.842474, 2.450330, 1.247142, 0.721454, 0.640374, 0.326440, 0.278038, 0.172482]

speedup = [time_vals[0] / t for t in time_vals]
ideal_speedup = threads

plt.figure(figsize=(10, 6))
plt.plot(threads, speedup, marker='o', linestyle='-', color='b', label='Экспериментальное ускорение')
plt.plot(threads, ideal_speedup, marker='', linestyle='--', color='gray', label='Идеальное ускорение')

plt.title('График зависимости ускорения от числа потоков (Численное интегрирование)')
plt.xlabel('Количество потоков (p)')
plt.ylabel('Ускорение (S)')
plt.legend()
plt.grid(True)
plt.xticks(threads)

plt.savefig('big_integral_speedup.png', dpi=300)
print("График интеграла сохранен!")
