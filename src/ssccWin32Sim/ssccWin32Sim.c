/// <summary>
/// System specific 'C' code for Win32 simulator platform.
/// </summary>

#include <stdlib.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "system_interface.h"

/*******************************
* GLOBALS
*******************************/

//Mutex used to emulate enabling and disabling interrupts.
static HANDLE  g_intMutex;

void system_disableInterrupts()
{
    WaitForSingleObject(g_intMutex, INFINITE);
}

void system_enableInterrupts()
{
    ReleaseMutex(g_intMutex);
}


void system_stop()
{
    exit(0);
}

void system_init()
{
    g_intMutex = CreateMutex(NULL, FALSE, NULL);
}

void system_yield_CPU()
{
    //TODO: By the moment, is just fine to do nothing on the simulator.
    //but it would be nice a mechanism to really yiend the CPU.
}
