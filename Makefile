CXX     = mpic++
CXXFLAGS = -O2 -std=c++17

TARGETS = 01_suma 02_maximo 03_pares 04_producto 05_busqueda \
          06_suma_mat 07_mult_mat 08_primos 09_circular 10_sort

all: $(TARGETS)

01_suma:     01_suma_vector_send_recv.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

02_maximo:   02_maximo_vector.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

03_pares:    03_conteo_pares.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

04_producto: 04_producto_escalar.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

05_busqueda: 05_busqueda_elemento.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

06_suma_mat: 06_suma_matrices.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

07_mult_mat: 07_multiplicacion_matrices.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

08_primos:   08_conteo_primos.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

09_circular: 09_desplazamiento_circular_sendrecv.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

10_sort:     10_merge_sort_distribuido.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

run: all
	@NP=4; \
	for t in $(TARGETS); do \
	    echo "--- $$t (np=$$NP) ---"; \
	    mpirun -np $$NP ./$$t; \
	    echo ""; \
	done

clean:
	rm -f $(TARGETS)

.PHONY: all run clean
