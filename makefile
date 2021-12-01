# makefile

CSTD=-std=c11

.PHONY: run build emsbuild clean

run: build
	./program.bin

build: program.bin

emsbuild: www/index.html

clean:
	rm -f ./program.bin
	rm -f ./tmp/stb_image.o
	rm -f ./tmp/stb_image_write.o
	rm -f ./www/index.data
	rm -f ./www/index.html
	rm -f ./www/index.js
	rm -f ./www/index.wasm

tmp:
	mkdir tmp

www:
	mkdir www

tmp/stb_image.o: vendor/stb_image.c vendor/stb_image.h | tmp
	gcc --output $@ ${CSTD} -c -w vendor/stb_image.c

tmp/stb_image_write.o: vendor/stb_image_write.c vendor/stb_image_write.h | tmp
	gcc --output $@ ${CSTD} -c -w vendor/stb_image_write.c

program.bin: main/program.c src/* tmp/stb_image.o tmp/stb_image_write.o
	gcc main/program.c src/*.c tmp/stb_image.o tmp/stb_image_write.o \
		--output $@ \
		${CSTD} -g -I src -I vendor \
		-Wall -Wextra -Wconversion -Wno-unused-function -Wno-unused-parameter \
		-lm -lSDL2 -lSDL2_ttf -lSDL2_mixer

www/index.html: main/program.c src/* vendor/* template.html | www
	emcc main/program.c src/*.c vendor/stb_image.c vendor/stb_image_write.c \
		-o www/index.html -O3 --shell-file template.html \
		-I src -I vendor \
		--preload-file assets \
		-s ALLOW_MEMORY_GROWTH=1 \
		-s USE_SDL=2 -s USE_SDL_TTF=2 -s USE_SDL_MIXER=2

# -s ALLOW_MEMORY_GROWTH=1
# -s ASSERTIONS=1
