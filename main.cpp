#include <cstdio>

#include <Windows.h>
//#include <gl/GL.h>

#include "glad/wgl.h"

#include "gpuinfo.h"

#ifdef __cplusplus
extern "C" {
#endif

// https://developer.download.nvidia.com/devzone/devcenter/gamegraphics/files/OptimusRenderingPolicies.pdf
__declspec(dllexport) DWORD NvOptimusEnablement = 0;

// https://community.amd.com/t5/archives-discussions/can-an-opengl-app-default-to-the-discrete-gpu-on-an-enduro/td-p/279440
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 0;


	void EnableAcceleration(bool bAcceleration)
	{
		if (bAcceleration)
		{
			NvOptimusEnablement = 1;
			AmdPowerXpressRequestHighPerformance = 1;
		}
		else
		{
			NvOptimusEnablement = 0;
			AmdPowerXpressRequestHighPerformance = 0;
		}
	}

#ifdef __cplusplus
}
#endif

// Enumerate cards
static void EnumerateDisplays()
{
    BOOL            FoundSecondaryDisp = FALSE;
    DWORD           DispNum = 0;
    DISPLAY_DEVICE  DisplayDevice;
    LONG            Result;
    TCHAR           szTemp[200];
    int             i = 0;
    DEVMODE   defaultMode;

    // initialize DisplayDevice
    ZeroMemory(&DisplayDevice, sizeof(DisplayDevice));
    DisplayDevice.cb = sizeof(DisplayDevice);

	printf("\n");
	printf("+-----+----------------+----------------------------------------+--------------------------------------------------+\n");
	printf("| No  | Device Name    | Device String                          | Device ID                                        |\n");
	printf("+-----+----------------+----------------------------------------+--------------------------------------------------+\n");

    // get all display devices
    while (EnumDisplayDevices(NULL, DispNum, &DisplayDevice, 0))
    {
        /*printf("Device Name : %s\n", DisplayDevice.DeviceName);
        printf("Device String : %s\n", DisplayDevice.DeviceString);
        printf("Device ID : %s\n", DisplayDevice.DeviceID);
		printf("Device Key : %s\n", DisplayDevice.DeviceKey);
		printf("---------------------\n");*/
		printf("| %-3d | %-14s | %-38s | %-48s |\n",
			DispNum + 1,
			DisplayDevice.DeviceName,
			DisplayDevice.DeviceString,
			DisplayDevice.DeviceID,
			DisplayDevice.DeviceKey);

        ZeroMemory(&defaultMode, sizeof(DEVMODE));
        defaultMode.dmSize = sizeof(DEVMODE);
        EnumDisplaySettings((LPSTR)DisplayDevice.DeviceName, ENUM_REGISTRY_SETTINGS, &defaultMode);


        DispNum++;
    } // end while for all display devices
	printf("+-----+----------------+----------------------------------------+--------------------------------------------------+\n");
}

// OpenGL window management
HGLRC           hRC = NULL;
HDC             hDC = NULL;
HWND            hWnd = NULL;
HINSTANCE       hInstance;

