#Generamos clave privada RSA
openssl genrsa -out certs/client/clientkey.pem 2048
#Creamos un certificado autofirmado
openssl req -new -key certs/client/clientkey.pem \
-subj '/C=ES/ST=Madrid/L=Madrid/O=UAM/OU=UAM/CN=G-2302-05-P3-client' \
-out certs/client/clientreq.pem
#La CA firma el certificado
openssl x509 -req -in certs/client/clientreq.pem \
-CA certs/ca.pem \
-out certs/client/clientcert.pem
#Combinamos certificados
cat certs/client/clientcert.pem certs/client/clientkey.pem certs/ca/cacert.pem > certs/cliente.pem
#Comprobamos certificado
openssl x509 -subject -issuer -noout -in certs/cliente.pem


