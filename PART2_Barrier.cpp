//#define _GNU_SOURCE
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <sched.h>
#include <time.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <string> 
#include <cstring>
#include <math.h> 
#include <iostream>

#define N 8
#define NUMBER_OF_CORE 4
using namespace std;
/*****************************************************
Compiler: g++ -pthread PART2_Barrier.cpp -o PART2_Barrier.out
Execute	: ./PART2_Barrier.out
******************************************************/
//Function Define
int determinant(int A[N][N], int n);
void *inverse(void *args);
void printSchedulerName(int PID);

template<class T>
void display(T A[N][N]);
bool Compare_Result(float single[N][N], float thread[N][N]);

//Global Variable
int MatrixA[N][N]={0};
int adj[N][N]={};
float inv[N][N]={0};
int global_det=0;
int cpu_alloc[NUMBER_OF_CORE]={0};

	
//Information struct
struct thread_data
{
	int start;
	int end;
	int det;
	int id;	
};



int main(int argc, char** argv)
{	
	srand(time(NULL));
	struct timeval start;
    struct timeval end;
	double Time_Use;
	float single[N][N],thread[N][N];

	
/********************************** Matrix Initialize ****************************************************************
******************************************************************************************************************/
	//Generating an initial random value
	for(int j=0;j<N;j++)
	{	
		for(int i=0;i<N;i++)
		{
			MatrixA[i][j] = 1+rand()%20;//i*j;
		}
	}
	
	//Print matrix value
	printf("##############################################################################\n");
	printf("Matrix A:\n");
	for (int i=0; i<N; i++)
	{
	  for (int j=0; j<N; j++) 
	    printf("%d \t", MatrixA[i][j]);
	  printf("\n"); 
	}
/********************************** Find Matrix determinant ****************************************************************
******************************************************************************************************************/
	// Find determinant of MatrixA[][]
    int det = determinant(MatrixA, N);
   	if (det == 0)
   	{
       	printf("Singular matrix, can't find its inverse\n");
        return 0;
    }
    global_det=det;

/********************************** Single Thread Matrix Inverse **********************************************************
******************************************************************************************************************/	
	printf("##############################################################################\n");
    // Find Inverse    	
    gettimeofday(&start, NULL);
  	inverse(NULL);
 	gettimeofday(&end, NULL);

	//print using time
	Time_Use = (end.tv_sec - start.tv_sec) + (double)(end.tv_usec - start.tv_usec)/1000000.0;
	cout << "Single Thread Spend time : " << Time_Use << endl;

	//print inverse matrix
	printf("Inverse Matrix A:\n");
	display(inv);

	//Saving result in <single>
 	memcpy(single,inv,N*N*sizeof(float));
	
/********************************** Clean Inverse Matrix *****************************************************
**********************************************************************************************************************/	
	printf("##############################################################################\n");	
	printf("Inverse Matrix clear:\n");
	memset(inv,0,N*N*sizeof(float));
	memset(adj,0,N*N*sizeof(int));
/********************************** Multi Thread Matrix Inverse *****************************************************
**********************************************************************************************************************/	
	printf("##############################################################################\n");
	//pthread initial 
	pthread_t thread1[NUMBER_OF_CORE];
	thread_data *t=new thread_data[NUMBER_OF_CORE];
	//int Last_Core_Index = NUMBER_OF_CORE - 1;
	
	// Initial each thread information
	for(int i=0;i<NUMBER_OF_CORE;i++)
	{
		if( i == NUMBER_OF_CORE-1 ){
			t[i].start = i*(N/NUMBER_OF_CORE);
			t[i].end = N;
		}
		else{
			t[i].start = i*(N/NUMBER_OF_CORE);
			t[i].end = (i+1)*(N/NUMBER_OF_CORE);
		}
		t[i].det=det;
		t[i].id=i;
	}

	// Find Inverse  
	gettimeofday(&start, NULL);
	for(int i=0;i<NUMBER_OF_CORE;i++)
	{
		pthread_create(&thread1[i] , NULL , inverse , (void*) &t[i] ) ;		
	}
	for(int i=0;i<NUMBER_OF_CORE;i++)
	{
		pthread_join(thread1[i],NULL);
	}
	gettimeofday(&end, NULL);

	//print using time	
	Time_Use = (end.tv_sec - start.tv_sec) + (double)(end.tv_usec - start.tv_usec)/1000000.0;
	cout << "Multi Thread Spend time : " << Time_Use << endl;

	//print inverse matrix
	printf("Inverse Matrix A:\n");
	display(inv);

	//Saving result in <thread>
	memcpy(thread,inv,N*N*sizeof(float));

/********************************** Compare Result *******************************************************************
**********************************************************************************************************************/	
	
	printf("##############################################################################\n");	
	if(!Compare_Result(single,thread))
		printf("Single is different with multi-thread!!!!!\n");
	else printf("Single thread is the same as multi-thread\n");

}





// Function to get cofactor of A[p][q] in temp[][]. n is current
// dimension of A[][]
void getCofactor(int A[N][N], int temp[N][N], int p, int q, int n)
{
    int i = 0, j = 0;
 
    // Looping for each element of the matrix
    for (int row = 0; row < n; row++)
    {
        for (int col = 0; col < n; col++)
        {
            //  Copying into temporary matrix only those element
            //  which are not in given row and column
            if (row != p && col != q)
            {
                temp[i][j++] = A[row][col];
 
                // Row is filled, so increase row index and
                // reset col index
                if (j == n - 1)
                {
                    j = 0;
                    i++;
                }
            }
        }
    }
}
 
