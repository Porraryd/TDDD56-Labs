
#include <stdio.h>

const int N = 1024;

const int blocksize = 128;


__global__
void matrix_add(float *a, float *b, float* c)
{

  int y = (blockIdx.x * blockDim.x + threadIdx.x);
  int x = (blockIdx.y * blockDim.y + threadIdx.y);
  int offset = x+ (y * 1024;
  //printf("x: %d, y: %d \n", x, y);
	c[offset] = a[offset]+b[offset];
}

int main()
{

	const int size = N*N*sizeof(float);

  float* a = new float[N*N];
	float* b = new float[N*N];
	float* result = new float[N*N];
  float theTime = 0;

	float* c;
	float* aa;
	float* bb;

  for (int i = 0; i < N; i++)
		for (int j = 0; j < N; j++)
		{
			a[i+j*N] = 10 + i;
			b[i+j*N] = (float)j / N;
		}

cudaError_t err;

  cudaEvent_t myEvent;
  cudaEvent_t mySecondEvent;
  cudaEventCreate(&myEvent);
  cudaEventSynchronize(myEvent);
  cudaEventCreate(&mySecondEvent);
	cudaMalloc( (void**)&c, size );
	cudaMalloc( (void**)&aa, size );
	cudaMalloc( (void**)&bb, size );

	cudaMemcpy(aa, a, size, cudaMemcpyHostToDevice);
  cudaMemcpy(bb, b, size, cudaMemcpyHostToDevice);

	dim3 dimBlock( 16, 16 );
	dim3 dimGrid( 64, 64 );

  cudaEventRecord(myEvent, 0);
	matrix_add<<<dimGrid, dimBlock>>>(aa,bb,c);
  cudaEventRecord(mySecondEvent, 0);
  cudaEventSynchronize(mySecondEvent);
  cudaEventElapsedTime(&theTime, myEvent, mySecondEvent);
  printf("Elapsed time: %f \n", theTime/1000);
  err = cudaPeekAtLastError();
  if(err) printf("cudaPeekAtLastError %d %s\n", err, cudaGetErrorString(err));


	cudaThreadSynchronize();
	cudaMemcpy( result, c, size, cudaMemcpyDeviceToHost );
	cudaFree( c );
  cudaFree( aa );
  cudaFree( bb );

/*
for (int i = 0; i < N; i++)
{
  for (int j = 0; j < N; j++)
  {
    printf("%0.2f ", result[i+j*N]);
  }
  printf("\n");
}*/
	printf("done\n");
	return EXIT_SUCCESS;
}
