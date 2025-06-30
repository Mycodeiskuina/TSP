import matplotlib.pyplot as plt
import pandas as pd

# ======= CONFIGURACIÓN ==========
archivo_sec = "sequential/tiempos.txt"
archivo_brute = "brute_force/tiempos_brute_force.txt"
nombre_salida_png = "grafica.png"
titulo = "Comparación de Tiempos Promedios por Tamaño N"
etiqueta_x = "Tamaño del problema (N)"
etiqueta_y = "Tiempo promedio (s)"
# ================================

# Leer ambos archivos
df_sec = pd.read_csv(archivo_sec, delim_whitespace=True, header=None, names=["N", "Tiempo", "Hilos"])
df_brute = pd.read_csv(archivo_brute, delim_whitespace=True, header=None, names=["N", "Tiempo", "Hilos"])

# Calcular promedio por tamaño N
df_sec_prom = df_sec.groupby("N")["Tiempo"].mean().reset_index()
df_brute_prom = df_brute.groupby("N")["Tiempo"].mean().reset_index()

# Crear la gráfica
plt.figure(figsize=(10, 6))
plt.plot(df_sec_prom["N"], df_sec_prom["Tiempo"], marker='o', linestyle='-', label="Branch & Bound", color="blue")
plt.plot(df_brute_prom["N"], df_brute_prom["Tiempo"], marker='s', linestyle='--', label="Fuerza Bruta", color="darkgreen")

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
