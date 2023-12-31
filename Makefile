TARGETS=life \
	mandelbrot \
	offscreen \
	pngview \
	radar_sweep \
	radar_sweep_alpha \
	rgb_triangle \
	game \
	spriteview \
	test_pattern \
	worms
.PHONY: all lib install clean $(TARGETS)

all: $(TARGETS)

$(TARGETS): lib
	$(MAKE) -C $@ $(TARGET)
lib:
	$(MAKE) -C $@ $(TARGET)
