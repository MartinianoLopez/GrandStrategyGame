#pragma once
#include <chrono>
#include <iostream>
#include <string>
 
struct Timer {
    std::string name;
    std::chrono::steady_clock::time_point start;
    Timer(std::string n) : name(std::move(n)), start(std::chrono::steady_clock::now()) {}
    ~Timer() {
        auto ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
        std::cout << name << ": " << ms << " ms\n";
    }
};
 