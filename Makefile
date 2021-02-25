cc = gcc
ccflag = -Wall -Werror -O0 -g2

all: demoBC
		$(cc) $(ccflag) -o demoBC demoBC.c

clean:
		rm demoBC ./cipher_out ./decrypted_text
	