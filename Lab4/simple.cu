// Simple CUDA example by Ingemar Ragnemalm 2009. Simplest possible?
// Assigns every element in an array with its index.

// nvcc simple.cu -L /usr/local/cuda/lib -lcudart -o simple

#include <stdio.h>

const int N = 16;
const int blocksize = 16;
const float input[16] = { 1.f, 2.f, 4.f, 8.f,
									16.f, 32.f, 64.f, 128.f,
								 256.f, 512.f, 1024.f, 2048.f,
								4096.f, 8192.f, 16284.f, 32568.f };

__global__
void simple(float *c)
{
	c[threadIdx.x] = sqrtf(c[threadIdx.x]);
}

int main()
{
	float *c = new float[N];
	float *cd;
	const int size = N*sizeof(float);
	cudaMalloc( (void**)&cd, size );

	cudaMemcpy(cd, (void*)input, size, cudaMemcpyHostToDevice);
	dim3 dimBlock( blocksize, 1 );
	dim3 dimGrid( 1, 1 );
	simple<<<dimGrid, dimBlock>>>(cd);
	cudaThreadSynchronize();
	cudaMemcpy( c, cd, size, cudaMemcpyDeviceToHost );
	cudaFree( cd );

	for (int i = 0; i < N; i++)
		printf("%f ", c[i]);
	printf("\n");

	for (int i = 0; i < N; i++)
		printf("%f ", sqrtf(input[i]));

	delete[] c;
	printf("done\n");
	return EXIT_SUCCESS;
}
