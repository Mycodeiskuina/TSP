import random

def generar_tsp(nombre_archivo, cantidad_puntos, rango=50):
    with open(nombre_archivo, 'w') as f:
        f.write("NAME: random_points\n")
        f.write("TYPE: TSP\n")
        f.write(f"DIMENSION: {cantidad_puntos}\n")
        f.write("EDGE_WEIGHT_TYPE: EUC_2D\n")
        f.write("NODE_COORD_SECTION\n")
        for i in range(1, cantidad_puntos + 1):
            x = random.randint(0, rango)
            y = random.randint(0, rango)
            f.write(f"{i} {x} {y}\n")
        f.write("EOF\n")

# Ejemplo de uso
if __name__ == "__main__":

    for i in range (2, 26):
       generar_tsp(f"data_{i}.tsp", cantidad_puntos=i, rango=1000)

    for i in range (5, 7):
       generar_tsp(f"data_{2**i}.tsp", cantidad_puntos=2**i, rango=1000)
    
    for i in range (30,51, 5):
        generar_tsp(f"data_{i}.tsp", cantidad_puntos=i, rango=1000)
