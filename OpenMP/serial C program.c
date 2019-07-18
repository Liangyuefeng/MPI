#include <stdio.h>
#include <math.h>
double x[2000000],y[2000000];
double calculateSD(double data[],double mean);
double mean(double data[]);
double pearson(double x[], double y[], double mean1, double mean2,double sd1, double sd2);

int main()
{
	int i=0;
	double avg1, avg2, sd1, sd2;	
	for (i = 0; i < 2000000; i++)
		x[i] = sin(i);
	for (i = 0; i < 2000000; i++)
		y[i] = sin(i + 5);

	avg1 = mean(x);
	avg2 = mean(y);
	sd1 = calculateSD(x, avg1);
	sd2 = calculateSD(y, avg2);

	printf("\n avga[i] = %.6f", avg1);
	printf("\n avgb[i] = %.6f", avg2);
	printf("\n stan_dev_a[i] = %.6f", sd1);
	printf("\n stan_dev_b[i] = %.6f", sd2);
	printf("\n pears = %.6f", pearson(x, y, avg1, avg2, sd1, sd2));
	return 0;
}

/*function mean to calculate the average values*/
double mean(double data[])
{
	double sum = 0.0, avg;
	int i;
	for (i = 0; i < 2000000; i++) {
		sum = sum + data[i];
	}
	return avg = sum / 2000000;
}

/*function calculateSD to compute the standard deviation*/
double calculateSD(double data[], double mean)
{
	double standardDeviation = 0.0;
	int i;
	for (i = 0; i < 2000000; i++)
		standardDeviation += pow(data[i] - mean, 2);

	return sqrt(standardDeviation/2000000);
}

/*function pearson to compute the Pearson coefficient*/
double pearson(double x[], double y[], double mean1, double mean2, double sd1, double sd2)
{
	int i,n=2000000;
	double a,k,sum = 0.0,xt,yt,sxy,j;
	for (i = 0; i < n; i++) {
		xt = x[i] - mean1;
		yt = y[i] - mean2;
		sxy = xt * yt;
		sum = sum + sxy;
	}
	printf("\n sum = %f", sum);
	a = sum / n;
	printf("  pau = %f", a);
	k = sd1*sd2;
	printf("  pad = %f", k);
	j = a / k;
	return j;


}