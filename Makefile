NAME = Sandnix 0.0.2 configuration
ARCH = x64
OUTPUT = fake-serial
OUTDIR = ./bin/$(ARCH)
MIDDIR = ./obj$(ARCH)
PREV = 
ARCHDEF = -DX64
DEP = gcc
DEPFLAGS = $(ARCHDEF)
DEPRULE = $(DEP) $(DEPFLAGS) -MM $< | awk 'NR == 1 {printf("%s%s\n","$(dir $@)",$$0)} NR != 1 {print $$0}' | cat > $@
CC = gcc
CFLAGS = -c $(ARCHDEF) -g
CCRULE = $(CC) $(CFLAGS) -o $@ $<
AS = gcc
ASFLAGS = -c $(ARCHDEF) -g
ASRULE = $(AS) $(ASFLAGS) -o $@ $<
LD = gcc
LDFLAGS = 
LDRULE = $(LD) $(LDFLAGS) -o $@ $^
AR = 
ARRULE = 
AFTER = 
LINKED = $(MIDDIR)/$(NAME).$(ARCH).linked

.PHONY : all clean delete rebuild subtarget target

all : target

	rm -f obj$(ARCH)/src/main.dep
clear : 
	rm -f obj$(ARCH)/src/main.o
	rm -f obj$(ARCH)/src/main.dep
	rm -f $(MIDDIR)/$(NAME).$(ARCH).linked

clean : 
	rm -f obj$(ARCH)/src/main.o
	rm -f obj$(ARCH)/src/main.dep
	rm -f $(MIDDIR)/$(NAME).$(ARCH).linked
	rm -f $(OUTDIR)/$(OUTPUT)

rebuild : 
	make clean
	make all

distclean : 
	rm -f obj$(ARCH)/src/main.o
	rm -f obj$(ARCH)/src/main.dep
	rm -f $(MIDDIR)/$(NAME).$(ARCH).linked
	rm -f $(OUTDIR)/$(OUTPUT)
	rm -f ./Makefile

target :
	mkdir -p $(OUTDIR)
	$(PREV)
	make $(LINKED)
	$(AFTER)

$(LINKED) : obj$(ARCH)/src/main.o
	$(LDRULE)

include obj$(ARCH)/src/main.dep
obj$(ARCH)/src/main.dep : src/main.c
	mkdir -p $(dir $@)
	$(DEPRULE)
obj$(ARCH)/src/main.o : src/main.c
	mkdir -p $(dir $@)
	$(CCRULE)
