
#include "NativeWinApp/Window.h"

void TestNormal()
{
    NWA::Window window(800, 600, "TestNormal");

    while (true)
    {
        window.EventLoop();

        if (std::ranges::any_of(window.PopAllEvent(), [](const NWA::WindowEvent& event) -> bool { return event.type == NWA::WindowEvent::Type::Close; }))
            break;
    }
}

void TestStyleNoResize()
{
    NWA::Window window(800, 600, "TestStyleNoResize", NWA::WindowStyleNoResize);

    while (true)
    {
        window.EventLoop();

        if (std::ranges::any_of(window.PopAllEvent(), [](const NWA::WindowEvent& event) -> bool { return event.type == NWA::WindowEvent::Type::Close; }))
            break;
    }
}

int main()
{
    TestNormal();
    TestStyleNoResize();
}