#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <mpi.h>
#define N 256 
#define size 16
#define bian 20
typedef struct ball 
{
	double px, py, vx, vy, ax, ay;
}ball;
ball ball_list[N];
void compute_force(int index) 
{
	ball_list[index].ax = 0;
	ball_list[index].ay = 0;
	for (int i = 0; i < N; i++)	
	{
		if (i != index) 
		{
			double dx = ball_list[i].px - ball_list[index].px;
			double dy = ball_list[i].py - ball_list[index].py;
			double d = (dx * dx + dy * dy);
			if (d == 0)
                continue;
			ball_list[index].ax += 6.67e-3 * dx / sqrt(d) / d;
			ball_list[index].ay += 6.67e-3 * dy / sqrt(d) / d;
		}
	}
}
void compute_velocities(int index) 
{
	ball_list[index].vx += ball_list[index].ax * 0.0001;
	ball_list[index].vy += ball_list[index].ay * 0.0001;
}
void compute_positions(int index) 
{
    compute_velocities(index);
	ball_list[index].px += ball_list[index].vx * 0.0001;
    if(ball_list[index].px < 0 )
        ball_list[index].px = 0;
    if(ball_list[index].px > bian)
        ball_list[index].px = bian;
	ball_list[index].py += ball_list[index].vy * 0.0001;
    if(ball_list[index].py < 0)
        ball_list[index].py = 0;
    if(ball_list[index].py > bian)
        ball_list[index].py = bian;
}
int main(int argc, char* argv[])
{
	int i, j, t;
	for (i = 0; i < N; i++)
		ball_list[i] = {i % size,i / size, 0, 0, 0, 0};//initial
    int rankid, numofprocess;
	double begin, end;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numofprocess);
	MPI_Comm_rank(MPI_COMM_WORLD, &rankid);
	double* mpi_buffer = (double*)malloc(sizeof(double)*6*N*N*100);
	MPI_Buffer_attach(mpi_buffer, sizeof(double)*6*N*N*100);
    begin = MPI_Wtime();
    int mm = N / numofprocess;
    for (t = 0; t < 100; t++) 
    {
		for (j = 0; j < numofprocess; j++)
			if (j != rankid)
				MPI_Bsend((ball_list + mm * rankid), sizeof(ball) * mm, MPI_BYTE, j,0, MPI_COMM_WORLD);
		for (j = 0; j < numofprocess; j++) 
			if (j != rankid) 
            {
				MPI_Status status;
				MPI_Recv((ball_list + mm * j), sizeof(ball) * mm, MPI_BYTE, j,0, MPI_COMM_WORLD, &status);
			}
		for (j = mm * rankid; j < mm * (rankid + 1); j++) 
			compute_force(j);
		MPI_Barrier(MPI_COMM_WORLD);
		for (j = mm * rankid; j < mm * (rankid + 1); j++) 
			compute_positions(j);
		MPI_Barrier(MPI_COMM_WORLD);
	}
	end = MPI_Wtime();
	printf("%.7lf\n", end - begin);
	MPI_Finalize();
}
