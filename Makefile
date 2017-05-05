CC=gcc 
LDFLAGS= -pthread -lircredes -lirctad -lircinterface -lsoundredes -lssl -lcrypt
CFLAGS = -std=c11 -Wall -pedantic
UDPECHO=ueserv
TCPECHO=teserv
IRC=ircserv
SSLECHOC=cliente_echo
SSLECHOS=servidor_echo
SDIR=src
ODIR=obj
DDIR=includes

_DEPS = G-2302-05-P1-udpechoserver.h G-2302-05-P1-tcpechoserver.h G-2302-05-P1-ircserver.h  G-2302-05-P1-main.h G-2302-05-P1-irccommands.h G-2302-05-P1-atiendecliente.h G-2302-05-P1-utilities.h G-2302-05-P3-ssl.h G-2302-05-P3-ssl_server.h tcp_tools.h
DEPS = $(patsubst %,$(DDIR)/%,$(_DEPS))

#UDP echo server 
_UESOURCES = G-2302-05-P1-udpechoserver.c 
UESOURCES = $(patsubst %,$(SDIR)/%,$(_SOURCES))

_UEOBJS = $(patsubst %.c, %.o, $(_UESOURCES))
UEOBJS = $(patsubst %,$(ODIR)/%,$(_UEOBJS))

#TCP echo server
_TESOURCES = G-2302-05-P1-tcpechoserver.c
TESOURCES = $(patsubst %,$(SDIR)/%,$(_SOURCES))

_TEOBJS = $(patsubst %.c, %.o, $(_TESOURCES))
TEOBJS = $(patsubst %,$(ODIR)/%,$(_TEOBJS))

#IRC server
_IRCSOURCES = G-2302-05-P1-main.c G-2302-05-P1-irccommands.c G-2302-05-P1-atiendecliente.c G-2302-05-P1-utilities.c G-2302-05-P1-ircserver.c
IRCSOURCES = $(patsubst %,$(SDIR)/%,$(_SOURCES))

_IRCOBJS = $(patsubst %.c, %.o, $(_IRCSOURCES))
IRCOBJS = $(patsubst %,$(ODIR)/%,$(_IRCOBJS))

#SSL echo client
_SSLECHOCSOURCES = G-2302-05-P3-ssl_echo_client.c G-2302-05-P3-ssl.c tcp_tools.c
SSLECHOCSOURCES = $(patsubst %,$(SDIR)/%,$(_SOURCES))

_SSLECHOCOBJS = $(patsubst %.c, %.o, $(_SSLECHOCSOURCES))
SSLECHOCOBJS = $(patsubst %,$(ODIR)/%,$(_SSLECHOCOBJS))

#SSL
ROOT_SSL=certs/ca.sh
SERVER_SSL=certs/server.sh
CLIENT_SSL=certs/client.sh


OBJS= $(UEOBJS) $(TEOBJS) $(IRCOBJS) $(SSLECHOCOBJS)


all: $(IRC) $(SSLECHOC)
	@echo "#--------------------------"
	@echo "          Redes 2"
	@echo "         Pareja 05"
	@echo "        David Nevado"
	@echo "        Maria Prieto"
	@echo "#--------------------------"

$(UDPECHO): $(UEOBJS)
	$(CC) -o $@ $(UEOBJS) $(CFLAGS) $(LDFLAGS)

$(TCPECHO): $(TEOBJS)
	$(CC) -o $@ $(TEOBJS) $(CFLAGS) $(LDFLAGS)

$(IRC): $(IRCOBJS)
	$(CC) -o $@ $(IRCOBJS) $(CFLAGS) $(LDFLAGS)

$(SSLECHOC): $(SSLECHOCOBJS)
	$(CC) -o $@ $(SSLECHOCOBJS) $(CFLAGS) $(LDFLAGS)

$(OBJS): $(ODIR)/%.o: $(SDIR)/%.c 
	$(CC) -c -o $@ $< $(CFLAGS) 


certificados:
	rm -f certs/*.pem certs/ca/*.pem certs/client/*.pem certs/server/*.pem
	@echo "Generando certificado raíz para la CA..."
	./$(ROOT_SSL)
	@echo "Generando certificado del servidor..."
	./$(SERVER_SSL)
	@echo "Generando certificado del cliente..."
	./$(CLIENT_SSL)

log:
	tail -f /var/log/syslog | grep IRCServ

doc:
	@echo "Generando documentación..."
	doxygen



.PHONY: clean
clean:
	rm -f $(OBJS) $(UDPECHO) $(TCPECHO) $(IRC) $(SSLECHOC)

