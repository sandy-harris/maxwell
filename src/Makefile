test: maxwell
	./maxwell -s -h 64 -f  0 | ent | grep Entropy
	./maxwell -s -h 64 -f 64 | ent | grep Entropy

CFLAGS=-Wall -O2

maxwell : maxwell.c maxwell.h qht.o timermod.o
	gcc $(CFLAGS) -o maxwell maxwell.c qht.o timermod.o

clean:
	rm *.o maxwell test.sam test.qht test.parity test.mix test.summary results/*

speed_test: maxwell
	time ./maxwell -s -h 64 -f 0 > /dev/null

full_test: maxwell test.sam test.qht test.parity test.mix results
	./maxwell -1 -s -h 64	| ent > results/max.1.out
	./test.sam		| ent > results/raw.out
	./test.sam -g		| ent > results/usec.out
	./test.qht		| ent > results/qht.out
	./test.mix		| ent > results/mix.out
	./test.parity		| ent > results/parity.out
	./test.parity -g	| ent > results/par.g.out
	grep Entropy results/*.out > test.summary

results:
	mkdir results

test.sam: test.sam.c maxwell.h timermod.o

test.parity: test.parity.c maxwell.h timermod.o

test.qht: test.qht.c maxwell.h qht.o

test.mix: test.mix.c maxwell.h
