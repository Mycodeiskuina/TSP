#include <bits/stdc++.h>
using namespace std;

struct Point {
    double x, y;
};

int dimension = 0;

// Lectura del archivo TSPLIB y parseo de coordenadas
vector<Point> parseTSPLIB(const string& filename) {
    ifstream infile(filename);
    string line;
    while (getline(infile, line)) {
        if (line.find("NODE_COORD_SECTION") != string::npos)
            break;
        if (line.find("DIMENSION") != string::npos) {
            stringstream ss(line);
            string tmp;
            while (ss >> tmp) {
                if (isdigit(tmp[0])) {
                    dimension = stoi(tmp);
                    break;
                }
            }
        }
    }

    int index;
    double x, y;
    vector<Point> coords(dimension);
    
    for (int i = 0; i < dimension; ++i) {
        infile >> index >> x >> y;
        coords[i] = {x, y};
    }

    return coords;
}

//matriz simétrica de distancias (distancia de i a j es igual a la de j a i)
vector<vector<int>> computeDistanceMatrix(const vector<Point>& points) {
    int n = points.size();
    vector<vector<int>> adj(n, vector<int>(n, 0));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            if (i != j) {
                // Calcular la distancia euclidiana y redondear al entero más cercano
                double dx = points[i].x - points[j].x;
                double dy = points[i].y - points[j].y;
                adj[i][j] = round(sqrt(dx * dx + dy * dy));
            }
    return adj;
}

int solveTSPBrute(const vector<vector<int>>& adj, vector<int>& best_path) {
    int N = adj.size();
    vector<int> cities;
    for (int i = 1; i < N; ++i)
        cities.push_back(i); // Start from 0, permute the rest

    int min_cost = INT_MAX;

    do {
        int cost = 0;
        int prev = 0;
        for (int i = 0; i < cities.size(); ++i) {
            cost += adj[prev][cities[i]];
            prev = cities[i];
        }
        cost += adj[prev][0]; // Return to origin

        if (cost < min_cost) {
            min_cost = cost;
            best_path = {0};
            best_path.insert(best_path.end(), cities.begin(), cities.end());
        }
    } while (next_permutation(cities.begin(), cities.end()));

    return min_cost;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Uso: " << argv[0] << " archivo.tsp" << endl;
        return 1;
    }

    vector<int> best_path;
    string filename = argv[1];
    vector<Point> points = parseTSPLIB(filename);
    vector<vector<int>> adj = computeDistanceMatrix(points);

    auto start = chrono::high_resolution_clock::now();
    int min_cost = solveTSPBrute(adj, best_path);
    auto end = chrono::high_resolution_clock::now();

    cout << "Archivo: " << filename << endl;
    cout << "Distancia mínima del TSP: " << min_cost << endl;
    cout << "Tiempo de ejecución: " << chrono::duration<double>(end - start).count() << " segundos" << endl;

    // Guardar tiempo
    ofstream times_out("tiempos_brute_force.txt", std::ios::app);
    times_out << dimension << " " << chrono::duration<double>(end-start).count() << " " << 1 << "\n";
    times_out.close();

    // Guardar mejor ruta
    ofstream out("ruta_brute_force" + to_string(dimension) + ".txt");
    for (int i : best_path)
        out << points[i].x << " " << points[i].y << "\n";
    out << points[best_path[0]].x << " " << points[best_path[0]].y << "\n";
    out << "Distancia: " << min_cost << endl;
    out.close();

    return 0;
}
