#include <mpi.h>
#include <iostream>
#include <vector>
#include <cmath>
using namespace std;

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, p;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    const int N_BASE = 240;
    const int N = (N_BASE / p) * p;

    if (N == 0) {
        if (rank == 0) cerr << "Demasiados procesos para el tamano de matriz.\n";
        MPI_Finalize();
        return 1;
    }

    const int filasLocal = N / p;

    vector<double> A, B(N * N), Csec, Cpar;

    if (rank == 0) {
        A.resize(N * N);
        Csec.assign(N * N, 0.0);
        Cpar.assign(N * N, 0.0);

        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j) {
                A[i * N + j] = (i + j) % 10;
                B[i * N + j] = (i * j) % 10;
            }
    }

    double tiempoSec = 0.0;

    // ── SECUENCIAL ────────────────────────────────────────────────────────────
    if (rank == 0) {
        double t1 = MPI_Wtime();

        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
                for (int k = 0; k < N; ++k)
                    Csec[i * N + j] += A[i * N + k] * B[k * N + j];

        tiempoSec = MPI_Wtime() - t1;
    }
    // ── PARALELO (MPI) ────────────────────────────────────────────────────────

    vector<double> localA(filasLocal * N);
    vector<double> localC(filasLocal * N, 0.0);

    MPI_Barrier(MPI_COMM_WORLD);
    double inicio = MPI_Wtime();

    // B completa se replica en todos los procesos con Bcast
    MPI_Bcast(B.data(), N * N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // A se divide por filas: cada proceso recibe su bloque
    MPI_Scatter(rank == 0 ? A.data() : nullptr,
                filasLocal * N, MPI_DOUBLE,
                localA.data(), filasLocal * N, MPI_DOUBLE,
                0, MPI_COMM_WORLD);

    // Cada proceso calcula las filas de C que le corresponden
    for (int i = 0; i < filasLocal; ++i)
        for (int j = 0; j < N; ++j)
            for (int k = 0; k < N; ++k)
                localC[i * N + j] += localA[i * N + k] * B[k * N + j];

    // Reunir los bloques de C en el proceso 0
    MPI_Gather(localC.data(), filasLocal * N, MPI_DOUBLE,
               rank == 0 ? Cpar.data() : nullptr,
               filasLocal * N, MPI_DOUBLE,
               0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);
    double tiempoLocal = MPI_Wtime() - inicio;
    double tiempoPar = 0.0;

    MPI_Reduce(&tiempoLocal, &tiempoPar, 1, MPI_DOUBLE,
               MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        bool iguales = true;

        for (int i = 0; i < N * N; ++i) {
            if (fabs(Csec[i] - Cpar[i]) > 1e-9) {
                iguales = false;
                break;
            }
        }

        cout << "=== MULTIPLICACION DE MATRICES ===\n";
        cout << "Dimension: " << N << " x " << N << '\n';
        cout << "Tiempo secuencial: " << tiempoSec << " s\n";
        cout << "Tiempo MPI:        " << tiempoPar << " s\n";
        cout << "Coinciden:         " << (iguales ? "SI" : "NO") << '\n';
    }

    MPI_Finalize();
    return 0;
}
