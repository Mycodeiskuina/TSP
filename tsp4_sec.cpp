#include <bits/stdc++.h>
using namespace std;

const int INF = numeric_limits<int>::max();

struct Point { int x, y; };

int euclideanDistance(const Point &a, const Point &b) {
    return static_cast<int>(sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y)) + 0.5);
}

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
            stringstream ss(line.substr(line.find(":") + 1));
            ss >> dimension;
        }
        if (line.find("NODE_COORD_SECTION") != string::npos) break;
    }

    vector<Point> points(dimension);
    for (int i = 0; i < dimension; ++i) {
        int index, x, y;
        file >> index >> x >> y;
        points[index - 1] = {x, y};
    }

    vector<vector<int>> matrix(dimension, vector<int>(dimension));
    for (int i = 0; i < dimension; ++i)
        for (int j = 0; j < dimension; ++j)
            matrix[i][j] = (i == j ? INF : euclideanDistance(points[i], points[j]));

    return matrix;
}

struct Node {
    vector<vector<int>> reducedMatrix;
    vector<pair<int, int>> path;
    int cost;
    int vertex;
    int level;
    bool operator>(const Node &other) const { return cost > other.cost; }
};

int reduceMatrix(vector<vector<int>> &matrix) {
    int reduction = 0, n = matrix.size();
    for (int i = 0; i < n; ++i) {
        int rowMin = *min_element(matrix[i].begin(), matrix[i].end());
        if (rowMin != INF && rowMin > 0) {
            reduction += rowMin;
            for (int j = 0; j < n; ++j)
                if (matrix[i][j] != INF) matrix[i][j] -= rowMin;
        }
    }
    for (int j = 0; j < n; ++j) {
        int colMin = INF;
        for (int i = 0; i < n; ++i)
            colMin = min(colMin, matrix[i][j]);
        if (colMin != INF && colMin > 0) {
            reduction += colMin;
            for (int i = 0; i < n; ++i)
                if (matrix[i][j] != INF) matrix[i][j] -= colMin;
        }
    }
    return reduction;
}

pair<int, int> selectEdgeWithMaxPenalty(const vector<vector<int>> &matrix) {
    int n = matrix.size(), maxPenalty = -1;
    pair<int, int> bestEdge = {-1, -1};

    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            if (matrix[i][j] == 0) {
                int rowMin = INF, colMin = INF;
                for (int k = 0; k < n; ++k) if (k != j) rowMin = min(rowMin, matrix[i][k]);
                for (int k = 0; k < n; ++k) if (k != i) colMin = min(colMin, matrix[k][j]);
                int penalty = (rowMin == INF ? 0 : rowMin) + (colMin == INF ? 0 : colMin);
                if (penalty > maxPenalty) {
                    maxPenalty = penalty;
                    bestEdge = {i, j};
                }
            }

    return bestEdge;
}

Node* createNode(vector<vector<int>> matrix, vector<pair<int, int>> path,
                 int level, int i, int j, int n, int costSoFar) {
    for (int k = 0; k < n; ++k) {
        matrix[i][k] = INF;
        matrix[k][j] = INF;
    }
    matrix[j][0] = INF;

    int reductionCost = reduceMatrix(matrix);
    path.push_back({i, j});
    int totalCost = costSoFar + reductionCost;

    return new Node{matrix, path, totalCost, j, level};
}

void printPath(const vector<pair<int, int>> &path) {
    cout << "Tour: ";
    if (path.empty()) return;
    unordered_map<int, int> next;
    for (auto &[u, v] : path) next[u] = v;

    int start = path.front().first;
    int curr = start;
    do {
        cout << curr << " -> ";
        curr = next[curr];
    } while (curr != start);
    cout << start << endl;
}

int solveTSP(vector<vector<int>> costMatrix) {
    int n = costMatrix.size();
    int initCost = reduceMatrix(costMatrix);
    Node *root = new Node{costMatrix, {}, initCost, 0, 0};

    priority_queue<Node*, vector<Node*>, greater<Node*>> pq;
    pq.push(root);

    int minCost = INF;
    vector<pair<int, int>> bestPath;

    while (!pq.empty()) {
        Node* minNode = pq.top(); pq.pop();
        int i = minNode->vertex;

        if (minNode->level == n - 1) {
            minNode->path.push_back({i, 0});
            int totalCost = minNode->cost + costMatrix[i][0];
            if (totalCost < minCost) {
                minCost = totalCost;
                bestPath = minNode->path;
            }
            delete minNode;
            continue;
        }

        pair<int, int> bestEdge = selectEdgeWithMaxPenalty(minNode->reducedMatrix);
        if (bestEdge.first != -1) {
            int i = bestEdge.first, j = bestEdge.second;
            int stepCost = costMatrix[i][j];
            Node* child = createNode(minNode->reducedMatrix, minNode->path,
                                     minNode->level + 1, i, j, n, minNode->cost + stepCost);

            // ✅ Poda
            if (child->cost < minCost)
                pq.push(child);
            else
                delete child;
        }

        delete minNode;
    }

    printPath(bestPath);
    return minCost;
}

int main() {
    string filename = "rbu10.tsp"; // Asegúrate de que el archivo exista
    auto costMatrix = loadTSPMatrix(filename);
    cout << "Resolviendo TSP con Branch and Bound (penalización de ceros)...\n";
    auto start = chrono::high_resolution_clock::now();
    int minCost = solveTSP(costMatrix);
    auto end = chrono::high_resolution_clock::now();
    cout << "Costo mínimo: " << minCost << endl;
    cout << "Tiempo de ejecución: " << chrono::duration<double>(end - start).count() << " segundos\n";
    return 0;
}
