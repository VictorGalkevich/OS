// PhilSemaphoreSolution.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "process.h"
#include <conio.h>

#define N			5
#define	LEFT		(i-1+N)%N /*(i>0)? (i-1)%N : N-1*/
#define	RIGHT		(i+1)%N
#define THINKING	0
#define HUNGRY		1
#define EATING		2

typedef unsigned int (__stdcall * THREADFUNC)(void *);
int state[N]={0,}; //Everybody THINKING

HANDLE	hMutex;
HANDLE	s		[N];
HANDLE	threads	[N];

bool bTerminate=false;

DWORD WINAPI philosopher(PVOID pParam);
void think     (int i);
void eat	   (int i);
void take_forks(int i);
void put_forks (int i);
void test      (int i);
void who	   (int i);

int main(int argc, char* argv[])
{
//Create Mutex
/*
	hMutex=CreateMutex(NULL,FALSE,NULL);
	if(!hMutex)
	{
		MessageBox(NULL,"CreateMutex failed...","Create Mutex",MB_OK);
		return 0;
	}
*/

	hMutex=CreateSemaphore(NULL,1,1,NULL);
		if(!hMutex)	{
			MessageBox(NULL,"CreateSemaphore hMutex failed...","Create Semaphore",MB_OK);
			return 0;
		}

//Create the array s of N semaphors
	for(int k=0; k<N; k++)
	{
		if(k==0)
		s[k]=CreateSemaphore(NULL,1,1,NULL);
		else
		s[k]=CreateSemaphore(NULL,0,1,NULL);

		if(!s[k]) {
			MessageBox(NULL,"CreateSemaphore failed...","Create Semaphore",MB_OK);
			return 0;
		}
	}
//An anonymous mutex and the array s[N] of anonymous semaphores have just created!
//--------------------------------------------------------------------------
//Create N threads all suspended
unsigned tid[N];
	for(int k=0; k<N; k++)
	{
		threads[k] = (HANDLE)_beginthreadex (NULL, 0,
											 (THREADFUNC)philosopher,
											 (void *)k,
											 CREATE_SUSPENDED,
											 &tid[k]);
		if(!threads[k])	{
			MessageBox(NULL,"CreateThread failed...","Create Thread",MB_OK);
			return 0;
		}
	}
	printf("All %ld threads have just created!\n",N);
//=======================================================
//Resume N threads  
	for(int k=0; k<N; k++)
	{	
		if(1!=ResumeThread(threads[k]))
		{
			MessageBox(NULL,"ResumeThread failed...","Resume Thread",MB_OK);
			return 0;
		}
	}
	printf("All %ld threads have just resumed their run!\n",N);
//=======================================================

	Sleep(400);
	bTerminate=true;
	WaitForMultipleObjects(N,threads,TRUE,INFINITE);

	printf("All %ld threads have just terminated !\n",N);
	printf("Hello World!\n");
	printf("Press any Key to exit...\n");
	getch();
	return 0;
}

DWORD WINAPI philosopher(PVOID pParam)
{
	while(!bTerminate)
	{
		think((int)pParam);
		take_forks((int)pParam);
		eat((int)pParam);
		put_forks((int)pParam);
	}
	return 0;
}

void think(int i)
{	
 
	//Here it is in the state THINKING and
	//can use its private resources like its brain to contemplate something
	Sleep(100 );
}

void eat(int i)
{
 
	//Here it is in the state EATING and
	//can use its private resources like its dish to have its diner		
	Sleep(100 );
}

void take_forks(int i)
{
	//down(&mutex);
	//Here it is in the state THINKING
	WaitForSingleObject(hMutex,INFINITE);
	test(i);// Test the current "state" which always is THINKING but unknown for others and 
			// if it is possible , deblock itself on the semaphore s[i]

	state[i]=HUNGRY; //Change its current state!
	//Do some work in the state HUNGRY
//??	who(i);

	test(i);//See again if it is possible deblock itself as its state and probably states of
			//others have already changed!!

	//up(&mutex);
//	ReleaseMutex(hMutex);
	ReleaseSemaphore(hMutex,1,NULL);

	//Here it can do some work in the state HUNGRY

	//down(&s[i]);
	WaitForSingleObject(s[i],INFINITE);

	//Here it is in the state EATING!!!!

}

void put_forks(int i)
{
 
	//down(&mutex);

	//Here it in the state EATING

	WaitForSingleObject(hMutex,INFINITE);

	//Here it is in the state EATING

	//This state can not affect its neighbors because they have been debloced or
	// are to be deblocked in this model!!! See the definition void test(int i).
	// Doing test(i) do not change its state and do not set up its semaphore s[i]!!!
	
	state[i]=THINKING;//Here it is save to change its "state"
	//Being in the new state test the current state others because 
	//left and right forks have become free!!
	//So one or two neighbors probably will be deblocked waiting on their semaphores!
	test(LEFT);
	test(RIGHT);
	//Here it is in the state THINKING
	
	//up(&mutex);		
//	ReleaseMutex(hMutex);
	ReleaseSemaphore(hMutex,1,NULL);

}

void test(int i)
{
	if(( state[i]    ==HUNGRY &&
		state[LEFT] !=EATING &&
		state[RIGHT]!=EATING    ) 
// 		||
//		( state[i]    ==THINKING &&
//		state[LEFT] !=EATING &&
//		state[RIGHT]!=EATING    )
 		)
	{
		state[i]=EATING;
		//up(&s[i]);
		//Here is the place where i-process really get access to its needed forks to
		// feed itself.
		//This is the very moment to signal i-process that it can use its resources.
		//When all processes starting, all are in the state THINKING and all semaphores
		//are in a signaled state. So who first is hungry he first gets forks.

		who(i);//PRINTF is possible because it is in the critical region!!
		//To print in think() or eat() it is necessary  to create there such a region.
		ReleaseSemaphore(s[i],1,NULL);

	}
	else{
	//Any other combination of states state[LEFT],state[i] and state[RIGHT]
	// do not affect behavior the whole system of N processes!!!
	}

}

void who(int i)
{
	char szReport[260],
		 szFormat[260];
	int l;	 
	strcpy(szFormat,"From ph(%d) ");			
	wsprintf(szReport,szFormat,i);
			 
	for(int k=0; k<N; k++){		l=strlen(szReport);
		wsprintf(&szReport[l],"s(%d)=%d ",k,state[k]);
	}
	strcat(szReport,"\n");
	printf(szReport);
}
