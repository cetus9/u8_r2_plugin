# Work around 'user' r2 installation...
prefix=/home/$$USER/bin/prefix/radare2
exec_prefix=${prefix}
libdir=/home/$$USER/bin/prefix/radare2/lib
includedir=${prefix}/include
CFLAGS=-g -fPIC -I${includedir}/libr
ASM_LDFLAGS=-shared -L${libdir} -lr_asm
ANAL_LDFLAGS=-shared -L${libdir} -lr_anal

# ...or use pkg-config if installed normally
#CFLAGS=-g -fPIC $(shell pkg-config --cflags r_asm)
#ASM_LDFLAGS=-shared $(shell pkg-config --libs r_asm)
#ANAL_LDFLAGS=-shared $(shell pkg-config --libs r_anal)

ASM_OBJS=asm_u8.o u8_disas.o u8_inst.o
ANAL_OBJS=anal_u8.o u8_inst.o

R2_PLUGIN_PATH=$(shell r2 -H R2_USER_PLUGINS)
LIBEXT=$(shell r2 -H LIBEXT)
ASM_LIB=asm_u8.$(LIBEXT)
ANAL_LIB=anal_u8.$(LIBEXT)

all: $(ASM_LIB) $(ANAL_LIB)

clean:
	rm -f $(ASM_LIB) $(ANAL_LIB) $(ASM_OBJS) $(ANAL_OBJS)

$(ASM_LIB): $(ASM_OBJS)
	$(CC) $(CFLAGS) $(ASM_LDFLAGS) $(ASM_OBJS) -o $(ASM_LIB)

$(ANAL_LIB): $(ANAL_OBJS)
	$(CC) $(CFLAGS) $(ANAL_LDFLAGS) $(ANAL_OBJS) -o $(ANAL_LIB)

install:
	cp -f asm_u8.$(LIBEXT) $(R2_PLUGIN_PATH)
	cp -f anal_u8.$(LIBEXT) $(R2_PLUGIN_PATH)

uninstall:
	rm -f $(R2_PLUGIN_PATH)/asm_u8.$(LIBEXT)
	rm -f $(R2_PLUGIN_PATH)/anal_u8.$(LIBEXT)

test:
	r2 -a u8 ../u8dis/rom.bin

backup:
	tar cvf ../u8_r2_plugin.tar .
