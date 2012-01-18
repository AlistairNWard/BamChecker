# Makefile for MEI detector

# Compiler
CC=g++
C=gcc

# Compiler flags
BAMTOOLS_ROOT=/share/home/wardag/programs/bamChecker/bamtools
BAMTOOLS_LIB_DIR=$(BAMTOOLS_ROOT)/lib
CFLAGS=-O3 -Wl,-rpath,$(BAMTOOLS_LIB_DIR)

LIBS = -lz -lm -L./ -L$(BAMTOOLS_ROOT)/lib -lbamtools-utils
INCLUDE = -I$(BAMTOOLS_ROOT)/src -I$(BAMTOOLS_ROOT)/include

all: ../bin/dupdist

.PHONY: clean all

# builds bamtools static lib, and copies into root
$(BAMTOOLS_ROOT)/lib/libbamtools.a:
	cd $(BAMTOOLS_ROOT) && mkdir -p build && cd build && cmake .. && $(MAKE)

OBJECTS=check.o \
        zaTag.o \
	$(BAMTOOLS_ROOT)/lib/libbamtools.a

# Objects

main.o: main.cpp $(BAMTOOLS_ROOT)/lib/libbamtools.a
	$(CC) $(CFLAGS) $(INCLUDE) -c -lbamtools_utils main.cpp

check.o: check.cpp $(BAMTOOLS_ROOT)/lib/libbamtools.a
	$(CC) $(CFLAGS) $(INCLUDE) -c -lbamtools_utils check.cpp

zaTag.o: zaTag.cpp $(BAMTOOLS_ROOT)/lib/libbamtools.a
	$(CC) $(CFLAGS) $(INCLUDE) -c -lbamtools_utils zaTag.cpp

# Executables

bamChecker ../bin/dupdist: main.o $(OBJECTS) $(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDE) main.o $(OBJECTS) -o ../bin/bamChecker $(LIBS)

clean:
	rm -rf *.o bamChecker ../bin/main
	cd $(BAMTOOLS_ROOT)/build && make clean
