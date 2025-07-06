import matplotlib.pyplot as plt
import pandas as pd
import os
import math

# ======= CONFIGURE THE FILE =======
file = "tiempos_omp.txt"
output_dir = "speedup/"
os.makedirs(output_dir, exist_ok=True)

# ======= READ DATA =======
df = pd.read_csv(file, sep=" ", header=None, names=["N", "time", "threads"])

# ======= FILTER EVEN THREADS + SEQUENTIAL (1 thread) =======
#df = df[(df["threads"] % 2 == 0) | (df["threads"] == 1)]
df = df[df["threads"]<=16]
# ======= AVERAGE TIME PER (N, threads) =======
df_avg = df.groupby(["N", "threads"])["time"].mean().reset_index()

# ======= BASELINE (SEQUENTIAL TIME PER N) =======
sequential = df_avg[df_avg["threads"] == 1].set_index("N")["time"]

# ======= COMPUTE SPEEDUP: Ts / Tp =======
df_speedup = df_avg.copy()
df_speedup["speedup"] = df_speedup.apply(
    lambda row: sequential.get(row["N"], float("nan")) / row["time"],
    axis=1
)

# ======= COLOR PALETTE =======
colors = [
    '#1f77b4', '#ff7f0e', '#2ca02c',
    '#4363d8', '#911eb4', '#f032e6',
    '#008080', '#d62728', '#005f73',
    '#3a0ca3', '#1982c4', '#283618',
    '#264653', '#6f1d1b'
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
    sub_df = df_speedup[(df_speedup["N"] >= n_min) & (df_speedup["N"] <= n_max)]

    unique_Ns = sorted(sub_df["N"].unique())
    max_speedup = 0

    for j, n in enumerate(unique_Ns):
        group = sub_df[sub_df["N"] == n].sort_values("threads")
        color = colors[n - 8]
        plt.plot(group["threads"], group["speedup"],
                 marker='o', label=f"N = {n}", color=color)

        # Update the actual maximum speedup reached
        max_speedup = max(max_speedup, group["speedup"].max())

    # Plot ideal speedup line (y = x)
    ideal_threads = sorted(df["threads"].unique())
    plt.plot(ideal_threads, ideal_threads, linestyle='--', color='black', label='Ideal speedup')

    # Aesthetics
    plt.xlabel("Number of threads", fontsize=fontsize_labels)
    plt.ylabel("Speedup ($T_s / T_p$)", fontsize=fontsize_labels)
    plt.title(f"Speedup vs Number of Threads for N = {n_min} to {n_max}", fontsize=fontsize_title)
    plt.ylim(0, math.ceil(max_speedup + 0.5))  # Add margin above actual max
    plt.tick_params(axis='both', which='major', labelsize=12)
    plt.tight_layout()
    plt.legend(fontsize=12)
    plt.grid(True)
    plt.tight_layout()

    # Save plot
    plt.savefig(f"{output_dir}/speedup_plot_{idx}.png")
    plt.close()
