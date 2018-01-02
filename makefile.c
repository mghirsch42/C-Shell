all: 1c.c
	gcc -o 1c 1c.c
clean: 1c.c 
	rm 1c
	gcc -o 1c 1c.c
run: 1c.c
	gcc -o 1c 1c.c
	./1c
