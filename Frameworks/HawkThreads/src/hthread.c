/*
  HawkThreads thread module
  Copyright (C) 2003-2005 Phil Frisbie, Jr. (phil@hawksoft.com)

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.
  
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.
    
  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the
  Free Software Foundation, Inc., 59 Temple Place - Suite 330,
  Boston, MA  02111-1307, USA.
      
  Or go to http://www.gnu.org/copyleft/lgpl.html
*/

#ifndef HL_WINDOWS_APP
/* Include nanosleep (POSIX.1b 1993) */
#define _POSIX_C_SOURCE 199309L
#endif

#include "htinternal.h"

/* These macros allow integers to be stored in a void pointer. */
#ifdef HL_WINDOWS_APP
#define VOID_CAST	(LPVOID)(DWORD_PTR)
#define VOID_CATCH	(DWORD_PTR)
#define KEY_CAST	(HTkey)(DWORD_PTR)
#define KEY_CATCH	(DWORD)(DWORD_PTR)
#else
#include <stdint.h>
#define VOID_CAST	(void *)(uintptr_t)
#define VOID_CATCH	(uintptr_t)
#define KEY_CAST	(HTkey)(uintptr_t)
#define KEY_CATCH	(pthread_key_t)(uintptr_t)
#endif

#define KEY_NULL    ((HTkey)HT_INVALID)
static HTkey prioritykey = KEY_NULL;

#ifdef HT_WIN_THREADS

typedef struct {
    void *(*func) (void *);
    void *arg;
    int   priority;
} ThreadParms;

static unsigned  __stdcall threadfunc(void *arg)
{
    void *(*func) (void *);
    void *args;
    
    func = ((ThreadParms *)arg)->func;
    args = ((ThreadParms *)arg)->arg;
    /* any thread specific data goes here since we are now running in the new thread */
    (void)htThreadSetKeyData(prioritykey, (VOID_CAST ((ThreadParms *)arg)->priority));
    free(arg);
    
    return (unsigned)((*func)(args));
}
#if defined (_WIN32_WCE)
#define _beginthreadex(security, \
		       stack_size, \
		       start_proc, \
		       arg, \
		       flags, \
		       pid) \
	CreateThread(security, \
		     stack_size, \
		     (LPTHREAD_START_ROUTINE) start_proc, \
		     arg, \
		     flags, \
		     pid)
#else /* !(_WIN32_WCE) */
#include <process.h>
#endif /* !(_WIN32_WCE) */

#else /* !HT_WIN_THREADS */
/* POSIX systems */

typedef struct {
    void *(*func) (void *);
    void *arg;
    int   priority;
} ThreadParms;

static void *threadfunc(void *arg)
{
    void *(*func) (void *);
    void *args;
    
    func = ((ThreadParms *)arg)->func;
    args = ((ThreadParms *)arg)->arg;
    /* any thread specific data goes here since we are now running in the new thread */
    (void)htThreadSetKeyData(prioritykey, (VOID_CAST ((ThreadParms *)arg)->priority));
    free(arg);
    
    return ((*func)(args));
}

#include <pthread.h>
#include <sched.h>
#endif /* !HT_WIN_THREADS */

#ifdef HL_WINDOWS_APP
#include <process.h>
#else
#include <unistd.h>
#endif /* HL_WINDOWS_APP */

