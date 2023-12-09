
hades_d-decoder-linux-x86: main.c genesis_crc.c genesis_scrambler.c *.h
	cc -g  -o hades_d-decoder-linux-x86 *.c -static -fno-omit-frame-pointer -lm
#        cc -g  -o hades_d-decoder-linux-x86 *.c -fsanitize=address -fno-omit-frame-pointer -lm

clean:
	rm -f hades_d-decoder-linux-x86

