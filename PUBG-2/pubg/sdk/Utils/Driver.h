#pragma once
#include "../memory.hpp"

class Driver
{
public:
    static void Move(int x, int y)
    {
        PubgMemory::MoveMouse(x, y);
    }

    static void Click()
    {
        Down();
        Up();
    }

    static void Down()
    {
        PubgMemory::MoveMouse(0, 0, 0x0001); // MOUSE_LEFT_BUTTON_DOWN
    }

    static void Up()
    {
        PubgMemory::MoveMouse(0, 0, 0x0002); // MOUSE_LEFT_BUTTON_UP
    }
};
