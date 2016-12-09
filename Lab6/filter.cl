/*
 * Image filter in OpenCL
 */

#define KERNELSIZE 10
#define BLOCK_SIZE 16
#define USE_SHARED 1

__kernel void filter(__global unsigned char *image, __global unsigned char *out, const unsigned int n, const unsigned int m)
{ 
  __local unsigned char shared[BLOCK_SIZE+KERNELSIZE*2][BLOCK_SIZE+KERNELSIZE*2][3];

  unsigned int i = get_global_id(1) % 512;
  unsigned int j = get_global_id(0) % 512;
  
  #if USE_SHARED == 1
  unsigned int ii = get_local_id(1);
  unsigned int jj = get_local_id(0);
  if (j < n && i < m) {
    shared[ii+KERNELSIZE][jj+KERNELSIZE][0] = image[(i*n+j)*3+0];
    shared[ii+KERNELSIZE][jj+KERNELSIZE][1] = image[(i*n+j)*3+1];
    shared[ii+KERNELSIZE][jj+KERNELSIZE][2] = image[(i*n+j)*3+2];
    if (i >= KERNELSIZE && i < m-KERNELSIZE && j >=KERNELSIZE && j < m-KERNELSIZE){
    if (ii <= KERNELSIZE) {
      shared[KERNELSIZE-ii-1][jj+KERNELSIZE][0] = image[((i-ii-1)*n+j)*3+0];
      shared[KERNELSIZE-ii-1][jj+KERNELSIZE][1] = image[((i-ii-1)*n+j)*3+1];
      shared[KERNELSIZE-ii-1][jj+KERNELSIZE][2] = image[((i-ii-1)*n+j)*3+2];
    }

    if (ii > BLOCK_SIZE-1-KERNELSIZE) {
      shared[ii+KERNELSIZE*2][jj+KERNELSIZE][0] = image[((i+KERNELSIZE)*n+j)*3+0];
      shared[ii+KERNELSIZE*2][jj+KERNELSIZE][1] = image[((i+KERNELSIZE)*n+j)*3+1];
      shared[ii+KERNELSIZE*2][jj+KERNELSIZE][2] = image[((i+KERNELSIZE)*n+j)*3+2];
    }

    if (jj > BLOCK_SIZE-1-KERNELSIZE) {
      shared[ii+KERNELSIZE][jj+KERNELSIZE*2][0] = image[(i*n+j+KERNELSIZE)*3+0];
      shared[ii+KERNELSIZE][jj+KERNELSIZE*2][1] = image[(i*n+j+KERNELSIZE)*3+1];
      shared[ii+KERNELSIZE][jj+KERNELSIZE*2][2] = image[(i*n+j+KERNELSIZE)*3+2];
    }
    if (jj <= KERNELSIZE) {
      shared[ii+KERNELSIZE][KERNELSIZE-jj-1][0] = image[(i*n+j-jj-1)*3+0];
      shared[ii+KERNELSIZE][KERNELSIZE-jj-1][1] = image[(i*n+j-jj-1)*3+1];
      shared[ii+KERNELSIZE][KERNELSIZE-jj-1][2] = image[(i*n+j-jj-1)*3+2];
    }
    if (jj < KERNELSIZE && ii < KERNELSIZE) {
        shared[KERNELSIZE-ii-1][KERNELSIZE-jj-1][0] = image[((i-ii-1)*n+j-jj-1)*3+0];
        shared[KERNELSIZE-ii-1][KERNELSIZE-jj-1][1] = image[((i-ii-1)*n+j-jj-1)*3+1];
        shared[KERNELSIZE-ii-1][KERNELSIZE-jj-1][2] = image[((i-ii-1)*n+j-jj-1)*3+2];
    }
    if (jj < KERNELSIZE && ii > BLOCK_SIZE-1-KERNELSIZE) {
        shared[ii+KERNELSIZE*2][KERNELSIZE-jj-1][0] = image[((i+KERNELSIZE)*n+j-jj-1)*3+0];
        shared[ii+KERNELSIZE*2][KERNELSIZE-jj-1][1] = image[((i+KERNELSIZE)*n+j-jj-1)*3+1];
        shared[ii+KERNELSIZE*2][KERNELSIZE-jj-1][2] = image[((i+KERNELSIZE)*n+j-jj-1)*3+2];
    }
    if (jj > BLOCK_SIZE-1-KERNELSIZE && ii < KERNELSIZE) {
        shared[KERNELSIZE-ii-1][jj+KERNELSIZE*2][0] = image[((i-ii-1)*n+j+KERNELSIZE)*3+0];
        shared[KERNELSIZE-ii-1][jj+KERNELSIZE*2][1] = image[((i-ii-1)*n+j+KERNELSIZE)*3+1];
        shared[KERNELSIZE-ii-1][jj+KERNELSIZE*2][2] = image[((i-ii-1)*n+j+KERNELSIZE)*3+2];
    }
    if (jj > BLOCK_SIZE-1-KERNELSIZE && ii > BLOCK_SIZE-1-KERNELSIZE) {
        shared[ii+KERNELSIZE*2][jj+KERNELSIZE*2][0] = image[((i+KERNELSIZE)*n+j+KERNELSIZE)*3+0];
        shared[ii+KERNELSIZE*2][jj+KERNELSIZE*2][1] = image[((i+KERNELSIZE)*n+j+KERNELSIZE)*3+1];
        shared[ii+KERNELSIZE*2][jj+KERNELSIZE*2][2] = image[((i+KERNELSIZE)*n+j+KERNELSIZE)*3+2];
    }
    }
  }
  
  barrier(CLK_LOCAL_MEM_FENCE);
  #endif


  int k, l;
  unsigned int sumx, sumy, sumz;
  

  int divby = (2*KERNELSIZE+1)*(2*KERNELSIZE+1);
  
  if (j < n && i < m) // If inside image
  {
    if (i >= KERNELSIZE && i < m-KERNELSIZE && j >= KERNELSIZE && j < n-KERNELSIZE)
    {
        sumx=0;sumy=0;sumz=0;
    for(k=-KERNELSIZE;k<=KERNELSIZE;k++)
        for(l=-KERNELSIZE;l<=KERNELSIZE;l++) { 
          #if USE_SHARED == 1
            sumx += shared[ii+k+KERNELSIZE][jj+l+KERNELSIZE][0];
            sumy += shared[ii+k+KERNELSIZE][jj+l+KERNELSIZE][1];
            sumz += shared[ii+k+KERNELSIZE][jj+l+KERNELSIZE][2];
          #else         
           sumx += image[((i+k)*n+(j+l))*3+0];
           sumy += image[((i+k)*n+(j+l))*3+1];
           sumz += image[((i+k)*n+(j+l))*3+2];
          #endif
        }
        out[(i*n+j)*3+0] = sumx/divby;
        out[(i*n+j)*3+1] = sumy/divby;
        out[(i*n+j)*3+2] = sumz/divby;
    }
    else {
      #if USE_SHARED == 1
        out[(i*n+j)*3+0] = shared[ii+KERNELSIZE][jj+KERNELSIZE][0];
        out[(i*n+j)*3+1] = shared[ii+KERNELSIZE][jj+KERNELSIZE][1];
        out[(i*n+j)*3+2] = shared[ii+KERNELSIZE][jj+KERNELSIZE][2];
        #else
        out[(i*n+j)*3+0] = image[(i*n+j)*3+0];
        out[(i*n+j)*3+1] = image[(i*n+j)*3+1];
        out[(i*n+j)*3+2] = image[(i*n+j)*3+2];
      #endif
    }
  }

}