HL_EXP HThreadID HL_APIENTRY htThreadCreate(HThreadFunc func, void *data, int joinable)
{
    /* Windows threads */
#ifdef HT_WIN_THREADS
    HANDLE          tid;
    unsigned        t;
    ThreadParms     *p;

    if(prioritykey == KEY_NULL)
    {
        prioritykey = htThreadNewKey();
    }
    p = (ThreadParms *)malloc(sizeof(ThreadParms));
    if(p == NULL)
    {
        SetLastError((DWORD)ENOMEM);
        return (HThreadID)HT_INVALID;
    }
    p->func = func;
    p->arg = data;
    p->priority = htThreadGetPriority();
    tid = (HANDLE)_beginthreadex(NULL, 0, threadfunc, p, 0, &t);
    if(tid == (HANDLE)(0))
    {
        return (HThreadID)HT_INVALID;
    }
    if(joinable == HT_FALSE)
    {
        (void)CloseHandle(tid);
        return NULL;
    }
    
    /* POSIX systems */
#else
    pthread_attr_t  attr;
    pthread_t       tid;
    int             result;
    ThreadParms     *p;
    
    if(prioritykey == KEY_NULL)
    {
        prioritykey = htThreadNewKey();
    }
    (void)pthread_attr_init(&attr);
    if(joinable == HT_FALSE)
    {
        (void)pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    }
    else
    {
        (void)pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    }
    p = (ThreadParms *)malloc(sizeof(ThreadParms));
    if(p == NULL)
    {
	/* Commented out since this is not valid on POSIX systems. */
        /* SetLastError((DWORD)ENOMEM); */
        return (HThreadID)HT_INVALID;
    }
    p->func = func;
    p->arg = data;
    p->priority = htThreadGetPriority();
    result = pthread_create(&tid, &attr, threadfunc, p);
    (void)pthread_attr_destroy(&attr);
    if(result != 0)
    {
#ifdef HL_WINDOWS_APP
            SetLastError((DWORD)result);
#endif
            return (HThreadID)HT_INVALID;
    }
    if(joinable == HT_FALSE)
    {
        return NULL;
    }
#endif
    return (HThreadID)tid;
}

HL_EXP void HL_APIENTRY htThreadYield(void)
{
    /* Windows threads */
#ifdef HT_WIN_THREADS
    Sleep((DWORD)0);

    /* POSIX systems */
#else
    (void)sched_yield();
    
#endif
}

HL_EXP int HL_APIENTRY htThreadJoin(HThreadID threadID, void **status)
{
#ifdef HT_WIN_THREADS
    /* Windows threads */
    if(WaitForSingleObject((HANDLE)threadID, INFINITE) == WAIT_FAILED)
    {
        return EINVAL;
    }
    if(status != NULL)
    {
        (void)GetExitCodeThread((HANDLE)threadID, (LPDWORD)status);
    }
    (void)CloseHandle((HANDLE)threadID);
    return 0;

#else
    /* POSIX systems */
    return pthread_join((pthread_t)threadID, status);
#endif
}

HL_EXP void HL_APIENTRY htThreadSleep(int mseconds)
{
#ifdef HL_WINDOWS_APP
    Sleep((DWORD)mseconds);
#else /* !HL_WINDOWS_APP */
    struct timespec     tv;

    tv.tv_sec = mseconds / 1000;
    tv.tv_nsec = (mseconds % 1000) * 1000;

    (void)nanosleep(&tv, NULL);
#endif /* !HL_WINDOWS_APP */
    /* can use usleep if nanosleep is not supported on your platform */
/*    (void)usleep(mseconds*1000); */
}


HL_EXP HThreadID HL_APIENTRY htThreadSelf(void)
{
#ifdef HT_WIN_THREADS
    /* Windows threads */
    return (HThreadID)GetCurrentThreadId();
#else
    /* POSIX systems */
    return (HThreadID)pthread_self();
#endif
}

HL_EXP int HL_APIENTRY htThreadEqual(HThreadID threadID1, HThreadID threadID2)
{
#ifdef HT_WIN_THREADS
    /* Windows threads */
    const int thread1 = (int)(VOID_CATCH threadID1);
    const int thread2 = (int)(VOID_CATCH threadID2);
    return (int)(thread1 == thread2);
#else
    /* POSIX systems */
    const pthread_t thread1 = (pthread_t)(VOID_CATCH threadID1);
    const pthread_t thread2 = (pthread_t)(VOID_CATCH threadID2);
    return (int)pthread_equal(thread1, thread2);
#endif
}

HL_EXP int HL_APIENTRY htThreadGetPriority(void)
{
    int p;

    if(prioritykey == KEY_NULL)
    {
        prioritykey = htThreadNewKey();
    }
    p = (int)(VOID_CATCH htThreadGetKeyData(prioritykey));
    if(p == 0)
    {
        /* key has never been set, so assume HT_THREAD_PRIORITY_NORMAL */
        (void)htThreadSetKeyData(prioritykey, (VOID_CAST HT_THREAD_PRIORITY_NORMAL));
        return HT_THREAD_PRIORITY_NORMAL;
    }
    else
    {
        return p;
    }
}

