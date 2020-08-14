#include <stdio.h>
#include <omp.h>
#include <cmath>
#include <time.h>
int main(int argc, char* argv[]) {
	int n = 1000;	
	omp_set_num_threads(1);
	int i, j, num = 0;
    double sum = 0;
	double begin, end;
	begin = omp_get_wtime();
    #pragma omp parallel for reduction(+:sum)
	for (i=2; i<=n; i++) {
        double x = (i - 0.5)/n;
		sum += 4.0/(1.0 + x * x);
	}
	end = omp_get_wtime();
	printf("%.3f\n",sum / n);
	printf("time%.9f\n",end-begin);
	return 0;
}
