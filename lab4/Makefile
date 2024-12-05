build: 
	gcc -c first_programm.c -o first.o
	gcc -fsanitize=address first.o ./libs/First_lib.so -o first
	gcc -fsanitize=address second_programm.c -o second
clean:
	rm first
	rm first.o
	rm second