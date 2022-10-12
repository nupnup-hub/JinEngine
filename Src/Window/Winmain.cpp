#include"../Application/JApplication.h"
#include"../Utility/JCommonUtility.h" 
#include<stdio.h>
#include<string> 
using namespace JinEngine;
 
class ApplicationStarter
{
public:
	static void Start(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nShowCm)
	{
		Application::JApplication{ hInstance, pCmdLine }.Run();
	}
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nShowCm)
{  
	try
	{
		ApplicationStarter::Start(hInstance, hPrevInstance, pCmdLine, nShowCm); 
	}
	catch (const Core::JException& e)
	{ 		
		MessageBox(GetDesktopWindow(), e.what().c_str(), e.GetType().c_str(), MB_OK | MB_ICONEXCLAMATION);
	}
	catch (const std::exception& e)
	{		
		MessageBox(GetDesktopWindow(), JCUtil::CharToWChar(e.what()), L"Standard Exception", MB_OK | MB_ICONEXCLAMATION);
	}
	catch (...)
	{
		MessageBox(GetDesktopWindow(), L"No details available", L"Unknown Exception", MB_OK | MB_ICONEXCLAMATION);
	}	 
}

  

/*
* HWND ghMainWnd = 0;
HWND ghMainWnd02 = 0;
MSG key;

bool InitWindowsApplication(HINSTANCE instanceHandle, int show);

int Run();

LRESULT CALLBACK
WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM IParam);


LRESULT CALLBACK
WndProc02(HWND hWnd, UINT msg, WPARAM wParam, LPARAM IParam);
void Change(int n)
{
	if (n == 0)
	{
		ShowWindow(ghMainWnd02, SW_HIDE);
		UpdateWindow(ghMainWnd02);
		ShowWindow(ghMainWnd, SW_NORMAL);
		UpdateWindow(ghMainWnd);
	}
	else if (n == 1)
	{
		ShowWindow(ghMainWnd, SW_HIDE);
		UpdateWindow(ghMainWnd);
		ShowWindow(ghMainWnd02, SW_NORMAL);
		UpdateWindow(ghMainWnd02);
	}
}
int Run()
{
	MSG msg = { ghMainWnd };
	key = msg;

	BOOL bRet = 1;
	while ((bRet == GetMessage(&key, 0, 0, 0)) != 0)
	{
		if (bRet == -1)
		{
			MessageBox(0, L"GetMessage FAILED", L"Error", MB_OK);
			break;
		}
		else
		{
			TranslateMessage(&key);
			DispatchMessage(&key);
		}
	}
	return (int)msg.wParam;
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM IParam)
{
	switch (msg)
	{
	case WM_LBUTTONDOWN:
		MessageBox(0, L"Hello, jung", L"Hello", MB_OK);
		return 0;

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
			DestroyWindow(ghMainWnd);
		else if (wParam == VK_SPACE)
		{
			MessageBox(0, L"Change", L"->2", MB_OK);
			Change(1);
			key = { ghMainWnd02 };
		}
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	}

	return DefWindowProc(hWnd, msg, wParam, IParam);
}

LRESULT CALLBACK WndProc02(HWND hWnd, UINT msg, WPARAM wParam, LPARAM IParam)
{
	switch (msg)
	{
	case WM_LBUTTONDOWN:
		MessageBox(0, L"BYE, jung", L"BYE", MB_OK);
		return 0;

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
			DestroyWindow(ghMainWnd);
		else if (wParam == VK_SPACE)
		{
			MessageBox(0, L"Change", L"->1", MB_OK);
			Change(0);
			key = { ghMainWnd };
		}
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, msg, wParam, IParam);
}

*bool InitWindowsApplication(HINSTANCE instanceHandle, int show)
{
	WNDCLASS wc, wc2;

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = instanceHandle;
	wc.hIcon = LoadIcon(0, IDI_ApplicationLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = L"BasicWndClass";

	if (!RegisterClass(&wc))
	{
		MessageBox(0, L"RegisterClass FAILED", 0, 0);
		return false;
	}

	ghMainWnd = CreateWindow(
		L"BasicWndClass",
		L"Win32Basic",
		WS_OVERLApplicationEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		0,
		0,
		instanceHandle,
		0);

	if (ghMainWnd == 0)
	{
		MessageBox(0, L"CreateWindow FAILED", 0, 0);
		return false;
	}


	ShowWindow(ghMainWnd, show);
	UpdateWindow(ghMainWnd);

	wc2.style = CS_HREDRAW | CS_VREDRAW;
	wc2.lpfnWndProc = WndProc02;
	wc2.cbClsExtra = 0;
	wc2.cbWndExtra = 0;
	wc2.hInstance = instanceHandle;
	wc2.hIcon = LoadIcon(0, IDI_ApplicationLICATION);
	wc2.hCursor = LoadCursor(0, IDC_ARROW);
	wc2.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc2.lpszMenuName = 0;
	wc2.lpszClassName = L"BasicWndClass02";

	if (!RegisterClass(&wc2))
	{
		MessageBox(0, L"RegisterClass FAILED", 0, 0);
		return false;
	}

	ghMainWnd02 = CreateWindow(
		L"BasicWndClass02",
		L"Win32Basic",
		WS_OVERLApplicationEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		0,
		0,
		instanceHandle,
		0);

	if (ghMainWnd02 == 0)
	{
		MessageBox(0, L"CreateWindow FAILED", 0, 0);
		return false;
	}

	ShowWindow(ghMainWnd02, show);
	UpdateWindow(ghMainWnd02);
	return true;
}*/
