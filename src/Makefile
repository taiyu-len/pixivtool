## Files
source = $(wildcard *.c)
object = $(source:.c=.o)
## Libs
CC=`curl-config --cc --cflags`
CFLAGS=`curl-config --libs`


all: $(object)

%.o:%.c %.h

test: $(object)
	$(CC) $(object) -o$@ $(CFLAGS)
