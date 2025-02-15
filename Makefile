# Unified Makefile for both normal builds and AFL builds

# Normal build uses clang.
CC = clang

# AFL build uses afl-clang-lto.
AFL_CC = afl-clang-lto

# Normal build flags: only address sanitizer.
CFLAGS = -fsanitize=address -Wall -Wextra -g
LDFLAGS = -fsanitize=address

# AFL build flags: both address and fuzzer sanitizers.
FUZZ_CFLAGS = -fsanitize=address,fuzzer -Wall -Wextra -g
FUZZ_LDFLAGS = -fsanitize=address,fuzzer

# Default target: build all executables.
all: main_tau main_tau_readfile fuzz

# ----------------------------------------------------------------------
# Normal Build Targets (using clang)
# ----------------------------------------------------------------------

# main_tau: built from main_tau.c and tau.c compiled with clang.
main_tau: main_tau.o tau.o
	$(CC) $(LDFLAGS) -o main_tau main_tau.o tau.o

# main_tau_readfile: built from main_tau_readfile.c and tau.c compiled with clang.
main_tau_readfile: main_tau_readfile.o tau.o
	$(CC) $(LDFLAGS) -o main_tau_readfile main_tau_readfile.o tau.o

# Pattern rule for normal object files (using clang).
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# ----------------------------------------------------------------------
# AFL Build Targets (using afl-clang-lto)
# ----------------------------------------------------------------------

# fuzz: built from fuzz.c and tau.c compiled with afl-clang-lto.
# Note: Here we compile tau.c with AFL flags to produce tau_afl.o.
fuzz: fuzz.o tau_afl.o
	$(AFL_CC) $(FUZZ_LDFLAGS) -o fuzz fuzz.o tau_afl.o

# Compile fuzz.c using afl-clang-lto.
fuzz.o: fuzz.c
	$(AFL_CC) $(FUZZ_CFLAGS) -c fuzz.c -o fuzz.o

# Compile tau.c using afl-clang-lto.
tau_afl.o: tau.c
	$(AFL_CC) $(FUZZ_CFLAGS) -c tau.c -o tau_afl.o

# ----------------------------------------------------------------------
# AFL-run Target
# ----------------------------------------------------------------------
afl-run: fuzz
	@echo "Setting up AFL input directory..."
	mkdir -p in out
	if [ ! -f in/seed.txt ]; then echo "seed" > in/seed.txt; fi
	@echo "Starting AFL fuzzer..."
	afl-fuzz -i in -o out -- ./fuzz

# ----------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------
clean:
	rm -f *.o main_tau main_tau_readfile fuzz