void DestroyGLWindow()
{
	if (hRC)
	{
		if (!wglMakeCurrent(NULL, NULL))
		{
			MessageBox(NULL, "Release Of DC And RC Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		}

		if (!wglDeleteContext(hRC))
		{
			MessageBox(NULL, "Release Rendering Context Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		}
		hRC = NULL;										// Set RC To NULL
	}

	if (hDC && !ReleaseDC(hWnd, hDC))					// Are We Able To Release The DC
	{
		MessageBox(NULL, "Release Device Context Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hDC = NULL;										// Set DC To NULL
	}

	if (hWnd && !DestroyWindow(hWnd))					// Are We Able To Destroy The Window?
	{
		MessageBox(NULL, "Could Not Release hWnd.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hWnd = NULL;										// Set hWnd To NULL
	}

	if (!UnregisterClass("OpenGL", hInstance))			// Are We Able To Unregister Class
	{
		MessageBox(NULL, "Could Not Unregister Class.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hInstance = NULL;									// Set hInstance To NULL
	}
}

LRESULT CALLBACK WndProc(HWND	hWnd,			// Handle For This Window
	UINT	uMsg,			// Message For This Window
	WPARAM	wParam,			// Additional Message Information
	LPARAM	lParam)			// Additional Message Information
{
	// Pass All Unhandled Messages To DefWindowProc
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

// Create OpenGL Context
bool CreateGLWindow(int width, int height, BYTE bits)
{
    GLuint		PixelFormat;			// Holds The Results After Searching For A Match
    WNDCLASS	wc;						// Windows Class Structure
    DWORD		dwExStyle;				// Window Extended Style
    DWORD		dwStyle;				// Window Style
    RECT		WindowRect;				// Grabs Rectangle Upper Left / Lower Right Values
    WindowRect.left = (long)0;			// Set Left Value To 0
    WindowRect.right = (long)width;		// Set Right Value To Requested Width
    WindowRect.top = (long)0;				// Set Top Value To 0
    WindowRect.bottom = (long)height;		// Set Bottom Value To Requested Height


    hInstance = GetModuleHandle(NULL);				// Grab An Instance For Our Window
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
    wc.lpfnWndProc = (WNDPROC)WndProc;					// WndProc Handles Messages
    wc.cbClsExtra = 0;									// No Extra Window Data
    wc.cbWndExtra = 0;									// No Extra Window Data
    wc.hInstance = hInstance;							// Set The Instance
    wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);			// Load The Default Icon
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
    wc.hbrBackground = NULL;									// No Background Required For GL
    wc.lpszMenuName = NULL;									// We Don't Want A Menu
    wc.lpszClassName = "OpenGL";								// Set The Class Name

    if (!RegisterClass(&wc))									// Attempt To Register The Window Class
    {
        return false;											// Return FALSE
    }

    dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// Window Extended Style
    dwStyle = WS_OVERLAPPEDWINDOW;							// Windows Style

    AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		// Adjust Window To True Requested Size

	// Create The Window
	if (!(hWnd = CreateWindowEx(dwExStyle,							// Extended Style For The Window
		"OpenGL",							// Class Name
		"title",								// Window Title
		dwStyle |							// Defined Window Style
		WS_CLIPSIBLINGS |					// Required Window Style
		WS_CLIPCHILDREN,					// Required Window Style
		0, 0,								// Window Position
		WindowRect.right - WindowRect.left,	// Calculate Window Width
		WindowRect.bottom - WindowRect.top,	// Calculate Window Height
		NULL,								// No Parent Window
		NULL,								// No Menu
		hInstance,							// Instance
		NULL)))								// Dont Pass Anything To WM_CREATE
	{
		DestroyGLWindow();								// Reset The Display
		return false;								// Return FALSE
	}

	static	PIXELFORMATDESCRIPTOR pfd =				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		bits,										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		16,											// 16Bit Z-Buffer (Depth Buffer)
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};

	hDC = GetDC(hWnd);
	if (hDC == nullptr)							// Did We Get A Device Context?
	{
		DestroyGLWindow();								// Reset The Display
		return false;								// Return FALSE
	}

	PixelFormat = ChoosePixelFormat(hDC, &pfd);
	if (PixelFormat == 0)	// Did Windows Find A Matching Pixel Format?
	{
		DestroyGLWindow();								// Reset The Display
		return false;								// Return FALSE
	}

	if (!SetPixelFormat(hDC, PixelFormat, &pfd))		// Are We Able To Set The Pixel Format?
	{
		DestroyGLWindow();								// Reset The Display
		return false;								// Return FALSE
	}

	if (!(hRC = wglCreateContext(hDC)))				// Are We Able To Get A Rendering Context?
	{
		DestroyGLWindow();								// Reset The Display
		return false;								// Return FALSE
	}

	if (!wglMakeCurrent(hDC, hRC))					// Try To Activate The Rendering Context
	{
		DestroyGLWindow();								// Reset The Display
		return false;								// Return FALSE
	}

	if (!gladLoaderLoadWGL(hDC))
	{
		DestroyGLWindow();
		return false;
	}

	if (!gladLoaderLoadGL())
	{
		DestroyGLWindow();
		return false;
	}

	return true;
}

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		printf("Usage :\n\t%s on|off\n", argv[0]);
		return 0;
	}

	EnumerateDisplays();

	bool bAcceleration = strcmp(argv[1], "on") == false;
	EnableAcceleration(bAcceleration);
	if (CreateGLWindow(640, 480, 16))
	{
		GPUInfo info;
		getInfo(info);
		displayInfo(info, true);

		DestroyGLWindow();
	}

	return 0;
}
