/// <summary>
/// Portable 'C' runtime (PCR) for FIL-S language.
/// Contains the minimum (portable) code needed to run a FIL-S program.
/// </summary>
/// <remarks>
/// The program entry point is not in this library, because it is considered
/// system - dependent.
/// </remarks>

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <assert.h>

#include "system_interface.h"

typedef unsigned char byte;

/// <summary>
/// Header of an actor message
/// </summary>
typedef struct {
    EndPointAddress     address;
    unsigned short      msgLength;
    byte                flags;
    byte                reserved;
    byte                params[0];
}MessageHeader;

typedef void (*MessageHandlerFunction)(void *actor, void* params);


/// <summary>
/// Defines internal state flags for actor messages.
/// </summary>
enum SystemMsgFlags
{
    MSGF_DELETED = 1
};

#define SYSTEM_QUEUE_SIZE 512

/// <summary>
/// System message queue structure.
/// </summary>
typedef struct {
    //TODO: Make queue size configurable. At least, at compile time.
    //Move this to the generated code?
    //A possible solution is to add a 'queue' parameter in message queue functions, 
    //but it may add a little extra overhead.
    byte    data[SYSTEM_QUEUE_SIZE];
    int     readIdx;
    int     writeIdx;
}SystemMsgQueue;

/*******************************
 * GLOBALS
 *******************************/

//System message queue.
SystemMsgQueue  g_msgQueue;

/**********************************
* Internal functions declarations.
***********************************/

void postMessage(const EndPointAddress* address, const void* params, size_t paramsSize);

static int queueAlloc(SystemMsgQueue* queue, size_t size);
static void lockSystemQueue();
static void unlockSystemQueue();

static void systemError(const char* message);

static int checkTimers();
static int dispatchActorMessages();

static MessageHeader* getHeadMessage();
static void popHeadMessage();


/**********************************
* Functions which should be defined
* in generated code.
***********************************/
void initActors();

/// <summary>
/// Initialices PCR globals.
/// </summary>
void initPcr()
{
    system_init();
    g_msgQueue.readIdx = -1;
    g_msgQueue.writeIdx = -1;

    initActors();
}

/// <summary>
/// Starts the scheduler. 
/// This function never returns.
/// </summary>
void runScheduler()
{
    while (1)
    {
        //const MessageHeader* msg = getHeadMessage();
        int active = 0;

        active = checkTimers();
        //puts("Dispatch start...");
        active |= dispatchActorMessages();
        if (!active)
            system_yield_CPU();
    }
}

/// <summary>
/// Checks system queue and sends messages to the actors if needed.
/// </summary>
/// <returns></returns>
static int dispatchActorMessages()
{
    MessageHeader*  msg = getHeadMessage();
    int             count = 0;

    while (msg)
    {
        assert(msg->address.inputPtr != NULL);
        MessageHandlerFunction  input = (MessageHandlerFunction)msg->address.inputPtr;

        printf("Dispatching message. Actor: %p Input: %p Message length: %d\n",
            msg->address.actorPtr, 
            msg->address.inputPtr, 
            (int)msg->msgLength);

        input(msg->address.actorPtr, msg->params);
        ++count;

        popHeadMessage();
        msg = getHeadMessage();
    }

    return count;
}

/// <summary>
/// Checks if some timers have reached its scheduled time.
/// </summary>
/// <returns></returns>
static int checkTimers()
{
    unsigned    now = current_time();
    int         count = 0;
    TimerInfo*  timer = timer_getFirst();

    while (timer != NULL && now - timer->base >= timer->periodMS)
    {
        printf("Executing timer. ID: %d Period: %d\n", timer->id, timer->periodMS);
        postMessage(&timer->destInput, NULL, 0);
        timer_schedule(timer);
        timer = timer_getFirst();
        ++count;
    }

    return count;
}

