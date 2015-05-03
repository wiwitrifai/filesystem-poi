all: mount-poi.c rp_poi.o rp_fuse.o
	gcc mount-poi.c rp_poi.o rp_fuse.o -D_FILE_OFFSET_BITS=64 `pkg-config fuse --cflags --libs` -o  mount-poi

rp_poi.o : rp_poi.h rp_poi.c
	gcc -Wall -c rp_poi.c -D_FILE_OFFSET_BITS=64

rp_fuse.o : rp_fuse.h rp_fuse.c
	gcc -Wall -c rp_fuse.c -D_FILE_OFFSET_BITS=64

clean:
	rm *~

clear:
	rm *.o