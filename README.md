# 10 Algoritmos Secuenciales → Paralelos con MPI

Implementación en C++ de 10 algoritmos que muestran la transformación de versiones secuenciales a versiones paralelas usando MPI (Message Passing Interface).

---

## Requisitos

- **MPI**: OpenMPI o MPICH
- **Compilador**: g++ con soporte C++17

### Instalación en Arch Linux
```bash
sudo pacman -S openmpi
```

### Instalación en Ubuntu/Debian
```bash
sudo apt install libopenmpi-dev openmpi-bin
```

---

## Estructura del proyecto

| Archivo | Algoritmo | MPI utilizado |
|---|---|---|
| `01_suma_vector_send_recv.cpp` | Suma de vector | `MPI_Send` / `MPI_Recv` con tags |
| `02_maximo_vector.cpp` | Máximo de vector | `MPI_Scatter` + `MPI_Reduce(MPI_MAX)` |
| `03_conteo_pares.cpp` | Conteo de pares | `MPI_Scatter` + `MPI_Reduce(MPI_SUM)` |
| `04_producto_escalar.cpp` | Producto escalar | Dos `MPI_Scatter` + reducción |
| `05_busqueda_elemento.cpp` | Búsqueda de elemento | `MPI_Scatter` + `MPI_Reduce(MPI_MIN)` |
| `06_suma_matrices.cpp` | Suma de matrices | `MPI_Scatter` + `MPI_Gather` |
| `07_multiplicacion_matrices.cpp` | Multiplicación de matrices | `MPI_Bcast` + `MPI_Scatter` + `MPI_Gather` |
| `08_conteo_primos.cpp` | Conteo de primos | División de rango + `MPI_Reduce` |
| `09_desplazamiento_circular_sendrecv.cpp` | Desplazamiento circular | `MPI_Sendrecv` en anillo |
| `10_merge_sort_distribuido.cpp` | Ordenamiento distribuido | `MPI_Scatter` + sort local + `MPI_Gather` |

---

## Compilación y ejecución

### Compilar todos
```bash
make
```

### Compilar uno específico
```bash
mpic++ -O2 01_suma_vector_send_recv.cpp -o 01_suma
```

### Ejecutar con 4 procesos
```bash
mpirun -np 4 ./01_suma
```

Cada programa imprime el resultado **secuencial** y el resultado **MPI**, y confirma si coinciden.

---

## Ejemplo de salida
```
=== SUMA DE VECTOR: SECUENCIAL vs MPI ===
Tiempo secuencial: 0.00812 s
Suma secuencial:   200100000000
Suma MPI:          200100000000
Tiempo MPI:        0.00231 s
Coinciden:         SI
```

---

## Informe

El archivo `informe_mpi.tex` contiene el informe completo en LaTeX. Para compilarlo:
```bash
pdflatex informe_mpi.tex
```

---

## Organización del código

Cada archivo sigue la misma estructura:

```
─ SECUENCIAL ──────────────────────
  Proceso 0 ejecuta el algoritmo completo
  y mide el tiempo con MPI_Wtime()

─ PARALELO (MPI) ──────────────────
  Se distribuye el trabajo entre p procesos
  Se comunican resultados parciales
  Proceso 0 verifica que coincidan
```
