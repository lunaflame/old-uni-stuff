lab25: lab25.c main.c
	gcc lab25.c main.c -lpthread -Wall -Wpedantic -o lab25

original_names: lab25.c main.c
	gcc lab25.c main.c -lpthread -Wall -Wpedantic -DPROPER_NAMES -o original_names

debug: lab25.c main.c
	gcc lab25.c main.c -lpthread -Wall -Wpedantic -DDEBUG -fsanitize=address -o debug