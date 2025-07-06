import matplotlib.pyplot as plt
import pandas as pd
import os
import math

# ======= CONFIGURE =======
file = "tiempos_omp.txt"
output_dir = "efficiency/"
os.makedirs(output_dir, exist_ok=True)

# ======= READ DATA =======
df = pd.read_csv(file, sep=" ", header=None, names=["N", "time", "threads"])

# ======= FILTER EVEN THREADS + SEQUENTIAL =======
#df = df[(df["threads"] % 2 == 0) | (df["threads"] == 1)]
df = df[df["threads"]<=16]
# ======= AVERAGE TIME PER (N, threads) =======
df_avg = df.groupby(["N", "threads"])["time"].mean().reset_index()

# ======= BASELINE SEQUENTIAL TIME =======
sequential = df_avg[df_avg["threads"] == 1].set_index("N")["time"]

# ======= COMPUTE SPEEDUP AND EFFICIENCY =======
df_eff = df_avg.copy()
df_eff["speedup"] = df_eff.apply(
    lambda row: sequential.get(row["N"], float("nan")) / row["time"],
    axis=1
)
df_eff["efficiency"] = df_eff["speedup"] / df_eff["threads"]

# ======= COLOR PALETTE =======
colors = [
    '#1f77b4', '#ff7f0e', '#2ca02c',
    '#4363d8', '#911eb4', '#f032e6',
    '#008080', '#d62728', '#005f73',
    '#3a0ca3', '#1982c4', '#283618',
    '#264653', '#6f1d1b'
]

# ======= RANGES =======
ranges = [
    (8, 10),
    (11, 14),
    (15, 17),
    (18, 20),
]

fontsize_labels = 15
fontsize_title = 16.5

# ======= GENERATE EFFICIENCY PLOTS =======
for idx, (n_min, n_max) in enumerate(ranges, start=1):
    plt.figure(figsize=(10, 6))
    sub_df = df_eff[(df_eff["N"] >= n_min) & (df_eff["N"] <= n_max)]

    unique_Ns = sorted(sub_df["N"].unique())
    max_eff = 0

    for j, n in enumerate(unique_Ns):
        group = sub_df[sub_df["N"] == n].sort_values("threads")
        color = colors[n - 8]
        plt.plot(group["threads"], group["efficiency"],
                 marker='o', label=f"N = {n}", color=color)
        max_eff = max(max_eff, group["efficiency"].max())

    # Ideal efficiency line (E = 0.8)
    plt.axhline(y=0.8, linestyle='--', color='black', label='Ideal efficiency')

    # Aesthetics
    plt.xlabel("Number of threads", fontsize=fontsize_labels)
    plt.ylabel("Efficiency ($E = \\frac{T_s}{T_p \\cdot p}$)", fontsize=fontsize_labels)
    plt.title(f"Efficiency vs Number of Threads for N = {n_min} to {n_max}", fontsize=fontsize_title)
    plt.ylim(0, min(1.1, math.ceil(max_eff * 10) / 10))  # Slight margin, max 1.1
    plt.legend(fontsize=12)
    plt.grid(True)

    plt.tick_params(axis='both', which='major', labelsize=12)
    plt.tight_layout()

    # Save plot
    plt.savefig(f"{output_dir}/efficiency_plot_{idx}.png")
    plt.close()
