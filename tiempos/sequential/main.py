import matplotlib.pyplot as plt
import pandas as pd

# ======= CONFIGURACIÓN ==========
archivo_tiempos = "tiempos_sec.txt"  # nombre del archivo con los datos
nombre_salida_png = "time_branch_and_bound.png"  # nombre del archivo de salida
titulo = "Mean Execution Time by Problem Size (Branch & Bound)"
etiqueta_x = "Problem Size (N)"
etiqueta_y = "Mean Execution Time (s)"
# ================================

# Leer los datos desde el archivo
df = pd.read_csv(archivo_tiempos, delim_whitespace=True, header=None, names=["N", "Tiempo", "Procesos"])

# Agrupar por N y calcular el promedio
df_prom = df.groupby("N")["Tiempo"].mean().reset_index()

fontsize_labels = 15
fontsize_title = 16.5

# Crear la gráfica
plt.figure(figsize=(10, 6))
plt.plot(df_prom["N"], df_prom["Tiempo"], marker='o', linestyle='-', color='blue',label="Branch & Bound")
plt.title(titulo, fontsize=fontsize_title)
plt.xlabel(etiqueta_x, fontsize=fontsize_labels)
plt.ylabel(etiqueta_y, fontsize=fontsize_labels)
plt.grid(True)

plt.tick_params(axis='both', which='major', labelsize=12)
plt.legend(fontsize=14)
plt.yscale("log")  # <- Escala logarítmica en eje Y

# Guardar la gráfica
plt.savefig(nombre_salida_png)
plt.close()

print(f"{nombre_salida_png} is saved")
