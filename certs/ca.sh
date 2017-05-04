#Generamos clave privada RSA
openssl genrsa -out certs/ca/cakey.pem 2048
#Creamos un certificado autofirmado
openssl req -new -x509 -key certs/ca/cakey.pem \
-subj '/C=ES/ST=Madrid/L=Madrid/O=UAM/OU=UAM/CN=Redes2 CA' \
-out certs/ca/cacert.pem
#Combinamos certificados
cat certs/ca/cacert.pem certs/ca/cakey.pem > certs/ca.pem
#Comprobamos que los campos coinciden
openssl x509 -subject -issuer -noout -in certs/ca.pem

