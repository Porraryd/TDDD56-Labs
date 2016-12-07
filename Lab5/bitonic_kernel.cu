
// This is not really C++-code but pretty plain C code, but we compile it
// as C++ so we can integrate with CUDA seamlessly.

// If you plan on submitting your solution for the Parallel Sorting Contest,
// please keep the split into main file and kernel file, so we can easily
// insert other data.
const int BLOCKSIZE = 1024;

__device__
static void exchange(int *i, int *j)
{
	int k;
	k = *i;
	*i = *j;
	*j = k;
}

__global__ void bitonic_sort(int *data, int N, int j, int k) {
		int i =  threadIdx.x + blockDim.x*blockIdx.x;
		if (i < N){

			int ixj=i^j; // Calculate indexing!
			if ((ixj)>i)
			{
				if ((i&k)==0 && data[i]>data[ixj]) exchange(&data[i],&data[ixj]);
				if ((i&k)!=0 && data[i]<data[ixj]) exchange(&data[i],&data[ixj]);
			}
		}
}



// No, this is not GPU code yet but just a copy of the CPU code, but this
// is where I want to see your GPU code!
void bitonic_gpu(int *data, int N)
{
  int i,j,k;
	int size = sizeof(int) * N;
	int *gpudata;
	cudaMalloc( (void**)&gpudata, size);
	cudaMemcpy(gpudata, data, size, cudaMemcpyHostToDevice );
	dim3 dimBlock( BLOCKSIZE, 1 );
	dim3 dimGrid( N/BLOCKSIZE,1 );


  for (k=2;k<=N;k=2*k) // Outer loop, double size for each step
  {
    for (j=k>>1;j>0;j=j>>1) // Inner loop, half size for each step
    {
			bitonic_sort<<<dimGrid, dimBlock>>>(gpudata, N, j, k);
			cudaThreadSynchronize();
    }
  }

	cudaMemcpy(data, gpudata, size, cudaMemcpyDeviceToHost );
}
