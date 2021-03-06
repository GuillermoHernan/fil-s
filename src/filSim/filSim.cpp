/// <summary>
/// This is the entry point for FIL-S hardware simulator.
/// Simulates a hardware device with its input and output signals inside
/// a Linux / Windows process.
/// All input data is read from standard input, and all outputs are written to 
/// the standard output.
/// </summary>
// filSim.cpp : Defines the entry point for the console application.
//

#include "filSim_pch.h"
#include "generated_interface.h"
#include "system_interface.h"

using namespace std;

/// <summary>
/// Represents any data readen from the input;
/// </summary>
struct InputEvent
{
    int address;
    int iValue;
};

/// <summary>
/// Queue in which contains the input signals read from standard input.
/// </summary>
class InputQueue
{
    //++Simulator specific.
public:
    /// <summary>
    /// Waits for a new message to arrive or the timeout expires.
    /// </summary>
    /// <param name="timeoutMS">Maximum time to wait, in milliseconds.</param>
    /// <returns></returns>
    bool waitForInput(int timeoutMS)
    {
        unique_lock<mutex> lock(m_mutex);
        if (m_queue.empty())
            m_msgEvent.wait_for(lock, chrono::milliseconds(timeoutMS));

        return !m_queue.empty();
    }

    /// <summary>
    /// Adds a new message to the queue, potentally awaking a client thread.
    /// </summary>
    /// <param name="msg"></param>
    void push(const InputEvent& msg)
    {
        unique_lock<mutex> lock(m_mutex);
        m_queue.push_back(msg);
        m_msgEvent.notify_one();
    }

    /// <summary>
    /// Removes the first message from the queue.
    /// </summary>
    /// <returns></returns>
    InputEvent pop()
    {
        unique_lock<mutex> lock(m_mutex);

        if (m_queue.empty())
            throw logic_error("Empty message queue");

        InputEvent result = m_queue.front();
        m_queue.pop_front();
        return result;
    }

    bool empty()
    {
        unique_lock<mutex> lock(m_mutex);

        return m_queue.empty();
    }

private:
    condition_variable		m_msgEvent;
    mutex					m_mutex;
    deque<InputEvent>		m_queue;
};

/// <summary>
/// Defines internal state flags for actor messages.
/// </summary>
enum SystemMsgFlags
{
    //++Generic
    MSGF_DELETED = 1
};

/// <summary>
/// Message which is sent when an input signal value changes.
/// </summary>
struct SignalValueMessage
{
    //++Specific
    SystemMsgHeader		h;
    int					iValue;

    SignalValueMessage(const ActorInputAddress& endPoint, int val)
    {
        h.flags = 0;
        h.endPoint = endPoint;
        h.msgLength = sizeof(*this);
        iValue = val;
    }
};


/// <summary>
/// Queues the messages sent to the actors.
/// </summary>
class SystemMsgQueue
{
    //++Generic
public:
    SystemMsgQueue(size_t capacity)
        : m_capacity(capacity)
    {
        if (capacity < 64)
            throw logic_error("System queue size too small. Minimum is 64 bytes");

        m_buffer = new byte[m_capacity];
    }

    ~SystemMsgQueue()
    {
        delete[]m_buffer;
    }

    /// <summary>
    /// Adds a new message to the queue.
    /// </summary>
    /// <param name="msg"></param>
    void push(const SystemMsgHeader* msg)
    {
        assert(msg != NULL);
        assert(msg->msgLength >= sizeof(SystemMsgHeader));

        int idx = alloc(msg->msgLength);
        memcpy(m_buffer + idx, msg, msg->msgLength);
        m_empty = false;
    }

    /// <summary>
    /// Checks if the queue is empty.
    /// </summary>
    /// <returns></returns>
    bool empty()const
    {
        return m_empty;
    }

    /// <summary>
    /// Gets a pointer to the head message.
    /// </summary>
    /// <returns></returns>
    const SystemMsgHeader* head()const
    {
        assert(!empty());
        return (SystemMsgHeader*)(m_buffer + m_readIdx);
    }

