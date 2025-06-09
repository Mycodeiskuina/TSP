#include <bits/stdc++.h>
using namespace std;

struct Point {
    double x, y;
};

int firstMin(const vector<vector<int>>& adj, int i) {
    int min = INT_MAX;
    for (int k = 0; k < adj.size(); ++k)
        if (adj[i][k] < min && i != k)
            min = adj[i][k];
    return min;
}

int secondMin(const vector<vector<int>>& adj, int i) {
    int first = INT_MAX, second = INT_MAX;
    for (int j = 0; j < adj.size(); ++j) {
        if (i == j) continue;
        if (adj[i][j] <= first) {
            second = first;
            first = adj[i][j];
        } else if (adj[i][j] <= second && adj[i][j] != first) {
            second = adj[i][j];
        }
    }
    return second;
}

void TSPRec(const vector<vector<int>>& adj, int curr_bound, int curr_weight,
            int level, vector<int>& curr_path, vector<bool>& visited, int& final_res) {

    int N = adj.size();
    if (level == N) {
        if (adj[curr_path[level - 1]][curr_path[0]] != 0) {
            int curr_res = curr_weight + adj[curr_path[level - 1]][curr_path[0]];
            if (curr_res < final_res)
                final_res = curr_res;
        }
        return;
    }

    for (int i = 0; i < N; ++i) {
        if (adj[curr_path[level - 1]][i] != 0 && !visited[i]) {
            int temp = curr_bound;
            curr_weight += adj[curr_path[level - 1]][i];

            if (level == 1)
                curr_bound -= ((firstMin(adj, curr_path[level - 1]) + firstMin(adj, i)) / 2);
            else
                curr_bound -= ((secondMin(adj, curr_path[level - 1]) + firstMin(adj, i)) / 2);

            if (curr_bound + curr_weight < final_res) {
                curr_path[level] = i;
                visited[i] = true;
                TSPRec(adj, curr_bound, curr_weight, level + 1, curr_path, visited, final_res);
            }

            curr_weight -= adj[curr_path[level - 1]][i];
            curr_bound = temp;
            fill(visited.begin(), visited.end(), false);
            for (int j = 0; j <= level - 1; ++j)
                visited[curr_path[j]] = true;
        }
    }
}

int solveTSP(const vector<vector<int>>& adj) {
    int N = adj.size();
    vector<int> curr_path(N + 1, -1);
    vector<bool> visited(N, false);
    int curr_bound = 0;

    for (int i = 0; i < N; ++i)
        curr_bound += (firstMin(adj, i) + secondMin(adj, i));

    curr_bound = (curr_bound & 1) ? curr_bound / 2 + 1 : curr_bound / 2;
    visited[0] = true;
    curr_path[0] = 0;

    int final_res = INT_MAX;
    TSPRec(adj, curr_bound, 0, 1, curr_path, visited, final_res);
    return final_res;
}

vector<Point> parseTSPLIB(const string& filename) {
    ifstream infile(filename);
    string line;
    int dimension = 0;
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

    vector<Point> coords(dimension);
    for (int i = 0; i < dimension; ++i) {
        int index;
        double x, y;
        infile >> index >> x >> y;
        coords[i] = {x, y};
    }

    return coords;
}

vector<vector<int>> computeDistanceMatrix(const vector<Point>& points) {
    int n = points.size();
    vector<vector<int>> adj(n, vector<int>(n, 0));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            if (i != j) {
                double dx = points[i].x - points[j].x;
                double dy = points[i].y - points[j].y;
                adj[i][j] = round(sqrt(dx * dx + dy * dy));
            }
    return adj;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Uso: " << argv[0] << " archivo.tsp" << endl;
        return 1;
    }

    string filename = argv[1];
    vector<Point> points = parseTSPLIB(filename);
    vector<vector<int>> adj = computeDistanceMatrix(points);

    auto start = chrono::high_resolution_clock::now();
    int min_cost = solveTSP(adj);
    auto end = chrono::high_resolution_clock::now();

    cout << "Archivo: " << filename << endl;
    cout << "Distancia mínima del TSP: " << min_cost << endl;
    cout << "Tiempo de ejecución: " << chrono::duration<double>(end - start).count() << " segundos" << endl;

    return 0;
}
