#clean:
#	rm *.o
CFLAGS = -g -O3 -Wall -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE=1 -D_LARGEFILE_SOURCE=1 

ceckout: novas.c novascon.c solsys3.c readeph0.c checkout-st.c\
          novas.o novascon.o solsys3.o readeph0.o checkout-st.o
	gcc novas.c -c -o novas.o $(CFLAGS)
	gcc novascon.c -c -o novascon.o $(CFLAGS)
	gcc solsys3.c -c -o solsys3.o $(CFLAGS)
	gcc readeph0.c -c -o readeph0.o $(CFLAGS)
	gcc checkout-st.c novas.o novascon.o solsys3.o readeph0.o -o checkout -lm $(CFLAGS)
