#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>
const int v0 = 0,vmax = 10,p = 5;
typedef struct car
{
	int v,d,pos;
}car;
int main(int argc,char *argv[])
{
	int i = 0,j = 0,num_car=100000;
	car car_list[num_car];
	for(i=0;i < num_car;i++)
	{
		car_list[i].v = v0;
		car_list[i].pos = i;
		car_list[i].d = 1;
	}
    car_list[num_car - 1].d = 999999; 
    int myid, numofprocess;
    clock_t start,end;
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&numofprocess);
    MPI_Comm_rank(MPI_COMM_WORLD,&myid);
	int* mpi_buffer=(int*)malloc(sizeof(int)*1000000);
	MPI_Buffer_attach(mpi_buffer,sizeof(int)*1000000);
	start = MPI_Wtime();
	for(j=0;j<2000;j++)
	{
		if(myid != 0)			
			MPI_Bsend(&(car_list[num_car/numofprocess*myid].pos),1,MPI_INT,myid-1,0,MPI_COMM_WORLD);
		for(i=(num_car/numofprocess*myid);i<num_car/numofprocess*(myid+1)-1;i++)
		{
			car_list[i].d=car_list[i+1].pos-car_list[i].pos;
			if(car_list[i].v < vmax)
                car_list[i].v++;
			if(car_list[i].d<=car_list[i].v)
                car_list[i].v = car_list[i].d;
			srand((unsigned)time(NULL));
            if(car_list[i].v > 1 && rand()%10 < p)
                car_list[i].v--;
			car_list[i].pos += car_list[i].v;
		}
		if(myid!=numofprocess-1)
		{
			int cac;
			MPI_Status status;
			MPI_Recv(&cac,1,MPI_INT,myid+1,0,MPI_COMM_WORLD,&status);
			car_list[i].d = cac - car_list[i].pos;	
		}
        if(car_list[i].v < vmax)
            car_list[i].v++;
		if(car_list[i].d<=car_list[i].v)
            car_list[i].v=car_list[i].d;
        srand((unsigned) time(NULL));
        if( car_list[i].v>1 && rand()%10<p )
            car_list[i].v--;
        car_list[i].pos += car_list[i].v;
        MPI_Barrier(MPI_COMM_WORLD);
    }
    end = MPI_Wtime();
    printf("%.3lf\n",(double)(end - start));
	MPI_Finalize();
}
