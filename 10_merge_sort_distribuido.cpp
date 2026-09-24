#include <mpi.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
using namespace std;

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, p;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    const int N_BASE = 1000000;
    const int N = (N_BASE / p) * p;
    const int localN = N / p;

    vector<int> original;
    vector<int> secuencial;
    vector<int> paralelo;

    double tiempoSec = 0.0;

    // ── SECUENCIAL ────────────────────────────────────────────────────────────
    if (rank == 0) {
        original.resize(N);
        secuencial.resize(N);
        paralelo.resize(N);

        mt19937 gen(12345);
        uniform_int_distribution<int> dist(0, 10000000);

        for (int& x : original) x = dist(gen);

        secuencial = original;

        double t1 = MPI_Wtime();
        sort(secuencial.begin(), secuencial.end());
        tiempoSec = MPI_Wtime() - t1;
    }
    // ── PARALELO (MPI) ────────────────────────────────────────────────────────

    vector<int> local(localN);

    MPI_Barrier(MPI_COMM_WORLD);
    double inicio = MPI_Wtime();

    // Distribuir el arreglo en bloques iguales
    MPI_Scatter(rank == 0 ? original.data() : nullptr,
                localN, MPI_INT,
                local.data(), localN, MPI_INT,
                0, MPI_COMM_WORLD);

    // Cada proceso ordena su bloque localmente
    sort(local.begin(), local.end());

    // Reunir los bloques ordenados en el proceso 0
    MPI_Gather(local.data(), localN, MPI_INT,
               rank == 0 ? paralelo.data() : nullptr,
               localN, MPI_INT,
               0, MPI_COMM_WORLD);

    // El proceso 0 combina los bloques con inplace_merge
    if (rank == 0) {
        int ordenados = localN;

        for (int bloque = 1; bloque < p; ++bloque) {
            int fin = ordenados + localN;

            inplace_merge(paralelo.begin(),
                          paralelo.begin() + ordenados,
                          paralelo.begin() + fin);

            ordenados = fin;
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double tiempoLocal = MPI_Wtime() - inicio;
    double tiempoPar = 0.0;

    MPI_Reduce(&tiempoLocal, &tiempoPar, 1, MPI_DOUBLE,
               MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        cout << "=== MERGE/ORDENAMIENTO DISTRIBUIDO ===\n";
        cout << "Elementos: " << N << '\n';
        cout << "Tiempo secuencial: " << tiempoSec << " s\n";
        cout << "Tiempo MPI:        " << tiempoPar << " s\n";
        cout << "Coinciden:         "
             << (secuencial == paralelo ? "SI" : "NO") << '\n';
    }

    MPI_Finalize();
    return 0;
}
