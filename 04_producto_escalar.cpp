#include <mpi.h>
#include <iostream>
#include <vector>
using namespace std;

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, p;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    const int N_BASE = 3000000;
    const int N = (N_BASE / p) * p;
    const int localN = N / p;

    vector<double> A, B;
    double productoSec = 0.0;
    double tiempoSec = 0.0;

    // ── SECUENCIAL ────────────────────────────────────────────────────────────
    if (rank == 0) {
        A.resize(N);
        B.resize(N);

        for (int i = 0; i < N; ++i) {
            A[i] = (i % 100) * 0.5;
            B[i] = (i % 50) * 0.25;
        }

        double t1 = MPI_Wtime();
        for (int i = 0; i < N; ++i)
            productoSec += A[i] * B[i];
        tiempoSec = MPI_Wtime() - t1;
    }
    // ── PARALELO (MPI) ────────────────────────────────────────────────────────

    vector<double> localA(localN), localB(localN);

    MPI_Barrier(MPI_COMM_WORLD);
    double inicio = MPI_Wtime();

    // Distribuir ambos vectores entre los procesos
    MPI_Scatter(rank == 0 ? A.data() : nullptr,
                localN, MPI_DOUBLE,
                localA.data(), localN, MPI_DOUBLE,
                0, MPI_COMM_WORLD);

    MPI_Scatter(rank == 0 ? B.data() : nullptr,
                localN, MPI_DOUBLE,
                localB.data(), localN, MPI_DOUBLE,
                0, MPI_COMM_WORLD);

    // Cada proceso calcula su producto escalar parcial
    double productoLocal = 0.0;
    for (int i = 0; i < localN; ++i)
        productoLocal += localA[i] * localB[i];

    // Sumar todos los productos parciales
    double productoPar = 0.0;
    MPI_Reduce(&productoLocal, &productoPar, 1, MPI_DOUBLE,
               MPI_SUM, 0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);
    double tiempoLocal = MPI_Wtime() - inicio;
    double tiempoPar = 0.0;

    MPI_Reduce(&tiempoLocal, &tiempoPar, 1, MPI_DOUBLE,
               MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        double diferencia = productoSec - productoPar;
        if (diferencia < 0) diferencia = -diferencia;

        cout << "=== PRODUCTO ESCALAR ===\n";
        cout << "Producto secuencial: " << productoSec << '\n';
        cout << "Producto MPI:        " << productoPar << '\n';
        cout << "Tiempo secuencial:   " << tiempoSec << " s\n";
        cout << "Tiempo MPI:          " << tiempoPar << " s\n";
        cout << "Coinciden:           " << (diferencia < 1e-6 ? "SI" : "NO") << '\n';
    }

    MPI_Finalize();
    return 0;
}
