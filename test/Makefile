CXX = g++
CXXFLAGS = -std=c++11 -Wall

all: gen qs verify

gen: gen.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

qs: qs.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

verify: verify.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< -lz

clean:
	rm -f gen qs verify
