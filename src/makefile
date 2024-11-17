default: parallel.c Logger.c
	gcc -pthread parallel.c Logger.c -lz -o vzip

v3: parallel.c Logger.c
	gcc -pthread parallel.c Logger.c -lz -o ../vzip

v2: serial2.c Logger.c
	gcc -pthread serial2.c Logger.c -lz -o ../vzip

v1: serial1.c Logger.c
	gcc -pthread serial1.c Logger.c -lz -o ../vzip

v0: serial0.c Logger.c
	gcc -pthread serial0.c Logger.c -lz -o ../vzip

test:
	cd .. && \
	rm -f video.vzip && \
	./vzip frames && \
	./check.sh

zip:
	cd .. && \
	rm -f vzip.zip && \
	zip -r vzip.zip src -x vzip

clean:
	cd .. && \
	rm -f vzip video.vzip vzip.zip vzip

