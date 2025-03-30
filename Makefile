# Compiler
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -Iinclude -Linclude/ftxui/lib # Include headers from "include/"
EIGEN = -I/opt/homebrew/Cellar/eigen/3.4.0_1/include/eigen3
FTXUI = -lftxui-component -lftxui-dom -lftxui-screen

main: main.cpp obj/Nff.o obj/Geometry.o obj/Rasterizer.o
	$(CXX) $(CXXFLAGS) $(EIGEN) $(FTXUI) obj/Geometry.o obj/Nff.o obj/Rasterizer.o main.cpp -o main

obj/Geometry.o: src/Geometry.cpp include/Geometry.h
	$(CXX) $(CXXFLAGS) $(EIGEN) -c src/Geometry.cpp -o obj/Geometry.o

obj/Nff.o: src/Nff.cpp include/Nff.h obj/Geometry.o
	$(CXX) $(CXXFLAGS) $(EIGEN) -c src/Nff.cpp -o obj/Nff.o

obj/Rasterizer.o: src/Rasterizer.cpp include/Rasterizer.h obj/Nff.o obj/Geometry.o
	$(CXX) $(CXXFLAGS) $(EIGEN) -c src/Rasterizer.cpp -o obj/Rasterizer.o

# Clean rule to remove compiled files
clean:
	rm obj/*.o
	rm output.ppm

# Phony targets (so "clean" always runs)
.PHONY: clean
