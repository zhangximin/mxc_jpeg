CC=$(CROSS_COMPILE)gcc
LINK=$(CROSS_COMPILE)gcc

OBJ = mxc_jpeg.o \
	  g2d.o

INC = -I/usr/src/kernel/include 

LFLAGS =-Wl,-rpath-link,/usr/lib

TARGET = libmxc_jpeg.so

all: $(TARGET)

$(TARGET): $(OBJ)
	$(LINK) -shared -fPIC -lpthread -o $(TARGET) $(OBJ) ${LFLAGS} -lrt -lvpu -lg2d

%.o: %.c
	$(CC) $(INC) -Wall -O2 -fPIC -g -c $^ -o $@

.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJ)

