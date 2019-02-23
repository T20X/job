INCLUDES = 
LIBS = 
DEBUG = -g

CXX = g++
CXXFLAGS = -O0 -Wall -std=c++17 -DLINUX=1 -m64 -pthread -D_REENTRANT $(INCLUDES)

all: smarkets 

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<

.cpp:
	$(CXX) $(CXXFLAGS) $< -o $@ 

BOOK_SRC = task_orig.cpp
BOOK_OBJ = $(addsuffix .o, $(basename $(BOOK_SRC)))

smarkets: $(BOOK_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(BOOK_OBJ)

clean:
	rm -f $(BOOK_OBJ) smarkets