/// <summary>
/// Posts a new message into the system queue.
/// </summary>
/// <param name="address"></param>
/// <param name="params"></param>
/// <param name="paramsSize"></param>
void postMessage(const EndPointAddress* address, const void* params, size_t paramsSize)
{
    lockSystemQueue();

    assert(address != NULL);

    printf("Posting message. Actor: %p Input: %p Params size: %d\n",
        address->actorPtr, address->inputPtr, (int)paramsSize);

    MessageHeader   header;
    const size_t    headerSize = sizeof(header);
    const size_t    msgLength = paramsSize + headerSize;

    header.address = *address;
    header.msgLength = (unsigned short)msgLength;
    header.flags = 0;
    header.reserved = 0;

    int     idx = queueAlloc(&g_msgQueue, msgLength);
    byte*   writePtr = g_msgQueue.data + idx;
    
    //Copy header.
    memcpy(writePtr, &header, headerSize);
    writePtr += headerSize;

    //Copy params
    memcpy(writePtr, params, paramsSize);

    unlockSystemQueue();
}

/// <summary>
/// Gets a pointer to the first message in the queue.
/// Returns NULL if empty.
/// </summary>
/// <returns></returns>
static MessageHeader* getHeadMessage()
{
    //Check if empty
    if (g_msgQueue.readIdx < 0)
        return NULL;
    else
        return (MessageHeader*)(g_msgQueue.data + g_msgQueue.readIdx);
}


/// <summary>
/// Removes the head message, and any invalid messages up to the
/// next valid message.
/// </summary>
static void popHeadMessage()
{
    const MessageHeader*  msg = getHeadMessage();

    if (msg == NULL)
        return;

    lockSystemQueue();

    SystemMsgQueue*     q = &g_msgQueue;

    q->readIdx = (q->readIdx + msg->msgLength) % SYSTEM_QUEUE_SIZE;
    if (q->readIdx == q->writeIdx)
        q->readIdx = q->writeIdx = -1;
    else
    {
        if (SYSTEM_QUEUE_SIZE - q->readIdx < sizeof(MessageHeader))
            q->readIdx = 0;

        //Check for deleted messages
        //TODO: It would be better to avoid this recursive call.
        msg = getHeadMessage();
        if (msg != NULL && msg->flags & MSGF_DELETED)
            popHeadMessage();
    }

    printf("POP head message. ReadIdx: %d WriteIdx: %d\n", q->readIdx, q->writeIdx);

    unlockSystemQueue();
}

/// <summary>
/// Allocates space in a message queue for a new message.
/// </summary>
/// <param name="q"></param>
/// <param name="size"></param>
/// <returns>The index in the queue of the new message.</returns>
static int queueAlloc(SystemMsgQueue* q, size_t size)
{

    if (q->writeIdx > q->readIdx)
    {
        const size_t available = SYSTEM_QUEUE_SIZE - q->writeIdx;
        const int result = q->writeIdx;

        if (size < available)
            q->writeIdx += size;
        else if (size == available)
            q->writeIdx = 0;
        else
        {
            if (available >= sizeof(MessageHeader))
            {
                MessageHeader*	header = (MessageHeader*)(q->data + q->writeIdx);

                header->flags = MSGF_DELETED;
                header->msgLength = (unsigned short)available;
            }
            q->writeIdx = 0;

            //Recursive call, once the chunk at the end has been skipped.
            return queueAlloc(q, size);
        }

        return result;
    }
    else
    {
        size_t available = q->readIdx - q->writeIdx;

        if (q->readIdx < 0)
        {
            available = SYSTEM_QUEUE_SIZE;
            q->readIdx = q->writeIdx = 0;
        }
        const int result = q->writeIdx;

        if (size > available)
            systemError("System queue overflow!");
        else
            q->writeIdx += size;

        return result;
    }
}

static void lockSystemQueue()
{
    system_disableInterrupts();
}

static void unlockSystemQueue()
{
    system_enableInterrupts();
}

static void systemError(const char* message)
{
    //TODO: Better logging
    fprintf(stderr, "System error: %s\n", message);

    system_stop(-1);
}

void quit(void* params)
{
    typedef struct {
        int code;
    }ParamsT;

    ParamsT*    pParams = (ParamsT*)params;
    system_stop(pParams->code);
}

void digitalOut(void* params)
{
    typedef struct {
        int address;
        unsigned char value;
    }ParamsT;

    ParamsT*    pParams = (ParamsT*)params;
    gpio_write(pParams->address, pParams->value);
}