import matplotlib.pyplot as plt
import pandas as pd

# ======= CONFIGURACIÓN ==========
archivo_sec = "sequential/tiempos_sec.txt"
archivo_brute = "brute_force/tiempos_brute_force.txt"
nombre_salida_png = "grafica_seq_vs_bf.png"
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

# Encontrar tamaños N comunes en ambos
n_comunes = sorted(set(df_sec_prom["N"]) & set(df_brute_prom["N"]))
df_sec_common = df_sec_prom[df_sec_prom["N"].isin(n_comunes)]
df_brute_common = df_brute_prom[df_brute_prom["N"].isin(n_comunes)]

# Crear la gráfica
plt.figure(figsize=(10, 6))
plt.plot(df_sec_common["N"], df_sec_common["Tiempo"], marker='o', linestyle='-', label="Branch & Bound", color ="blue")
plt.plot(df_brute_common["N"], df_brute_common["Tiempo"], marker='s', linestyle='--', label="Fuerza Bruta", color="darkgreen")

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
