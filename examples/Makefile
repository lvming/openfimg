CC = gcc
CFLAGS += -I../include

OBJS = gles-test.o
LIBS = -lpthread -lGLES_fimg

gles-test: $(OBJS)
	$(CC) -o $@ $< $(LIBS)

.PHONY: clean
clean:
	rm -f *.o gles-test