HL_EXP int HL_APIENTRY htThreadSetPriority(int priority)
{
#ifdef HT_WIN_THREADS
    BOOL result;
    int p;

    /* Windows threads */
    switch (priority) {
    case HT_THREAD_PRIORITY_LOWEST:
        p = THREAD_PRIORITY_IDLE;
        break;
    case HT_THREAD_PRIORITY_LOW:
        p = THREAD_PRIORITY_BELOW_NORMAL;
        break;
    case HT_THREAD_PRIORITY_NORMAL:
        p = THREAD_PRIORITY_NORMAL;
        break;
    case HT_THREAD_PRIORITY_HIGH:
        p = THREAD_PRIORITY_ABOVE_NORMAL;
        break;
    case HT_THREAD_PRIORITY_HIGHEST:
        p = THREAD_PRIORITY_TIME_CRITICAL;
        break;
    default:
        p = -100;/* -100 is just a priority that cannot happen */
    }
    if(prioritykey == KEY_NULL)
    {
        prioritykey = htThreadNewKey();
    }
    if(p != -100)
    {
        result = SetThreadPriority(GetCurrentThread(), p);
    }
    else
    {
        result = 0;
    }
    if(result == 0)
    {
        return EINVAL;
    }
    else
    {
        (void)htThreadSetKeyData(prioritykey, (void *)priority);
        return 0;
    }
#else
    /* POSIX systems */
    int mypid = getpid();
    int scheduler = sched_getscheduler(mypid);
    int minimum = sched_get_priority_min(scheduler);
    int maximum = sched_get_priority_max(scheduler);
    int spread = maximum - minimum;
    int result;
    float correction = (float)spread / (HT_THREAD_PRIORITY_HIGHEST - 1);
    struct sched_param param;

    if(prioritykey == KEY_NULL)
    {
        prioritykey = htThreadNewKey();
    }
    /* validate priority */
    priority = (priority < HT_THREAD_PRIORITY_LOWEST? HT_THREAD_PRIORITY_LOWEST:
                (priority > HT_THREAD_PRIORITY_HIGHEST? HT_THREAD_PRIORITY_HIGHEST:priority));
    /* zero base the priority */
    priority--;
    param.sched_priority = (int)(priority * correction) + minimum;

    result = pthread_setschedparam(pthread_self(), scheduler, &param);
    if(result != 0)
    {
        return result;
    }
    else
    {
        priority++;
        (void)htThreadSetKeyData(prioritykey, (VOID_CAST priority));
        return 0;
    }
#endif
}

HL_EXP HTkey HL_APIENTRY htThreadNewKey(void)
{
#ifdef HT_WIN_THREADS
    /* Windows threads */
    DWORD key = TlsAlloc();

    if(key == 0xFFFFFFFF)
    {
        return KEY_NULL;
    }
    else
    {
        return (KEY_CAST key);
    }
#else
    /* POSIX systems */
    pthread_key_t *key = malloc(sizeof(pthread_key_t));

    if(key == NULL)
    {
        return KEY_NULL;
    }
    else
    {
        int result = pthread_key_create(key, NULL);

        if(result != 0)
        {
#ifdef HL_WINDOWS_APP
            SetLastError((DWORD)result);
#endif
            return KEY_NULL;
        }
        else
        {
            return (KEY_CAST *key);
        }
    }
#endif
}

HL_EXP int HL_APIENTRY htThreadSetKeyData(HTkey key, void* data)
{
#ifdef HT_WIN_THREADS
    /* Windows threads */
    BOOL result = TlsSetValue((KEY_CATCH key), (LPVOID)data);

    if(result == 0)
    {
        return GetLastError();
    }
    else
        return 0;
#else
    /* POSIX systems */
    return pthread_setspecific((KEY_CATCH key), data);
#endif
}

HL_EXP void* HL_APIENTRY htThreadGetKeyData(HTkey key)
{
#ifdef HT_WIN_THREADS
    /* Windows threads */
    return (void *)TlsGetValue((KEY_CATCH key));

#else
    /* POSIX systems */
    return (void *)pthread_getspecific((KEY_CATCH key));
#endif
}

HL_EXP int HL_APIENTRY htThreadDeleteKey(HTkey key)
{
#ifdef HT_WIN_THREADS
    /* Windows threads */
    BOOL result = TlsFree((KEY_CATCH key));
    if(result == 0)
    {
        return GetLastError();
    }
    else
    {
        return 0;
    }
#else
    /* POSIX systems */
    return pthread_key_delete((KEY_CATCH key));
#endif
}

