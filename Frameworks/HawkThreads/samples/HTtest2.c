/*
  Sample program for the HawkNL cross platform network library
  Copyright (C) 2000-2002 Phil Frisbie, Jr. (phil@hawksoft.com)
*/
/*
  This app shows a multithreaded client/server app with thread pooling.

  This is a stripped down version of threadpool.c that comes with HawkNL
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hawkthreads.h"

#if defined WIN32 || defined WIN64
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define sleep(x)    Sleep(1000 * (x))
#else
#include <unistd.h>
#endif

#define MAX_THREADS     4
#define MAX_WAKEUPS     12

HTmutex     startmutex, datamutex;
HTcond      servercond;
int         shutdown = HT_FALSE;

static void *mainServerLoop(void *t)
{
    int         thread = (int)t;
    int         result;

    /* wait for start */
    (void)htMutexLock(&startmutex);
    printf("THREADPOOL: thread %d starting\n", thread);
    (void)htMutexUnlock(&startmutex);
    htThreadYield();
    /* wait on the condition forever */
    while((result = htCondWait(&servercond, 0)) == 0)
    {
        if(shutdown == HT_TRUE)
        {
            printf("THREADPOOL: thread %d asked to shut down\n", thread);
            return NULL;
        }
        (void)htMutexLock(&datamutex);

        /* This is where work would be done by the thread */
        printf("THREADPOOL: thread %d woke up\n", thread);



        (void)htMutexUnlock(&datamutex);
    }
    /* if we got here, there was a problem with the condition */
    /* or we are shutting down */

    printf("THREADPOOL: thread %d exiting due to error\n", thread, result);
    return NULL;
}

int main(int argc, char **argv)
{
    int             connects = 0;
    int             i;
    int             result;

    if((result = htCondInit(&servercond)) != 0)
        return result;

	(void)htMutexInit(&startmutex);
	(void)htMutexInit(&datamutex);

    /* create the server threads */
    (void)htMutexLock(&startmutex);
    for(i=0;i<MAX_THREADS;i++)
    {
        /* pass the thread number */
        (void)htThreadCreate(mainServerLoop, (void *)i, HT_FALSE);
    }
    /* now release the threads */
    (void)htMutexUnlock(&startmutex);
    htThreadYield();

    printf("MAIN:  testing htCondSignal\n\n");
    /* main dispatch loop */
    while(connects < MAX_WAKEUPS)
    {
            int result;

            /* wake up one thread */
            printf("MAIN: waking up one thread\n");
			(void)htMutexLock(&datamutex);
			(void)htMutexUnlock(&datamutex);
            htThreadYield();
            if((result = htCondSignal(&servercond)) != 0)
            {
                printf("MAIN: htCondSignal error %d\n", result);
                return result;
            }
            connects++;
    }

    printf("\nMAIN: testing htCondBroadcast\n\n");
    connects = 0;
    while(connects < MAX_WAKEUPS)
    {
            int result;

            /* wake up one thread */
            printf("MAIN: waking up all threads\n");
			(void)htMutexLock(&datamutex);
			(void)htMutexUnlock(&datamutex);
            htThreadYield();
            if((result = htCondBroadcast(&servercond)) != 0)
            {
                printf("MAIN: htCondBroadcast error %d\n", result);
                return result;
            }
            connects++;
    }
    /* shutdown the thread pool */
    printf("\nMAIN: Begin shutdown\n\n");
    shutdown = HT_TRUE;
    htCondBroadcast(&servercond);
    /* wait for all the threads to exit */
    sleep(4);
	(void)htMutexDestroy(&startmutex);
	(void)htMutexDestroy(&datamutex);
    (void)htCondDestroy(&servercond);

    printf("\nMAIN: Shutdown finished\n\n");
    return 0;
}

