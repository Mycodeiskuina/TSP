import pandas as pd
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import numpy as np
import os

# ======= CONFIGURE =======
file = "tiempos_omp.txt"

output_dir = "efficiency/"
os.makedirs(output_dir, exist_ok=True)

output_file = "efficiency_3d.png"

# ======= READ DATA =======
df = pd.read_csv(file, sep=" ", header=None, names=["N", "time", "threads"])

# ======= FILTER EVEN THREADS + SEQUENTIAL =======
df = df[(df["threads"] % 2 == 0) | (df["threads"] == 1)]

# ======= AVERAGE TIME PER (N, threads) =======
df_avg = df.groupby(["N", "threads"])["time"].mean().reset_index()

# ======= SEQUENTIAL BASELINE =======
sequential_times = df_avg[df_avg["threads"] == 1].set_index("N")["time"]

# ======= COMPUTE EFFICIENCY =======
df_eff = df_avg.copy()
df_eff["efficiency"] = df_eff.apply(
    lambda row: (sequential_times.get(row["N"], np.nan) / (row["time"] * row["threads"])) * 100,
    axis=1
)
df_eff.dropna(inplace=True)

# ======= CREATE PIVOT TABLE FOR SURFACE PLOT =======
pivot = df_eff.pivot(index="threads", columns="N", values="efficiency")

# Ensure ordering
pivot = pivot.sort_index()
pivot = pivot.reindex(sorted(pivot.columns), axis=1)

# Prepare meshgrid
X, Y = np.meshgrid(pivot.columns.values, pivot.index.values)  # X = N, Y = threads
Z = pivot.values

# ======= 3D SURFACE PLOT =======
fig = plt.figure(figsize=(12, 8))
ax = fig.add_subplot(111, projection='3d')

# Plot the surface
surf = ax.plot_surface(X, Y, Z, cmap='viridis', edgecolor='k', alpha=0.95)

fontsize_labels = 15
fontsize_title = 20

ax.set_xlabel("Problem Size $N$", fontsize=fontsize_labels)
ax.set_ylabel("Number of Threads $p$", fontsize=fontsize_labels)
ax.set_zlabel("Efficiency $E$ (\%)", fontsize=fontsize_labels)

ax.set_title("TSP Scalability Efficiency (E = Ts / (Tp · p))", fontsize=fontsize_title, fontweight='bold')

ax.tick_params(axis='both', which='major', labelsize=12)
ax.zaxis.set_tick_params(labelsize=12)


fig.colorbar(surf, ax=ax, shrink=0.5, aspect=10)

# View angle
ax.view_init(elev=30, azim=135)

# Save and show
plt.tight_layout()
plt.savefig(f"{output_dir}/{output_file}")
plt.show()
