medit: 	server.c header.h client.c header-client.h
	gcc server.c header.h -o server -lpthread
	gcc client.c header.h -o client -lpthread	
