# Make Actions
.PHONY: clean compile build
.PHONY: clean-serial compile-serial build-serial
.PHONY: clean-omp compile-omp build-omp
.DEFAULT_GOAL := build



MAKE_CMD=$(MAKE) --no-print-directory

clean: clean-serial clean-omp
compile: compile-serial compile-omp
build: build-serial build-omp
rebuild: rebuild-serial rebuild-omp



clean-serial:
	@ $(MAKE_CMD) clean -C serial

compile-serial:
	@ $(MAKE_CMD) compile -C serial

build-serial:
	@ $(MAKE_CMD) build -C serial

rebuild-serial:
	@ $(MAKE_CMD) rebuild -C serial




clean-omp:
	@ $(MAKE_CMD) clean -C omp

compile-omp:
	@ $(MAKE_CMD) compile -C omp

build-omp:
	@ $(MAKE_CMD) build -C omp

rebuild-omp:
	@ $(MAKE_CMD) rebuild -C omp
