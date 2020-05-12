#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sched.h>
#include <time.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
#include <unistd.h>
#include <algorithm>


#define CORE_NUM 4
#define MATRIX_SIZE 800		//1600
using namespace std;
/*****************************************************
Compiler: g++ -pthread PART1_Reentrant.cpp -o PART1_Reentrant.out
Execute	: ./PART1_Reentrant.out
******************************************************/

struct Thread_Data
{
	int Start;
	int End;
	int Total_Size;
	int Thread_ID;
	int Core;
	float** Input_Matrix;
	float** Output_Matrix;	
};

void Single_Matrix_Multiplication(Thread_Data Thread);
void* Global_Multi_Matrix_Multiplication(void *args);
void Print_Thread_Data(Thread_Data Thread);
void Compare_Result(float** Matrix1, float** Matrix2, int Matrix_Size);

int main(int argc, char** argv)
{
	int Num_Thread = CORE_NUM;
	string line;
	struct timeval start;
    struct timeval end;
	double Time_Use;
	
/********************************** Matrix Initialize ************************************************************
******************************************************************************************************************/
	float **Input_Matrix;
	float **Single_Output_Matrix;
	float **Multi_Output_Matrix;

	Input_Matrix = new float*[MATRIX_SIZE];
	for( int i = 0; i < MATRIX_SIZE; i++){
		Input_Matrix[i] = new float[MATRIX_SIZE];
	}

	Single_Output_Matrix = new float*[MATRIX_SIZE];
	for( int i = 0; i < MATRIX_SIZE; i++){
		Single_Output_Matrix[i] = new float[MATRIX_SIZE];
	}

	Multi_Output_Matrix = new float*[MATRIX_SIZE];
	for( int i = 0; i < MATRIX_SIZE; i++){
		Multi_Output_Matrix[i] = new float[MATRIX_SIZE];
	}

	//Initial Matrix Value
	for( int i = 0; i < MATRIX_SIZE; i++){
		for( int j = 0; j < MATRIX_SIZE; j++){	
			Input_Matrix[i][j] = ((float) rand()/ (RAND_MAX));
		}
	}
/********************************** Single Thread Matrix Multiplication *********************************************
******************************************************************************************************************/	
	Thread_Data Single_Thread_Data;	
	Single_Thread_Data.Input_Matrix = Input_Matrix;
	Single_Thread_Data.Output_Matrix = Single_Output_Matrix;
	Single_Thread_Data.Start = 0;
	Single_Thread_Data.End = MATRIX_SIZE;
	Single_Thread_Data.Total_Size = MATRIX_SIZE;
	
	cout << "\n===========Start Single Thread Matrix Multiplication===========" << endl; 
	gettimeofday(&start, NULL);
	Single_Matrix_Multiplication(Single_Thread_Data);
	gettimeofday(&end, NULL);
	Time_Use = (end.tv_sec - start.tv_sec) + (double)(end.tv_usec - start.tv_usec)/1000000.0;
	cout << "Single Thread Spend time : " << Time_Use << endl;

	//Print_Thread_Data(Single_Thread_Data);
/********************************** Global Multi Thread Matrix Multiplication ************************************
******************************************************************************************************************/
	Thread_Data Multi_Thread_Data[CORE_NUM];	
	for( int i = 0; i < CORE_NUM; i++)
	{
		Multi_Thread_Data[i].Input_Matrix = Input_Matrix;
		Multi_Thread_Data[i].Output_Matrix = Multi_Output_Matrix;
		Multi_Thread_Data[i].Start = (MATRIX_SIZE / CORE_NUM) * i ;
		Multi_Thread_Data[i].End = (MATRIX_SIZE / CORE_NUM) * (i+1);
		Multi_Thread_Data[i].Total_Size = MATRIX_SIZE;
		Multi_Thread_Data[i].Thread_ID = i;
	}
	
	cout << "\n===========Start Global Multi Thread Matrix Multiplication===========" << endl;

	pthread_t pthread_Thread[CORE_NUM]; 


	//Start pthread execution
	gettimeofday(&start, NULL);
	for(int i = 0; i < CORE_NUM; i++)
	{
		pthread_create(&pthread_Thread[i] , NULL ,  Global_Multi_Matrix_Multiplication , &Multi_Thread_Data[i] ) ;		
	}
	for(int i = 0; i < CORE_NUM; i++)
	{
		pthread_join(pthread_Thread[i],NULL);
	}
	gettimeofday(&end, NULL);
	Time_Use = (end.tv_sec - start.tv_sec) + (double)(end.tv_usec - start.tv_usec)/1000000.0;
	cout << "Global Multi Thread Spend time : " << Time_Use << endl;
	//Print_Thread_Data(Multi_Thread_Data[0]);
	
	//cout << "\n===========Global Multi Thread Compare Result===========" << endl; 
	Compare_Result(Single_Output_Matrix, Multi_Output_Matrix, MATRIX_SIZE);


/********************************** Output ***********************************************************************
******************************************************************************************************************/	

	cout << "\nSuccess Execution !!" << endl; 
	//sleep(10);
	return 0;
}


