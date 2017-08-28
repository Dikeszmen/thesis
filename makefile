CC=gcc
CFLAGS=-g -Wall -lpthread -lwiringPi -Iheader
ALL=main.o counting.o crc.o reading.o Init.o measuring.o
RESULT=/home/herczig/thesis/thesis/app/thesis

$(RESULT): $(ALL)
	$(CC) -o $@ $(ALL) $(CFLAGS)
main.o: main.c 
	$(CC) -c main.c $(CFLAGS) 
%.o: sources/%.c 
	$(CC) -c -o $@ $<  $(CFLAGS)


clean:
	rm *.o 


#$*	Teljes forrasfile neve kiterjesztes nelkul
#$<	out-of-date forrasfile teljes neve(kiterjesztessel)
#$.	forrasfile teljes neve path nelkul
#$&.	forrasfile neve path nelku(kiterjesztes nelkul)
#$:	csak a path
#$@	Teljes aktualis target neve