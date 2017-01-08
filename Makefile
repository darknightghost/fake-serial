NAME = fake-serial
ARCH = x86
OUTPUT = fake-serial
OUTDIR = ./bin/$(ARCH)
MIDDIR = ./obj/$(ARCH)
PREV = 
ARCHDEF = -DX86
DEP = gcc
DEPFLAGS = $(ARCHDEF)
DEPRULE = $(DEP) $(DEPFLAGS) -MM $< | awk 'NR == 1 {printf("%s%s\n","$(dir $@)",$$0)} NR != 1 {print $$0}' | cat > $@
CC = gcc
CFLAGS = -Wall $(ARCHDEF) -Werror-implicit-function-declaration -Wunused-parameter -std=gnu99 -Werror -g
CCRULE = $(CC) $(CFLAGS) -c -o $@ $<
AS = gcc
ASFLAGS = $(ARCHDEF) -D_ASM -Wall -Werror-implicit-function-declaration -Wunused-parameter -std=gnu99 -Werror -g
ASRULE = $(AS) -c $(ASFLAGS) -o $@ $<
LD = gcc
LDFLAGS = -lutil -lpthread
LDRULE = $(LD) $(LDFLAGS) -o $@ $^
AR = ar
ARRULE = $(AR) rcs $@ $^
AFTER = cp $(LINKED) $(OUTDIR)/$(OUTPUT)
LINKED = $(MIDDIR)/$(NAME).$(ARCH).linked

.PHONY : all clean delete rebuild subtarget target

all : target

	rm -f obj/$(ARCH)/src/main.dep
	rm -f obj/$(ARCH)/src/pty.dep
	rm -f obj/$(ARCH)/src/unix-socket.dep
clear : 
	rm -f obj/$(ARCH)/src/main.o
	rm -f obj/$(ARCH)/src/main.dep
	rm -f obj/$(ARCH)/src/pty.o
	rm -f obj/$(ARCH)/src/pty.dep
	rm -f obj/$(ARCH)/src/unix-socket.o
	rm -f obj/$(ARCH)/src/unix-socket.dep
	rm -f $(MIDDIR)/$(NAME).$(ARCH).linked

clean : 
	rm -f obj/$(ARCH)/src/main.o
	rm -f obj/$(ARCH)/src/main.dep
	rm -f obj/$(ARCH)/src/pty.o
	rm -f obj/$(ARCH)/src/pty.dep
	rm -f obj/$(ARCH)/src/unix-socket.o
	rm -f obj/$(ARCH)/src/unix-socket.dep
	rm -f $(MIDDIR)/$(NAME).$(ARCH).linked
	rm -f $(OUTDIR)/$(OUTPUT)

rebuild : 
	make clean
	make all

distclean : 
	rm -f obj/$(ARCH)/src/main.o
	rm -f obj/$(ARCH)/src/main.dep
	rm -f obj/$(ARCH)/src/pty.o
	rm -f obj/$(ARCH)/src/pty.dep
	rm -f obj/$(ARCH)/src/unix-socket.o
	rm -f obj/$(ARCH)/src/unix-socket.dep
	rm -f $(MIDDIR)/$(NAME).$(ARCH).linked
	rm -f $(OUTDIR)/$(OUTPUT)
	rm -f ./Makefile

target :
	mkdir -p $(OUTDIR)
	$(PREV)
	make $(LINKED)
	$(AFTER)

$(LINKED) : obj/$(ARCH)/src/main.o \
	obj/$(ARCH)/src/pty.o \
	obj/$(ARCH)/src/unix-socket.o
	$(LDRULE)

include obj/$(ARCH)/src/main.dep
obj/$(ARCH)/src/main.dep : src/main.c
	mkdir -p $(dir $@)
	$(DEPRULE)
obj/$(ARCH)/src/main.o : src/main.c
	mkdir -p $(dir $@)
	$(CCRULE)
include obj/$(ARCH)/src/pty.dep
obj/$(ARCH)/src/pty.dep : src/pty.c
	mkdir -p $(dir $@)
	$(DEPRULE)
obj/$(ARCH)/src/pty.o : src/pty.c
	mkdir -p $(dir $@)
	$(CCRULE)
include obj/$(ARCH)/src/unix-socket.dep
obj/$(ARCH)/src/unix-socket.dep : src/unix-socket.c
	mkdir -p $(dir $@)
	$(DEPRULE)
obj/$(ARCH)/src/unix-socket.o : src/unix-socket.c
	mkdir -p $(dir $@)
	$(CCRULE)
