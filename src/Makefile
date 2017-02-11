INCS = -I.

NELMA_CAP_OBJS = 	main.o \
			data.o \
			num.o \
			lists.o \
			config.o \
			loadobj.o \
			error.o \
			capacitance.o \
			pngutil.o \
			dump.o \
			object.o \
			sor.o \
			space.o \
			malloc.o \
			block.o

NELMA_DRC_OBJS =	drc.o \
			error.o \
			pngutil.o \
			num.o

DECOMPOSE_OBJS =	pngutil.o \
			error.o \
			decompose.o

CFLAGS?= -Wall -O2 -ffast-math
#CFLAGS= -Wall -O3 -march=athlon-xp -ffast-math
#CFLAGS = -Wall -g -march=athlon-xp -pg
#CFLAGS = -Wall -g -march=athlon-xp -DDEBUG
#CFLAGS = -Wall -g -march=athlon-xp -ffast-math -O2

#LDFLAGS = -pg

LDADD = `pkg-config --libs libpng` `pkg-config --libs libconfuse` -lm
CCADD = `pkg-config --cflags libpng` `pkg-config --cflags libconfuse`

all: nelma-cap decompose nelma-drc

.c.o:
	$(CC) $(CFLAGS) $(CCADD) $(INCS) -c $< -o $@

nelma-cap: $(NELMA_CAP_OBJS)
	$(CC) $(NELMA_CAP_OBJS) -o $@ $(LDFLAGS) $(LDADD)

nelma-drc: $(NELMA_DRC_OBJS)
	$(CC) $(NELMA_DRC_OBJS) -o $@ $(LDFLAGS) $(LDADD)

decompose: $(DECOMPOSE_OBJS)
	$(CC) $(DECOMPOSE_OBJS) -o $@ $(LDFLAGS) $(LDADD)

clean: 
	rm -f $(NELMA_CAP_OBJS)
	rm -f $(NELMA_DRC_OBJS)
	rm -f $(DECOMPOSE_OBJS)
	rm -f $(EMPEM_OBJS)
	rm -f nelma-cap
	rm -f nelma-drc

INSTALL_PROGRAM?= install

install: all
	$(INSTALL_PROGRAM) nelma-cap $(PREFIX)/bin
	$(INSTALL_PROGRAM) nelma-drc $(PREFIX)/bin

.PHONY:	all clean install
