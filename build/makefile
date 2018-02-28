#把烧片日期加上

CC = g++
RM = rm -rf
TOPDIR = ../
SRCDIR = $(TOPDIR)src
INCDIR = $(TOPDIR)include
BINDIR = $(TOPDIR)bin
BUIDIR = $(TOPDIR)build

CFLAGS += -c -Wall
CFLAGS += -I$(INCDIR)
CFLAGS += -I$(SRCDIR)

OBJ_O += $(SRCDIR)/macserver.o
OBJ_O += $(SRCDIR)/manager.o
OBJ_O += $(SRCDIR)/thread.o
OBJ_O += $(SRCDIR)/msgdata.o
OBJ_O += $(SRCDIR)/reactor.o
OBJ_O += $(SRCDIR)/parsemsg.o
OBJ_O += $(SRCDIR)/newmac.o

OBJ_LIB += -leznat
OBJ_LIB += -lezprint
OBJ_LIB += -lezsocket
OBJ_LIB += -lInfra
OBJ_LIB += -lpthread -o

MacServer: $(OBJ_O)
	$(CC) $^ -L$(TOPDIR)lib $(OBJ_LIB) $@ -lpthread -L/usr/lib
	mv $(BUIDIR)/MacServer $(BINDIR)

.cpp.o:
	$(CC) $(CFLAGS) $^ -o $@

clean:
	$(RM) $(BUIDIR)/MacServer $(SRCDIR)/*.o










