    /// <summary>
    /// Removes the head message, and any invalid messages up to the
    /// next valid message.
    /// </summary>
    void popHead()
    {
        auto msg = head();

        m_readIdx = (m_readIdx + msg->msgLength) % m_capacity;
        if (m_readIdx == m_writeIdx)
        {
            m_readIdx = m_writeIdx = 0;
            m_empty = true;
            return;
        }

        if (m_capacity - m_readIdx < sizeof(SystemMsgHeader))
            m_readIdx = 0;

        //Check for deleted messages
        msg = head();
        if (msg->flags & MSGF_DELETED)
            popHead();
    }

private:
    /// <summary>
    /// Allocates space in the queue for a new message.
    /// If the message does not fit, it throws a 'logic_error' exception.
    /// </summary>
    /// <param name="size">Size of the message, in bytes.</param>
    /// <returns>Index of the allocated message.</returns>
    size_t alloc(size_t size)
    {
        size_t result = m_writeIdx;

        if (m_writeIdx > m_readIdx)
        {
            const size_t available = m_capacity - m_writeIdx;

            if (size <= available)
                m_writeIdx = (m_writeIdx + size) % m_capacity;
            else
            {
                if (available >= sizeof(SystemMsgHeader))
                {
                    SystemMsgHeader*	header = (SystemMsgHeader*)(m_buffer + m_writeIdx);

                    header->flags = MSGF_DELETED;
                    header->msgLength = available;
                }
                m_writeIdx = 0;

                //Recursive call, once the chunk at the end has been skipped.
                return alloc(size);
            }
        }
        else
        {
            const size_t available = m_empty ? m_capacity : m_readIdx - m_writeIdx;

            if (size > available)
                throw logic_error("System queue overflow!");
            else
                m_writeIdx += size;
        }

        return result;
    }

private:
    typedef unsigned char byte;

    const size_t m_capacity;

    size_t	m_readIdx = 0;
    size_t	m_writeIdx = 0;
    bool	m_empty = true;

    byte*	m_buffer;
};


/// <summary>
/// Maps simulated hardware inputs with actor input messages.
/// </summary>
class InputMap
{
    //++Specific
public:
    ActorInputAddress find(int signalIndex)const
    {
        auto it = m_signalMap.find(signalIndex);

        if (it != m_signalMap.end())
            return it->second;
        else
            return ActorInputAddress{ NULL, NULL };
    }

    void set(int signalIndex, ActorInputAddress addr)
    {
        m_signalMap[signalIndex] = addr;
    }

private:
    map<int, ActorInputAddress>		m_signalMap;
};

typedef chrono::high_resolution_clock	SysClock;

bool startInputThread(InputQueue* pInput);
void inputThread(InputQueue* pInput);
int checkTimers();
bool readInputQueue(InputQueue* pInput);
bool dispatchActorMessages();
bool simFinished();

bool decodeInputCommand(const string& command, InputEvent* msg);
TimerInfo* scheduleTimer_int(TimerInfo* head, TimerInfo* timer);

/// <summary>
/// Global instance of the input map.
/// </summary>
static InputMap g_inputMap;

/// <summary>
/// Global instance of the system message queue.
/// The queue which holds the actor pending messages.
/// </summary>
static SystemMsgQueue g_systemQueue(2048);

/// <summary>
/// Head entry of the timer queue.
/// </summary>
static TimerInfo * g_headTimer = NULL;

SysClock::time_point g_timeBase;

/// <summary>
/// Fil-S simulator entry point.
/// </summary>
/// <returns></returns>
int main()
{
    //++mixed
    try
    {
        InputQueue inQueue;

        if (!startInputThread(&inQueue))
            return 101;

        initActors();

        while (!simFinished())
        {
            int sleepTimeMS = checkTimers();

            if (readInputQueue(&inQueue))
                sleepTimeMS = 0;

            if (dispatchActorMessages())
                sleepTimeMS = 0;

            if (sleepTimeMS > 0)
                inQueue.waitForInput(sleepTimeMS);
        }
        return 0;
    }
    catch (const exception ex)
    {
        cerr << "Runtime error: " << ex.what() << "\n";
        return 201;
    }
}

/// <summary>
/// Starts the thread which reads the standard input.
/// </summary>
/// <param name="pInput">Input queue pointer.</param>
/// <returns></returns>
bool startInputThread(InputQueue* pInput)
{
    //++Specific
    thread t(inputThread, pInput);

    t.detach();
    return true;
}

/// <summary>
/// Reads and decodes inputs from the process standard input.
/// </summary>
/// <param name="pInput"></param>
void inputThread(InputQueue* pInput)
{
    //++Specific
    while (!cin.eof())
    {
        string line;

        getline(cin, line);

        if (line.substr(0, 1) == "i")
        {
            InputEvent msg;
            if (decodeInputCommand(line, &msg))
                pInput->push(msg);
        }

        //In other cases, the input line is just ignored.
    }//while
}

