#include <mpi.h>
#include <iostream>
#include <vector>
#include <climits>
using namespace std;

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, p;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    const int N_BASE = 4000000;
    const int N = (N_BASE / p) * p;
    const int localN = N / p;

    vector<int> datos;
    int maxSec = INT_MIN;
    double tiempoSec = 0.0;

    // ── SECUENCIAL ────────────────────────────────────────────────────────────
    if (rank == 0) {
        datos.resize(N);
        for (int i = 0; i < N; ++i)
            datos[i] = (i * 37) % 1000003;

        double t1 = MPI_Wtime();
        for (int x : datos)
            if (x > maxSec) maxSec = x;
        tiempoSec = MPI_Wtime() - t1;
    }
    // ── PARALELO (MPI) ────────────────────────────────────────────────────────

    vector<int> local(localN);

    MPI_Barrier(MPI_COMM_WORLD);
    double inicio = MPI_Wtime();

    // Repartir el vector en bloques iguales
    MPI_Scatter(rank == 0 ? datos.data() : nullptr,
                localN, MPI_INT,
                local.data(), localN, MPI_INT,
                0, MPI_COMM_WORLD);

    // Cada proceso calcula el máximo de su bloque
    int maxLocal = INT_MIN;
    for (int x : local)
        if (x > maxLocal) maxLocal = x;

    // Reducción global con MPI_MAX
    int maxPar = INT_MIN;
    MPI_Reduce(&maxLocal, &maxPar, 1, MPI_INT,
               MPI_MAX, 0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);
    double tiempoLocal = MPI_Wtime() - inicio;
    double tiempoPar = 0.0;

    MPI_Reduce(&tiempoLocal, &tiempoPar, 1, MPI_DOUBLE,
               MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        cout << "=== MAXIMO DE VECTOR ===\n";
        cout << "Maximo secuencial: " << maxSec << '\n';
        cout << "Maximo MPI:        " << maxPar << '\n';
        cout << "Tiempo secuencial: " << tiempoSec << " s\n";
        cout << "Tiempo MPI:        " << tiempoPar << " s\n";
        cout << "Coinciden:         " << (maxSec == maxPar ? "SI" : "NO") << '\n';
    }

    MPI_Finalize();
    return 0;
}
