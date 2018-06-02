CXX=gcc
CXXFLAGS=-g -Wall
INCLUDES=
BIN=nmap_jobs

SRCS=$(wildcard *.c)

%.o : %.c
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -c -o $@

OBJS   = $(patsubst %.c,%.o,$(SRCS))

$(BIN): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(BIN)

clean:
	rm -f $(OBJS) $(BIN)
