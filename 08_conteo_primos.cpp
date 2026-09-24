#include <mpi.h>
#include <iostream>
#include <cmath>
using namespace std;

bool esPrimo(int n) {
    if (n < 2) return false;
    if (n == 2) return true;
    if (n % 2 == 0) return false;

    int limite = static_cast<int>(sqrt(n));

    for (int d = 3; d <= limite; d += 2)
        if (n % d == 0) return false;

    return true;
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, p;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    const int LIMITE = 1500000;

    long long primosSec = 0;
    double tiempoSec = 0.0;

    // ── SECUENCIAL ────────────────────────────────────────────────────────────
    if (rank == 0) {
        double t1 = MPI_Wtime();

        for (int n = 2; n <= LIMITE; ++n)
            if (esPrimo(n)) ++primosSec;

        tiempoSec = MPI_Wtime() - t1;
    }
    // ── PARALELO (MPI) ────────────────────────────────────────────────────────

    MPI_Barrier(MPI_COMM_WORLD);
    double inicio = MPI_Wtime();

    // Cada proceso calcula su subintervalo usando rank y p
    int totalNumeros = LIMITE - 1;
    int inicioLocal = 2 + (rank * totalNumeros) / p;
    int finLocal    = 2 + ((rank + 1) * totalNumeros) / p;

    long long primosLocal = 0;
    for (int n = inicioLocal; n < finLocal; ++n)
        if (esPrimo(n)) ++primosLocal;

    // Sumar todos los conteos parciales
    long long primosPar = 0;
    MPI_Reduce(&primosLocal, &primosPar, 1, MPI_LONG_LONG,
               MPI_SUM, 0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);
    double tiempoLocal = MPI_Wtime() - inicio;
    double tiempoPar = 0.0;

    MPI_Reduce(&tiempoLocal, &tiempoPar, 1, MPI_DOUBLE,
               MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        cout << "=== CONTEO DE NUMEROS PRIMOS ===\n";
        cout << "Primos secuencial: " << primosSec << '\n';
        cout << "Primos MPI:        " << primosPar << '\n';
        cout << "Tiempo secuencial: " << tiempoSec << " s\n";
        cout << "Tiempo MPI:        " << tiempoPar << " s\n";
        cout << "Coinciden:         " << (primosSec == primosPar ? "SI" : "NO") << '\n';
    }

    MPI_Finalize();
    return 0;
}
