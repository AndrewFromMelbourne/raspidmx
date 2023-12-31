OBJS=test_pattern.o
BIN=test_pattern

CFLAGS+=-Wall -g -O3 -I../common
LDFLAGS+=-L/opt/vc/lib/ -lvchostif -lbcm_host -lm -L../lib -lraspidmx

INCLUDES+=-I/opt/vc/include/ -I/opt/vc/include/interface/vcos/pthreads -I/opt/vc/include/interface/vmcs_host/linux

all: $(BIN)

%.o: %.c
	@rm -f $@ 
	$(CC) $(CFLAGS) $(INCLUDES) -g -c $< -o $@ -Wno-deprecated-declarations

$(BIN): $(OBJS)
	$(CC) -o $@ -Wl,--whole-archive $(OBJS) $(LDFLAGS) -Wl,--no-whole-archive -rdynamic

install: $(BIN)
	install -d $(DESTDIR)/usr/bin
	install -m 0755 $(BIN) $(DESTDIR)/usr/bin

clean:
	@rm -f $(OBJS)
	@rm -f $(BIN)
