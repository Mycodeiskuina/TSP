#include <bits/stdc++.h>
using namespace std;

const int INF = numeric_limits<int>::max();

struct Point {
    int x, y;
};

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
        if (line.find("NODE_COORD_SECTION") != string::npos)
            break;
    }

    vector<Point> points(dimension);
    for (int i = 0; i < dimension; ++i) {
        int index, x, y;
        file >> index >> x >> y;
        points[index - 1] = {x, y};
    }

    vector<vector<int>> matrix(dimension, vector<int>(dimension));
    for (int i = 0; i < dimension; ++i) {
        for (int j = 0; j < dimension; ++j) {
            matrix[i][j] = (i == j ? INF : euclideanDistance(points[i], points[j]));
        }
    }

    return matrix;
}

struct Node {
    vector<vector<int>> reducedMatrix;
    vector<pair<int, int>> path;
    int cost;
    int vertex;
    int level;

    bool operator>(const Node &other) const {
        return cost > other.cost;
    }
};

int reduceMatrix(vector<vector<int>> &matrix) {
    int reduction = 0;
    int n = matrix.size();

    for (int i = 0; i < n; ++i) {
        int rowMin = INF;
        for (int j = 0; j < n; ++j)
            rowMin = min(rowMin, matrix[i][j]);
        if (rowMin != INF && rowMin > 0) {
            reduction += rowMin;
            for (int j = 0; j < n; ++j)
                if (matrix[i][j] != INF)
                    matrix[i][j] -= rowMin;
        }
    }

    for (int j = 0; j < n; ++j) {
        int colMin = INF;
        for (int i = 0; i < n; ++i)
            colMin = min(colMin, matrix[i][j]);
        if (colMin != INF && colMin > 0) {
            reduction += colMin;
            for (int i = 0; i < n; ++i)
                if (matrix[i][j] != INF)
                    matrix[i][j] -= colMin;
        }
    }

    return reduction;
}

Node* createNode(const vector<vector<int>> &parentMatrix, const vector<pair<int,int>> &path,
                 int level, int i, int j, int n, bool include) {
    Node* node = new Node;
    node->reducedMatrix = parentMatrix;
    node->path = path;
    node->level = level;
    node->vertex = include ? j : i;  // next node if included, same if excluded

    if (include) {
        node->path.push_back({i, j});
        for (int k = 0; k < n; ++k) {
            node->reducedMatrix[i][k] = INF;
            node->reducedMatrix[k][j] = INF;
        }
        node->reducedMatrix[j][0] = INF;
    } else {
        node->reducedMatrix[i][j] = INF;
    }

    node->cost = reduceMatrix(node->reducedMatrix);
    return node;
}

void printPath(const vector<pair<int,int>> &path) {
    if (path.empty()) return;
    cout << "Tour: " << path[0].first;
    for (auto &p : path)
        cout << " -> " << p.second;
    cout << " -> " << path[0].first << endl;
}

int solveTSP(const vector<vector<int>> &costMatrix) {
    int n = costMatrix.size();

    Node* root = new Node;
    root->path = {};
    root->reducedMatrix = costMatrix;
    root->cost = reduceMatrix(root->reducedMatrix);
    root->vertex = 0;
    root->level = 0;

    priority_queue<Node*, vector<Node*>, greater<Node*>> pq;
    pq.push(root);

    int minCost = INF;
    vector<pair<int,int>> bestPath;

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

        for (int j = 0; j < n; ++j) {
            if (minNode->reducedMatrix[i][j] != INF) {
                // incluir (i,j)
                Node* includeNode = createNode(minNode->reducedMatrix, minNode->path, minNode->level + 1, i, j, n, true);
                includeNode->cost += minNode->cost + costMatrix[i][j];
                if (includeNode->cost < minCost) pq.push(includeNode);
                else delete includeNode;

                // excluir (i,j)
                Node* excludeNode = createNode(minNode->reducedMatrix, minNode->path, minNode->level, i, j, n, false);
                excludeNode->cost += minNode->cost;
                if (excludeNode->cost < minCost) pq.push(excludeNode);
                else delete excludeNode;
            }
        }

        delete minNode;
    }

    printPath(bestPath);
    return minCost;
}

int main() {
    string filename = "rbu10.tsp";
    vector<vector<int>> costMatrix = loadTSPMatrix(filename);

    cout << "Resolviendo TSP con Branch and Bound...\n";
    auto start = chrono::high_resolution_clock::now();
    int minCost = solveTSP(costMatrix);
    auto end = chrono::high_resolution_clock::now();

    cout << "Costo mínimo: " << minCost << endl;
    cout << "Tiempo de ejecución: " << chrono::duration<double>(end - start).count() << " segundos" << endl;
    return 0;
}
