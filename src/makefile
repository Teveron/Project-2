default: serial1.c Logger.c
	gcc -pthread serial1.c Logger.c -lz -o vzip

v3: serial3.c Logger.c
	gcc -pthread serial3.c Logger.c -lz -o ../vzip && \
	cd .. && \
	rm vzip.zip && \
	zip -r vzip.zip src -x vzip

v2: serial2.c Logger.c
	gcc -pthread serial2.c Logger.c -lz -o ../vzip && \
	cd .. && \
	rm vzip.zip && \
	zip -r vzip.zip src -x vzip

v1: serial1.c Logger.c
	gcc -pthread serial1.c Logger.c -lz -o ../vzip && \
	cd .. && \
	rm vzip.zip && \
	zip -r vzip.zip src -x vzip

v0: serial0.c Logger.c
	gcc -pthread serial0.c Logger.c -lz -o ../vzip && \
	cd .. && \
	rm vzip.zip && \
	zip -r vzip.zip src -x vzip

test:
	cd .. && \
	rm -f video.vzip && \
	./vzip frames && \
	./check.sh

clean:
	cd .. && \
	rm -f vzip video.vzip vzip.zip vzip

