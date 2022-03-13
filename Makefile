#------------------------------------------------------------------------------
CXX_SRCS := $(shell find src/ -maxdepth 1 -name "*.cpp")
CU_SRCS := $(shell find src/ -maxdepth 1 -name "*.cu")

CXX_OBJS := ${CXX_SRCS:%.cpp=%.o}
CU_OBJS := ${CU_SRCS:%.cu=%.o}

CXX_OBJS = $(foreach file,$(CXX_SRCS),build/${file:%.cpp=%.o})
CU_OBJS = $(foreach file,$(CU_SRCS),build/${file:%.cu=%.cuo})

DEPS := ${CXX_OBJS:.o=.d} ${CU_OBJS:.cuo=.cud}

$(shell mkdir -p build/src/)

#------------------------------------------------------------------------------
CUDA_DIR := /mnt/WXRC0020/users/shenfalong/shen/thirdparty/cuda/cuda-10.1/
CUDA_ARCH := -gencode arch=compute_75,code=sm_75 -gencode arch=compute_75,code=compute_75 \
			 -gencode arch=compute_70,code=sm_70 -gencode arch=compute_70,code=compute_70 \
			 -gencode arch=compute_61,code=sm_61 -gencode arch=compute_61,code=compute_61 \
			 -gencode arch=compute_60,code=sm_60 -gencode arch=compute_60,code=compute_60 
			 
OPENCV_DIR := /mnt/WXRC0020/users/shenfalong/shen/.local

INCLUDE_DIRS := include ${OPENCV_DIR}/include ${CUDA_DIR}/include \
/mnt/WXRC0020/users/shenfalong/shen/.local/lib/python3.7/site-packages/pybind11/include \
/mnt/WXRC0020/users/shenfalong/shen/.local/include/python3.7/ \
/mnt/WXRC0020/users/shenfalong/shen/.local/lib/python3.7/site-packages/numpy/core/include

NVCCFLAGS := -ccbin=g++ -m64 -Xcompiler -fPIC -Xcompiler  -fopenmp -Xcompiler -fPIC -std=c++11 $(CUDA_ARCH)

LIBRARY_DIRS := ${CUDA_DIR}/lib64 ${OPENCV_DIR}/lib
LIBRARIES := cudart cublas curand opencv_core opencv_highgui opencv_imgproc 

LDFLAGS := -shared -lgomp -lpthread -Wl,--dynamic-linker=/lib64/ld-linux-x86-64.so.2
LDFLAGS += $(foreach dir,$(LIBRARY_DIRS),-L$(dir))
LDFLAGS += $(foreach lib,$(LIBRARIES),-l$(lib))

CXXFLAGS := -MMD -MP -Wno-unused-result -Wsign-compare -DNDEBUG -g -fwrapv -Wall -O2 -std=c++11 -MMD -MP -fPIC -fopenmp 
#CXXFLAGS := -DDEBUG -O0 -g -std=c++11 -MMD -MP -fPIC -fopenmp -Wl,--dynamic-linker=/lib64/ld-linux-x86-64.so.2

COMMON_FLAGS := $(foreach dir, $(INCLUDE_DIRS), -I$(dir))

#-------------------------------------------------------------------------------
.PHONY: all clean

all: _pydb.so
	@ echo successfully compiled

-include $(DEPS)

clean:
	rm -rf build/*
	rm -rf *.so

#--------------------------link-----------------------------
_pydb.so: ${CU_OBJS} ${CXX_OBJS}
	g++ -o $@ ${CU_OBJS} ${CXX_OBJS} ${LDFLAGS} 
	
#--------------------------compile-----------------------------
${CU_OBJS}: build/%.cuo:%.cu
	nvcc $(NVCCFLAGS) -M $< -o ${@:%.cuo=%.cud} $(COMMON_FLAGS)
	nvcc $(NVCCFLAGS) -c $< -o $@ $(COMMON_FLAGS)
	
${CXX_OBJS}: build/%.o:%.cpp
	g++ $(CXXFLAGS) -c $< -o $@ $(COMMON_FLAGS)
	
