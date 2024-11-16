#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <functional>
#include <condition_variable>

class TaskScheduler {
    std::queue<std::function<void()>> tasksToHandle;
    std::thread workerThread;
    std::mutex tasksLocker;
    std::condition_variable cv;
    bool acceptingTasks = true;
    bool shutdownRequested = false;

public:
    TaskScheduler();                     // Initializes the scheduler and starts the worker thread.
    ~TaskScheduler();                    // Ensures all tasks are completed and resources are cleaned up.
    void submitTask(std::function<void()> task); // Adds a task to the queue for asynchronous execution.
    void shutdown();                     // Stops accepting new tasks and shuts down gracefully.

private:
    void workingThread();
};

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

int main() {
    TaskScheduler scheduler;

    // Submit some tasks
    scheduler.submitTask([] { std::cout << "Task 1 is running\n"; });
    scheduler.submitTask([] { std::cout << "Task 2 is running\n"; });
    scheduler.submitTask([] { std::cout << "Task 3 is running\n"; });

    // Gracefully shutdown after all tasks are complete
    scheduler.shutdown();

    return 0;
}
