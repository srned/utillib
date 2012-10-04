# Makefile

uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')
OPTIMIZATION?=-O2
ifeq ($(uname_S),SunOS)
      CFLAGS?= -std=c99 -pedantic $(OPTIMIZATION) -Wall -W -D__EXTENSIONS__ -D_XPG6
        CCLINK?= -ldl -lnsl -lsocket -lm -lpthread
    else
      CFLAGS?= -std=c99 -pedantic $(OPTIMIZATION) -Wall -W $(ARCH) $(PROF)
        CCLINK?= -lm -pthread
    endif
    CCOPT= $(CFLAGS) $(CCLINK) $(ARCH) $(PROF)
    DEBUG?= -g -rdynamic -ggdb 

MAINOBJ = adlist.o ae.o anet.o crc64.o dict.o endianconv.o lzf_c.o lzf_d.o pqsort.o rand.o sds.o sha1.o util.o ziplist.o zipmap.o zmalloc.o main.o

PLUGINOBJ = adlist.o ae.o anet.o crc64.o dict.o endianconv.o lzf_c.o lzf_d.o pqsort.o rand.o sds.o sha1.o util.o ziplist.o zipmap.o zmalloc.o plugin_manager.o plugin_discovery.o plugin_main.o

CCOPT_SO = $(CCFLAGS) -fpic
MALLOC=libc
PLUGIN_DIR=plugins

MAINPRGNAME = main
PLUGINPRGNAME = plugin_main

all: main plugin_main $(PLUGIN_DIR)/alternate.so

include Makefile.dep

main: $(MAINOBJ)
	$(CC) -o $(MAINPRGNAME) $(CCOPT) $(DEBUG) $(MAINOBJ)

plugin_main: $(PLUGINOBJ)
	$(CC) -o $(PLUGINPRGNAME) $(CCOPT) $(PLUGINOBJ) -ldl -Wl,--export-dynamic

$(PLUGIN_DIR)/alternate.o: $(PLUGIN_DIR)/alternate.c 
	$(CC) -c $< -o $@ $(CCOPT_SO) -I.

$(PLUGIN_DIR)/alternate.so: $(PLUGIN_DIR)/alternate.o
	$(CC) -o $@ $^ -shared

.c.o:
	$(CC) -c $(CFLAGS) $(DEBUG) $(COMPILE_TIME) $<

clean:
	rm -rf $(MAINPRGNAME) $(PLUGINPRGNAME) plugins/*.so plugins/*.o *.o *.gcda *.gcno *.gcov

dep:
	$(CC) -MM *.c > Makefile.dep

log:
	git log '--pretty=format:%ad %s' --date=short > Changelog

32bit:
	make ARCH="-arch i386"

gprof:
	make PROF="-pg"

gcov:
	make PROF="-fprofile-arcs -ftest-coverage"

noopt:
	make OPTIMIZATION=""

32bitgprof:
	make PROF="-pg" ARCH="-arch i386"



