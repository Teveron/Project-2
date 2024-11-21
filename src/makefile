<<<<<<< Updated upstream
default: serial1.c Logger.c
	gcc -pthread serial1.c Logger.c -lz -o vzip

v3: parallel.c Logger.c
	gcc -pthread parallel.c Logger.c -lz -o ../vzip
=======
default: serial3.c Logger.c threadpool.c
	gcc -pthread serial3.c Logger.c threadpool.c -lz -o ../vzip

v3: serial3.c Logger.c threadpool.c
	gcc -pthread serial3.c Logger.c threadpool.c -lz -o ../vzip
>>>>>>> Stashed changes

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

