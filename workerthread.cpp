#include "workerthread.h"
#include <QDebug>

WorkerThread::WorkerThread()
{
    shouldQuit = false;
    shouldPause = false;
    workerThreadThread = nullptr;
}

WorkerThread::~WorkerThread()
{
    quit();
    while(running())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void WorkerThread::quit()
{
    shouldQuit = true;
    shouldPause = false;
}

void WorkerThread::start()
{
    if(workerThreadThread != nullptr)
    {
        throw std::exception();
    }

    workerThreadThread = new std::thread(&WorkerThread::run, this);
    workerThreadThread->detach();
}

bool WorkerThread::running()
{
    return (workerThreadThread != nullptr) && !shouldPause;
}

void WorkerThread::setPaused(bool pause)
{
    shouldPause = pause;
}

void WorkerThread::run()
{
    try
    {
        while(1)
        {
            loop();

            while(shouldPause)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }

            if(shouldQuit)
            {
                throw std::exception();
            }
        }
    }
    catch (std::exception &e)
    {
        qCritical("error WorkerThread");
        shouldQuit = false;
        delete workerThreadThread;
        workerThreadThread = nullptr;
    }
}
