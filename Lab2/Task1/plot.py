import matplotlib.pyplot as plt

threads = [1, 2, 4, 7, 8, 16, 20, 40]

time_20k = [1.284710, 0.685136, 0.319801, 0.193153, 0.165974, 0.086106, 0.067321, 0.037416]
speedup_20k = [time_20k[0] / t for t in time_20k]

time_40k = [6.221484, 4.118782, 1.706965, 0.915792, 0.683131, 0.324849, 0.267043, 0.267401]
speedup_40k = [time_40k[0] / t for t in time_40k]

ideal_speedup = threads

plt.figure(figsize=(10, 6))
plt.plot(threads, speedup_20k, marker='o', linestyle='-', label='Размер 20000x20000')
plt.plot(threads, speedup_40k, marker='s', linestyle='-', label='Размер 40000x40000')
plt.plot(threads, ideal_speedup, marker='', linestyle='--', color='gray', label='Идеальное ускорение')

plt.title('График зависимости линейного ускорения от числа потоков')
plt.xlabel('Количество потоков (p)')
plt.ylabel('Ускорение (S)')
plt.legend()
plt.grid(True)
plt.xticks(threads)

plt.savefig('speedup_graph.png', dpi=300)
print("График успешно сохранен в speedup_graph.png")
