TARGETS=	life \
			mandelbrot \
			pngview \
			rgb_triangle \
			game \
			spriteview \
			test_pattern \
			worms

default :all

all:
	for target in $(TARGETS); do (make -C $$target); done

clean:
	for target in $(TARGETS); do (make -C $$target clean); done

