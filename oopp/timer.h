#pragma once

#include "oopp/precompiled.h"

namespace oopp
{

namespace timer
{

class timer
{
private:
    std::chrono::time_point<std::chrono::system_clock> t1;
    std::chrono::time_point<std::chrono::system_clock> t2;
    bool running;

public:
    timer () : running (false)
    {
        start ();
    }
    void start ()
    {
        t1 = std::chrono::system_clock::now ();
        running = true;
    }
    void stop ()
    {
        t2 = std::chrono::system_clock::now ();
        running = false;
    }
    double elapsed_ns()
    {
        using namespace std::chrono;
        return running
            ? duration_cast<nanoseconds> (system_clock::now () - t1).count ()
            : duration_cast<nanoseconds> (t2 - t1).count ();
    }
};

} // namespace timer

} // namespace oopp

