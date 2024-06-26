
#include "NativeWinApp/Window.h"

void TestNormal()
{
    NWA::Window window(800, 600, "TestNormal");

    while (true)
    {
        window.EventLoop();

        bool shouldWindowClose = false;
        while (window.HasEvent())
        {
            auto event = window.PopEvent();
            if (event.type == NWA::WindowEvent::Type::Close)
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
    NWA::Window window(800, 600, "TestStyleNoResize", NWA::WindowStyleNoResize);

    while (true)
    {
        window.EventLoop();

        bool shouldWindowClose = false;
        while (window.HasEvent())
        {
            auto event = window.PopEvent();
            if (event.type == NWA::WindowEvent::Type::Close)
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

int main()
{
    TestNormal();
    TestStyleNoResize();
}