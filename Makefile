CC=gcc
CFLAGS=-std=gnu17 -ggdb -Wall -Werror
INC=-Ideps/cglm/include -Ideps/glad/include -Ideps/glfw/include -Ideps/stb_image
LIBS=-Ldeps -lglfw -lcglm -lm -lglad -lstb_image

SRC=$(wildcard src/*.c)
OBJS=$(addprefix obj/, $(SRC:.c=.o))

# glfw required frameworks on macos
ifeq ($(UNAME_S), Darwin)
	LIBS += -framework OpenGL -framework IOKit -framework CoreVideo -framework Cocoa
endif

.PHONY: dirs deps

all: dirs target

target: $(OBJS)
	$(CC) $(CFLAGS) $(INC) $^ $(LIBS)

obj/src/%.o: src/%.c
	$(CC) $(CFLAGS) $(INC) -c $< -o $@ $(LIBS)

deps:
	cd deps/cglm && cmake . -DCGLM_STATIC=ON && make && mv libcglm.a ..
	cd deps/glad && $(CC) -c src/glad.c -Iinclude && ar rc ../libglad.a glad.o
	cd deps/glfw && cmake . && make && mv src/libglfw3.a ../libglfw.a
	cd deps/stb_image && $(CC) -c stb_image.c && ar rc ../libstb_image.a stb_image.o

dirs:
	mkdir -p obj/src

clean:
	-rm -rf obj/ a.out

