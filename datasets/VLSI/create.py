import os

# Leer datos originales
with open("data.tsp") as f:
    lines = f.readlines()

# Encontrar la sección de coordenadas
header_lines = []
coord_lines = []
in_coords = False
for line in lines:
    if "NODE_COORD_SECTION" in line:
        in_coords = True
        header_lines.append("NODE_COORD_SECTION\n")
        continue
    if not in_coords:
        if "DIMENSION" in line:
            continue  # la reemplazaremos más adelante
        header_lines.append(line)
    else:
        if line.strip() == "EOF":
            break
        coord_lines.append(line)



# Generar archivos con los primeros i puntos (i = 2 hasta 64)
for it in range(1, 8):
    i = 2**it
    filename = f"data_{i}.tsp"
    with open(filename, "w") as f:
        # Escribir encabezado modificado
        f.write("NAME : data_{}\n".format(i))
        f.write("TYPE : TSP\n")
        f.write("COMMENT : Subset of xqf131 with {} nodes\n".format(i))
        f.write("DIMENSION : {}\n".format(i))
        f.write("EDGE_WEIGHT_TYPE : EUC_2D\n")
        f.write("NODE_COORD_SECTION\n")
        
        # Escribir las primeras i líneas de coordenadas
        f.writelines(coord_lines[:i])
        
        # Finalizar archivo
        f.write("EOF\n")

print("¡Archivos .tsp generados correctamente!")
