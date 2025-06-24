#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <sstream>
#include <limits>
#include <algorithm>

using namespace std;
const int INF = numeric_limits<int>::max();

struct Point { int x, y; };

int euclideanDistance(const Point &a, const Point &b) {
    return static_cast<int>(sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y)) + 0.5);
}

// Carga coordenadas y construye matriz de costos
vector<vector<int>> loadTSPMatrix(const string &filename) {
    ifstream file(filename);
    if (!file) {
        cerr << "No se pudo abrir el archivo: " << filename << endl;
        exit(1);
    }

    string line;
    int dimension = 0;
    while (getline(file, line)) {
        if (line.find("DIMENSION") != string::npos) {
            stringstream ss(line.substr(line.find(':') + 1));
            ss >> dimension;
        }
        if (line.find("NODE_COORD_SECTION") != string::npos) break;
    }

    vector<Point> points(dimension);
    for (int i = 0; i < dimension; ++i) {
        int idx, x, y;
        file >> idx >> x >> y;
        points[idx-1] = {x, y};
    }

    vector<vector<int>> matrix(dimension, vector<int>(dimension));
    for (int i = 0; i < dimension; ++i) {
        for (int j = 0; j < dimension; ++j) {
            matrix[i][j] = (i==j ? INF : euclideanDistance(points[i], points[j]));
        }
    }
    return matrix;
}

// Reduce filas y columnas, devuelve suma de reducciones
int reduceMatrix(vector<vector<int>> &mat) {
    int n = mat.size(), cost = 0;
    // filas
    for (int i = 0; i < n; ++i) {
        int rowMin = INF;
        for (int j = 0; j < n; ++j) rowMin = min(rowMin, mat[i][j]);
        if (rowMin != INF && rowMin > 0) {
            cost += rowMin;
            for (int j = 0; j < n; ++j) if (mat[i][j] != INF) mat[i][j] -= rowMin;
        }
    }
    // columnas
    for (int j = 0; j < n; ++j) {
        int colMin = INF;
        for (int i = 0; i < n; ++i) colMin = min(colMin, mat[i][j]);
        if (colMin != INF && colMin > 0) {
            cost += colMin;
            for (int i = 0; i < n; ++i) if (mat[i][j] != INF) mat[i][j] -= colMin;
        }
    }
    return cost;
}

int bestCost;
vector<int> bestPath;

void tspRec(const vector<vector<int>> &origCost, vector<vector<int>> matrix,
            int currentCost, int level, int vertex, vector<int> &path) {
    int n = origCost.size();
    if (level == n-1) {
        // cierro ciclo
        int finalCost = currentCost + origCost[vertex][0];
        if (finalCost < bestCost) {
            bestCost = finalCost;
            bestPath = path;
            bestPath.push_back(0);
        }
        return;
    }
    
    for (int j = 0; j < n; ++j) {
        if (matrix[vertex][j] != INF) {
            // Generar subproblema
            vector<vector<int>> newMat = matrix;
            // marcar aristas
            for (int k = 0; k < n; ++k) {
                newMat[vertex][k] = INF;
                newMat[k][j] = INF;
            }
            newMat[j][0] = INF;
            // calcular cota inferior
            int bound = reduceMatrix(newMat);
            int newCost = currentCost + origCost[vertex][j] + bound;
            if (newCost < bestCost) {
                path.push_back(j);
                tspRec(origCost, newMat, currentCost + origCost[vertex][j], level+1, j, path);
                path.pop_back();
            }
        }
    }
}

int main() {
    string filename = "rbu10.tsp";
    auto costMatrix = loadTSPMatrix(filename);
    int n = costMatrix.size();

    // reducción inicial
    bestCost = INF;
    vector<vector<int>> reduced = costMatrix;
    int initialBound = reduceMatrix(reduced);

    vector<int> path;
    path.push_back(0);
    tspRec(costMatrix, reduced, initialBound, 0, 0, path);

    // imprimir resultado
    cout << "Tour óptimo: ";
    for (int v : bestPath) cout << v << " -> ";
    cout << bestPath.front() << endl;
    cout << "Costo mínimo: " << bestCost << endl;
    return 0;
}
