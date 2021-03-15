l0: syntax.l syntax.y ast.h ast.c eval.h eval.c value.h value.c
	bison -d syntax.y
	flex syntax.l
	bison -v --debug syntax.y
	gcc -c -g lex.yy.c
	gcc -c -g value.c
	gcc -c -g ast.c
	gcc -c -g eval.c
	gcc -c -g syntax.tab.c
	gcc -g lex.yy.o value.o ast.o eval.o syntax.tab.o -o l0
