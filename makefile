all: deps fbcm install clean

all-debian: debian-deps fbcm install clean

.ONESHELL:
deps:
	wget https://github.com/DaveGamble/cJSON/archive/master.zip -O cjson.zip
	unzip cjson.zip
	make -C cJSON-master
	make -C cJSON-master install
	ln -s /usr/local/lib/libcjson.so.1 /usr/lib/libcjson.so.1
	rm cjson.zip
	rm -rf cJSON-master

fbcm: source/library/argparse.o source/library/ini.o source/fbcm.o
	gcc -o fbcm source/library/argparse.o source/library/ini.o source/fbcm.o -lcurl -lcjson
	cp fbcm package/usr/local/bin/fbcm
	cp source/fbcm.conf package/etc/fbcm.conf

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
	mv package.deb fbcm.deb

.PHONY: clean
clean:
	rm *.o
	rm library/*.o
