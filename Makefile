# Compiler
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -Iinclude -O2
EIGEN = -I/usr/include/eigen3
SDL2 = -I/usr/include/SDL2 -L/usr/local/lib -l:libSDL2.a 
OBJS = obj/Geometry.o obj/Controller.o obj/Nff.o obj/NaiveRasterizer.o obj/Renderer.o obj/Movement.o

main: main.cpp $(OBJS)
	$(CXX) $(CXXFLAGS) $(EIGEN) $(OBJS) main.cpp $(SDL2) -o main

obj/Movement.o: src/Movement.cpp include/Movement.h
	$(CXX) $(CXXFLAGS) -c -I/usr/include/SDL2 src/Movement.cpp -o obj/Movement.o

obj/Geometry.o: src/Geometry.cpp include/Geometry.h
	$(CXX) $(CXXFLAGS) $(EIGEN) -c src/Geometry.cpp -o obj/Geometry.o

obj/Controller.o: src/Controller.cpp include/Controller.h
	$(CXX) $(CXXFLAGS) $(EIGEN) -c -I/usr/include/SDL2 src/Controller.cpp -o obj/Controller.o  # unhappy with long sdl2 import here, otherwise causes unused from lib and link

obj/Nff.o: src/Nff.cpp include/Nff.h obj/Geometry.o
	$(CXX) $(CXXFLAGS) $(EIGEN) -c src/Nff.cpp -o obj/Nff.o

obj/NaiveRasterizer.o: src/NaiveRasterizer.cpp include/NaiveRasterizer.h obj/Nff.o obj/Geometry.o
	$(CXX) $(CXXFLAGS) $(EIGEN) -c src/NaiveRasterizer.cpp -o obj/NaiveRasterizer.o

obj/Renderer.o: src/Renderer.cpp include/Renderer.h 
	$(CXX) $(CXXFLAGS) $(EIGEN) -c src/Renderer.cpp -o obj/Renderer.o

clean:
	rm -f obj/*.o
	rm -f main