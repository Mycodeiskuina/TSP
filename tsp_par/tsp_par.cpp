#include <bits/stdc++.h>
#include <omp.h>
using namespace std;

struct Point {
    double x, y;
};

int dimension;

// Precomputed first and second minimum costs
vector<int> firstMinVec, secondMinVec;

void TSPRec(const vector<vector<int>>& adj, int curr_bound, int curr_weight,
            int level, vector<int>& curr_path, vector<bool>& visited, int& final_res, vector<int>& best_path) {//) {

    int N = adj.size();
    if (level == N) {
        if (adj[curr_path[level - 1]][curr_path[0]] != 0) {
            int curr_res = curr_weight + adj[curr_path[level - 1]][curr_path[0]];

            #pragma omp critical
            {
                if (curr_res < final_res){
                    final_res = curr_res;
                    best_path = curr_path; // save the best path
                }
            }
        }
        return;
    }

    // Reorder neighbors by ascending cost
    vector<pair<int, int>> neighbors;
    for (int i = 0; i < N; ++i) {
        if (!visited[i] && adj[curr_path[level - 1]][i] != 0)
            neighbors.emplace_back(adj[curr_path[level - 1]][i], i);
    }
    sort(neighbors.begin(), neighbors.end());

    for (auto& [cost, i] : neighbors) {
        int temp_bound = curr_bound;
        curr_weight += cost;

        if (level == 1)
            curr_bound -= ((firstMinVec[curr_path[level - 1]] + firstMinVec[i]) / 2);
        else
            curr_bound -= ((secondMinVec[curr_path[level - 1]] + firstMinVec[i]) / 2);

        if (curr_bound + curr_weight < final_res) {
            curr_path[level] = i;
            visited[i] = true;
            TSPRec(adj, curr_bound, curr_weight, level + 1, curr_path, visited, final_res, best_path);
            visited[i] = false;  // Clean backtrack
        }

        curr_weight -= cost;
        curr_bound = temp_bound;
    }
}

int solveTSP(const vector<vector<int>>& adj, vector<int>& best_path) {
    int N = adj.size();
    firstMinVec.resize(N);
    secondMinVec.resize(N);

    int curr_bound = 0;

    // Preprocesamiento de mínimos
    #pragma omp parallel for reduction(+:curr_bound)
    for (int i = 0; i < N; ++i) {
        int first = INT_MAX, second = INT_MAX;
        for (int j = 0; j < N; ++j) {
            if (i == j) continue;
            if (adj[i][j] <= first) {
                second = first;
                first = adj[i][j];
            } else if (adj[i][j] < second) {
                second = adj[i][j];
            }
        }
        firstMinVec[i] = first;
        secondMinVec[i] = second;
        curr_bound += first + second;
    }

    curr_bound = (curr_bound & 1) ? (curr_bound / 2 + 1) : (curr_bound / 2);

    best_path.assign(N + 1, -1); // ruta final
    int final_res = INT_MAX;

    // Primer nivel paralelo
    #pragma omp parallel
    {
        #pragma omp single
        {
            for (int i = 1; i < N; ++i) {
                if (adj[0][i] == 0) continue;

                vector<int> path(N + 1, -1);
                vector<bool> visited(N, false);

                path[0] = 0;
                path[1] = i;
                visited[0] = true;
                visited[i] = true;

                int weight = adj[0][i];
                int reduction = (firstMinVec[0] + firstMinVec[i]) / 2;
                int bound = curr_bound - reduction;

                #pragma omp task firstprivate(path, visited, bound, weight)
                {
                    int local_res = INT_MAX;
                    vector<int> local_path = path;

                    TSPRec(adj, bound, weight, 2, local_path, visited, local_res, local_path);

                    #pragma omp critical
                    {
                        if (local_res < final_res) {
                            final_res = local_res;
                            best_path = local_path;
                            best_path[N] = best_path[0]; // cerrar ciclo
                        }
                    }
                }
            }

            #pragma omp taskwait
        }
    }

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

    int index;
    double x, y;
    vector<Point> coords(dimension);
    
    for (int i = 0; i < dimension; ++i) {
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

    vector<int> best_path; // to save the optimal route
    string filename = argv[1];
    int hilos = atoi(argv[2]);
    omp_set_num_threads(hilos);

    vector<Point> points = parseTSPLIB(filename);
    vector<vector<int>> adj = computeDistanceMatrix(points);

    auto start = omp_get_wtime();
    int min_cost = solveTSP(adj, best_path);
    auto end = omp_get_wtime();

    cout << "Archivo: " << filename << endl;
    cout << "Distancia mínima del TSP: " << min_cost << endl;
    cout << "Tiempo de ejecución: " << chrono::duration<double>(end - start).count() << " segundos" << endl;

    // to save time
    ofstream times_out("tiempos_omp.txt", std::ios::app);

    // sec times
    times_out << dimension << " " << chrono::duration<double>(end-start).count()<< " " <<  hilos <<"\n"; 
    times_out.close();

    // to save results:
    ofstream out("ruta_omp_"+to_string(dimension)+".txt");
    best_path.resize(points.size());

    for (int i : best_path)
        out << points[i].x << " " << points[i].y << "\n";    
    out << points[best_path[0]].x << " " << points[best_path[0]].y << "\n";
    out << "Distancia: "<<min_cost<<endl;
    out.close();
    

    return 0;
}
