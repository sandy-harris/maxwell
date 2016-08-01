test: maxwell
	./maxwell -s -h 64 -f  0 | ent | grep Entropy
	./maxwell -s -h 64 -f 64 | ent | grep Entropy

CFLAGS=-Wall -O2

maxwell : maxwell.c maxwell.h qht.o timermod.o
	gcc $(CFLAGS) -o maxwell maxwell.c qht.o timermod.o

clean:
	rm *.o maxwell test.sam test.qht test.mix test.summary results/*

speed_test: maxwell
	time ./maxwell -s -h 64 -f 0 > /dev/null

full_test: results/max.1.out results/raw.out results/qht.out results/mix.out
	grep Entropy results/*.out > test.summary

results/max.1.out: maxwell results
	./maxwell -1 -s -h 64	| ent > results/max.1.out

results/raw.out: test.sam results
	./test.sam		| ent > results/raw.out

results/usec.out: test.sam results
	./test.sam -g		| ent > results/usec.out

results/qht.out: test.qht results
	./test.qht		| ent > results/qht.out

results/mix.out: test.mix results
	./test.mix		| ent > results/mix.out

results:
	mkdir results

test.sam: test.sam.c maxwell.h timermod.o

test.qht: test.qht.c maxwell.h qht.o

test.mix: test.mix.c maxwell.h
