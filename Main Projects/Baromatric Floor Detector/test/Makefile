# Host test of the altitude algorithm (gcc on Linux/macOS/WSL/MSYS2)
sim_test: sim_test.c ../main/altitude.c ../main/altitude.h
	gcc -O2 -Wall -Wextra -I../main sim_test.c ../main/altitude.c -lm -o $@

run: sim_test
	./sim_test 1.5 0.3
	./sim_test 4 1.0

clean:
	rm -f sim_test

.PHONY: run clean
