#include <mpi.h>
#include <iostream>
#include <vector>
using namespace std;

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, p;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    vector<int> secuencial;

    // ── SECUENCIAL ────────────────────────────────────────────────────────────
    // [0, 1, 2, 3] -> [3, 0, 1, 2]  (cada elemento avanza una posición)
    if (rank == 0) {
        vector<int> original(p);
        secuencial.resize(p);

        for (int i = 0; i < p; ++i) original[i] = i;

        for (int i = 0; i < p; ++i)
            secuencial[(i + 1) % p] = original[i];

        cout << "=== DESPLAZAMIENTO CIRCULAR EN ANILLO ===\n";
        cout << "Secuencial: ";
        for (int x : secuencial) cout << x << ' ';
        cout << '\n';
    }
    // ── PARALELO (MPI) ────────────────────────────────────────────────────────

    // Cada proceso almacena un elemento; comunica en anillo con Sendrecv
    int dato = rank;
    int recibido = -1;

    int der = (rank + 1) % p;
    int izq = (rank - 1 + p) % p;

    // Sendrecv evita el deadlock que causaría Send+Recv con todos los procesos
    MPI_Sendrecv(&dato, 1, MPI_INT, der, 0,
                 &recibido, 1, MPI_INT, izq, 0,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    vector<int> resultado;
    if (rank == 0) resultado.resize(p);

    MPI_Gather(&recibido, 1, MPI_INT,
               rank == 0 ? resultado.data() : nullptr,
               1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        cout << "MPI:        ";
        for (int x : resultado) cout << x << ' ';
        cout << '\n';

        cout << "Coinciden:  "
             << (resultado == secuencial ? "SI" : "NO") << '\n';

        cout << "\nMPI_Sendrecv evita el patron inseguro:\n";
        cout << "MPI_Send(...); MPI_Recv(...);\n";
        cout << "donde todos los procesos podrian quedar esperando.\n";
    }

    MPI_Finalize();
    return 0;
}
