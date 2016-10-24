OBJS = main.o init_window.o
CC = gcc
CFLAGS = -march=native -O3
INCLUDE_DIRS = -I.
CCC = $(CC) $(CFLAGS) $(INCLUDE_DIRS)

PROGRAM = Program

.PHONY: all
all: $(PROGRAM)

$(PROGRAM): $(OBJS) libmyy.so
	$(CCC) -o $(PROGRAM) $(OBJS) -lEGL -lX11 -L. -lmyy

main.o: main.c
	$(CCC) -c main.c

init_window.o: init_window.c
	$(CCC) -c init_window.c

libmyy.so: myy.c
	$(CCC) --shared -o libmyy.so myy.c

.PHONY: clean
clean:
	$(RM) *.{o,so} $(PROGRAM)
