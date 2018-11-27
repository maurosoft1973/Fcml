all: deps fbcm install clean

all-debian: debian-deps fcml install clean

.ONESHELL:
deps:
	wget https://github.com/DaveGamble/cJSON/archive/master.zip -O cjson.zip
	unzip cjson.zip
	make -C cJSON-master
	make -C cJSON-master install
	ln -s /usr/local/lib/libcjson.so.1 /usr/lib/libcjson.so.1
	rm cjson.zip
	rm -rf cJSON-master

fcml: src/library/argparse.o src/library/ini.o src/fcml.o
	gcc -o fcml src/library/argparse.o src/library/ini.o src/fcml.o -lcurl -lcjson
	cp fcml package/usr/local/bin/fcml
	cp src/fcml.conf package/etc/fcml.conf

install:
	cp fbcm /usr/local/bin/fbcm
	chmod +x /usr/local/bin/fbcm
	cp fbcm.conf /etc/fbcm.conf

debian-deps:
	apt-get -y install libcurl4-gnutls-dev zip unzip
	wget https://github.com/DaveGamble/cJSON/archive/master.zip -O cjson.zip
	unzip cjson.zip	
	make -C cJSON-master
	make -C cJSON-master install
	ln -s /usr/local/lib/libcjson.so.1 /usr/lib/libcjson.so.1
	rm cjson.zip
	rm -rf cJSON-master	

debian-package:
	dpkg-deb --build package
	mv package.deb fcml.deb

.PHONY: clean
clean:
	rm src/*.o
	rm src/library/*.o
