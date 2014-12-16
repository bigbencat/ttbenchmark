CC = g++

INC = ../base
OPT = -Wall -Wno-deprecated -g

OBJS = ClientConn.o MsgServInfo.o ttbenchmark.o ../base/libbase.a 

OS_NAME = $(shell uname)
LC_OS_NAME = $(shell echo $(OS_NAME) | tr '[A-Z]' '[a-z]')
ifeq ($(LC_OS_NAME), darwin)
LIBS = -lpthread
else 
LIBS = -lpthread -luuid
endif

SERVER = ttbenchmark

all: $(SERVER)

$(SERVER): $(OBJS)
	$(CC)  $(OPT) -o $@ $(OBJS) $(LIBS)

ClientConn.o: ClientConn.cpp
	$(CC) -I $(INC) $(OPT) -c -o $@ $<
	
MsgServInfo.o: MsgServInfo.cpp
	$(CC) -I $(INC) $(OPT) -c -o $@ $<

ttbenchmark.o: ttbenchmark.cpp
	$(CC) -I $(INC) $(OPT) -c -o $@ $<


clean:
	rm -f $(SERVER) *.o
checkos:
	@echo $(LC_OS_NAME)
