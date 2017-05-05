CC = gcc
CFLAGS = -L$(LDIR) -I$(IDIR) -g `pkg-config --cflags gtk+-3.0` 
LDFLAGS = -lpthread -lircredes -lircinterface -lsoundredes -lirctad -lsoundredes -lpulse -lpulse-simple `pkg-config --libs gtk+-3.0` -lssl -lcrypto -rdynamic 
AR = ar 

TAR_FILE= G-2302-05-P3.tar.gz
SDIR = src
SLDIR = srclib
IDIR = includes
LDIR = lib
ODIR = obj
MDIR = man
DDIR = doc
BDIR = .

_LIB = libredes2-G-2302-05-P3.a
LIB = $(patsubst %,$(LDIR)/%,$(_LIB))

_LOBJ =G-2302-05-P1-irccommands.o G-2302-05-P1-atiendecliente.o G-2302-05-P1-utilities.o G-2302-05-P1-ircserver.o G-2302-05-P2-tcp_tools.o G-2302-05-P3-ssl_tools.o G-2302-05-P2-ucommands.o G-2302-05-P2-rcommands.o G-2302-05-P2-udp_tools.o
LOBJ = $(patsubst %,$(ODIR)/%,$(_LOBJ))

_DEPS = 
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ =G-2302-05-P1-irc_server.o  G-2302-05-P3-ssl_echo_server.o G-2302-05-P3-ssl_echo_client.o irc_client.o 
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

_BIN =  G-2302-05-P1-irc_server  G-2302-05-P3-ssl_echo_client G-2302-05-P3-ssl_echo_server irc_client
BIN = $(patsubst %,$(BDIR)/%,$(_BIN))

all: $(BIN) 
	@echo "#--------------------------"
	@echo "          Redes 2"
	@echo "         Pareja 05"
	@echo "        David Nevado"
	@echo "        Maria Prieto"
	@echo "#--------------------------"

$(LIB): $(LOBJ)
	$(AR) rcv $@ $^

$(LOBJ):$(ODIR)/%.o: $(SLDIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(OBJ):$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) 

$(BIN):%: $(ODIR)/%.o $(LIB)
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)

compress: clean doc
	rm -rf $(TAR_FILE)
	rm -rf G-2302-05-P3
	tar -zcvf ../$(TAR_FILE) ../G-2302-05-P3
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
