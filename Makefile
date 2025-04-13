# Compiler
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -Iinclude 
EIGEN = -I/opt/homebrew/Cellar/eigen/3.4.0_1/include/eigen3
SDL2 = -I/opt/homebrew/Cellar/sdl2/2.32.4/include/ -L/opt/homebrew/Cellar/sdl2/2.32.4/lib/  -lSDL2

main: main.cpp obj/Nff.o obj/Geometry.o obj/Rasterizer.o obj/Controller.o
	$(CXX) $(CXXFLAGS) $(EIGEN) $(SDL2) obj/Geometry.o obj/Nff.o obj/Rasterizer.o obj/Controller.o main.cpp -o main

obj/Geometry.o: src/Geometry.cpp include/Geometry.h
	$(CXX) $(CXXFLAGS) $(EIGEN) -c src/Geometry.cpp -o obj/Geometry.o

obj/Controller.o: src/Controller.cpp include/Controller.h
	$(CXX) $(CXXFLAGS) $(EIGEN) -c src/Controller.cpp -o obj/Controller.o

obj/Nff.o: src/Nff.cpp include/Nff.h obj/Geometry.o
	$(CXX) $(CXXFLAGS) $(EIGEN) -c src/Nff.cpp -o obj/Nff.o

obj/Rasterizer.o: src/Rasterizer.cpp include/Rasterizer.h obj/Nff.o obj/Geometry.o
	$(CXX) $(CXXFLAGS) $(EIGEN) -c src/Rasterizer.cpp -o obj/Rasterizer.o


# Clean rule to remove compiled files
clean:
	rm obj/*.o

# Phony targets (so "clean" always runs)
.PHONY: clean
