#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>  
#include "math.h"
int num = 2000000;
double x_array[2000000], y_array[2000000];
double myX[2000000], myY[2000000];

int main(void) {
	int root = 0;
	int i;
	int myRank, numRanks;
	double sumx = 0.0, sumy = 0.0, globalCountx = 0.0, globalCounty = 0.0;

	MPI_Init(NULL, NULL);
	MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
	MPI_Comm_size(MPI_COMM_WORLD, &numRanks);
	
	

	/* rank 0 create x,y array */
	if (myRank == root) {

		/* populate arrays  */
		for (i = 0; i < num; i++) {
			x_array[i] = sin(i);
			y_array[i] = sin(i + 5);
		}
	}

        /* distribute arrays to each process */
		/* malloc space for local random numbers */
		printf("Distributing (x,y)...");
		MPI_Scatter(&x_array, num / numRanks, MPI_DOUBLE, &myX, num / numRanks, MPI_DOUBLE, 0, MPI_COMM_WORLD);
		MPI_Scatter(&y_array, num / numRanks, MPI_DOUBLE, &myY, num / numRanks, MPI_DOUBLE, 0, MPI_COMM_WORLD);
		printf(" DONE\n");

		/* each process count sum */
		sumx = 0;
		sumy = 0;
		for (i = 0; i < num / numRanks; i++) {
			sumx = sumx + myX[i];
			sumy = sumy + myY[i];
		}
		
		/* return values, summing local counts to find means */
		MPI_Reduce(&sumx, &globalCountx, 1, MPI_DOUBLE, MPI_SUM,
0, MPI_COMM_WORLD);
		MPI_Reduce(&sumy, &globalCounty, 1, MPI_DOUBLE, MPI_SUM,
0, MPI_COMM_WORLD);

		double meana = (double)globalCountx / (double)num;
		double meanb = (double)globalCounty / (double)num;

		/*ensure each core knows about the num*/
    MPI_Bcast(&num, 1, MPI_INT, root, MPI_COMM_WORLD);

	    /*sum (myX[i] - meana)*(myX[i] - meana) and (myY[i] - meanb)*(myY[i] - meanb) on the each process */
		double local_sumx = 0.0;
		double local_sumy = 0.0;
		for (i = 0; i < num / numRanks; i++) {
			local_sumx = local_sumx + (myX[i] - meana)*(myX[i] - meana);
			local_sumy = local_sumy + (myY[i] - meanb)*(myY[i] - meanb);
                }
		double globalsumx = 0;
		double globalsumy = 0;

		/* return values, summing local counts to find globalsum   */
		MPI_Reduce(&local_sumx, &globalsumx, 1, MPI_DOUBLE, 
MPI_SUM, 0, MPI_COMM_WORLD);
		MPI_Reduce(&local_sumy, &globalsumy, 1, MPI_DOUBLE, 
MPI_SUM, 0, MPI_COMM_WORLD);

		/*caculate the pearson_connection*/
		double local_p = 0;
		for (i = 0; i < num / numRanks; i++) {
			local_p = local_p + (myX[i] - meana)*(myY[i] - meanb);
		}
		double globalp = 0;
		MPI_Reduce(&local_p, &globalp, 1, MPI_DOUBLE, MPI_SUM, 0, 
MPI_COMM_WORLD);

		/*output 0 process*/
		if (myRank == root) {
			double stddeva = sqrt(globalsumx / num);
			printf("Meana = %f, Standard deviation a = %f\n", meana, stddeva);

			double stddevb = sqrt(globalsumx / num);
			printf("Meanb = %f, Standard deviation b = %f\n", meanb, stddevb);

			double pear_con = globalp / (stddeva*stddevb*num);
			printf("pearson_conn = %f\n", pear_con);
		}

	MPI_Finalize();
}