/* Recursive function for finding determinant of matrix.
   n is current dimension of A[][]. */
int determinant(int det[N][N], int size)
{
	int temp[N][N],a=0,b=0,i,j,k;
    int sum=0,sign;  /* sum will hold value of determinant of the current matrix */
 
    if(size==2) return (det[0][0]*det[1][1]-det[1][0]*det[0][1]);
    sign=1;
    for(i=0;i<size;i++)  // note that 'i' will indicate column no.
    {
        a=0;
        b=0;
 
        // copy into submatrix and recurse
        for(j=1;j<size;j++) // should start from the next row !!
        {
            for(k=0;k<size;k++)
            {
                if(k==i) continue;
                temp[a][b++]=det[j][k];
            }
            a++;
            b=0;
        }
        sum+=sign*det[0][i]*determinant(temp,size-1);   // increnting row & decrementing size
        sign*=-1;
    }
    return sum;
}
 
// Function to get adjoint of A[N][N] in adj[N][N].
void* inverse(void *args)
{
    int sign = 1,temp[N][N];	
    if (N == 1)
    {
        adj[0][0] = 1;
        return NULL;
    }
	//Multi thread execution 
    if(args!=NULL)
    {
		int id=((struct thread_data*)args)->id;
		int start=((struct thread_data*)args)->start;
		int end=((struct thread_data*)args)->end;
		int det=((struct thread_data*)args)->det;
		//print thread information
		printf("The thread %d PID %ld is on CPU%d\n",id,syscall(SYS_gettid),sched_getcpu());
		//Find adjoint matrix
    	for (int i=start; i<end; i++)
    	{
        	for (int j=0; j<N; j++)
        	{
            	// Get cofactor of A[i][j]
            	getCofactor(MatrixA, temp, i, j, N);
           		// sign of adj[j][i] positive if sum of row
            	// and column indexes is even.
            	sign = ((i+j)%2==0)? 1: -1;
 				
            	// Interchanging rows and columns to get the
            	// transpose of the cofactor matrix
            	            	
            	adj[j][i] = (sign)*(determinant(temp, N-1));
            	if(sched_getcpu()!=cpu_alloc[id])
				{
					printf("The thread %d PID %ld is moved from CPU%d to CPU%d\n", id, 
					syscall(SYS_gettid),cpu_alloc[id], sched_getcpu());
					cpu_alloc[id]= sched_getcpu();			
				}
        	}
    	}

		//Find inverse matrix
		for (int i=start; i<end; i++)
        {	for (int j=0; j<N; j++)
            {
            	inv[i][j] = adj[i][j]/float(det);
            	if(sched_getcpu()!=cpu_alloc[id])
				{
					printf("The thread %d PID %ld is moved from CPU%d to CPU%d \n", id, 
					syscall(SYS_gettid),cpu_alloc[id], sched_getcpu());
					cpu_alloc[id]= sched_getcpu();			
				}
           	}
		}
    	return NULL;
    }
	//Single thread execution 
	else
	{    
		// temp is used to store cofactors of A[][]
		//int sign = 1, temp[N][N];
		for (int i=0; i<N; i++)
		{
		    for (int j=0; j<N; j++)
		    {
		        // Get cofactor of A[i][j]
		        getCofactor(MatrixA, temp, i, j, N);
		        // sign of adj[j][i] positive if sum of row
		        // and column indexes is even.
		        sign = ((i+j)%2==0)? 1: -1;
	 
		        // Interchanging rows and columns to get the
		        // transpose of the cofactor matrix
		        adj[j][i] =(sign)*(determinant(temp, N-1)); //(sign)*(determinant(temp, N-1));
		    }
		}
		for (int i=0; i<N; i++)
		    for (int j=0; j<N; j++)
		        inv[i][j] = adj[i][j]/float(global_det);
	}
}
 
 
// Generic function to display the matrix.  We use it to display
// both adjoin and inverse. adjoin is integer matrix and inverse
// is a float.
template<class T>
void display(T A[N][N])
{
    for (int i=0; i<N; i++)
    {
        for (int j=0; j<N; j++)
           printf("%f ", A[i][j]);
        printf("\n");
    }
}

//Validate inverse matrix of single thread and multi-thread. 
//To check if the answer we calucating is right.
bool Compare_Result(float single[N][N], float thread[N][N])
{
	for(int i=0;i<N;i++)
	{
		for(int j=0;j<N;j++)
		{
			if(single[i][j]!=thread[i][j]){
				printf("[%d,%d]\n",i,j);
				return false;
			}
		}
	}
	return true;
}

void printSchedulerName(int PID)
{	
	int policy = sched_getscheduler(PID);	
	switch(policy){		
	  case SCHED_OTHER:
		printf("Thread: %d, Policy: Normal\n", PID);
		break;
	  case SCHED_RR:
		printf("Thread: %d, Policy: Round-Robin\n", PID);
		break;
	  case SCHED_FIFO:
		printf("Thread: %d, Policy: FIFO\n", PID);
		break;	
	  case -1:
		perror("sched_getscheduler");
		break;
	  default:
		printf("Thread: %d, Policy: Unknown\n", PID);
		break;	
	}
}



