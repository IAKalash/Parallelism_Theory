import pandas as pd
import matplotlib.pyplot as plt
import os

def create_plots(csv_file):
    if not os.path.exists(csv_file):
        print(f"Error: {csv_file} does not exist")
        return

    # Load data
    df = pd.read_csv(csv_file, names=['threads', 'time_v1', 'time_v2'])
    
    # Sort just in case
    df = df.sort_values(by='threads')
    
    threads = df['threads']
    t1_v1 = df['time_v1'].iloc[0] # time on 1 thread
    t1_v2 = df['time_v2'].iloc[0] # time on 1 thread
    
    # Calculate Speedup S(p) = T(1) / T(p)
    df['speedup_v1'] = t1_v1 / df['time_v1']
    df['speedup_v2'] = t1_v2 / df['time_v2']
    
    # Calculate Efficiency E(p) = S(p) / p
    df['eff_v1'] = df['speedup_v1'] / df['threads']
    df['eff_v2'] = df['speedup_v2'] / df['threads']

    # Plot Time
    plt.figure(figsize=(10, 6))
    plt.plot(threads, df['time_v1'], marker='o', label='Version 1 (many parallel for)')
    plt.plot(threads, df['time_v2'], marker='s', label='Version 2 (one parallel)')
    plt.xlabel('Number of threads')
    plt.ylabel('Time (seconds)')
    plt.title('Execution Time vs Number of Threads')
    plt.legend()
    plt.grid(True)
    plt.savefig('time_plot.png')
    plt.close()
    
    # Plot Speedup
    plt.figure(figsize=(10, 6))
    plt.plot(threads, df['speedup_v1'], marker='o', label='Version 1')
    plt.plot(threads, df['speedup_v2'], marker='s', label='Version 2')
    plt.plot(threads, threads, 'k--', label='Ideal Speedup')
    plt.xlabel('Number of threads')
    plt.ylabel('Speedup')
    plt.title('Speedup vs Number of Threads')
    plt.legend()
    plt.grid(True)
    plt.savefig('speedup_plot.png')
    plt.close()

    # Plot Efficiency
    plt.figure(figsize=(10, 6))
    plt.plot(threads, df['eff_v1'], marker='o', label='Version 1')
    plt.plot(threads, df['eff_v2'], marker='s', label='Version 2')
    plt.axhline(y=1.0, color='k', linestyle='--', label='Ideal Efficiency')
    plt.xlabel('Number of threads')
    plt.ylabel('Efficiency')
    plt.title('Efficiency vs Number of Threads')
    plt.legend()
    plt.grid(True)
    plt.savefig('efficiency_plot.png')
    plt.close()
    print("Plots generated: time_plot.png, speedup_plot.png, efficiency_plot.png")

if __name__ == "__main__":
    create_plots("results.csv")