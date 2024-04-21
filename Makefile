CC = gcc
CFLAGS = -pedantic -Wall -std=gnu99 -I/local/courses/csse2310/include
LDFLAGS = -L/local/courses/csse2310/lib -lcsse2310a3
SOURCE = helper.c jobThing.c job.c signals.c parsing.c
PROG = jobthing

all: $(PROG)
$(PROG): $(SOURCE)
	$(CC) $(CFLAGS) $(LDFLAGS) $(SOURCE) -o $(PROG)
clean:
	rm -f *.o jobthing


