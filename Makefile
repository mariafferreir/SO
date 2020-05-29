all: msgdist_server.c msgdist_client.c
	gcc msgdist_server.c msgdist_utils.h -o msgser -lpthread
	gcc msgdist_client.c msgdist_utils.h -o msgcli -lncurses
	gcc verificador.c -o verificador

verif: verificador.c
	gcc verificador.c -o verificador

server: msgdist_server.c
	gcc msgdist_server.c -o msgser -lpthread

client: msgdist_client.c
	gcc msgdist_client.c -o msgcli

clear:
	rm msgcli msgser verificador
