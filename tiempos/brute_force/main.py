import matplotlib.pyplot as plt
import pandas as pd
from matplotlib.ticker import LogFormatterMathtext  # Para potencias de 10 elegantes

# ======= CONFIGURACIÓN ==========
archivo_tiempos = "tiempos_brute_force.txt"  # nombre del archivo con los datos
nombre_salida_png = "time_brute_force.png"
titulo = "Mean Execution Time by Problem Size (Brute Force)"
etiqueta_x = "Problem Size (N)"
etiqueta_y = "Mean Execution Time (s)"

# ================================

# Leer los datos (ignoramos la columna de hilos)
df = pd.read_csv(archivo_tiempos, delim_whitespace=True, header=None, names=["N", "Tiempo", "Hilos"])

# Agrupar por N y calcular el promedio de tiempo
df_prom = df.groupby("N")["Tiempo"].mean().reset_index()

# Crear la gráfica
plt.figure(figsize=(10, 6))
plt.plot(df_prom["N"], df_prom["Tiempo"], marker='o', linestyle='-', color='darkgreen', label="Brute Force")

fontsize_labels = 15
fontsize_title = 16.5

# Configurar el gráfico
plt.title(titulo, fontsize=fontsize_title)
plt.xlabel(etiqueta_x, fontsize=fontsize_labels)
plt.ylabel(etiqueta_y, fontsize=fontsize_labels)

plt.tick_params(axis='both', which='major', labelsize=12)
plt.legend(fontsize=14)
plt.yscale("log")  # <- Escala logarítmica en eje Y

# Mostrar potencias de 10 como 10⁰, 10¹, etc.
plt.gca().yaxis.set_major_formatter(LogFormatterMathtext(base=10))

plt.grid(True, which="both", linestyle="--", linewidth=0.5)

# Guardar la gráfica
plt.savefig(nombre_salida_png)
plt.close()

print(f"{nombre_salida_png} is saved")
