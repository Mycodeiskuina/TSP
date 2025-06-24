#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <sstream>
#include <limits>
#include <queue>
#include <algorithm>

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

    // Crear matriz de adyacencia
    vector<vector<int>> matrix(dimension, vector<int>(dimension));
    for (int i = 0; i < dimension; ++i) {
        for (int j = 0; j < dimension; ++j) {
            if (i == j) matrix[i][j] = INF;
            else matrix[i][j] = euclideanDistance(points[i], points[j]);
        }
    }

    return matrix;
}

// Estructura de nodo para Branch and Bound
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
            if (matrix[i][j] < rowMin)
                rowMin = matrix[i][j];
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
            if (matrix[i][j] < colMin)
                colMin = matrix[i][j];
        if (colMin != INF && colMin > 0) {
            reduction += colMin;
            for (int i = 0; i < n; ++i)
                if (matrix[i][j] != INF)
                    matrix[i][j] -= colMin;
        }
    }

    return reduction;
}

Node* createNode(vector<vector<int>> parentMatrix, vector<pair<int,int>> path,
                 int level, int i, int j, int n) {
    Node* node = new Node;
    node->reducedMatrix = parentMatrix;
    node->path = path;

    if (level != 0)
        node->path.push_back({i, j});

    for (int k = 0; k < n; ++k) {
        node->reducedMatrix[i][k] = INF;
        node->reducedMatrix[k][j] = INF;
    }

    node->reducedMatrix[j][0] = INF;
    node->cost = 0;
    node->level = level;
    node->vertex = j;

    node->cost += reduceMatrix(node->reducedMatrix);
    return node;
}

void printPath(const vector<pair<int,int>> &path) {
    cout << "Tour: ";
    for (auto &p : path)
        cout << p.first << " -> ";
    cout << path.front().first << endl;
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
        Node* minNode = pq.top();
        pq.pop();

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
                Node* child = createNode(minNode->reducedMatrix, minNode->path,
                                         minNode->level + 1, i, j, n);
                child->cost += minNode->cost + costMatrix[i][j];
                pq.push(child);
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

    cout << "Resolviendo TSP con 20 nodos usando Branch and Bound...\n";
    int minCost = solveTSP(costMatrix);
    cout << "Costo mínimo: " << minCost << endl;

    return 0;
}
