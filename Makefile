cc = gcc
ccflag = -Wall -Werror -O0 -g2
target = demoCBC

$(target): main.c libcbc.a
		$(cc) $(ccflag) $^ -o $@ 

# main.o : main.c 
# 		$(cc) $(ccflag) $< -o $@

libcbc.a : libcbc.o utils.o libcbc.h utils.h
		ar rcs $@ libcbc.o utils.o

libcbc.o : libcbc.c libcbc.h utils.h
		$(cc) $(ccflag) -c -o $@ $<

utils.o : utils.c utils.h
		$(cc) $(ccflag) -c -o $@ $<

clean:
		rm $(target) *.o *.a ./cipher_out ./decrypted_text	
