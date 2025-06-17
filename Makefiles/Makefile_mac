# Compiler
CXX = clang++
CXXFLAGS = -Wall -Wextra -std=c++17 -Iinclude -O2
EIGEN = -I/opt/homebrew/Cellar/eigen/3.4.0_1/include/eigen3
SDL2 = -I/opt/homebrew/Cellar/sdl2/2.32.4/include/ -L/opt/homebrew/Cellar/sdl2/2.32.4/lib/ -lSDL2
OBJS = obj/Geometry.o obj/Controller.o obj/Nff.o obj/NaiveRasterizer.o obj/Renderer.o obj/ControllerState.o

main: main.cpp $(OBJS)
	$(CXX) $(CXXFLAGS) $(EIGEN) $(SDL2) $(OBJS) main.cpp -o main

obj/ControllerState.o: src/ControllerState.cpp include/ControllerState.h
	$(CXX) $(CXXFLAGS) -I/opt/homebrew/Cellar/sdl2/2.32.4/include/ -c src/ControllerState.cpp -o obj/ControllerState.o

obj/Geometry.o: src/Geometry.cpp include/Geometry.h
	$(CXX) $(CXXFLAGS) $(EIGEN) -c src/Geometry.cpp -o obj/Geometry.o

obj/Controller.o: src/Controller.cpp include/Controller.h
	$(CXX) $(CXXFLAGS) $(EIGEN) -c -I/opt/homebrew/Cellar/sdl2/2.32.4/include/ src/Controller.cpp -o obj/Controller.o  # unhappy with long sdl2 import here, otherwise causes unused from lib and link

obj/Nff.o: src/Nff.cpp include/Nff.h obj/Geometry.o
	$(CXX) $(CXXFLAGS) $(EIGEN) -c src/Nff.cpp -o obj/Nff.o

obj/NaiveRasterizer.o: src/NaiveRasterizer.cpp include/NaiveRasterizer.h obj/Nff.o obj/Geometry.o
	$(CXX) $(CXXFLAGS) $(EIGEN) -c src/NaiveRasterizer.cpp -o obj/NaiveRasterizer.o

obj/Renderer.o: src/Renderer.cpp include/Renderer.h 
	$(CXX) $(CXXFLAGS) $(EIGEN) -c src/Renderer.cpp -o obj/Renderer.o

clean:
	rm -f obj/*.o
	rm -f main