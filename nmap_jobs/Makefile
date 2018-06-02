CXX=gcc
CXXFLAGS=-g -Wall -I..
INCLUDES=
BIN=nmap_jobs

SRCS=$(wildcard *.c)
SRCS += ../ipify.c
SRCS += ../inet_sockets.c

%.o : %.c
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -c -o $@

OBJS   = $(patsubst %.c,%.o,$(SRCS))

$(BIN): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(BIN)

clean:
	rm -f $(OBJS) $(BIN)
