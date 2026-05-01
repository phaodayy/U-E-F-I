#pragma once
#include <Windows.h>

// Setup overlay window (hijack Xbox Game Bar) + DirectX + run main loop
void SetupOverlay();
void RunMainLoop();

// Background exploit thread management
void StartExploitThread();
void StopExploitThread();
