#include <stdio.h>
#include <omp.h>
#include <cmath>
#include <time.h>
int main(int argc, char* argv[]) {
	int n = 500000;	
	omp_set_num_threads(8);
	int i, j, num = 0;
	double begin, end;
	begin = omp_get_wtime();
    #pragma omp parallel for reduction(+:num) private(j)
	for (i=2; i<=n; i++) {
		int s = sqrt(i * 1.0);
		for (j=2; j<=s; j++) 
		{
			if (i % j == 0) 
				break;
		}
		if (j > s) 
			num++;
	}
	end = omp_get_wtime();
	printf("%d\n",num);
	printf("time%.9f\n",end-begin);
	return 0;
}
