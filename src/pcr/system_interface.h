#pragma once

/// <summary>
/// Functions which should implement the system - specific 'C' runtime code.
/// </summary>


/// <summary>
/// Message endpoint address structure
/// </summary>
typedef struct {
    void *actorPtr;
    void *inputPtr;
}EndPointAddress;

/// <summary>
/// Timer information structure.
/// </summary>
typedef struct _TimerInfo {
    EndPointAddress     destInput;
    struct _TimerInfo*  next;
    unsigned            base;
    unsigned            periodMS;
    int                 id;
}TimerInfo;

void system_disableInterrupts();
void system_enableInterrupts();
void system_stop(int code);
void system_init();

void system_yield_CPU();

TimerInfo* timer_getFirst();
void timer_stop_id(int id);
void timer_schedule(TimerInfo* timer);
unsigned current_time();


void gpio_write(int address, int value);
