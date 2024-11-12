default: serial2.c Logger.c
	gcc -pthread serial2.c Logger.c -lz -o vzip

v2: serial2.c Logger.c
	gcc -pthread serial2.c Logger.c -lz -o ../vzip

v1: serial1.c Logger.c
	gcc -pthread serial1.c Logger.c -lz -o ../vzip

v0: serial0.c Logger.c
	gcc -pthread serial0.c Logger.c -lz -o ../vzip

zip:
	cd .. && \
	zip -r vzip.zip src

test:
	cd .. && \
	rm -f video.vzip && \
	./vzip frames && \
	./check.sh

clean:
	cd .. && \
	rm -f vzip video.vzip vzip.zip

