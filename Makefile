#Naveen Bharadwaj

calc: calc.l calc.y calc.h c-code.c c-code.h data-dep.c data-dep.h cse.c cse.h copy-stmt-elim.c copy-stmt-elim.h
	bison -d calc.y
	flex calc.l
	gcc -O3 -Wall lex.yy.c calc.tab.c c-code.c data-dep.c cse.c copy-stmt-elim.c -o calc


bison-dbg:
	bison -v calc.y


ccode: Output/backend.c Output/backend-opt.c Output/backend-timing.c Output/backend-opt-timing.c
	gcc -O0 -o Output/prog Output/backend.c -lm
	gcc -O0 -o Output/prog-opt Output/backend-opt.c -lm
	gcc -O0 -o Output/prog-time Output/backend-timing.c -lm
	gcc -O0 -o Output/prog-opt-time Output/backend-opt-timing.c -lm

ccodew: Output/backend.c Output/backend-opt.c Output/backend-timing.c Output/backend-opt-timing.c
	gcc -Wall -O0 -o Output/prog Output/backend.c -lm
	gcc -Wall -O0 -o Output/prog-opt Output/backend-opt.c -lm
	gcc -Wall -O0 -o Output/prog-time Output/backend-timing.c -lm
	gcc -Wall -O0 -o Output/prog-opt-time Output/backend-opt-timing.c -lm

clean:
	rm -f calc.tab.* lex.yy.c calc.output calc
	rm -f Output/*.txt Output/*.c Output/prog*
