make:
	cc main.c -L./libs -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -o jetpack-fella