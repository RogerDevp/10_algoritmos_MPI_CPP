#include <mpi.h>
#include <iostream>
#include <vector>
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
    long long paresSec = 0;
    double tiempoSec = 0.0;

    // ── SECUENCIAL ────────────────────────────────────────────────────────────
    if (rank == 0) {
        datos.resize(N);
        for (int i = 0; i < N; ++i) datos[i] = i;

        double t1 = MPI_Wtime();
        for (int x : datos)
            if (x % 2 == 0) ++paresSec;
        tiempoSec = MPI_Wtime() - t1;
    }
    // ── PARALELO (MPI) ────────────────────────────────────────────────────────

    vector<int> local(localN);

    MPI_Barrier(MPI_COMM_WORLD);
    double inicio = MPI_Wtime();

    // Distribuir el vector entre todos los procesos
    MPI_Scatter(rank == 0 ? datos.data() : nullptr,
                localN, MPI_INT,
                local.data(), localN, MPI_INT,
                0, MPI_COMM_WORLD);

    // Cada proceso cuenta los pares de su bloque
    long long paresLocal = 0;
    for (int x : local)
        if (x % 2 == 0) ++paresLocal;

    // Sumar todos los conteos parciales
    long long paresPar = 0;
    MPI_Reduce(&paresLocal, &paresPar, 1, MPI_LONG_LONG,
               MPI_SUM, 0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);
    double tiempoLocal = MPI_Wtime() - inicio;
    double tiempoPar = 0.0;

    MPI_Reduce(&tiempoLocal, &tiempoPar, 1, MPI_DOUBLE,
               MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        cout << "=== CONTEO DE PARES ===\n";
        cout << "Pares secuencial: " << paresSec << '\n';
        cout << "Pares MPI:        " << paresPar << '\n';
        cout << "Tiempo secuencial: " << tiempoSec << " s\n";
        cout << "Tiempo MPI:        " << tiempoPar << " s\n";
        cout << "Coinciden:         " << (paresSec == paresPar ? "SI" : "NO") << '\n';
    }

    MPI_Finalize();
    return 0;
}
