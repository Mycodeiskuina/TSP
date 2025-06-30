import matplotlib.pyplot as plt
import pandas as pd

# ======= CONFIGURACIÓN ==========
archivo_tiempos = "tiempos_brute_force.txt"  # nombre del archivo con los datos
nombre_salida_png = "grafica_tiempos_fuerza_bruta.png"  # nombre del archivo de salida
titulo = "Tiempos Promedios por Tamaño N (Fuerza Bruta)"
etiqueta_x = "Tamaño del problema (N)"
etiqueta_y = "Tiempo promedio (s)"
# ================================

# Leer los datos (ignoramos la columna de hilos)
df = pd.read_csv(archivo_tiempos, delim_whitespace=True, header=None, names=["N", "Tiempo", "Hilos"])

# Agrupar por N y calcular el promedio de tiempo
df_prom = df.groupby("N")["Tiempo"].mean().reset_index()

# Crear la gráfica
plt.figure(figsize=(10, 6))
plt.plot(df_prom["N"], df_prom["Tiempo"], marker='o', linestyle='-', color='darkgreen', label="Fuerza Bruta")

# Configurar el gráfico
plt.title(titulo)
plt.xlabel(etiqueta_x)
plt.ylabel(etiqueta_y)
plt.grid(True)
plt.legend()

# Guardar la gráfica
plt.savefig(nombre_salida_png)
plt.close()

print(f"Gráfica guardada como '{nombre_salida_png}'")
