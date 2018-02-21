RAWtoPNG: RAWtoPNG.o lodepng.o
	g++ -o RAWtoPNG lodepng.o RAWtoPNG.o 

lodepng.o: lodepng.cpp lodepng.h
	g++ -c lodepng.cpp

RAWtoPNG.o: RAWtoPNG.cpp
	g++ -c RAWtoPNG.cpp