# Compiler
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++23 -Iinclude -O2 -D_USE_MATH_DEFINES -DSDL_MAIN_HANDLED
EIGEN = -IE:/msys64/ucrt64/include/eigen3
SDL2 = -IE:/msys64/ucrt64/include -LE:/msys64/ucrt64/lib -lSDL2
OBJS = obj/Geometry.o obj/bvh.o obj/Ray.o obj/Controller.o obj/Nff.o obj/NaiveRasterizer.o obj/Renderer.o obj/ControllerState.o obj/Engine.o obj/NaiveRaytracer.o

main: main.cpp $(OBJS)
	$(CXX) $(CXXFLAGS) $(EIGEN) $(OBJS) main.cpp $(SDL2) -o main.exe

obj/Engine.o: src/Engine.cpp include/Engine.h
	$(CXX) $(CXXFLAGS) $(EIGEN) -IE:/msys64/ucrt64/include -c src/Engine.cpp -o obj/Engine.o

obj/Ray.o: src/Ray.cpp include/Ray.h
	$(CXX) $(CXXFLAGS) $(EIGEN) -c src/Ray.cpp -o obj/Ray.o

obj/ControllerState.o: src/ControllerState.cpp include/ControllerState.h
	$(CXX) $(CXXFLAGS) -IE:/msys64/ucrt64/include -c src/ControllerState.cpp -o obj/ControllerState.o

obj/Geometry.o: src/Geometry.cpp include/Geometry.h
	$(CXX) $(CXXFLAGS) $(EIGEN) -c src/Geometry.cpp -o obj/Geometry.o

obj/bvh.o: src/bvh.cpp include/bvh.h obj/Ray.o obj/Geometry.o
	$(CXX) $(CXXFLAGS) $(EIGEN) -c src/bvh.cpp -o obj/bvh.o

obj/Controller.o: src/Controller.cpp include/Controller.h
	$(CXX) $(CXXFLAGS) $(EIGEN) -c -IE:/msys64/ucrt64/include src/Controller.cpp -o obj/Controller.o

obj/Nff.o: src/Nff.cpp include/Nff.h obj/Geometry.o
	$(CXX) $(CXXFLAGS) $(EIGEN) -c src/Nff.cpp -o obj/Nff.o

obj/NaiveRasterizer.o: src/NaiveRasterizer.cpp include/NaiveRasterizer.h obj/Nff.o obj/Geometry.o
	$(CXX) $(CXXFLAGS) $(EIGEN) -c src/NaiveRasterizer.cpp -o obj/NaiveRasterizer.o

obj/NaiveRaytracer.o: src/NaiveRaytracer.cpp include/NaiveRaytracer.h obj/Nff.o obj/Geometry.o obj/Ray.o
	$(CXX) $(CXXFLAGS) $(EIGEN) -c src/NaiveRaytracer.cpp -o obj/NaiveRaytracer.o

obj/Renderer.o: src/Renderer.cpp include/Renderer.h
	$(CXX) $(CXXFLAGS) $(EIGEN) -c src/Renderer.cpp -o obj/Renderer.o

clean:
	rm -f obj/*.o
	rm -f main.exe
