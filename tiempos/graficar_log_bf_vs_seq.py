import matplotlib.pyplot as plt
import pandas as pd

# ======= CONFIGURACIÓN ==========
archivo_sec = "sequential/tiempos_sec.txt"
archivo_brute = "brute_force/tiempos_brute_force.txt"
nombre_salida_png = "log_scale_seq_vs_bf_plot.png"
title = "Comparison of Average Execution Times by Problem Size"
xlabel = "Problem Size (N)"
ylabel = "Average Execution Time (s)"
# ================================

# Leer los archivos
df_sec = pd.read_csv(archivo_sec, delim_whitespace=True, header=None, names=["N", "Tiempo", "Hilos"])
df_brute = pd.read_csv(archivo_brute, delim_whitespace=True, header=None, names=["N", "Tiempo", "Hilos"])

# Calcular promedio por tamaño N
df_sec_prom = df_sec.groupby("N")["Tiempo"].mean().reset_index()
df_brute_prom = df_brute.groupby("N")["Tiempo"].mean().reset_index()

# Graficar
plt.figure(figsize=(10, 6))
plt.plot(df_sec_prom["N"], df_sec_prom["Tiempo"], marker='o', linestyle='-', label="Branch & Bound", color="blue")
plt.plot(df_brute_prom["N"], df_brute_prom["Tiempo"], marker='s', linestyle='--', label="Brute Force", color="darkgreen")

# Escala logarítmica
plt.yscale("log")

fontsize_labels = 15
fontsize_title = 16.5

# Decoración
plt.title(title, fontsize=fontsize_title)
plt.xlabel(xlabel, fontsize=fontsize_labels)
plt.ylabel(ylabel + " (log)", fontsize=fontsize_labels)
plt.grid(True, which="both", ls="--", linewidth=0.5)
plt.legend(fontsize=14)
plt.tight_layout(pad=2.5)  # Aumenta el padding

# Guardar
plt.savefig(nombre_salida_png)
plt.close()

print(f"{nombre_salida_png} is saved")
