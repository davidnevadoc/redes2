CC = gcc
CFLAGS = -L$(LDIR) -I$(IDIR) -g `pkg-config --cflags gtk+-3.0` 
LDFLAGS = -lpthread -lircredes -lircinterface -lsoundredes -lirctad -lsoundredes -lpulse -lpulse-simple `pkg-config --libs gtk+-3.0` -lssl -lcrypto -rdynamic 
AR = ar 

TAR_FILE= G-2302-05.tar.gz
SDIR = src
SLDIR = srclib
IDIR = includes
LDIR = lib
ODIR = obj
MDIR = man
DDIR = doc
BDIR = .

C_ECHO = cliente_echo
S_ECHO = servidor_echo
C_IRC = cliente_IRC
S_IRC = servidor_IRC
#Certificados SSL
ROOT_SSL=certs/ca.sh
SERVER_SSL=certs/server.sh
CLIENT_SSL=certs/client.sh

_LIB = libredes2-G-2302-05-P3.a
LIB = $(patsubst %,$(LDIR)/%,$(_LIB))

_LOBJ =G-2302-05-P1-irccommands.o G-2302-05-P1-atiendecliente.o G-2302-05-P1-utilities.o G-2302-05-P1-ircserver.o G-2302-05-P2-tcp_tools.o G-2302-05-P3-ssl_tools.o G-2302-05-P2-ucommands.o G-2302-05-P2-rcommands.o G-2302-05-P2-udp_tools.o
LOBJ = $(patsubst %,$(ODIR)/%,$(_LOBJ))

_DEPS = 
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = servidor_IRC.o  servidor_echo.o cliente_echo.o cliente_IRC.o servidor_irc.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

_BIN =  servidor_IRC  cliente_IRC cliente_echo servidor_echo cliente_IRC servidor_irc
BIN = $(patsubst %,$(BDIR)/%,$(_BIN))


all: $(BIN) 
	@echo "#--------------------------"
	@echo "          Redes 2"
	@echo "         Pareja 05"
	@echo "        David Nevado"
	@echo "        Maria Prieto"
	@echo "#--------------------------"
	@mv $(C_ECHO) echo
	@mv $(S_ECHO) echo
	@mv $(C_IRC) cliente_servidor
	@mv $(S_IRC) cliente_servidor

$(LIB): $(LOBJ)
	$(AR) rcv $@ $^

$(LOBJ):$(ODIR)/%.o: $(SLDIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(OBJ):$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) 

$(BIN):%: $(ODIR)/%.o $(LIB)
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)
	



help:
	@echo "-------- OPCIONES DEL MAKEFILE ----------"
	@echo " <make> para crear ejecutables"
	@echo " <make compress> para crear comprimido tar.gz"
	@echo " <make doc> genera documentacion"
	@echo " <make log> muestra syslog"
	@echo " <make certificados> crea certificados SSL"
	@echo " <make clean> elimina los ejecutables"
compress: clean doc
	rm -rf $(TAR_FILE)
	rm -rf G-2302-05-P3
	tar -zcvf ../$(TAR_FILE) ../G-2302-05
	mv ../$(TAR_FILE) $(TAR_FILE)

doc:
	@echo "Generando documentación..."
	doxygen

log: 
	tail -F /var/log/syslog | grep irc

certificados:
	rm -f certs/*.pem certs/ca/*.pem certs/client/*.pem certs/server/*.pem
	@echo "Generando certificado raíz para la CA..."
	./$(ROOT_SSL)
	@echo "Generando certificado del servidor..."
	./$(SERVER_SSL)
	@echo "Generando certificado del cliente..."
	./$(CLIENT_SSL)

.PHONY: clean
clean:
	@rm -frv $(BIN) $(LIB) $(OBJ) $(LOBJ) 
	@mkdir -p obj lib
	@rm -fv $(TAR_FILE)
	@rm -fv core vgcore* 
	@rm -f echo/servidor_echo echo/cliente_echo
	@rm -f cliente_servidor/cliente_IRC cliente_servidor/servidor_IRC
	
