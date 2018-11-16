#include <stdio.h>
#include <omp.h>
#include<math.h>
#include <malloc.h>
int n = 2000000;
int main() {
	int i;
	int tid;
	int chunk;
	double suma = 0.0, meana = 0.0;
	double sumb = 0.0, meanb = 0.0;
	double globala = 0.0, globalb = 0.0;
	double deta = 0.0, detb = 0.0;
	double globalpear = 0.0;
	double pear = 0.0;
	double start, finish;
	double * a = (double *)malloc(2000000 * sizeof(double));
	double * b = (double *)malloc(2000000 * sizeof(double));
	
	//some initializations
  for (i = 0; i < n; i++) {
		a[i] = sin(i);
		b[i] = sin(i + 5);
  }
   tid = omp_get_num_threads();
   chunk = n / tid;
   start = omp_get_wtime();

#pragma omp parallel for default(shared)private(i)schedule(static,chunk)reduction(+:suma)
		for (i = 0; i < n; i++)
			suma += a[i];
   
#pragma omp parallel for default(shared)private(i)schedule(static,chunk)reduction(+:sumb)
		for (i = 0; i < n; i++)
			sumb += b[i];

	//serial compute
  meana = suma / n;
  meanb = sumb / n;

#pragma omp parallel for schedule(static,chunk)reduction(+:globala)
		for (i = 0; i < n; i++)
			globala += (a[i] - meana)*(a[i] - meana);
#pragma omp parallel for default(shared)private(i,meanb)schedule(static,chunk)reduction(+:globalb)
		for (i = 0; i < n; i++)
			globalb += (b[i] - meanb)*(b[i] - meanb);


	//serial compute
	deta = sqrt(globala / n);
	detb = sqrt(globalb / n);
	printf("deta is %f\n", deta);
	printf("detb is %f\n", detb);

#pragma omp parallel for default(shared)private(i,meana,meanb,deta,detb)schedule(static,chunk)reduction(+:globalpear)
		for (i = 0; i < n; i++)
			globalpear += (a[i] - meana)*(b[i] - meanb);

	finish = omp_get_wtime();
	pear = globalpear / (n*deta*detb);
	printf("Pearison is %f\n", pear);
	printf("Time: %f seconds\n", (finish - start));

}