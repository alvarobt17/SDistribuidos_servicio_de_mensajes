BIN_FILES  = servidor

CC = gcc
CPPFLAGS = -I$(INSTALL_PATH)/include -include lines.h
CFLAGS = -fPIC -g

LDFLAGS = -L$(INSTALL_PATH)/lib/ -L./
SLDLIBS = -lpthread 

OBJ = lines.o implementacion.o servidor.o

all: $(BIN_FILES)
.PHONY : all

servidor: lines.o implementacion.o servidor.o 
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^ $(SLDLIBS)

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@ $(LDLIBS)

clean:
	rm -f $(BIN_FILES) *.o *.so

.SUFFIXES:
.PHONY : clean