#pragma once
#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <functional>
#include <condition_variable>

class TaskScheduler {
    std::queue<std::function<void()>> tasksToHandle;
    std::vector<std::thread> workerThreads;
    std::mutex tasksLocker;
    std::condition_variable cv;
    bool acceptingTasks = true;
    bool shutdownRequested = false;

public:
    TaskScheduler(size_t numWorkers);                     // Initializes the scheduler and starts the worker thread.
    ~TaskScheduler();                    // Ensures all tasks are completed and resources are cleaned up.
    void submitTask(std::function<void()> task); // Adds a task to the queue for asynchronous execution.
    void shutdown();                     // Stops accepting new tasks and shuts down gracefully.

private:
    void workingThread();
};
