CC = g++
OPT = -g -O3
LIBS = -lz -lcvp -lz
FLAGS = -std=c++14 -L. $(LIBS) $(OPT) -no-pie

OBJ = mypredictor.o
DEPS = cvp.h mypredictor.h

all: cvp

cvp: $(OBJ)
	$(CC) $(FLAGS) -o $@ $^

%.o: %.cc $(DEPS)
	$(CC) $(FLAGS) -c -o $@ $<

.PHONY: clean

clean:
	rm -f *.o cvp