void Single_Matrix_Multiplication(Thread_Data Thread)
{
	for( int i = 0 ; i < Thread.Total_Size; i++){
		for( int j = 0 ; j < Thread.Total_Size; j++)
		{
			Thread.Output_Matrix[i][j] = 0;
			for( int k = 0 ; k < Thread.Total_Size; k++)
			{
				Thread.Output_Matrix[i][j] += Thread.Input_Matrix[i][k]*Thread.Input_Matrix[k][j];
			}	
			//cout << Thread.Output_Matrix[i][j] << endl;			
		}
	}
}

void* Global_Multi_Matrix_Multiplication(void *args)
{
	float Share_Sum = 0;
	Thread_Data *Thread = (struct Thread_Data*) args;
	int Core = sched_getcpu();
	int PID = syscall(SYS_gettid);
	
	cout << "The thread " << Thread->Thread_ID << " PID : " << PID << " is on CPU" << Core << endl;	
	
	//cout << "Start :" << Thread->Start << "End :" << Thread->End << "Total_Size :" << Thread->Total_Size << endl; 
	for( int i = 0 ; i < Thread->Total_Size; i++){
		for( int j = Thread->Start ; j < Thread->End; j++)
		{
			Share_Sum = 0;
			Thread->Output_Matrix[i][j] = 0;
			for( int k = 0 ; k < Thread->Total_Size; k++)
			{
				Share_Sum += Thread->Input_Matrix[i][k]*Thread->Input_Matrix[k][j];
			}
			Thread->Output_Matrix[i][j] = Share_Sum;
			//cout << Thread->Output_Matrix[i][j] << endl;
			if ( Core != sched_getcpu() )
			{
				cout << "The thread " << Thread->Thread_ID << " PID " << PID << " is moved from CPU " << Core << " to CPU" << sched_getcpu() << endl;
				Core = sched_getcpu();
			}				
		}
	}
}

void Print_Thread_Data(Thread_Data Thread)
{
	for( int i = Thread.Start ; i < Thread.Total_Size; i++){
		for( int j = 0 ; j < Thread.Total_Size; j++)
		{
			cout << Thread.Output_Matrix[i][j]	<< ", " ;		
		}
		cout << endl;
	}
}

void Compare_Result(float** Matrix1, float** Matrix2, int Matrix_Size)
{
	//Matrix 2 will be set zero
	for( int i = 0 ; i < Matrix_Size; i++){
		for( int j = 0 ; j < Matrix_Size; j++)
		{
			if( Matrix1[i][j] != Matrix2[i][j])
			{
				cout << "Result Not Same !! " << endl;
				return ;
			} 	
			Matrix2[i][j] = 0;				
		}
	}
	cout << "====Result PASS====" << endl;		
}









