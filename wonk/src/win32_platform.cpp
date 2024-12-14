

#include "platform.h"
#include "wonk_lib.h"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "wglext.h"

//      WINDOWS GLOBALS
static HWND window;

//      PLATFORM IMPLEMENTATIONS
LRESULT CALLBACK windows_windows_callback(HWND window, UINT msg, WPARAM wparam, LPARAM lparam)

{
    LRESULT result = 0;

    switch(msg)
    {
        case WM_CLOSE:
        {
            running = false;
            break;
        }

        default:
        {
            result = DefWindowProcA(window, msg, wparam, lparam);
        }
    }
    return result;
}

bool platform_create_window(int width, int height, char* title)
{
    HINSTANCE instance = GetModuleHandleA(0);
    WNDCLASSA wc = {};
    wc.hInstance = instance;
    wc.hIcon = LoadIcon(instance, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = title;
    wc.lpfnWndProc = windows_windows_callback;

    if(!RegisterClassA(&wc))
    {
        return false;
    }

    int dwStyle = WS_OVERLAPPEDWINDOW;

    PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = nullptr;
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;

    //                FAKE WINDOW INITIALIZING OPENGL
    {

        window = CreateWindowExA(0, title, title, dwStyle, 100, 100, width, height, NULL, NULL, instance, NULL);

        if(window == NULL)
        {
            SM_ASSERT(false, "Failed to create Windows Window")
            return false;
        }

        HDC fakeDC = GetDC(window);
        if(!fakeDC)
        {
            SM_ASSERT(false, "Failed to get HDC");
        return false;
        }

        PIXELFORMATDESCRIPTOR pfd = {0};
        pfd.nSize = sizeof(pfd);
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 32;
        pfd.cAlphaBits = 8;
        pfd.cDepthBits = 24;

        int pixelFormat = ChoosePixelFormat(fakeDC, &pfd);
        if(!pixelFormat)
        {
            SM_ASSERT(false, "Failed to choose pixel Format");
            return false;
        }

        if(!SetPixelFormat(fakeDC, pixelFormat, &pfd))
        {
            SM_ASSERT(false, "Failed to set pixel format");
            return false;
        }

        HGLRC fakeRC = wglCreateContext(fakeDC);
        if(!fakeRC)
        {
            SM_ASSERT(false, "failed to create Render context");
            return false;
        }

        if(!wglMakeCurrent(fakeDC, fakeRC))
        {
            SM_ASSERT(false, "failed to make current");
            return false;
        }

        wglChoosePixelFormatARB = 
        (PFNWGLCHOOSEPIXELFORMATARBPROC)platform_load_gl_function("wglChoosePixelFormatARB");
        wglCreateContextAttribsARB =
        (PFNWGLCREATECONTEXTATTRIBSARBPROC)platform_load_gl_function("wglCreateContextAttribsARB");

        if(!wglCreateContextAttribsARB || !wglChoosePixelFormatARB)
        {
            SM_ASSERT(false, "Failed to load OpenGl functions");
            return false;
        }

        wglMakeCurrent(fakeDC, 0);
        wglDeleteContext(fakeRC);
        ReleaseDC(window, fakeDC);

        DestroyWindow(window); 


    }

    //Actual opengl initianalization
    {

        {
            RECT borderRect = {};
            AdjustWindowRectEx(&borderRect, dwStyle, 0, 0);

            width += borderRect.right - borderRect.left;
            height += borderRect.bottom - borderRect.top;
        }

      window = CreateWindowExA(0, title, title, dwStyle, 100, 100, width, height, NULL, NULL, instance, NULL);

          if(window == NULL)
       {
              SM_ASSERT(false, "Failed to create Windows Window");
      return false;
       }

      HDC dc = GetDC(window);
      if(!dc)
       {
         SM_ASSERT(false, "Failed to get DC");
         return false;
      }
          #define GL_TRUE                           1

          const int pixelAttribs[] =
        {
          WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
          WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
          WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
          WGL_SWAP_METHOD_ARB,    WGL_SWAP_COPY_ARB,
          WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
          WGL_ACCELERATION_ARB,   WGL_FULL_ACCELERATION_ARB,
          WGL_COLOR_BITS_ARB,     32,
          WGL_ALPHA_BITS_ARB,     8,
          WGL_DEPTH_BITS_ARB,     24,
          0 // Terminate with 0, otherwise OpenGL will throw an Error!

        };

        UINT numPixelFormats;
       int pixelFormat = 0;
        if(!wglChoosePixelFormatARB(dc, pixelAttribs,
                                    0, // Float List
                                    1, // Max Formats
                                    &pixelFormat,
                                    &numPixelFormats))
        {
          SM_ASSERT(0, "Failed to wglChoosePixelFormatARB");
          return false;
        }

        PIXELFORMATDESCRIPTOR pfd = {0};
        DescribePixelFormat(dc, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

        if(!SetPixelFormat(dc, pixelFormat, &pfd))
        {
            SM_ASSERT(0, "Failed to SetPixelFormat");
            return true;
        }

            const int contextAttribs[] =
    {
      WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
      WGL_CONTEXT_MINOR_VERSION_ARB, 3,
      WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
      WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
      0 // Terminate the Array
    };

    HGLRC rc = wglCreateContextAttribsARB(dc, 0, contextAttribs);
    if(!rc)
    {
      SM_ASSERT(0, "Failed to crate Render Context for OpenGL");
      return false;
    }

    if(!wglMakeCurrent(dc, rc))
    {
      SM_ASSERT(0, "Faield to wglMakeCurrent");
      return false;
    }

    }

    ShowWindow(window, SW_SHOW);

    return true;
}

void platform_update_window()
    {
        MSG msg;

        while(PeekMessageA(&msg, window, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }



void* platform_load_gl_function(char* funName)
{
    PROC proc = wglGetProcAddress(funName);
    if(!proc)
    {
        static HMODULE openglDLL = LoadLibraryA("opengl132.dll");
        proc = GetProcAddress(openglDLL, funName);
        if(!proc)
        {
        SM_ASSERT(false, "Failed to load gl function %s", "glCreateProgram");
        return nullptr;
        }


    }
    return (void*)proc;
}