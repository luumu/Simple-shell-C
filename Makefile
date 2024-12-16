# # CT30A3370 Käyttöjärjestelmät ja systeemiohjelmointi ht Makefile

CC = gcc
CFLAGS  = -Wall -g
TARGET = sysht

all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c 

clean:
	$(RM) $(TARGET)
