all: wbs379 wbc379

wbs379: wbs379.c wbs379.h
	gcc -Wall wbs379.c -o wbs379 -pthread
	
wbc379: wbc379.c wbc379.h
	gcc -Wall wbc379.c -lcrypto -o wbc379

clean:
	rm *.o -rf