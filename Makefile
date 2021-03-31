BISON = /opt/homebrew/opt/bison/bin/bison
FLEX = /opt/homebrew/opt/flex/bin/flex

l0: syntax.l syntax.y ast.h ast.c eval.h eval.c value.h value.c main.c
	${BISON} -d syntax.y
	${FLEX} syntax.l
	${BISON} -v --debug syntax.y
	gcc -c -g lex.yy.c
	gcc -c -g value.c
	gcc -c -g ast.c
	gcc -c -g eval.c
	gcc -c -g syntax.tab.c
	gcc -c -g main.c
	gcc -g lex.yy.o value.o ast.o eval.o syntax.tab.o main.o -o l0
