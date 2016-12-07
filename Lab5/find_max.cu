// Reduction lab, find maximum

#include <stdio.h>
#include "milli.h"

const int BLOCKSIZE = 1024;
__global__ void find_max(int *indata, int *outdata, int N)
{

  __shared__ int sdata[BLOCKSIZE];
  int i;
  i = threadIdx.x + blockDim.x*blockIdx.x;
  int tid = threadIdx.x;
  sdata[tid] = (i < N ? indata[i] : -1);

  __syncthreads();

  for (unsigned int s = blockDim.x/2; s > 0; s>>=1){
    if (tid < s){
      sdata[tid] = (sdata[tid] < sdata[tid+s] ? sdata[tid+s] : sdata[tid]);
    }
    __syncthreads();
  }

  if (tid==0)
    outdata[blockIdx.x] = sdata[0];

}

void launch_cuda_kernel(int *data, int N)
{
	// Handle your CUDA kernel launches in this function

	int *devdata;
  int *outdata;
	int size = sizeof(int) * N;
	cudaMalloc( (void**)&devdata, size);
	cudaMemcpy(devdata, data, size, cudaMemcpyHostToDevice );
	cudaMalloc( (void**)&outdata, size);
	// Dummy launch
  int currentCount = N;
  while(currentCount > 1){
  	dim3 dimBlock( BLOCKSIZE, 1 );
  	dim3 dimGrid( (currentCount/BLOCKSIZE)+1, 1 );
  	find_max<<<dimGrid, dimBlock>>>(devdata, outdata, currentCount);

  	//cudaMemcpy(data, outdata, sizeof(int)*currentCount/2, cudaMemcpyDeviceToHost );
    /*
    for (int i = 0; i < currentCount/2; i ++){
      printf("%d ,", data[i]);
    }
    printf("\n");*/
    int * tmp = outdata;
    outdata = devdata;
    devdata = tmp;

    cudaError_t err = cudaPeekAtLastError();
  	if (err) printf("cudaPeekAtLastError %d %s\n", err, cudaGetErrorString(err));


    currentCount /= BLOCKSIZE;
  }
	// Only the result needs copying!

	cudaMemcpy(data, devdata, sizeof(int), cudaMemcpyDeviceToHost );
	cudaFree(devdata);
  cudaFree(outdata);
}

// CPU max finder (sequential)
void find_max_cpu(int *data, int N)
{
  int i, m;

	m = data[0];
	for (i=0;i<N;i++) // Loop over data
	{
		if (data[i] > m)
			m = data[i];
	}
	data[0] = m;
}

#define SIZE 50000000
//#define SIZE 16
// Dummy data in comments below for testing
int data[SIZE];// = {1, 2, 5, 3, 6, 8, 5, 3, 1, 65, 8, 5, 3, 34, 2, 54};
int data2[SIZE];// = {1, 2, 5, 3, 6, 8, 5, 3, 1, 65, 8, 5, 3, 34, 2, 54};
//int data[] = {1, 2, 5, 3, 6, 8, 5, 3, 1, 65, 8, 5, 3, 34, 2, 54};
//int data2[] = {1, 2, 5, 3, 6, 8, 5, 3, 1, 65, 8, 5, 3, 34, 2, 54};

int main()
{
  // Generate 2 copies of random data

  srand(time(NULL));
  for (long i=0;i<SIZE;i++)
  {
    data[i] = rand() % (SIZE * 5);
    data2[i] = data[i];
  }

  // The GPU will not easily beat the CPU here!
  // Reduction needs optimizing or it will be slow.
  ResetMilli();
  find_max_cpu(data, SIZE);
  printf("CPU time %f\n", GetSeconds());
  ResetMilli();
  launch_cuda_kernel(data2, SIZE);
  printf("GPU time %f\n", GetSeconds());

  // Print result
  printf("\n");
  printf("CPU found max %d\n", data[0]);
  printf("GPU found max %d\n", data2[0]);
}
