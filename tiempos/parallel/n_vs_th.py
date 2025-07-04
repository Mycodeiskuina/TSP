import matplotlib.pyplot as plt
import pandas as pd

# ======= CONFIGURA EL ARCHIVO =======
archivo = "tiempos_omp.txt"

# ======= LEER DATOS =======
df = pd.read_csv(archivo, sep=" ", header=None, names=["N", "tiempo", "hilos"])

# ======= PROMEDIAR TIEMPOS POR (N, hilos) =======
df_prom = df.groupby(["N", "hilos"])["tiempo"].mean().reset_index()

# ======= GRAFICAR =======
plt.figure(figsize=(10, 6))

# Agrupar por N y graficar cada línea
for n, grupo in df_prom.groupby("N"):
    grupo_ordenado = grupo.sort_values("hilos")
    plt.plot(grupo_ordenado["hilos"], grupo_ordenado["tiempo"],
             marker='o', label=f"N = {n}")

# Estética del gráfico
plt.xlabel("Número de hilos")
plt.ylabel("Tiempo promedio (segundos)")
plt.title("Tiempo promedio vs Número de hilos por N")
plt.legend()
plt.grid(True)
plt.tight_layout()

# Mostrar o guardar
# plt.show()
plt.savefig("grafica.png")
