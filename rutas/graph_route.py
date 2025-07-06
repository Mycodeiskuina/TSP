import matplotlib.pyplot as plt
import os
import re

# Folder paths
input_folder = "parallel"
output_folder = "plots"
os.makedirs(output_folder, exist_ok=True)

fontsize_labels = 15
fontsize_title = 16.5

# Process each file
for filename in os.listdir(input_folder):
    if filename.startswith("ruta_") and filename.endswith(".txt"):
        filepath = os.path.join(input_folder, filename)
        x, y = [], []
        distance = None

        # Read coordinates and distance
        with open(filepath) as f:
            for line in f:
                line = line.strip()
                if "Distancia" in line:
                    match = re.search(r"[-+]?\d*\.\d+|\d+", line)
                    if match:
                        distance = match.group()
                else:
                    try:
                        px, py = map(float, line.split())
                        x.append(px)
                        y.append(py)
                    except ValueError:
                        continue

        # Extract N from filename
        match = re.search(r"ruta_(\d+)\.txt", filename)
        N = match.group(1) if match else "?"
        print(N)
        main_title = f"Optimal TSP Route (N = {N})"

        # Plot the route
        plt.figure(figsize=(8, 6))
        plt.plot(x, y, '-o', color='blue')
        for i, (xi, yi) in enumerate(zip(x, y)):
            plt.text(xi, yi, str(i), fontsize=9, color='red')

        # Set correct title
        subtitle = f"Distance: {distance}" if distance else ""
        plt.suptitle(main_title, fontsize=18)
        plt.title(subtitle, fontsize=fontsize_title)
        plt.xlabel("X", fontsize=fontsize_labels)
        plt.ylabel("Y", fontsize=fontsize_labels)
        plt.tick_params(axis='both', which='major', labelsize=12)
        plt.grid(True)
        plt.axis('equal')
        plt.tight_layout(rect=[0, 0, 1, 0.95])  # Make room for suptitle

        # Save plot
        save_path = os.path.join(output_folder, f"{filename[:-4]}_parallel.png")
        plt.savefig(save_path)
        plt.close()
