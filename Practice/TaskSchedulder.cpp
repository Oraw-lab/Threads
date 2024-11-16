#include "TaskScheduler.h"

TaskScheduler::TaskScheduler(size_t numWorkers)
    : acceptingTasks(true), shutdownRequested(false) // Start the worker thread
{
    for (size_t i = 0; i < numWorkers; ++i) {
        workerThreads.emplace_back(&TaskScheduler::workingThread, this);
    }
}

TaskScheduler::~TaskScheduler() {
    shutdown();            // Signal shutdown
    for (std::thread& worker : workerThreads) {
        if (worker.joinable()) {
            worker.join(); // Ensure all worker threads complete
        }
    } // Ensure worker thread completes before destruction
}

void TaskScheduler::submitTask(std::function<void()> task) {
    {
        std::lock_guard<std::mutex> guard(tasksLocker);
        if (acceptingTasks) {
            tasksToHandle.push(std::move(task));
        }
    }
    cv.notify_one(); // Notify one waiting worker thread
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
            // Wait for tasks or shutdown signal
            cv.wait(lock, [this] { return !tasksToHandle.empty() || shutdownRequested; });

            if (shutdownRequested && tasksToHandle.empty()) {
                break; // Exit if shutdown is requested and no tasks remain
            }

            if (!tasksToHandle.empty()) {
                task = std::move(tasksToHandle.front());
                tasksToHandle.pop();
            }
        }

        // Execute the task outside of the lock to avoid holding it during task execution
        if (task) {
            task();
        }
    }
}
