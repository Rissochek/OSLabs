build:
	gcc -fPIC -c First_lib.c -o First_lib.o
	gcc -fPIC -c Second_lib.c -o Second_lib.o
	gcc -shared -o First_lib.so First_lib.o
	gcc -shared -o Second_lib.so Second_lib.o
clean:
	rm First_lib.o
	rm First_lib.so
	rm Second_lib.o
	rm Second_lib.so