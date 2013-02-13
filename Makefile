CXX  = g++
CXXFLAGS = -O3 -Wall
#CXXFLAGS = -g -pg -Wall
SOURCES = main.cc global.cc rectangle.cc shade.cc optimizer.cc
HEADERS = global.h rectangle.h shade.h optimizer.h
LIBS = -lboost_program_options
OBJECTS = $(SOURCES:.cc=.o)
EXECUTABLE=strawberryfields
DEL_FILE = rm -f

all: $(SOURCES) $(EXECUTABLE)

clean:
	$(DEL_FILE) *.o core.* strawberryfields optimal_covering.txt

$(SOURCES): $(HEADERS)

$(EXECUTABLE): $(OBJECTS)
#				 $(CXX) $(OBJECTS) -o $@ 
				 $(CXX) $(OBJECTS) -o $@ $(LIBS)
#				 $(CXX) $(OBJECTS) -pg -o $@

.cc.o:
	$(CXX) -c $(CXXFLAGS) -o $@ $<
