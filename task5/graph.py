import matplotlib.pyplot as plt
import numpy as np
import os

def plot_results(filename):
    if not os.path.exists(filename):
        print(f"Error: {filename} not found. Please run the C++ program first.")
        return

    # Read all lines from the output file
    with open(filename, 'r') as file:
        lines = [line.strip() for line in file if line.strip()]

    num_test_cases = len(lines)
    if num_test_cases == 0:
        print("No valid data found in the file.")
        return

    # Data arrays
    fcfs_wt, fcfs_tat = [], []
    sjf_wt, sjf_tat = [], []
    rr_wt, rr_tat = [], []

    for line in lines:
        data = list(map(float, line.split()))
        if len(data) == 6:
            fcfs_wt.append(data[0])
            fcfs_tat.append(data[1])
            sjf_wt.append(data[2])
            sjf_tat.append(data[3])
            rr_wt.append(data[4])
            rr_tat.append(data[5])

    # Setup the X-axis positions
    x = np.arange(num_test_cases)
    width = 0.25  # Bar width

    # Create a single figure with 2 subplots side-by-side
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(15, 6))

    # --- Subplot 1: Average Wait Time ---
    b1 = ax1.bar(x - width, fcfs_wt, width, label='FCFS', color='#4C72B0')
    b2 = ax1.bar(x, sjf_wt, width, label='SJF', color='#55A868')
    b3 = ax1.bar(x + width, rr_wt, width, label='Round Robin', color='#C44E52')

    ax1.set_ylabel('Average Wait Time (Units)')
    ax1.set_title('Wait Time Comparison')
    ax1.set_xticks(x)
    ax1.set_xticklabels([f'Test Case {i+1}' for i in range(num_test_cases)])
    ax1.legend()

    for bars in [b1, b2, b3]:
        ax1.bar_label(bars, padding=3, fmt='%.2f')

    # --- Subplot 2: Average Turn Around Time ---
    b4 = ax2.bar(x - width, fcfs_tat, width, label='FCFS', color='#4C72B0')
    b5 = ax2.bar(x, sjf_tat, width, label='SJF', color='#55A868')
    b6 = ax2.bar(x + width, rr_tat, width, label='Round Robin', color='#C44E52')

    ax2.set_ylabel('Average Turn Around Time (Units)')
    ax2.set_title('Turn Around Time Comparison')
    ax2.set_xticks(x)
    ax2.set_xticklabels([f'Test Case {i+1}' for i in range(num_test_cases)])
    ax2.legend()

    for bars in [b4, b5, b6]:
        ax2.bar_label(bars, padding=3, fmt='%.2f')

    # Final layout adjustments
    plt.suptitle('CPU Scheduling Algorithms Performance', fontsize=16, fontweight='bold')
    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    plot_results("output.txt")