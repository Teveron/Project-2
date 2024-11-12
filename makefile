vzip: serial.c Logger.c
	gcc -pthread serial.c Logger.c -lz -o vzip

v2: serial2.c Logger.c
	gcc -pthread serial2.c Logger.c -lz -o vzip

test:
	rm -f video.vzip
	./vzip frames
	./check.sh

clean:
	rm -f vzip video.vzip

