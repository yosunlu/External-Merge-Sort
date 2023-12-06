CXX = g++
CXXFLAGS = -std=c++11 -Wall

all: gen.exe qs.exe verify.exe sort.exe

gen.exe: gen.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

qs.exe: qs.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

verify.exe: verify.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< -lz

sort.exe: sort.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f gen.exe qs.exe verify.exe sort.exe