/// <summary>
/// Functions and structures provided by the run-time to the 
/// generated code.
/// </summary>
#pragma once


#ifdef __cplusplus
extern "C" {
#endif

    /// <summary>
    /// Prototype of an input message handler function of an actor.
    /// </summary>
    typedef void(*MessageFunction)(void* actor, void* params);

    /// <summary>
    /// Address of an actor input
    /// </summary>
    typedef struct
    {
        void*				actorPtr;
        MessageFunction		inputPtr;
    }ActorInputAddress;


    /// <summary>
    /// Timer information structure.
    /// </summary>
    typedef struct TimerInfo_
    {
        ActorInputAddress		destInput;
        struct TimerInfo_*		next;
        int						scheduledTime;
        int						periodMS;
    }TimerInfo;


    /// <summary>
    /// Header for all actor messages
    /// </summary>
    typedef struct
    {
        ActorInputAddress	endPoint;
        int					msgLength;
        int					flags;
    }SystemMsgHeader;


    void scheduleTimer(TimerInfo* timer);
    void postMessage(const SystemMsgHeader* msg);
    void outputSignalWrite(int address, int value);


#ifdef __cplusplus
}
#endif
