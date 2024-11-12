vzip: src/serial2.c src/Logger.c
	gcc -pthread src/serial2.c src/Logger.c -lz -o vzip

v0: src/serial0.c src/Logger.c
	gcc -pthread src/serial0.c src/Logger.c -lz -o vzip

v1: src/serial1.c src/Logger.c
	gcc -pthread src/serial1.c src/Logger.c -lz -o vzip

v2: src/serial2.c src/Logger.c
	gcc -pthread src/serial2.c src/Logger.c -lz -o vzip

test:
	rm -f video.vzip
	./vzip frames
	./check.sh

clean:
	rm -f vzip video.vzip

