/// <summary>
/// System specific 'C' code for Win32 simulator platform.
/// </summary>

#include <stdlib.h>
#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "system_interface.h"

static TimerInfo* schedule_timer_int(TimerInfo* head, TimerInfo* timer);
static int new_timer_id(TimerInfo* head);
static int compare_timer_time(TimerInfo* t1, TimerInfo* t2);



/*******************************
* GLOBALS
*******************************/

//Mutex used to emulate enabling and disabling interrupts.
static HANDLE  g_intMutex;

// Head of the timer queue.
static TimerInfo * g_headTimer = NULL;


void system_disableInterrupts()
{
    WaitForSingleObject(g_intMutex, INFINITE);
}

void system_enableInterrupts()
{
    ReleaseMutex(g_intMutex);
}


void system_stop(int code)
{
    puts("Bye!!!");
    exit(code);
}

void system_init()
{
    g_intMutex = CreateMutex(NULL, FALSE, NULL);
}

void system_yield_CPU()
{
    //TODO: By the moment, is just fine to do nothing on the simulator.
    //but it would be nice a mechanism to really yield the CPU.
}

void gpio_write(int address, int value)
{
    printf("o%d=%d\n", address, value);
}


int timer_start(void* params)
{
    struct Params {
        int periodMS;               //Timer period, in milliseconds.
        EndPointAddress endPoint;   //Timer message destination end point.
        TimerInfo*      info;       //Timer information structure.
    };
    struct Params* pParams = (struct Params*)params;

    if (pParams->info->id >= 0)
        timer_stop_id(pParams->info->id);

    pParams->info->destInput = pParams->endPoint;
    pParams->info->base = current_time();
    pParams->info->periodMS = pParams->periodMS;
    pParams->info->id = new_timer_id(g_headTimer);
    pParams->info->next = NULL;

    g_headTimer = schedule_timer_int(g_headTimer, pParams->info);

    return pParams->info->id;
}

void timer_stop(void* params)
{
    struct Params {
        int timerID;                //Timer ID.
    };
    struct Params* pParams = (struct Params*)params;

}

//Returns the first timer in timer queue.
TimerInfo* timer_getFirst()
{
    return g_headTimer;
}


//Internal version of timer ID. Receives an integer timer ID parameter.
void timer_stop_id(int id)
{
    TimerInfo*  prev = g_headTimer;
    TimerInfo*  timer = NULL;

    //look for timer.
    while (timer != NULL && timer->id != id)
    {
        prev = timer;
        timer = timer->next;
    }

    //Remove timer if found.
    if (timer != NULL)
    {
        if (prev == NULL)
            g_headTimer = timer->next;
        else
            prev->next = timer->next;

        timer->id = -1;
        timer->next = NULL;
    }
}

//Assigns a timer a position in the timer queue.
void timer_schedule(TimerInfo* timer)
{
    if (timer == NULL)
        return;

    //printf("Start scheduling...\nTimer next: %p\n", timer->next);

    if (timer == g_headTimer)
        g_headTimer = g_headTimer->next;

    timer->base = current_time();

    g_headTimer = schedule_timer_int(g_headTimer, timer);
    //printf("Done scheduling!\n");
}


//Assigns a timer a position in the timer queue.
static TimerInfo* schedule_timer_int(TimerInfo* head, TimerInfo* timer)
{
    if (head == NULL)
        return timer;

    if (compare_timer_time(head, timer) > 0)
    {
        timer->next = head;
        return timer;
    }
    else
    {
        //printf("Scheduling timer. Head: %p\n", head);
        head->next = schedule_timer_int(head->next, timer);
        return head;
    }
}

//Generates a new timer identifier
static int new_timer_id(TimerInfo* head)
{
    int id = 1;
    
    for (TimerInfo* t = head; t != NULL; t = t->next)
    {
        if (t->id > id)
            id = t->id + 1;
    }

    return id;
}

// Gets current time, in milliseconds.
unsigned current_time()
{
    return GetTickCount();
}

//Compares 2 timer times.
//Returns:
//  >0 if t1 is greater.
//  <0 if t2 is greater.
//  ==0 if they are in the very same millisecond.
int compare_timer_time(TimerInfo* t1, TimerInfo* t2)
{
    return (t1->base + t1->periodMS) - (t2->base + t2->periodMS);
}
