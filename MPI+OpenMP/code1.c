#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include <omp.h>

#define BODIES 5000
#define TIMESTEPS 100
#define GRAVCONST 0.0000001

// global vars
double mass[BODIES];
double rootvx[BODIES], rootvy[BODIES];
double localvx[BODIES], localvy[BODIES];
double rootx[BODIES], rooty[BODIES];
double dx, dy, d, F, ax, ay;

void testInit();
void testInit2();
void randomInit();
void outputBody(int);//

int main(int argc, char** argv) {
	int time, i, j;
	int mpiID, numMPI,numOMP;
    omp_set_nested(1);

	// init MPI
	MPI_Init(NULL, NULL);
	MPI_Comm_rank(MPI_COMM_WORLD, &mpiID);
	MPI_Comm_size(MPI_COMM_WORLD, &numMPI);

	//check that "N" is divisible by number of processes*threads in use
	numOMP = omp_get_num_threads();  //until we add OpenMP
	if (BODIES< numMPI*numOMP) {
		if (mpiID == 0) printf("too trivial\n");    // we do not cater for idle cores
	}
	else if (BODIES % (numMPI*numOMP) != 0) {
		if (mpiID == 0) printf("too complex\n");    // we only have MPI_Scatter
	}
	else {
		randomInit();

		int numLocal = BODIES / numMPI;
   
    MPI_Bcast(&mass,BODIES,MPI_DOUBLE,0,MPI_COMM_WORLD);
    MPI_Bcast(&rootx,BODIES,MPI_DOUBLE,0,MPI_COMM_WORLD);
    MPI_Bcast(&rooty,BODIES,MPI_DOUBLE,0,MPI_COMM_WORLD);

		for (time = 0; time < TIMESTEPS; time++) {
			printf("Timestep %d\n", time);

			//mpi region
			MPI_Scatter(rootvx, numLocal, MPI_DOUBLE, localvx, numLocal, MPI_DOUBLE, 0, MPI_COMM_WORLD);
			MPI_Scatter(rootvy, numLocal, MPI_DOUBLE, localvy, numLocal, MPI_DOUBLE, 0, MPI_COMM_WORLD);

			//Openmp region
#pragma omp parallel for private(i) shared(numLocal)
			for (i = 0; i < numLocal; i++) {
				// calc forces on body i due to bodies (j != i)
        #pragma omp parallel for private(i) shared(numLocal)
				for (j = 0; j < numLocal; j++) {
        
					if (j != i) {
						dx = rootx[j] - rootx[i];
						dy = rooty[j] - rooty[i];
						d = sqrt(dx*dx + dy * dy);  
						if (d < 0.01) {
							printf("too close - resetting\n");
							d = 1.0;
						}
						F = GRAVCONST * mass[i] * mass[j] / (d*d);  
						ax = (F / mass[i]) * dx / d;  
						ay = (F / mass[i]) * dy / d;  
						localvx[i] += ax;
						localvy[i] += ay;
					}
				} // body j
			} // body i

    MPI_Allgather(&localvx, numLocal, MPI_DOUBLE, rootvx, numLocal, MPI_DOUBLE, 
				MPI_COMM_WORLD);
	MPI_Allgather(&localvy, numLocal, MPI_DOUBLE, rootvy, numLocal, MPI_DOUBLE, 
				MPI_COMM_WORLD);

			// having worked out all velocities we now apply and determine new position
			for (i = 0; i < BODIES; i++) {
				rootx[i] += rootvx[i];
				rooty[i] += rootvy[i];
				//DEBUG ONLY: outputBody(i);
			}

			printf("---\n");
	
		} // time

		if (mpiID == 0) {
			printf("Final data\n");
			for (i = 0; i < BODIES; i++) {
				outputBody(i);
			}
		}

	}

	MPI_Finalize();
}


void randomInit() {
	int i;
	for (i = 0; i < BODIES; i++) {
		mass[i] = 0.001 + (float)rand() / (float)RAND_MAX;            // 0.001 to 1.001

		rootx[i] = -250.0 + 500.0*(float)rand() / (float)RAND_MAX;   //  -10 to +10 per axis
		rooty[i] = -250.0 + 500.0*(float)rand() / (float)RAND_MAX;   //

		rootvx[i] = -0.2 + 0.4*(float)rand() / (float)RAND_MAX;   // -0.25 to +0.25 per axis
		rootvy[i] = -0.2 + 0.4*(float)rand() / (float)RAND_MAX;
	}
	printf("Randomly initialised\n");
	return;
}


void testInit() {
	/* test: initial zero velocity ==> attraction only ie bodies should move closer together */
	int i;
	for (i = 0; i < BODIES; i++) {
		mass[i] = 1.0;
		rootx[i] = (float)i;
		rooty[i] = (float)i;
		rootvx[i] = 0.0;
		rootvy[i] = 0.0;
	}
}

void testInit2() {
	/* test data */
	mass[0] = 1.0;
	rootx[0] = 0.0;
	rooty[0] = 0.0;
	rootvx[0] = 0.01;
	rootvy[0] = 0.0;
	mass[1] = 0.1;
	rootx[1] = 1.;
	rooty[1] = 1.;
	rootvx[1] = 0.;
	rootvy[1] = 0.;
	mass[2] = .001;
	rootx[2] = 0.;;
	rooty[2] = 1.;
	rootvx[2] = .01;
	rootvy[2] = -.01;
}

void outputBody(int i) {
	printf("Body %d: Position=(%f,%f) Velocity=(%f,%f)\n", i, rootx[i], rooty[i], rootvx[i], rootvy[i]);
	return;
}