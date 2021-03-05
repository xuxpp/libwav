all: wav_test

wav_test: example.c wav.c
	$(CC) -Wall -Wno-multichar -O2 -I.. -o $@ $^

clean:
	rm -f wav_test