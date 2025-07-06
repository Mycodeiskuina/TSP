import matplotlib.pyplot as plt
import pandas as pd
import os

# ======= CONFIGURE THE FILE =======
file = "tiempos_omp.txt"
output_dir = "times/"  # Directory to save plots
os.makedirs(output_dir, exist_ok=True)

# ======= READ DATA =======
df = pd.read_csv(file, sep=" ", header=None, names=["N", "time", "threads"])

# ======= FILTER EVEN THREADS + SEQUENTIAL (1 thread) =======
#df = df[(df["threads"] % 2 == 0) | (df["threads"] == 1)]
df = df[df["threads"]<=16]
# ======= AVERAGE TIME PER (N, threads) =======
df_avg = df.groupby(["N", "threads"])["time"].mean().reset_index()

# ======= COLOR PALETTE (merged, ordered) =======
colors = [
    '#1f77b4', '#ff7f0e', '#2ca02c',      # original matplotlib
    '#4363d8', '#911eb4', '#f032e6',      # vivid blue/purple
    '#008080', '#d62728', '#005f73',      # teal, red, deep teal
    '#3a0ca3', '#1982c4', '#283618',      # indigo, steel blue, dark green
    '#264653', '#6f1d1b'                  # gray-blue, oxblood
]

# ======= RANGES FOR SPLITTING INTO PLOTS =======
ranges = [
    (8, 10),
    (11, 14),
    (15, 17),
    (18, 20),
]

fontsize_labels = 15
fontsize_title = 16.5

# ======= GENERATE 1 PLOT PER RANGE =======
for idx, (n_min, n_max) in enumerate(ranges, start=1):
    plt.figure(figsize=(10, 6))
    sub_df = df_avg[(df_avg["N"] >= n_min) & (df_avg["N"] <= n_max)]

    unique_Ns = sorted(sub_df["N"].unique())  # ensures consistent order
    for j, n in enumerate(unique_Ns):
        group = sub_df[sub_df["N"] == n].sort_values("threads")
        color = colors[j % len(colors)]
        plt.plot(group["threads"], group["time"],
                 marker='o', label=f"N = {n}", color=colors[n-8])

    # Plot aesthetics
    plt.xlabel("Number of threads", fontsize=fontsize_labels)
    plt.ylabel("Mean time (seconds)", fontsize=fontsize_labels)
    plt.title(f"Mean time vs Number of Threads for N = {n_min} to {n_max}", fontsize=fontsize_title)
    plt.tick_params(axis='both', which='major', labelsize=12)
    plt.tight_layout()
    plt.legend(fontsize=12)
    plt.grid(True)
    plt.tight_layout()

    # Save plot
    plt.savefig(f"{output_dir}/parallel_time_{idx}.png")
    plt.close()
