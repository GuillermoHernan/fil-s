//ADDED***
#include "system_interface.h"
//***

#include <stdio.h>
typedef unsigned char bool;
static const bool true = 1;
static const bool false = 0;
//************ Prolog

//REMOVED***
//typedef struct {
//  void *actorPtr;
//  void *inputPtr;
//}MessageSlot;
//***

//ADDED***
typedef struct {
	TimerInfo	timer;
	ActorInputAddress	tickOutput;
}TimerActor;

typedef struct {
	int period;
}TimerActor_params;

void TimerActor_tick(TimerActor* actor, void* params)
{
	if (actor->tickOutput.inputPtr != NULL)
	{
		SystemMsgHeader		msg;

		msg.endPoint = actor->tickOutput;
		msg.flags = 0;
		msg.msgLength = sizeof(msg);
		postMessage(&msg);
	}
}

void TimerActor_constructor(TimerActor* actor, TimerActor_params* params)
{
	actor->timer.periodMS = params->period;
	actor->timer.scheduledTime = params->period;
	actor->timer.destInput.actorPtr = actor;
	actor->timer.destInput.inputPtr = TimerActor_tick;

	scheduleTimer(&actor->timer);
}
//***


typedef struct {
	ActorInputAddress o1_0000;
//ADDED***
TimerActor  timer;
char		active;
//***


}MainActor;

//Parameters for 'i1' input message
typedef struct {
int a_0003;
}_unnamed_0002;

//Code for 'i1' input message
static void i1_0001(MainActor* _gen_actor, _unnamed_0002* _gen_params){
typedef struct {
int a_0003;
}_unnamed_0002;

}


//ADDED***
static void mainActor_i2(MainActor* actor, void* params)
{
	actor->active = (actor->active + 1) % 2;
	outputSignalWrite(12, actor->active);
}
//***


//Code for 'Test' actor constructor
static void MainActor_constructor(MainActor* _gen_actor){
	//ADDED***
	TimerActor_params params;
	params.period = 1000;

	TimerActor_constructor(&_gen_actor->timer, &params);

	_gen_actor->timer.tickOutput.actorPtr = _gen_actor;
	_gen_actor->timer.tickOutput.inputPtr = mainActor_i2;

	_gen_actor->active = 0;
	//***
}


//************ Epilog

MainActor mainActor;

//ADDED***
void initActors()
{
	MainActor_constructor(&mainActor);
}
//***

//REMOVED***
//int main()
//{
//  return 0;
//}
