#include "TaskScheduler.h"

TaskScheduler::TaskScheduler()
    : workerThread(&TaskScheduler::workingThread, this) // Start the worker thread
{
}

TaskScheduler::~TaskScheduler() {
    shutdown();            // Signal shutdown
    workerThread.join();   // Ensure worker thread completes before destruction
}

void TaskScheduler::submitTask(std::function<void()> task) {
    std::lock_guard<std::mutex> guard(tasksLocker);
    if (acceptingTasks) {
        tasksToHandle.push(task);
        cv.notify_one(); // Notify the worker thread that a new task is available
    }
}

void TaskScheduler::shutdown() {
    {
        std::lock_guard<std::mutex> guard(tasksLocker);
        acceptingTasks = false;
        shutdownRequested = true;
    }
    cv.notify_all(); // Wake up the worker thread to allow it to exit
}

void TaskScheduler::workingThread() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(tasksLocker);
            cv.wait(lock, [this] { return !tasksToHandle.empty() || shutdownRequested; });

            if (shutdownRequested && tasksToHandle.empty()) {
                break; // Exit the loop if shutdown is requested and no tasks remain
            }

            task = tasksToHandle.front();
            tasksToHandle.pop();
        }

        // Execute the task outside of the lock to avoid holding the lock during task execution
        if (task) {
            task();
        }
    }
}
