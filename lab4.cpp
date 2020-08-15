#include <mpi.h>
#include <iostream>
#include <stdio.h>
#include <algorithm>
#include <time.h>
using namespace std;

int main(int argc, char* argv[]) {
	int bankid, numofprocess;
    int n = 64;//要排序的数字数目
    int* globArr = (int*)malloc(n * sizeof(int));
    srand(time(0));
    for (int i = 0; i < n; ++i)
		globArr[i] = rand() % (4 * n);//初始化结束
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &bankid);
	MPI_Comm_size(MPI_COMM_WORLD, &numofprocess);
	double begin, end;
    if(bankid == 0)
        begin = MPI_Wtime();//开始计时
	int i_begin = bankid * n / numofprocess, i_end = (bankid + 1) * n / numofprocess;
	int* localArr = (int*)malloc((i_end - i_begin) * sizeof(int));
    for(int i = 0;i < i_end - i_begin;i++)
        localArr[i] = globArr[i + i_begin];
	sort(localArr, localArr + i_end - i_begin);//完成分配和局部排序
	int* sample = (int*)malloc(numofprocess * sizeof(int));
	for (int i = 0; i < numofprocess; ++i)
		sample[i] = localArr[i * (i_end - i_begin) / numofprocess]; 
	int* fullsample = (int*)malloc(numofprocess * numofprocess * sizeof(int));
	MPI_Gather(sample, numofprocess, MPI_INT, fullsample, numofprocess, MPI_INT, 0, MPI_COMM_WORLD);
    free(sample);//汇集好所有的样本准备进入p-1的挑选工作
		int* privot = new int[numofprocess - 1];
		if (bankid == 0) 
        {
			sort(fullsample,fullsample + numofprocess * numofprocess);
            for (int i = 0;i < numofprocess - 1;i++)
                privot[i] = fullsample[(i + 1) * numofprocess];
		}
		free(fullsample);
		MPI_Bcast(privot, numofprocess - 1, MPI_INT, 0, MPI_COMM_WORLD);//传播完了主元

		long long* lowerbound = new long long[numofprocess];
		long long* upperbound = new long long[numofprocess];
		int* class_begin = (int*)malloc(numofprocess * sizeof(int));
		int* class_len = (int*)malloc(numofprocess * sizeof(int));
		lowerbound[0] = localArr[0] - (long long)1; class_begin[0] = 0;
		upperbound[numofprocess - 1] = localArr[i_end - i_begin - 1];
		for (int i = 0; i < numofprocess - 1; ++i)
			lowerbound[i + 1] = upperbound[i] = privot[i];
		for (int k = 0, i = 0; k < numofprocess; ++k) {
			while (i < i_end - i_begin && localArr[i] > lowerbound[k] && localArr[i] <= upperbound[k])
				++i;
			class_len[k] = i - class_begin[k];
			if (k < numofprocess - 1) 
                class_begin[k + 1] = i;
		}
		delete[] lowerbound;
		delete[] upperbound;
		int* ith_class_len = new int[numofprocess];
		for (int i = 0; i < numofprocess; ++i) 
			MPI_Gather(class_len + i, 1, MPI_INT, ith_class_len, 1, MPI_INT, i, MPI_COMM_WORLD);
		int sumlen = 0;
		int* ith_arr_displ = new int[numofprocess];
		ith_arr_displ[0] = 0;
		for (int i = 0; i < numofprocess; ++i) sumlen += ith_class_len[i];
		for (int i = 1; i < numofprocess; ++i) ith_arr_displ[i] = ith_arr_displ[i - 1] + ith_class_len[i - 1];
		int* ith_class_arr = new int[sumlen];
		for (int i = 0; i < numofprocess; ++i) 
			MPI_Gatherv(localArr + class_begin[i], class_len[i], MPI_INT, ith_class_arr, ith_class_len, ith_arr_displ, MPI_INT, i, MPI_COMM_WORLD);
		free(localArr);
		free(class_begin);
        free(class_len);
		int* cluster_cluster_begin_pos = (int*)malloc(numofprocess * sizeof(int));
		for (int i = 0; i < numofprocess; ++i) 
            cluster_cluster_begin_pos[i] = ith_arr_displ[i];
		int* sorted_ith_class = new int[sumlen];
		for (int k = 0; k < sumlen; ++k) 
        {
			int i = 0;
			while (i < numofprocess && cluster_cluster_begin_pos[i] >= ith_arr_displ[i] + ith_class_len[i]) ++i;
			if (i >= numofprocess) 
                break;
			for (int j = i + 1; j < numofprocess; ++j)
				if (cluster_cluster_begin_pos[j] < ith_arr_displ[j] + ith_class_len[j])
					if (ith_class_arr[cluster_cluster_begin_pos[i]] > ith_class_arr[cluster_cluster_begin_pos[j]])
						i = j;
			sorted_ith_class[k] = ith_class_arr[cluster_cluster_begin_pos[i]];
			cluster_cluster_begin_pos[i] += 1;
		}
		free(cluster_cluster_begin_pos);
		delete[] ith_class_arr;
		delete[] ith_class_len;
		delete[] ith_arr_displ;
		int* clusterLen = (int*)malloc(numofprocess*sizeof(int));//每个分部已经完全排好顺序，只需要进一步将他们合并即可
		MPI_Gather(&sumlen, 1, MPI_INT, clusterLen, 1, MPI_INT, 0, MPI_COMM_WORLD);
		int* clusterBeginPos = (int*)malloc(numofprocess*sizeof(int));
		if (bankid == 0) 
        {
			clusterBeginPos[0] = 0;
			for (int i = 1; i < numofprocess; ++i) 
                clusterBeginPos[i] = clusterBeginPos[i - 1] + clusterLen[i - 1];
		}
		MPI_Gatherv(sorted_ith_class, sumlen, MPI_INT, globArr, clusterLen, clusterBeginPos, MPI_INT, 0, MPI_COMM_WORLD);
		delete[] sorted_ith_class;
		free(clusterLen);
        free(clusterBeginPos);	
		if (bankid == 0) 
			for (int i = 0; i < n; ++i) 
                printf("%d%c", globArr[i], ' ');
		free(globArr);
		if(bankid == 0)
		{
            end = MPI_Wtime();
		    cout << (end - begin)<< "seconds\n";
        }
	MPI_Finalize();
}