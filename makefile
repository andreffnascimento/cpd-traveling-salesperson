# Make Actions
.PHONY: clean compile build rebuild
.PHONY: serial-clean serial-compile serial-build serial-rebuild
.PHONY: omp-clean omp-compile omp-build omp-rebuild
.PHONY: mpi-clean mpi-compile mpi-build mpi-rebuild
.DEFAULT_GOAL := build



MAKE_CMD=$(MAKE) --no-print-directory

clean: serial-clean omp-clean mpi-clean
compile: serial-compile omp-compile mpi-compile
build: serial-build omp-build mpi-build
rebuild: serial-rebuild omp-rebuild mpi-rebuild



serial-clean:
	@ $(MAKE_CMD) clean -C serial

serial-compile:
	@ $(MAKE_CMD) compile -C serial

serial-build:
	@ $(MAKE_CMD) build -C serial

serial-rebuild:
	@ $(MAKE_CMD) rebuild -C serial



omp-clean:
	@ $(MAKE_CMD) clean -C omp

omp-compile:
	@ $(MAKE_CMD) compile -C omp

omp-build:
	@ $(MAKE_CMD) build -C omp

omp-rebuild:
	@ $(MAKE_CMD) rebuild -C omp



mpi-clean:
	@ $(MAKE_CMD) clean -C mpi

mpi-compile:
	@ $(MAKE_CMD) compile -C mpi

mpi-build:
	@ $(MAKE_CMD) build -C mpi

mpi-rebuild:
	@ $(MAKE_CMD) rebuild -C mpi
