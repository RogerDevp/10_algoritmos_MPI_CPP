#include <iostream>
#include <mpi.h>
#include <vector>
using namespace std;

int main(int argc, char **argv) {
  MPI_Init(&argc, &argv);

  int rank, p;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &p);

  const int N_BASE = 4000000;
  const int N = (N_BASE / p) * p;
  const int localN = N / p;

  vector<long long> datos;
  long long sumaSec = 0;

  // ── SECUENCIAL ────────────────────────────────────────────────────────────
  if (rank == 0) {
    datos.resize(N);
    for (int i = 0; i < N; ++i)
      datos[i] = (i % 100) + 1;

    double t1 = MPI_Wtime();
    for (long long x : datos)
      sumaSec += x;
    double t2 = MPI_Wtime();

    cout << "=== SUMA DE VECTOR: SECUENCIAL vs MPI ===\n";
    cout << "Tiempo secuencial: " << t2 - t1 << " s\n";
  }
  // ── PARALELO (MPI) ────────────────────────────────────────────────────────

  MPI_Barrier(MPI_COMM_WORLD);
  double inicioPar = MPI_Wtime();

  vector<long long> local(localN);

  const int TAG_DATOS = 10;
  const int TAG_RESULTADO = 20;

  // Proceso 0 distribuye bloques; cada proceso calcula su suma parcial
  if (rank == 0) {
    for (int destino = 1; destino < p; ++destino) {
      MPI_Send(datos.data() + destino * localN, localN, MPI_LONG_LONG, destino,
               TAG_DATOS, MPI_COMM_WORLD);
    }

    for (int i = 0; i < localN; ++i)
      local[i] = datos[i];
  } else {
    MPI_Recv(local.data(), localN, MPI_LONG_LONG, 0, TAG_DATOS, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);
  }

  long long sumaLocal = 0;
  for (long long x : local)
    sumaLocal += x;

  long long sumaPar = 0;

  // Proceso 0 recoge los resultados parciales con TAG_RESULTADO
  if (rank == 0) {
    sumaPar = sumaLocal;

    for (int origen = 1; origen < p; ++origen) {
      long long parcial = 0;
      MPI_Recv(&parcial, 1, MPI_LONG_LONG, origen, TAG_RESULTADO,
               MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      sumaPar += parcial;
    }
  } else {
    MPI_Send(&sumaLocal, 1, MPI_LONG_LONG, 0, TAG_RESULTADO, MPI_COMM_WORLD);
  }

  MPI_Barrier(MPI_COMM_WORLD);
  double finPar = MPI_Wtime();
  double tiempoLocal = finPar - inicioPar;
  double tiempoPar = 0.0;

  MPI_Reduce(&tiempoLocal, &tiempoPar, 1, MPI_DOUBLE, MPI_MAX, 0,
             MPI_COMM_WORLD);

  if (rank == 0) {
    cout << "Suma secuencial: " << sumaSec << '\n';
    cout << "Suma MPI:        " << sumaPar << '\n';
    cout << "Tiempo MPI:      " << tiempoPar << " s\n";
    cout << "Coinciden:       " << (sumaSec == sumaPar ? "SI" : "NO") << '\n';
  }

  MPI_Finalize();
  return 0;
}
