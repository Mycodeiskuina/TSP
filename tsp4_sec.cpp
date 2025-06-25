
#include <bits/stdc++.h>
using namespace std;

const int INF = numeric_limits<int>::max();



struct Point {
    int x, y;
};

int euclideanDistance(const Point &a, const Point &b) {
    return static_cast<int>(sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y)) + 0.5);
}


struct Node {
    vector<vector<int>> reducedMatrix;
    vector<pair<int, int>> path;
    int lowerBound;    // Only lowerBound remains
    int vertex;
    int level;
    set<pair<int, int>> excludedEdges;
    bool operator>(const Node &other) const { return lowerBound > other.lowerBound; }
};

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

int reduceMatrix(vector<vector<int>> &matrix) {
    int n = matrix.size(), reduction = 0;
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
        for (int i = 0; i < n; ++i) colMin = min(colMin, matrix[i][j]);
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

void forbidReverseEdge(vector<vector<int>> &matrix, int from, int to) {
    matrix[to][from] = INF;
}

Node* createInclusionNode(Node* parent, int i, int j) {
    int n = parent->reducedMatrix.size();
    vector<vector<int>> matrix = parent->reducedMatrix;
    for (int k = 0; k < n; ++k) {
        matrix[i][k] = INF;
        matrix[k][j] = INF;
    }

    unordered_map<int, int> forward, backward;
    for (const auto& edge : parent->path) {
        forward[edge.first] = edge.second;
        backward[edge.second] = edge.first;
    }

    int start = i;
    while (backward.count(start)) start = backward[start];
    int end = j;
    while (forward.count(end)) end = forward[end];

    if (start != end) matrix[end][start] = INF;

    int reduction = reduceMatrix(matrix);
    vector<pair<int, int>> newPath = parent->path;
    newPath.push_back({i, j});
    return new Node{matrix, newPath, parent->lowerBound + reduction, j, parent->level + 1};
}

Node* createExclusionNode(Node* parent, int i, int j) {
    vector<vector<int>> matrix = parent->reducedMatrix;
    matrix[i][j] = INF;
    int reduction = reduceMatrix(matrix);
    return new Node{matrix, parent->path, parent->lowerBound + reduction, parent->vertex, parent->level};
}

vector<int> reconstructTour(const vector<pair<int, int>> &path) {
    unordered_map<int, int> next;
    for (auto [u, v] : path) next[u] = v;

    vector<int> tour;
    int curr = 0;
    do {
        tour.push_back(curr);
        curr = next[curr];
    } while (curr != 0 && tour.size() <= path.size());

    tour.push_back(0);
    return tour;
}

void printPriorityQueue(priority_queue<Node*, vector<Node*>, greater<Node*>> pq) {
    cout << "Contenido de la priority_queue (nivel | lowerBound | path parcial):\n";
    while (!pq.empty()) {
        Node* node = pq.top(); pq.pop();
        cout << "  Nivel: " << node->level << ", LB: " << node->lowerBound << ", Path: ";
        for (auto [u, v] : node->path) {
            cout << "(" << u << "->" << v << ") ";
        }
        cout << endl;
    }
    cout << "----------------------------------------\n";
}

int solveTSP(vector<vector<int>> costMatrix) {
    int n = costMatrix.size();
    int reduction = reduceMatrix(costMatrix);
    Node* root = new Node{costMatrix, {}, reduction, 0, 0};
    priority_queue<Node*, vector<Node*>, greater<Node*>> pq;
    pq.push(root);
    int minCost = INF;
    vector<pair<int, int>> bestPath;
    int x = 10;
    while (!pq.empty()) {
         //printPriorityQueue(pq);
        x--;
        Node* node = pq.top(); pq.pop();
        if (node->level == n - 1) {

            
            unordered_set<int> fromSet, toSet;
            for (auto &[from, to] : node->path) {
                fromSet.insert(from);
                toSet.insert(to);
            }

            int missingFrom = -1, missingTo = -1;

            for (int i = 0; i < n; ++i) {
                if (!fromSet.count(i)) missingFrom = i; // nodo sin salida
                if (!toSet.count(i)) missingTo = i;     // nodo sin entrada
            }

            if (missingFrom != -1 && missingTo != -1) {
                node->path.push_back({missingFrom, missingTo});
            }
            int finalCost = node->lowerBound;

            if (finalCost < minCost) {
                minCost = finalCost;
                bestPath = node->path;
            }

            if (finalCost <= pq.top()->lowerBound) {
                //cout << "Solución óptima encontrada temprano.\n";
                break;
            }

            delete node;
            continue;
        }


        pair<int, int> edge = selectEdgeWithMaxPenalty(node->reducedMatrix);
        if (edge.first == -1) continue;

        pq.push(createInclusionNode(node, edge.first, edge.second));
        pq.push(createExclusionNode(node, edge.first, edge.second));
        delete node;
    }

    // Reconstruct the best path
    vector<int> tour = reconstructTour(bestPath);

    cout << "Mejor recorrido: ";
    for (int city : tour) {
        cout << city << " ";
    }
    return minCost;
}

int main() {
    // vector<vector<int>> costMatrix = {
    //     {INF, 27, 43, 16, 30, 26},
    //     {7, INF, 16, 1, 30, 25},
    //     {20, 13, INF, 35, 5, 0},
    //     {21, 16, 25, INF, 18, 18},
    //     {12, 46, 27, 48, INF, 5},
    //     {23, 5, 5, 9, 5, INF}
    // };
    cout << "Resolviendo TSP con Branch and Bound...\n ";

    string filename = "rbu10.tsp";
    vector<vector<int>> costMatrix = loadTSPMatrix(filename);
    auto start = chrono::high_resolution_clock::now();
    int minCost = solveTSP(costMatrix);
    auto end = chrono::high_resolution_clock::now();
    cout << "Costo mínimo: " << minCost << endl;
    cout << "Tiempo de ejecución: " << chrono::duration<double>(end - start).count() << " segundos\n";
    return 0;
}
