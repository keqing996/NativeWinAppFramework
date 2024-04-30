

#include "NativeWinApp/Window.h"
#include "Infra/Timer.hpp"

void TestNormal()
{
    Infra::Window window(800, 600, "TestNormal");

    while (true)
    {
        window.EventLoop();

        bool shouldWindowClose = false;
        while (window.HasEvent())
        {
            auto event = window.PopEvent();
            if (event.type == Infra::WindowEvent::Type::Close)
            {
                shouldWindowClose = true;
                break;
            }
        }

        if (shouldWindowClose)
        {
            break;
        }
    }
}

void TestStyleNoResize()
{
    Infra::Window window(800, 600, "TestStyleNoResize", (int)Infra::WindowStyle::HaveTitleBar | (int)Infra::WindowStyle::HaveClose);

    while (true)
    {
        window.EventLoop();

        bool shouldWindowClose = false;
        while (window.HasEvent())
        {
            auto event = window.PopEvent();
            if (event.type == Infra::WindowEvent::Type::Close)
            {
                shouldWindowClose = true;
                break;
            }
        }

        if (shouldWindowClose)
        {
            break;
        }
    }
}

void TestStyleNoClose()
{
    Infra::Window window(800, 600, "TestStyleNoClose", (int)Infra::WindowStyle::HaveTitleBar);

    Infra::Timer<Infra::TimePrecision::Seconds> timer;
    timer.SetNow();
    while (true)
    {
        window.EventLoop();

        auto interval = timer.GetInterval();
        if (interval > 3)
            break;
    }
}

int main()
{
    TestNormal();
    TestStyleNoResize();
    TestStyleNoClose();
}