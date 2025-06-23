import matplotlib.pyplot as plt

# Leer puntos
x, y = [], []
with open("ruta.txt") as f:
    for line in f:
        px, py = map(float, line.strip().split())
        x.append(px)
        y.append(py)

# Graficar
plt.figure(figsize=(8, 6))
plt.plot(x, y, '-o', color='blue')  # ruta
for i, (xi, yi) in enumerate(zip(x, y)):
    plt.text(xi, yi, str(i), fontsize=9, color='red')  # etiqueta

plt.title("Ruta óptima del TSP")
plt.xlabel("X")
plt.ylabel("Y")
plt.grid(True)
plt.axis('equal')
plt.show()
