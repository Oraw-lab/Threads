#include <iostream>
#include "TaskScheduler.h"

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
