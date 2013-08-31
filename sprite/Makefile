OBJS=main.o ../common/loadpng.o ../common/image.o ../common/key.o ../common/seamlessBackground.o ../common/sprite.o
BIN=sprite

CFLAGS+=-Wall -g -O3 -I../common
LDFLAGS+=-L/opt/vc/lib/ -lbcm_host -lm -L/usr/local/lib -lpng

INCLUDES+=-I/opt/vc/include/ -I/opt/vc/include/interface/vcos/pthreads -I/opt/vc/include/interface/vmcs_host/linux

all: $(BIN)

%.o: %.c
	@rm -f $@ 
	$(CC) $(CFLAGS) $(INCLUDES) -g -c $< -o $@ -Wno-deprecated-declarations

$(BIN): $(OBJS)
	$(CC) -o $@ -Wl,--whole-archive $(OBJS) $(LDFLAGS) -Wl,--no-whole-archive -rdynamic

clean:
	@rm -f $(OBJS)
	@rm -f $(BIN)
