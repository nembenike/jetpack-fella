make:
	cc main.c -L./libs -I./include -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -o jetpack-fella
