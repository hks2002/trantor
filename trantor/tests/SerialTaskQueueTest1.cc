#include "trantor/logger/Logger.h"
#include "trantor/net/core/SerialTaskQueue.h"
#include <iostream>
#include <stdio.h>
#include <thread>

using namespace std::chrono_literals;
int main() {
  trantor::Logger::setLogLevel(trantor::Logger::kTrace);
  trantor::SerialTaskQueue queue1("test queue1");
  trantor::SerialTaskQueue queue2("");

  queue1.runTaskInQueue([&]() {
    for (int i = 0; i < 5; ++i) {
      std::this_thread::sleep_for(1s);
      printf("task(%s) i=%d\n", queue1.getName().c_str(), i);
    }
  });
  queue2.runTaskInQueue([&]() {
    for (int i = 0; i < 5; ++i) {
      std::this_thread::sleep_for(1s);
      printf("task(%s) i=%d\n", queue2.getName().c_str(), i);
    }
  });
  queue1.waitAllTasksFinished();
  queue2.waitAllTasksFinished();
}
