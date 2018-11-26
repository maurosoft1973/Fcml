all: dependencies fbcm install clean

.ONESHELL:
dependencies:
	apt-get -y install libcurl4-gnutls-dev
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

install:
	cp fbcm /usr/local/bin/fbcm
	chmod +x /usr/local/bin/fbcm
	cp fbcm.conf /etc/fbcm.conf

createpackage:
	dpkg-deb --build package
	mv package.deb fbcm.deb

.PHONY: clean
clean:
	rm *.o
	rm library/*.o
