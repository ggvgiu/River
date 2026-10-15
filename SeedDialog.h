#pragma once
#include "framework.h"
#include <cstdint>

// Only updates seed on OK; Cancel and closing the dialog leave it unchanged.
bool RequestSeed(HWND owner, HINSTANCE instance, uint16_t& seed);
