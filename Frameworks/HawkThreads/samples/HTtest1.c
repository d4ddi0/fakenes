/*
  Test program for the HawkThreads library
  Copyright (C) 2005 Phil Frisbie, Jr. (phil@hawksoft.com)
*/
/*
  Test1 confirms that thread creation, joining, priority, and local storage all work
*/

/*
  To test UNICODE on Windows NT/2000/XP/CE, uncomment both the defines below and compile
  this program.
*/
//#define _UNICODE
//#define UNICODE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hawkthreads.h"

#ifdef HL_WINDOWS_APP
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#endif

#ifndef _INC_TCHAR
#ifdef _UNICODE
#define TEXT(x)    L##x
#define _tmain      wmain
#define _tprintf    wprintf
#define _stprintf   swprintf
#define _tcslen     wcslen
#ifdef HL_WINDOWS_APP
#define _ttoi       _wtoi
#else /* !HL_WINDOWS_APP*/
#define _ttoi       wtoi
#endif /* !HL_WINDOWS_APP*/
#else /* !UNICODE */
#define TEXT(x)    x
#define _TCHAR      char
#define _tmain      main
#define _tprintf    printf
#define _stprintf   sprintf
#define _tcslen     strlen
#endif /* !UNICODE */
#endif /* _INC_TCHAR */

static void *threadfunc(void *t)
{
    int       thread = (int)t;

    printf("Thread %d priority set to %d\n", thread, htThreadGetPriority());

    return NULL;
}

#if defined (_WIN32_WCE)
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
				   LPWSTR lpCmdLine, int nShowCmd )
#else
int _tmain(int argc, _TCHAR **argv)
#endif
{
    HThreadID       tid;

    printf("Default thread priority is %d\n", htThreadGetPriority());

    htThreadSetPriority(HT_THREAD_PRIORITY_LOWEST);
    printf("Thread priority set to HT_THREAD_PRIORITY_LOWEST, now priority is %d\n", htThreadGetPriority());
    printf("Creating thread 1\n");
    tid = htThreadCreate(threadfunc, (void *)1, HT_TRUE);
    (void)htThreadJoin(tid, NULL);

    htThreadSetPriority(HT_THREAD_PRIORITY_LOW);
    printf("Thread priority set to HT_THREAD_PRIORITY_LOW, now priority is %d\n", htThreadGetPriority());
    printf("Creating thread 2\n");
    tid = htThreadCreate(threadfunc, (void *)2, HT_TRUE);
    (void)htThreadJoin(tid, NULL);
    
    htThreadSetPriority(HT_THREAD_PRIORITY_NORMAL);
    printf("Thread priority set to HT_THREAD_PRIORITY_NORMAL, now priority is %d\n", htThreadGetPriority());
    printf("Creating thread 3\n");
    tid = htThreadCreate(threadfunc, (void *)3, HT_TRUE);
    (void)htThreadJoin(tid, NULL);
    
    htThreadSetPriority(HT_THREAD_PRIORITY_HIGH);
    printf("Thread priority set to HT_THREAD_PRIORITY_HIGH, now priority is %d\n", htThreadGetPriority());
    printf("Creating thread 4\n");
    tid = htThreadCreate(threadfunc, (void *)4, HT_TRUE);
    (void)htThreadJoin(tid, NULL);
    
    htThreadSetPriority(HT_THREAD_PRIORITY_HIGHEST);
    printf("Thread priority set to HT_THREAD_PRIORITY_HIGHEST, now priority is %d\n", htThreadGetPriority());
    printf("Creating thread 5\n");
    tid = htThreadCreate(threadfunc, (void *)5, HT_TRUE);
    (void)htThreadJoin(tid, NULL);
    
    htThreadSetPriority(HT_THREAD_PRIORITY_NORMAL);
    printf("Thread priority set to HT_THREAD_PRIORITY_NORMAL, now priority is %d\n", htThreadGetPriority());
    printf("Creating thread 6\n");
    tid = htThreadCreate(threadfunc, (void *)6, HT_TRUE);
    (void)htThreadJoin(tid, NULL);

    return 0;
}

