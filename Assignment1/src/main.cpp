#include <iostream>
#include <cstdlib>
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#include <fstream>
#include <sstream>
#include <vector>
#include "initialization.h"
#include "utils.h"
#include "A1task1.h"
#include "A1task2.h"
#include "renderdoc.h"

// DON'T CHANGE IT!

std::string errorStr(std::string task, bool error) {
    std::string t = "TASK" + task;
    return error ? t + ": ERRORS! " : t + ": PASSED. ";
}

void run_A1_task1(A1_Task1 task) {
    std::stringstream stream;

    task.prepare(1024 * 1024 + 31);
    stream << "error; dx; dy; dz; vecsize; timing" << std::endl;

    bool hasErrors = false;
    for (int i = 32; i < 1024; i = i << 1) {
        task.compute(i, 1, 1);
        bool error = !task.checkDefaultValues();
        hasErrors |= error;
        stream << error << "; " << i << "; " << 1 << "; " << 1 << "; " << 1024 * 1024 << "; " << task.mstime <<
                std::endl;
    }

    task.cleanup();

    std::cout << errorStr("1.0", hasErrors) << std::endl << stream.str() << std::endl;
}

void run_A1_task2(A1_Task2 A1task2) {
    A1task2.prepare(3200, 4000);

    A1task2.compute(32, 16, 1, "matrixRotNaive");

    bool hasErrors = false;

    hasErrors = !A1task2.checkDefaultValues();

    std::cout << errorStr("2.1", hasErrors) << "took: " << A1task2.mstime << " ms" << std::endl;

    A1task2.cleanup();
}

void run_A1_task2_opti(A1_Task2 A1task2) {
    A1task2.prepare(3200, 4000);

    A1task2.compute(32, 16, 1, "matrixRotOpti");

    bool hasErrors = false;

    hasErrors = !A1task2.checkDefaultValues();
    std::cout << errorStr("2.2", hasErrors) << "took: " << A1task2.mstime << " ms" << std::endl;

    A1task2.cleanup();
}

int main() {
    setenv("RADV_PERFTEST", "cswave32", 0);

    try {
        //AppResources is a struct = Instance, Physical Device, Queues, CommandPool and QueryPool are some of the members
        AppResources app;

        //Initialises the app with instance, Physical device , Compute & Transfer Queues
        initApp(app);

        renderdoc::initialize();
        renderdoc::startCapture();

        A1_Task1 task(app);
        run_A1_task1(task);

        A1_Task2 A1task2(app);
        run_A1_task2(A1task2);

        A1_Task2 A1task2_opti(app);
        run_A1_task2_opti(A1task2_opti);

        renderdoc::endCapture();

        app.destroy();
    } catch (vk::SystemError& err) {
        std::cout << "vk::SystemError: " << err.what() << std::endl;
        exit(-1);
    }
    catch (std::exception& err) {
        std::cout << "std::exception: " << err.what() << std::endl;
        exit(-1);
    }
    catch (...) {
        std::cout << "unknown error\n";
        exit(-1);
    }
    return EXIT_SUCCESS;
}
