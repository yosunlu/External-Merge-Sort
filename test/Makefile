CXX = g++
CXXFLAGS = -std=c++11 -Wall

all: gen qs verify sort

gen: gen.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

qs: qs.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

verify: verify.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< -lz

sort: sort.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f gen qs verify
