#Generamos clave privada RSA
openssl genrsa -out certs/server/serverkey.pem 2048
#Creamos un certificado autofirmado
openssl req -new -key certs/server/serverkey.pem \
-subj '/C=ES/ST=Madrid/L=Madrid/O=UAM/OU=UAM/CN=G-2302-05-P3-server' \
-out certs/server/serverreq.pem
#La CA firma el certificado
openssl x509 -req -in certs/server/serverreq.pem \
-CA certs/ca.pem \
-out certs/server/servercert.pem
#Combinamos certificados
cat certs/server/servercert.pem certs/server/serverkey.pem certs/ca/cacert.pem > certs/servidor.pem
#Comprobamos certificado
openssl x509 -subject -issuer -noout -in certs/servidor.pem
