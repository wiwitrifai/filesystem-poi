all: mount-poi.c rp_poi.o rp_fuse.o
	gcc mount-poi.cpp rp_poi.o rp_fuse.o `pkg-config fuse --cflags --libs` -o mount-poi
rp_poi.o : rp_poi.h rp_poi.c
	gcc -Wall -c rp_poi.c
fuse_impl.o : rp_fuse.h rp_fuse.c
	gcc -Wall -c rp_fuse.c
clean:
	rm *~
clear:
	rm *.o