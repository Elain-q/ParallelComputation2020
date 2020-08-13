#include <stdio.h>
#include <math.h>
#include <time.h>
#include <mpi.h>
int issushu(int num) {
	int flag = 1;
	int s = sqrt(double(num));
	for (int i=2; i<=s; i++) {
		if (num%i == 0) {
			flag = 0;
			break;
		}
	}
	return flag;
}



int main(int argc, char* argv[]) {
	int n = 500000;
    int numofprocess,rankid;
	double begin, end;
	MPI_Init(&argc, &argv);//initialize
	MPI_Comm_size(MPI_COMM_WORLD, &numofprocess);//get the number of process and put it into numofprocess
	MPI_Comm_rank(MPI_COMM_WORLD, &rankid);//get the id of the rank
	MPI_Barrier(MPI_COMM_WORLD); 
	begin = MPI_Wtime();// align all processes and get the start time
	int fenpian = n / numofprocess;
	MPI_Status status;
	int cnt = 0,partcnt = 0;
	int i;
	if(rankid == 0)
	{
		for(i = rankid * fenpian;i < (rankid + 1) * fenpian;i++)
                {
			if(issushu(i))
			cnt += 1;
		}
	        for(i = 1;i < numofprocess;i++)
		{
			MPI_Recv(&partcnt,1,MPI_INT,i,0,MPI_COMM_WORLD,&status);
			cnt += partcnt;
		}
	}
	else
	{
		for(i = rankid * fenpian;i < (rankid + 1) * fenpian;i++)
                {
			if(issushu(i))
			cnt += 1;
		}
		MPI_Send(&cnt,1,MPI_INT,0,0,MPI_COMM_WORLD);
	}
	MPI_Barrier(MPI_COMM_WORLD);
	end = MPI_Wtime();
        if(rankid == 0)
	{
		printf("the time is %.5f\n",end - begin);
		printf("the number is %d\n",cnt);
	}
	MPI_Finalize();
	return 0;
}
