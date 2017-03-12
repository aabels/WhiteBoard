all: wb client

wb: wb.c
	gcc -Wall wb.c -o wb -pthread

client: client.c
	gcc -Wall client.c -lcrypto -o client