#include <bits/stdc++.h>
#include "omp.h"
using namespace std;

int dimension = 0;
int th = 0;
long long flop_counter = 0; // Contador global de FLOPs

void copyToFinal(const vector<int>& curr_path, vector<int>& final_path) {
    int N = curr_path.size();
    final_path.resize(N + 1);
    for (int i = 0; i < N; i++)
        final_path[i] = curr_path[i];
    final_path[N] = curr_path[0]; // Cierra el ciclo
}

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
            int level, vector<int>& curr_path, vector<bool>& visited,
            int& final_res, vector<int>& final_path) {
    int N = adj.size();
    if (level == N) {
        if (adj[curr_path[level - 1]][curr_path[0]] != 0) {
            int curr_res = curr_weight + adj[curr_path[level - 1]][curr_path[0]];
            #pragma omp atomic
            flop_counter++; // suma final
            if (curr_res < final_res) {
                final_res = curr_res;
                final_path = curr_path;
            }
        }
        return;
    }

    for (int i = 0; i < N; ++i) {
        if (adj[curr_path[level - 1]][i] != 0 && !visited[i]) {
            int temp = curr_bound;
            curr_weight += adj[curr_path[level - 1]][i];
            #pragma omp atomic
            flop_counter++; // suma de peso

            if (level == 1) {
                curr_bound -= ((firstMin(adj, curr_path[level - 1]) + firstMin(adj, i)) / 2);
                #pragma omp atomic
                flop_counter += 3; // 2 mins + 1 suma + 1 división
            } else {
                curr_bound -= ((secondMin(adj, curr_path[level - 1]) + secondMin(adj, i)) / 2);
                #pragma omp atomic
                flop_counter += 3;
            }

            if (curr_bound + curr_weight < final_res) {
                #pragma omp atomic
                flop_counter++; // suma para comparar
                curr_path[level] = i;
                visited[i] = true;
                TSPRec(adj, curr_bound, curr_weight, level + 1, curr_path, visited, final_res, final_path);
            }

            curr_weight -= adj[curr_path[level - 1]][i];
            #pragma omp atomic
            flop_counter++; // resta
            curr_bound = temp;

            fill(visited.begin(), visited.end(), false);
            for (int j = 0; j <= level - 1; ++j)
                visited[curr_path[j]] = true;
        }
    }
}

int solveTSP(const vector<vector<int>>& adj, vector<int>& final_path) {
    int N = adj.size();
    vector<int> curr_path(N + 1, -1);
    vector<bool> visited(N, false);
    int curr_bound = 0;

    #pragma omp parallel for reduction(+:curr_bound) schedule(dynamic)
    for (int i = 0; i < N; ++i)
        curr_bound += (firstMin(adj, i) + secondMin(adj, i));

    curr_bound = (curr_bound & 1) ? curr_bound / 2 + 1 : curr_bound / 2;

    int final_res = INT_MAX;
    vector<int> best_path;

    #pragma omp parallel for shared(final_res, best_path) schedule(dynamic)
    for (int i = 1; i < N; ++i) {
        vector<int> curr_path(N + 1, -1);
        vector<bool> visited(N, false);
        int local_res = INT_MAX;
        vector<int> local_path;

        visited[0] = visited[i] = true;
        curr_path[0] = 0;
        curr_path[1] = i;

        int curr_weight = adj[0][i];
        int bound = curr_bound - ((firstMin(adj, 0) + firstMin(adj, i)) / 2);
        #pragma omp atomic
        flop_counter += 3;

        TSPRec(adj, bound, curr_weight, 2, curr_path, visited, local_res, local_path);

        #pragma omp critical
        {
            if (local_res < final_res) {
                final_res = local_res;
                best_path = local_path;
            }
        }
    }

    best_path.push_back(best_path[0]);
    final_path = best_path;
    return final_res;
}

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
                #pragma omp atomic
                flop_counter += 2; // restas
                adj[i][j] = round(sqrt(dx * dx + dy * dy));
                #pragma omp atomic
                flop_counter += 3; // mults + suma + raíz
            }
    return adj;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Uso: " << argv[0] << " archivo.tsp num_hilos" << endl;
        return 1;
    }

    vector<int> final_path;
    string filename = argv[1];
    th = stoi(argv[2]);
    omp_set_num_threads(th);

    vector<Point> points = parseTSPLIB(filename);
    vector<vector<int>> adj = computeDistanceMatrix(points);

    auto start = omp_get_wtime();
    int min_cost = solveTSP(adj, final_path);
    auto end = omp_get_wtime();
    double exec_time = end - start;

    cout << "Archivo: " << filename << endl;
    cout << "Distancia mínima del TSP: " << min_cost << endl;
    cout << "Tiempo de ejecución: " << exec_time << " segundos" << endl;
    cout << "FLOPs estimados: " << flop_counter << endl;
    cout << "GFLOPS: " << (double)flop_counter / (exec_time * 1e9) << endl;

    ofstream times_out("tiempos_omp.txt", std::ios::app);
    times_out << dimension << " " << exec_time << " " << th << " " << flop_counter << " " << (double)flop_counter / (exec_time * 1e9) << "\n";
    times_out.close();

    // ofstream out("ruta_" + to_string(dimension) + ".txt");
    // final_path.resize(points.size());
    // for (int i : final_path)
    //     out << points[i].x << " " << points[i].y << "\n";
    // out << points[final_path[0]].x << " " << points[final_path[0]].y << "\n";
    // out << "Distancia: " << min_cost << endl;
    // out.close();

    return 0;
}
