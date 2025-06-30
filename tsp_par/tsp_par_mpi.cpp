// tsp_mpi.cpp
// Paralelización del TSP con Branch & Bound usando MPI (memoria distribuida)

#include <bits/stdc++.h>
#include <mpi.h>
using namespace std;

struct Point {
    double x, y;
};

// Para el Branch & Bound secuencial dentro de cada proceso
void TSPRecSeq(const vector<vector<int>>& adj,
               const vector<int>& firstMinVec,
               const vector<int>& secondMinVec,
               int curr_bound,
               int curr_weight,
               int level,
               vector<int>& curr_path,
               vector<bool>& visited,
               int& final_res)
{
    int N = adj.size();
    if (level == N) {
        int last = curr_path[level - 1], first = curr_path[0];
        if (adj[last][first] != 0) {
            int res = curr_weight + adj[last][first];
            final_res = min(final_res, res);
        }
        return;
    }

    // ordenar vecinos por coste ascendente
    vector<pair<int,int>> neigh;
    int u = curr_path[level - 1];
    for (int v = 0; v < N; ++v) {
        if (!visited[v] && adj[u][v] != 0)
            neigh.emplace_back(adj[u][v], v);
    }
    sort(neigh.begin(), neigh.end());

    for (auto &p : neigh) {
        int cost = p.first, v = p.second;
        int saved_bound  = curr_bound;
        int saved_weight = curr_weight;

        curr_weight += cost;
        if (level == 1)
            curr_bound -= ( firstMinVec[u] + firstMinVec[v] ) / 2;
        else
            curr_bound -= ( secondMinVec[u] + firstMinVec[v] ) / 2;

        if (curr_bound + curr_weight < final_res) {
            curr_path[level] = v;
            visited[v] = true;
            TSPRecSeq(adj, firstMinVec, secondMinVec,
                      curr_bound, curr_weight,
                      level+1, curr_path, visited, final_res);
            visited[v] = false;
        }

        curr_weight = saved_weight;
        curr_bound  = saved_bound;
    }
}

// Lee coordenadas desde un archivo .tsp en formato TSPLIB
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
        int idx; double x,y;
        infile >> idx >> x >> y;
        coords[i] = {x,y};
    }
    return coords;
}

// Construye matriz de distancias redondeadas
vector<vector<int>> computeDistanceMatrix(const vector<Point>& pts) {
    int n = pts.size();
    vector<vector<int>> adj(n, vector<int>(n,0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (i == j) continue;
            double dx = pts[i].x - pts[j].x;
            double dy = pts[i].y - pts[j].y;
            adj[i][j] = int(round(sqrt(dx*dx + dy*dy)));
        }
    }
    return adj;
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 2) {
        if (rank == 0)
            cerr << "Uso: mpirun -np <P> " << argv[0] << " archivo.tsp\n";
        MPI_Finalize();
        return 1;
    }

    // Paso 1: Rank 0 lee y distribuye datos
    int N;
    vector<vector<int>> adj;
    if (rank == 0) {
        auto pts = parseTSPLIB(argv[1]);
        adj = computeDistanceMatrix(pts);
        N = adj.size();
    }

    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
    // aplanar y broadcast de la matriz
    vector<int> flat;
    if (rank == 0) {
        flat.reserve(N*N);
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
                flat.push_back(adj[i][j]);
    } else {
        flat.resize(N*N);
    }
    MPI_Bcast(flat.data(), N*N, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank != 0) {
        adj.assign(N, vector<int>(N));
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
                adj[i][j] = flat[i*N + j];
    }

    // Paso 2: Precompute bounds locales
    vector<int> firstMinVec(N), secondMinVec(N);
    int init_bound = 0;
    for (int i = 0; i < N; ++i) {
        int f = INT_MAX, s = INT_MAX;
        for (int j = 0; j < N; ++j) {
            if (i == j) continue;
            int c = adj[i][j];
            if (c <= f) { s = f; f = c; }
            else if (c < s) s = c;
        }
        firstMinVec[i]  = f;
        secondMinVec[i] = s;
        init_bound += f + s;
    }
    init_bound = (init_bound & 1) ? init_bound/2 + 1 : init_bound/2;

    // Paso 3: Cada proceso explora subárboles iniciales asignados estáticamente
    vector<int> assigned;
    for (int i = 1; i < N; ++i) {
        if ((i-1) % size == rank)
            assigned.push_back(i);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double t0 = MPI_Wtime();

    int local_best = INT_MAX;
    vector<int> path(N+1);
    vector<bool> visited(N);

    for (int next : assigned) {
        // inicializar ruta y visited
        fill(visited.begin(), visited.end(), false);
        path[0] = 0; visited[0] = true;
        path[1] = next; visited[next] = true;

        int cb = init_bound
               - ( firstMinVec[0] + firstMinVec[next] )/2;
        int cw = adj[0][next];

        TSPRecSeq(adj, firstMinVec, secondMinVec,
                  cb, cw, 2, path, visited, local_best);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double t1 = MPI_Wtime();

    // Paso 4: Reducir para obtener el mínimo global
    int global_best;
    MPI_Reduce(&local_best, &global_best, 1,
               MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        cout << "Distancia mínima del TSP: " << global_best << "\n"
             << "Tiempo de ejecución (paralelo MPI, "
             << size << " procesos): "
             << (t1 - t0) << " segundos\n";
    }

    MPI_Finalize();
    return 0;
}
