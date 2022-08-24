all: pzip

clean:
	rm -vf *.o pzip

pzip: pzip_linkedlist.o pzip_files.o pzip_msgq.o pzip_threads.o main.o
	gcc -g -o pzip pzip_linkedlist.o pzip_files.o pzip_msgq.o pzip_threads.o main.o -lpthread -lrt

main.o: main.c
	gcc -g -c -o main.o main.c

pzip_threads.o: pzip_threads.h pzip_threads.c
	gcc -g -c -o pzip_threads.o pzip_threads.c

pzip_msgq.o: pzip_msgq.h pzip_msgq.c
	gcc -g -c -o pzip_msgq.o pzip_msgq.c

pzip_files.o: pzip_files.h pzip_files.c
	gcc -g -c -o pzip_files.o pzip_files.c

pzip_linkedlist.o: pzip_linkedlist.h pzip_linkedlist.c
	gcc -g -c -o pzip_linkedlist.o pzip_linkedlist.c
