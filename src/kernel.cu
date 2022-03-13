#include <iostream>
#include <cuda_fp16.h>
#include <stdio.h>


void gpu_copy(void *dest, void *src, size_t count) 
{
	cudaMemcpy(dest, src, count, cudaMemcpyHostToDevice);
}	  
