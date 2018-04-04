/// <summary>
/// Portable 'C' runtime (PCR) for FIL-S language.
/// Contains the minimum (portable) code needed to run a FIL-S program.
/// </summary>
/// <remarks>
/// The program entry point is not in this library, because it is considered
/// system - dependent.
/// </remarks>

#include <stdlib.h>
#include <memory.h>
#include <assert.h>

#include "system_interface.h"

typedef unsigned char byte;

/// <summary>
/// Message age endpoint address structure
/// </summary>
typedef struct {
  void *actorPtr;
  void *inputPtr;
}EndPointAddress;

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

static int queueAlloc(SystemMsgQueue* queue, size_t size);
static void lockSystemQueue();
static void unlockSystemQueue();

static void systemError(const char* message);

/// <summary>
/// Initialices PCR globals.
/// </summary>
void initPcr()
{
    g_msgQueue.readIdx = -1;
    g_msgQueue.writeIdx = -1;
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

    MessageHeader   header;
    const size_t    headerSize = sizeof(header);
    const size_t    msgLength = paramsSize + headerSize;

    header.address = *address;
    header.msgLength = (unsigned short)msgLength;

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
const MessageHeader* getHeadMessage()
{
    //Check if empty
    if (g_msgQueue.readIdx < 0)
        return NULL;
    else
        return (const MessageHeader*)(g_msgQueue.data + g_msgQueue.readIdx);
}


/// <summary>
/// Removes the head message, and any invalid messages up to the
/// next valid message.
/// </summary>
void popHeadMessage()
{
    const MessageHeader*  msg = getHeadMessage();

    if (msg == NULL)
        return;

    lockSystemQueue();

    SystemMsgQueue*     q = &g_msgQueue;

    q->readIdx = (q->readIdx + msg->msgLength) % SYSTEM_QUEUE_SIZE;
    if (q->readIdx == q->readIdx)
        q->readIdx = q->readIdx = -1;
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
    int result = q->writeIdx;

    if (q->writeIdx > q->readIdx)
    {
        const size_t available = SYSTEM_QUEUE_SIZE - q->writeIdx;

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
    }
    else
    {
        const size_t available = q->readIdx < 0 ? SYSTEM_QUEUE_SIZE : q->readIdx - q->writeIdx;

        if (size > available)
            systemError("System queue overflow!");
        else
            q->writeIdx += size;
    }

    return result;
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
    //TODO: log the message.
    system_stop();
}