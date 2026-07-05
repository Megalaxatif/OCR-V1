all:
	gcc src/*.c -lSDL2_image -lSDL2_ttf -lSDL2_gfx -lSDL2 -lm -fsanitize=address -g
