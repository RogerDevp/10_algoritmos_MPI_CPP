#include <mpi.h>
#include <iostream>
#include <vector>
using namespace std;

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, p;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    const int N_BASE = 800;
    const int N = (N_BASE / p) * p;

    if (N == 0) {
        if (rank == 0) cerr << "Demasiados procesos para el tamano de matriz.\n";
        MPI_Finalize();
        return 1;
    }

    const int filasLocal = N / p;
    const int elementosLocal = filasLocal * N;

    vector<int> A, B, Csec, Cpar;

    if (rank == 0) {
        A.resize(N * N);
        B.resize(N * N);
        Csec.resize(N * N);
        Cpar.resize(N * N);

        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j) {
                A[i * N + j] = i + j;
                B[i * N + j] = i - j;
            }
    }

    double tiempoSec = 0.0;

    // ── SECUENCIAL ────────────────────────────────────────────────────────────
    if (rank == 0) {
        double t1 = MPI_Wtime();

        for (int i = 0; i < N * N; ++i)
            Csec[i] = A[i] + B[i];

        tiempoSec = MPI_Wtime() - t1;
    }
    // ── PARALELO (MPI) ────────────────────────────────────────────────────────

    vector<int> localA(elementosLocal);
    vector<int> localB(elementosLocal);
    vector<int> localC(elementosLocal);

    MPI_Barrier(MPI_COMM_WORLD);
    double inicio = MPI_Wtime();

    // Distribuir filas de A y B entre los procesos
    MPI_Scatter(rank == 0 ? A.data() : nullptr,
                elementosLocal, MPI_INT,
                localA.data(), elementosLocal, MPI_INT,
                0, MPI_COMM_WORLD);

    MPI_Scatter(rank == 0 ? B.data() : nullptr,
                elementosLocal, MPI_INT,
                localB.data(), elementosLocal, MPI_INT,
                0, MPI_COMM_WORLD);

    // Cada proceso suma su bloque de filas
    for (int i = 0; i < elementosLocal; ++i)
        localC[i] = localA[i] + localB[i];

    // Reunir los bloques resultado en el proceso 0
    MPI_Gather(localC.data(), elementosLocal, MPI_INT,
               rank == 0 ? Cpar.data() : nullptr,
               elementosLocal, MPI_INT,
               0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);
    double tiempoLocal = MPI_Wtime() - inicio;
    double tiempoPar = 0.0;

    MPI_Reduce(&tiempoLocal, &tiempoPar, 1, MPI_DOUBLE,
               MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        cout << "=== SUMA DE MATRICES ===\n";
        cout << "Dimension: " << N << " x " << N << '\n';
        cout << "Tiempo secuencial: " << tiempoSec << " s\n";
        cout << "Tiempo MPI:        " << tiempoPar << " s\n";
        cout << "Coinciden:         " << (Csec == Cpar ? "SI" : "NO") << '\n';
    }

    MPI_Finalize();
    return 0;
}
