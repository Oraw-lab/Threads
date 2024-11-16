#include <iostream>
#include "TaskScheduler.h"

int main() {
    TaskScheduler scheduler(4);

    // Submit some tasks
    for (int i = 0; i < 10; ++i) {
        scheduler.submitTask([i]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            std::cout << "Task " << i << " executed by thread "
                << std::this_thread::get_id() << "\n";
            });
    }

    // Give the scheduler some time to process tasks
    std::this_thread::sleep_for(std::chrono::seconds(2));

    scheduler.shutdown(); // Clean shutdown

    return 0;
}
