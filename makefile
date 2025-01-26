#THIS MAKEFILE IS DEPRECATED! USE qbs TO BUILD THE PROJECT INSTEAD!
PROJECT_ROOT:=$(shell dirname "$(realpath $(firstword $(MAKEFILE_LIST)))")
CXXFLAGS = -fPIC -I$(PROJECT_ROOT)/include
CFLAGS = -fPIC -I$(PROJECT_ROOT)/include

#include .o files of every cpp file to be included in this line! that's it!
objects=$(PROJECT_ROOT)/build/PriorityMutex.o
testobjects=$(PROJECT_ROOT)/test/main.o $(PROJECT_ROOT)/test/tst_synchronizedValue.o

all: lib test
	$(PROJECT_ROOT)/test/testmain

lib: $(objects)
	g++ -shared -o $(PROJECT_ROOT)/lib/libDonTools.so $^
	ar rcs $(PROJECT_ROOT)/lib/libDonTools.a $^
$(objects): $(PROJECT_ROOT)/build/%.o: $(PROJECT_ROOT)/src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f build/*.o
	rm -f lib/*
	rm -f test/*.o
	rm -f test/testmain

$(testobjects): $(PROJECT_ROOT)/test/%.o: $(PROJECT_ROOT)/test/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: $(testobjects)
	g++ -lgtest -lgmock -lpthread -o $(PROJECT_ROOT)/test/testmain $^
