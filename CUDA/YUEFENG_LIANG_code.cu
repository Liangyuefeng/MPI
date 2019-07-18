#include <stdio.h>
#include<math.h>
#include <time.h>
#include <omp.h>
#define N 10000000
//#define N 30000000
//#define N 50000000

__global__ void vecKernel(float*x, float *y, int num) {
	// parallel control via varying index
	int my_i = threadIdx.x + blockIdx.x*blockDim.x;
	// handle my_i exceeds num
 if(my_i<num){
	y[my_i] = 1 / exp((x[my_i] - 2)*(x[my_i] - 2)) 
            + 1 / exp((x[my_i] - 6)*(x[my_i] - 6) / 10) 
            + 1 / (x[my_i] * x[my_i] + 1);
	}	
}

int main()
{
	float *x, *y;
  float max=0;
	int i;
  int index;
  float len;
  len = 200.0/N;
  
  // device timer
  cudaEvent_t start, stop;
  cudaEventCreate(&start); cudaEventCreate(&stop); 
  
  clock_t start_cpu, finish1_cpu,finish2_cpu,finish3_cpu,finish4_cpu,start_serial,stop_serial;
  start_cpu = clock();
  
  //memory for device(GPU)
	float *x_dev, *y_dev;
	//set up array;
	x = (float *)malloc(N * sizeof(*x));
	y = (float *)malloc(N * sizeof(*y));
	//set up device memory
	cudaMalloc(&x_dev, N * sizeof(*x));
	cudaMalloc(&y_dev, N * sizeof(*y));
  
  finish1_cpu = clock();
  // init vars
#pragma omp parallel for 
	      for (i = 0; i < N ; i++)
	      {
		       x[i] = (-100)+len*i; 
	      }
  // copy to device
	cudaMemcpy(x_dev, x, N * sizeof(float), cudaMemcpyHostToDevice);
  // see if any errors
  cudaError err = cudaGetLastError();
  if ( err != cudaSuccess) {
      printf("(1) CUDA RT error: %s \n",cudaGetErrorString(err));
  }
  finish2_cpu = clock();

	// init the kernel
	int threadsPerBlock;
  float maxThreadsPerBlock = 1024;
	threadsPerBlock = (N > maxThreadsPerBlock ? maxThreadsPerBlock : N);
	int blocks = ceil((float)N / (float)threadsPerBlock);                           // ensure cover all elements of x
	printf("eg %d threads/block on %d blocks\n", threadsPerBlock, blocks);

	cudaEventRecord(start,0);
  vecKernel <<<blocks, threadsPerBlock >>> (x_dev,y_dev, N);
  // copy results back  
	cudaMemcpy(y, y_dev, N * sizeof(float), cudaMemcpyDeviceToHost);
  cudaEventRecord(stop);
  finish3_cpu = clock();
  
  // see if any errors 
   err = cudaGetLastError();
   if ( err != cudaSuccess) {
      printf("(2) CUDA RT error: %s \n",cudaGetErrorString(err));
   }
  
  #pragma omp parallel for
	for (i = 0; i < N ; i++)
	{
		if (max < y[i]) {
			max = y[i];
			index = i;
		}
	}
	printf("CUDA: Max is %dth number = %.4f\n",index,max);
	
	/* --- CLEAN UP GPU --- */
  // release device memory
	cudaFree(x_dev);    cudaFree(y_dev);  
  finish4_cpu = clock();  
  
  //serial code
  start_serial = clock();
  for (i=0;i<N;i++)
  {
      x[i] = (-100)+len*i;
  }
  
  for (i=0;i<N;i++)
  { 
      y[i]= 1 / exp((x[i] - 2)*(x[i] - 2)) + 1 / exp((x[i] - 6)*(x[i] - 6) / 10) + 1 / (x[i] * x[i] + 1);
  
  }
  for (i = 0; i < N ; i++)
	{
		if (max < y[i]) {
			max = y[i];
			index = i;
		}
	}
  printf("Serial: Max is %dth number = %.4f\n",index,max);
  stop_serial = clock();
  
  // determine GPU time
  float eTime;
  cudaEventElapsedTime(&eTime, start, stop);
  printf("elapsed GPU time: %f millisecs\n", eTime);
  printf("elapsed CPU time for init+cpy:   %f secs\n", (float)(finish2_cpu-finish1_cpu)/CLOCKS_PER_SEC);
  printf("elapsed CPU time for kernel:   %f secs\n", (float)(finish3_cpu-finish2_cpu)/CLOCKS_PER_SEC);
  printf("elapsed CPU time for finding Max:   %f secs\n", (float)(finish4_cpu-finish3_cpu)/CLOCKS_PER_SEC);
  printf("elapsed CPU time *total*:   %f secs\n", (float)(finish4_cpu-start_cpu)/CLOCKS_PER_SEC);
  printf("Serial Code: %.3f seconds\n", (float)(stop_serial-start_serial)/CLOCKS_PER_SEC);

 
}