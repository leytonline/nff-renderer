# MSVC/NMAKE build. Run from an x64 Native Tools prompt, or call vcvars64.bat first.

TARGET = main.exe
OBJDIR = obj

CXX = cl
NVCC = nvcc
LINK = link

!IFNDEF CUDA_COMPUTE
CUDA_COMPUTE = 89
!ENDIF

!IFNDEF CUDA_ROOT
CUDA_ROOT = E:\Applications\CUDA
!ENDIF

!IFNDEF DEPS_ROOT
DEPS_ROOT = E:\VSLibs
!ENDIF

!IFNDEF EIGEN_INCLUDE_DIR
EIGEN_INCLUDE_DIR = E:\msys64\ucrt64\include\eigen3
!ENDIF

CUDA_INCLUDE_DIR = $(CUDA_ROOT)\include
CUDA_LIB_DIR = $(CUDA_ROOT)\lib\x64
SDL2_INCLUDE_DIR = $(DEPS_ROOT)\include
SDL2_LIB_DIR = $(DEPS_ROOT)\lib\x64
SDL2_DLL = $(SDL2_LIB_DIR)\SDL2.dll

DEFINES = /D_USE_MATH_DEFINES /DSDL_MAIN_HANDLED /DNFF_ENABLE_CUDA
INCLUDES = /Iinclude /I"$(EIGEN_INCLUDE_DIR)" /I"$(SDL2_INCLUDE_DIR)" /I"$(CUDA_INCLUDE_DIR)"
CXXFLAGS = /nologo /std:c++20 /EHsc /O2 /W4 /openmp $(DEFINES) $(INCLUDES)
NVCCFLAGS = -std=c++20 -Iinclude -I"$(EIGEN_INCLUDE_DIR)" -I"$(SDL2_INCLUDE_DIR)" -I"$(CUDA_INCLUDE_DIR)" -Xcompiler /EHsc -O2 --use_fast_math -gencode arch=compute_$(CUDA_COMPUTE),code=sm_$(CUDA_COMPUTE) -D_USE_MATH_DEFINES -DSDL_MAIN_HANDLED -DNFF_ENABLE_CUDA --expt-relaxed-constexpr
LIBPATHS = /LIBPATH:"$(CUDA_LIB_DIR)" /LIBPATH:"$(SDL2_LIB_DIR)"
LIBS = SDL2.lib cudart.lib

OBJS = $(OBJDIR)\main.obj $(OBJDIR)\Geometry.obj $(OBJDIR)\bvh.obj $(OBJDIR)\Ray.obj $(OBJDIR)\Controller.obj $(OBJDIR)\Nff.obj $(OBJDIR)\NaiveRasterizer.obj $(OBJDIR)\Renderer.obj $(OBJDIR)\ControllerState.obj $(OBJDIR)\Engine.obj $(OBJDIR)\NaiveRaytracer.obj $(OBJDIR)\CudaRaytracer.obj $(OBJDIR)\DebugUtils.obj

all: $(TARGET)

main: $(TARGET)

$(TARGET): $(OBJDIR) $(OBJS)
	$(LINK) /NOLOGO /OUT:$(TARGET) $(OBJS) $(LIBPATHS) $(LIBS)
	if exist "$(SDL2_DLL)" copy /Y "$(SDL2_DLL)" . >nul

$(OBJDIR):
	if not exist "$(OBJDIR)" mkdir "$(OBJDIR)"

$(OBJDIR)\main.obj: main.cpp include\Engine.h include\Controller.h include\CudaRaytracer.h include\NaiveRaytracer.h
	$(CXX) $(CXXFLAGS) /c main.cpp /Fo$(OBJDIR)\main.obj

$(OBJDIR)\Engine.obj: src\Engine.cpp include\Engine.h include\CudaRaytracer.h
	$(CXX) $(CXXFLAGS) /c src\Engine.cpp /Fo$(OBJDIR)\Engine.obj

$(OBJDIR)\Ray.obj: src\Ray.cpp include\Ray.h
	$(CXX) $(CXXFLAGS) /c src\Ray.cpp /Fo$(OBJDIR)\Ray.obj

$(OBJDIR)\ControllerState.obj: src\ControllerState.cpp include\ControllerState.h
	$(CXX) $(CXXFLAGS) /c src\ControllerState.cpp /Fo$(OBJDIR)\ControllerState.obj

$(OBJDIR)\Geometry.obj: src\Geometry.cpp include\Geometry.h include\Ray.h
	$(CXX) $(CXXFLAGS) /c src\Geometry.cpp /Fo$(OBJDIR)\Geometry.obj

$(OBJDIR)\bvh.obj: src\bvh.cpp include\bvh.h include\Ray.h include\Geometry.h
	$(CXX) $(CXXFLAGS) /c src\bvh.cpp /Fo$(OBJDIR)\bvh.obj

$(OBJDIR)\Controller.obj: src\Controller.cpp include\Controller.h include\ControllerState.h
	$(CXX) $(CXXFLAGS) /c src\Controller.cpp /Fo$(OBJDIR)\Controller.obj

$(OBJDIR)\Nff.obj: src\Nff.cpp include\Nff.h include\Geometry.h
	$(CXX) $(CXXFLAGS) /c src\Nff.cpp /Fo$(OBJDIR)\Nff.obj

$(OBJDIR)\NaiveRasterizer.obj: src\NaiveRasterizer.cpp include\NaiveRasterizer.h include\Nff.h include\Geometry.h
	$(CXX) $(CXXFLAGS) /c src\NaiveRasterizer.cpp /Fo$(OBJDIR)\NaiveRasterizer.obj

$(OBJDIR)\NaiveRaytracer.obj: src\NaiveRaytracer.cpp include\NaiveRaytracer.h include\Nff.h include\Geometry.h include\Ray.h include\bvh.h
	$(CXX) $(CXXFLAGS) /c src\NaiveRaytracer.cpp /Fo$(OBJDIR)\NaiveRaytracer.obj

$(OBJDIR)\CudaRaytracer.obj: CUDA\CudaRaytracer.cu include\CudaRaytracer.h include\Renderer.h include\Nff.h include\Geometry.h include\Ray.h include\bvh.h Makefile
	$(NVCC) $(NVCCFLAGS) -c CUDA\CudaRaytracer.cu -o $(OBJDIR)\CudaRaytracer.obj

$(OBJDIR)\Renderer.obj: src\Renderer.cpp include\Renderer.h include\Nff.h
	$(CXX) $(CXXFLAGS) /c src\Renderer.cpp /Fo$(OBJDIR)\Renderer.obj

$(OBJDIR)\DebugUtils.obj: src\DebugUtils.cpp include\DebugUtils.h
	$(CXX) $(CXXFLAGS) /c src\DebugUtils.cpp /Fo$(OBJDIR)\DebugUtils.obj

clean:
	@if exist "$(TARGET)" del /Q "$(TARGET)"
	@if exist "SDL2.dll" del /Q "SDL2.dll"
	@if exist "$(OBJDIR)" for /R "$(OBJDIR)" %F in (*.obj *.o) do @del /Q "%F"
