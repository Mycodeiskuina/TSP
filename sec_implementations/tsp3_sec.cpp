#include <bits/stdc++.h>
using namespace std;

const int INF = numeric_limits<int>::max();

struct Point { int x, y; };

// Representación 1D de matriz n x n (modificable)
inline int& at(vector<int>& mat, int n, int i, int j) {
    return mat[i * n + j];
}

// Representación 1D de matriz n x n (solo lectura)
inline int at(const vector<int>& mat, int n, int i, int j) {
    return mat[i * n + j];
}

int euclideanDistance(const Point &a, const Point &b) {
    int dx = a.x - b.x, dy = a.y - b.y;
    return (int)(sqrt(dx * dx + dy * dy) + 0.5);
}

// Carga datos y genera matriz de costos
vector<int> loadTSPMatrix(const string& filename, int& n) {
    ifstream file(filename);
    if (!file) {
        cerr << "No se pudo abrir: " << filename << endl;
        exit(1);
    }

    string line;
    while (getline(file, line)) {
        if (line.find("DIMENSION") != string::npos) {
            stringstream ss(line.substr(line.find(':') + 1));
            ss >> n;
        }
        if (line.find("NODE_COORD_SECTION") != string::npos) break;
    }

    vector<Point> points(n);
    for (int i = 0; i < n; ++i) {
        int idx, x, y;
        file >> idx >> x >> y;
        points[idx - 1] = {x, y};
    }

    vector<int> mat(n * n, 0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            at(mat, n, i, j) = (i == j ? INF : euclideanDistance(points[i], points[j]));
    return mat;
}

// Reducción de matriz
int reduceMatrix(vector<int>& mat, int n) {
    int cost = 0;
    // Filas
    for (int i = 0; i < n; ++i) {
        int rowMin = INF;
        for (int j = 0; j < n; ++j)
            rowMin = min(rowMin, at(mat, n, i, j));
        if (rowMin != INF && rowMin > 0) {
            cost += rowMin;
            for (int j = 0; j < n; ++j)
                if (at(mat, n, i, j) != INF)
                    at(mat, n, i, j) -= rowMin;
        }
    }
    // Columnas
    for (int j = 0; j < n; ++j) {
        int colMin = INF;
        for (int i = 0; i < n; ++i)
            colMin = min(colMin, at(mat, n, i, j));
        if (colMin != INF && colMin > 0) {
            cost += colMin;
            for (int i = 0; i < n; ++i)
                if (at(mat, n, i, j) != INF)
                    at(mat, n, i, j) -= colMin;
        }
    }
    return cost;
}

int bestCost = INF;
vector<int> bestPath;

void tspRec(const vector<int>& origCost, vector<int>& mat,
            int currentCost, int level, int vertex, vector<int>& path, int n) {
    if (level == n - 1) {
        int finalCost = currentCost + at(origCost, n, vertex, 0);
        if (finalCost < bestCost) {
            bestCost = finalCost;
            bestPath = path;
            bestPath.push_back(0);
        }
        return;
    }

    // Ordenar vecinos por costo
    vector<pair<int, int>> neighbors;
    for (int j = 0; j < n; ++j)
        if (at(mat, n, vertex, j) != INF)
            neighbors.emplace_back(at(mat, n, vertex, j), j);
    sort(neighbors.begin(), neighbors.end());

    for (auto& [cost, j] : neighbors) {
        vector<int> newMat = mat;

        for (int k = 0; k < n; ++k) {
            at(newMat, n, vertex, k) = INF;
            at(newMat, n, k, j) = INF;
        }
        at(newMat, n, j, 0) = INF;

        int bound = reduceMatrix(newMat, n);
        int newCost = currentCost + at(origCost, n, vertex, j) + bound;

        if (newCost < bestCost) {
            path.push_back(j);
            tspRec(origCost, newMat, currentCost + at(origCost, n, vertex, j), level + 1, j, path, n);
            path.pop_back();
        }
    }
}

int main() {
    int n;
    string filename = "rbu10.tsp";

    vector<int> costMatrix = loadTSPMatrix(filename, n);
    vector<int> reduced = costMatrix;
    int initialBound = reduceMatrix(reduced, n);

    vector<int> path = {0};
    auto start = chrono::high_resolution_clock::now();
    tspRec(costMatrix, reduced, initialBound, 0, 0, path, n);
    auto end = chrono::high_resolution_clock::now();

    // Resultados
    cout << "Tour óptimo: ";
    for (int v : bestPath) cout << v << " -> ";
    cout << bestPath.front() << endl;

    cout << "Costo mínimo: " << bestCost << endl;
    cout << "Tiempo de ejecución: " << chrono::duration<double>(end - start).count() << " segundos" << endl;
    return 0;
}