/// <summary>
/// Checks system timers, and notifies the appropiate actors if necessary.
/// </summary>
/// <returns>Amount of time which is safe to sleep to service the next timer. In milliseconds.</returns>
int checkTimers()
{
    //++Generic
    int		maxSleepTime = 100;
    int		sleepTime = maxSleepTime;

    while (g_headTimer != NULL)
    {
        auto now = chrono::high_resolution_clock::now();
        auto elapsed = (int)chrono::duration_cast<chrono::milliseconds>(now - g_timeBase).count();

        if (g_headTimer->scheduledTime > elapsed)
        {
            sleepTime = max(g_headTimer->scheduledTime - (5 + elapsed), maxSleepTime);
            break;
        }
        else
        {
            auto			timerPtr = g_headTimer;
            SystemMsgHeader	msg;

            g_timeBase += chrono::milliseconds(timerPtr->scheduledTime);

            msg.endPoint = timerPtr->destInput;
            msg.flags = 0;
            msg.msgLength = sizeof(msg);

            g_systemQueue.push(&msg);
            g_headTimer = timerPtr->next;

            if (timerPtr->periodMS > 0)
            {
                timerPtr->scheduledTime = timerPtr->periodMS;
                g_headTimer = scheduleTimer_int(g_headTimer, timerPtr);
            }

            //If some timer has been executed, the return value shall be 0;
            maxSleepTime = sleepTime = 0;
        }
    }

    return sleepTime;
}


/// <summary>
/// Reads events from the input queue.
/// </summary>
/// <param name="pInput"></param>
/// <returns></returns>
bool readInputQueue(InputQueue* pInput)
{
    //++Specific
    bool	somethingDone = false;

    while (!pInput->empty())
    {
        auto event = pInput->pop();

        auto endPoint = g_inputMap.find(event.address);

        if (endPoint.actorPtr != NULL)
        {
            SignalValueMessage sv(endPoint, event.iValue);
            g_systemQueue.push(&sv.h);
            somethingDone = true;
        }
    }//while

    return somethingDone;
}

/// <summary>
/// Checks system queue and sends messages to the actors if needed.
/// </summary>
/// <returns></returns>
bool dispatchActorMessages()
{
    //++Generic
    bool msgSent = false;

    while (!g_systemQueue.empty())
    {
        auto msg = g_systemQueue.head();

        assert(msg->endPoint.inputPtr != NULL);

        msg->endPoint.inputPtr(msg->endPoint.actorPtr, (void*)(msg + 1));

        g_systemQueue.popHead();
        msgSent = true;
    }

    return msgSent;
}



static bool g_simFinished = false;

/// <summary>
/// Checks is the simulator should stop.
/// </summary>
/// <returns></returns>
bool simFinished()
{
    //++Generic
    return g_simFinished;
}

/// <summary>
/// Decodes an input line which seems to be an input signal value.
/// </summary>
/// <param name="line"></param>
/// <param name="msg"></param>
/// <returns></returns>
bool decodeInputCommand(const string& line, InputEvent* msg)
{
    //++Specific
    try
    {
        auto index = line.find('=');

        if (index == line.npos)
            return false;

        msg->address = stoi(line.substr(1, index - 1));
        msg->iValue = stoi(line.substr(index + 1));

        return true;
    }
    catch (const exception&)
    {
        return false;
    }
}

/// <summary>
/// Schedules a timer, placing it in the right spot at the time queue.
/// </summary>
/// <param name="head">Head of the timer queue</param>
/// <param name="timer">Timer to schedule.</param>
/// <returns>New head of the timer queue</returns>
static TimerInfo* scheduleTimer_int(TimerInfo* head, TimerInfo* timer)
{
    //++Generic
    if (head == NULL)
    {
        timer->next = NULL;
        return timer;
    }
    else if (head->scheduledTime > timer->scheduledTime)
    {
        head->scheduledTime -= timer->scheduledTime;
        timer->next = head;
        return timer;
    }
    else
    {
        timer->scheduledTime -= head->scheduledTime;

        head->next = scheduleTimer_int(head->next, timer);
        return head;
    }
}

/// <summary>
/// Schedules a timer, placing it in the right spot at the time queue.
/// </summary>
/// <param name="timer"></param>
void scheduleTimer(TimerInfo* timer)
{
    //++Generic
    if (g_headTimer == NULL)
        g_timeBase = SysClock::now();

    g_headTimer = scheduleTimer_int(g_headTimer, timer);
}

/// <summary>
/// Post a message to an actor.
/// </summary>
/// <param name="msg"></param>
void postMessage(const SystemMsgHeader* msg)
{
    //++Generic
    g_systemQueue.push(msg);
}

/// <summary>
/// Writes an output signal value.
/// </summary>
/// <param name="address"></param>
/// <param name="value"></param>
void outputSignalWrite(int address, int value)
{
    //++Specific
    cout << "o" << address << "=" << value << "\n";
}
