#include <bits/stdc++.h>
#include <omp.h>
using namespace std;

struct Point {
    double x, y;
};

int firstMin(const vector<vector<int>>& adj, int i) {
    int m = INT_MAX;
    for (int k = 0; k < adj.size(); ++k)
        if (i != k && adj[i][k] < m)
            m = adj[i][k];
    return m;
}

int secondMin(const vector<vector<int>>& adj, int i) {
    int first = INT_MAX, second = INT_MAX;
    for (int j = 0; j < adj.size(); ++j) {
        if (i == j) continue;
        if (adj[i][j] <= first) {
            second = first;
            first = adj[i][j];
        } else if (adj[i][j] < second && adj[i][j] != first) {
            second = adj[i][j];
        }
    }
    return second;
}

inline void updateBest(int &best, int candidate) {
    #pragma omp critical
    {
        if (candidate < best) best = candidate;
    }
}

void TSPRec(const vector<vector<int>>& adj,
            int curr_bound, int curr_weight, int level,
            vector<int> curr_path, vector<bool> visited,
            int &best) {
    int N = adj.size();

    if (level == N) {
        if (adj[curr_path[level - 1]][curr_path[0]] != 0) {
            int curr_res = curr_weight + adj[curr_path[level - 1]][curr_path[0]];
            updateBest(best, curr_res);
        }
        return;
    }

    for (int i = 0; i < N; ++i) {
        if (adj[curr_path[level - 1]][i] != 0 && !visited[i]) {
            int temp_bound = curr_bound;
            int temp_weight = curr_weight + adj[curr_path[level - 1]][i];

            if (level == 1)
                curr_bound -= (firstMin(adj, curr_path[level - 1]) + firstMin(adj, i)) / 2;
            else
                curr_bound -= (secondMin(adj, curr_path[level - 1]) + firstMin(adj, i)) / 2;

            if (curr_bound + temp_weight < best) {
                curr_path[level] = i;
                visited[i] = true;

                #pragma omp task firstprivate(adj, curr_bound, temp_weight, level, curr_path, visited) shared(best) if(level <= 2)
                {
                    TSPRec(adj, curr_bound, temp_weight, level + 1, curr_path, visited, best);
                }
            }
            curr_bound = temp_bound; // restore bound for next sibling
        }
    }
    #pragma omp taskwait
}

int solveTSP(const vector<vector<int>>& adj) {
    int N = adj.size();
    vector<int> curr_path(N + 1, -1);
    vector<bool> visited(N, false);

    int curr_bound = 0;
    for (int i = 0; i < N; ++i)
        curr_bound += firstMin(adj, i) + secondMin(adj, i);
    curr_bound = (curr_bound & 1) ? curr_bound / 2 + 1 : curr_bound / 2;

    visited[0] = true;
    curr_path[0] = 0;
    int best = INT_MAX;

    #pragma omp parallel
    {
        #pragma omp single nowait
        {
            TSPRec(adj, curr_bound, 0, 1, curr_path, visited, best);
        }
    }
    return best;
}

vector<Point> parseTSPLIB(const string &filename) {
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
        int idx; double x, y;
        infile >> idx >> x >> y;
        coords[i] = {x, y};
    }
    return coords;
}

vector<vector<int>> computeDistanceMatrix(const vector<Point>& pts) {
    int n = pts.size();
    vector<vector<int>> adj(n, vector<int>(n));

    #pragma omp parallel for schedule(static)
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (i == j) {
                adj[i][j] = 0;
            } else {
                double dx = pts[i].x - pts[j].x;
                double dy = pts[i].y - pts[j].y;
                adj[i][j] = round(sqrt(dx * dx + dy * dy));
            }
        }
    }
    return adj;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cerr << "Uso: " << argv[0] << " archivo.tsp" << endl;
        return 1;
    }

    string filename = argv[1];
    vector<Point> pts = parseTSPLIB(filename);
    vector<vector<int>> adj = computeDistanceMatrix(pts);

    auto start = chrono::high_resolution_clock::now();
    int min_cost = solveTSP(adj);
    auto end = chrono::high_resolution_clock::now();

    cout << "Archivo: " << filename << "\n";
    cout << "Distancia mínima del TSP: " << min_cost << "\n";
    cout << "Tiempo de ejecución: " << chrono::duration<double>(end - start).count() << " segundos" << endl;

    return 0;
}
