
#pragma once

//   PLATFORM GLOBALS
static bool running = true;

// PLATFORM FUNCTIONS
bool platform_create_window(int width, int height, char* title);
void platform_update_window();
void* platform_load_gl_function(char* funName);