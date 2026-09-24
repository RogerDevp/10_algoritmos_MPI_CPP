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
    const int objetivo = N - 123;

    vector<int> datos;
    int posSec = -1;
    double tiempoSec = 0.0;

    // ── SECUENCIAL ────────────────────────────────────────────────────────────
    if (rank == 0) {
        datos.resize(N);
        for (int i = 0; i < N; ++i) datos[i] = i;

        double t1 = MPI_Wtime();
        for (int i = 0; i < N; ++i) {
            if (datos[i] == objetivo) {
                posSec = i;
                break;
            }
        }
        tiempoSec = MPI_Wtime() - t1;
    }
    // ── PARALELO (MPI) ────────────────────────────────────────────────────────

    vector<int> local(localN);

    MPI_Barrier(MPI_COMM_WORLD);
    double inicio = MPI_Wtime();

    // Cada proceso recibe su bloque del vector
    MPI_Scatter(rank == 0 ? datos.data() : nullptr,
                localN, MPI_INT,
                local.data(), localN, MPI_INT,
                0, MPI_COMM_WORLD);

    // Buscar el objetivo y convertir índice local a global
    int posLocal = INT_MAX;
    for (int i = 0; i < localN; ++i) {
        if (local[i] == objetivo) {
            posLocal = rank * localN + i;
            break;
        }
    }

    // MPI_MIN devuelve la primera posición encontrada entre todos los procesos
    int posPar = INT_MAX;
    MPI_Reduce(&posLocal, &posPar, 1, MPI_INT,
               MPI_MIN, 0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);
    double tiempoLocal = MPI_Wtime() - inicio;
    double tiempoPar = 0.0;

    MPI_Reduce(&tiempoLocal, &tiempoPar, 1, MPI_DOUBLE,
               MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        if (posPar == INT_MAX) posPar = -1;

        cout << "=== BUSQUEDA DE ELEMENTO ===\n";
        cout << "Posicion secuencial: " << posSec << '\n';
        cout << "Posicion MPI:        " << posPar << '\n';
        cout << "Tiempo secuencial:   " << tiempoSec << " s\n";
        cout << "Tiempo MPI:          " << tiempoPar << " s\n";
        cout << "Coinciden:           " << (posSec == posPar ? "SI" : "NO") << '\n';
    }

    MPI_Finalize();
    return 0;
}
