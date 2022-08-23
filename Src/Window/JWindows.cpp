#include"JWindows.h"  
#include"../../../Lib/imgui/imgui.h"
#include"../Utility/JD3DUtility.h" 
#include"../Core/Guid/GuidCreator.h"
#include"../Core/Exception/JExceptionMacro.h"
#include"resource.h"
#include<tchar.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
namespace JinEngine
{
	namespace Window
	{
		struct FrameContext
		{
			ID3D12CommandAllocator* CommandAllocator;
			UINT64                  FenceValue;
		};

#define UM_CHECKSTATECHANGE (WM_USER + 100)
		using namespace std;

		RECT JWindowImpl::GetWindowR()const noexcept
		{
			RECT rect;
			GetWindowRect(hwnd, &rect);
			return rect;
		}
		RECT JWindowImpl::GetPreWindowR()const noexcept
		{
			return preWindowRect;
		}
		RECT JWindowImpl::GetClientR()const noexcept
		{
			RECT rect;
			GetClientRect(hwnd, &rect);
			return rect;
		}
		RECT JWindowImpl::GetPreClientR()const noexcept
		{
			return preClinetRect;
		}
		int JWindowImpl::GetDisplayWidth()const noexcept
		{
			return GetSystemMetrics(SM_CXSCREEN);
		}
		int JWindowImpl::GetDisplayHeight()const noexcept
		{
			return GetSystemMetrics(SM_CYSCREEN);
		}
		int JWindowImpl::GetMinWidth()const noexcept
		{
			return minWidth;
		}
		int JWindowImpl::GetMinHeight()const noexcept
		{
			return minHeight;
		}
		int JWindowImpl::GetWindowPositionX()const noexcept
		{
			return preWindowRect.left;
		}
		int JWindowImpl::GetWindowPositionY()const noexcept
		{
			return preWindowRect.top;
		}
		int JWindowImpl::GetWindowWidth()const noexcept
		{
			return preWindowRect.right - preWindowRect.left;
		}
		int JWindowImpl::GetWindowHeight()const noexcept
		{
			return preWindowRect.bottom - preWindowRect.top;
		}
		int JWindowImpl::GetClientPositionX()const noexcept
		{
			return preClinetRect.left;
		}
		int JWindowImpl::GetClientPositionY()const noexcept
		{
			return preClinetRect.top;
		}
		int JWindowImpl::GetClientWidth()const noexcept
		{
			return preClinetRect.right - preClinetRect.left;
		}
		int JWindowImpl::GetClientHeight()const noexcept
		{
			return preClinetRect.bottom - preClinetRect.top;
		}
		JWindowHandleInterface* JWindowImpl::HandleInterface() 
		{
			return this;
		}
		JWindowImpl::JEventInterface* JWindowImpl::EvInterface()
		{
			return this;
		}
		JWindowAppInterface* JWindowImpl::AppInterface()
		{
			return this;
		}
		void JWindowImpl::Initialize(HINSTANCE hInstance)
		{
			hInst = hInstance;
		}
		void JWindowImpl::OpenWindow()
		{
			int displayWidth = GetSystemMetrics(SM_CXSCREEN);
			int displayHeight = GetSystemMetrics(SM_CYSCREEN);

			RegisterWindowClass();

			int windowX = 0;
			int windowY = 0;
			int windowWidth = displayWidth;
			int windowHeight = displayHeight;

			hwnd = CreateWindowEx(
				WS_EX_ACCEPTFILES,
				windowClassName.c_str(), windowName.c_str(),
				WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
				windowX, windowY, windowWidth, windowHeight,
				nullptr, nullptr, hInst, this);

			if (hwnd == nullptr)
				ThrowIfFailedW(false);
			// newly created windows start off as hidden
			ShowWindow(hwnd, SW_SHOW);
			ThrowIfFailedW(UpdateWindow(hwnd));

			//SHFullScreen(m_hWnd, SHFS_HIDETASKBAR);		 
			//pGfx = std::make_unique<Graphics>(hWnd, width, height);*/
			// register mouse raw input device
			RAWINPUTDEVICE rid;
			rid.usUsagePage = 0x01; // mouse page
			rid.usUsage = 0x02; // mouse usage
			rid.dwFlags = 0;
			rid.hwndTarget = nullptr;
			 
			ThrowIfFailedW(RegisterRawInputDevices(&rid, 1, sizeof(rid)))
		}
		void JWindowImpl::CloseWindow()
		{
			DestroyWindow(hwnd);
		}
		void JWindowImpl::SetProjectSelectorWindow()
		{
			LONG displayWidth = GetSystemMetrics(SM_CXSCREEN);
			LONG displayHeight = GetSystemMetrics(SM_CYSCREEN);

			int widthHalf = (int)(displayWidth * 0.5f);
			int heightHalf = (int)(displayHeight * 0.5f);
			int widthOffset = (int)(displayWidth * 0.25f);
			int heightOffset = (int)(displayHeight * 0.4f);

			int left = widthHalf - widthOffset;
			int right = widthHalf + widthOffset;
			int top = heightHalf - heightOffset;
			int bottom = heightHalf + heightOffset;

			int windowX = left;
			int windowY = top;
			int windowWidth = right - left;
			int windowHeight = bottom - top;

			SetWindowPos(hwnd, nullptr, windowX, windowY, windowWidth, windowHeight, 0);
			ThrowIfFailedW(UpdateWindow(hwnd));
		}
		void JWindowImpl::SetEngineWindow()
		{
			int displayWidth = GetSystemMetrics(SM_CXSCREEN);
			int displayHeight = GetSystemMetrics(SM_CYSCREEN);

			int left = 0;
			int right = displayWidth;
			int top = 0;
			int bottom = displayHeight;

			int windowX = left;
			int windowY = top;
			int windowWidth = right - left;
			int windowHeight = bottom - top;

			SetWindowPos(hwnd, nullptr, windowX, windowY, windowWidth, windowHeight, 0);
			ThrowIfFailedW(UpdateWindow(hwnd));
		}
		std::optional<int> JWindowImpl::ProcessMessages()
		{
			MSG msg = { 0 };
			while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT)
					return (int)msg.wParam;
				// TranslateMessage will post auxilliary WM_CHAR messages from key msgs
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			return {};
		}	 
		HWND JWindowImpl::GetHandle()const noexcept
		{
			return hwnd;
		}
		LRESULT CALLBACK JWindowImpl::HandleMsgSetup(HWND hwnd, uint msg, WPARAM wParam, LPARAM lParam)
		{
			// use create parameter passed in from CreateWindow() to store window class pointer at WinAPI side
			if (msg == WM_NCCREATE)
			{
				// extract ptr to window class from creation data
				const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
				JWindowImpl* const pWnd = static_cast<JWindowImpl*>(pCreate->lpCreateParams);
				// set WinAPI-managed user data to store ptr to window instance
				SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));
				// set message libc to normal (non-setup) handler now that setup is finished
				SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&JWindowImpl::HandleMsgThunk));
				// forward message to window instance handler
				return pWnd->HandleMsg(hwnd, msg, wParam, lParam);
			}
			// if we get a message before the WM_NCCREATE message, handle with default handler
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
		LRESULT CALLBACK JWindowImpl::HandleMsgThunk(HWND hwnd, uint msg, WPARAM wParam, LPARAM lParam)
		{
			JWindowImpl* const pWnd = reinterpret_cast<JWindowImpl*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
			// forward message to window instance handler
			return pWnd->HandleMsg(hwnd, msg, wParam, lParam);
		}
		LRESULT JWindowImpl::HandleMsg(HWND hwnd, uint msg, WPARAM wParam, LPARAM lParam)
		{
			if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
				return true;

			//if (inputManager.HasUpkey())
				//inputManager.ReleaseUpKey()

			switch (msg)
			{
			case WM_CLOSE:
			{
				ImGui_ImplDX12_Shutdown();
				ImGui_ImplWin32_Shutdown();
				ImGui::DestroyContext();
				DestroyWindow(JWindow::Instance().GetHandle());
				return 0;
			}
			case WM_DESTROY:
			{
				PostQuitMessage(0);
				return 0;
			}
			case WM_QUIT:
				break;
				return 0;
			case WM_SIZING:
			{
				int newWidth = ((RECT*)lParam)->right - ((RECT*)lParam)->left;
				int newHeight = ((RECT*)lParam)->bottom - ((RECT*)lParam)->top;
				int minWidth = JWindow::Instance().GetMinWidth();
				int minHeight = JWindow::Instance().GetMinHeight();

				if (newWidth <= minWidth || newHeight <= minHeight)
				{
					RECT preWindowRect = JWindow::Instance().GetPreWindowR();
					((RECT*)lParam)->left = preWindowRect.left;
					((RECT*)lParam)->right = preWindowRect.right;
					((RECT*)lParam)->top = preWindowRect.top;
					((RECT*)lParam)->bottom = preWindowRect.bottom;
				}
				if (newWidth > GetSystemMetrics(SM_CXSCREEN) || newHeight > GetSystemMetrics(SM_CYSCREEN))
				{
					RECT preWindowRect = JWindow::Instance().GetPreClientR();
					((RECT*)lParam)->left = preWindowRect.left;
					((RECT*)lParam)->right = preWindowRect.right;
					((RECT*)lParam)->top = preWindowRect.top;
					((RECT*)lParam)->bottom = preWindowRect.bottom;
				}
				return TRUE;
			}
			case WM_SIZE:
			{
				JWindow::Instance().Resize(wParam);
				return 0;
			}
			case WM_NCDESTROY:
				break;
			case WM_INPUT:
				break;
			case WM_KEYDOWN:
				//inputManager.PressKey(wParam);
				break;
			case WM_KEYUP:
				//inputManager.UpKey(wParam);
				break;
			}
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
		void JWindowImpl::Resize(WPARAM wParam)
		{
			GetWindowRect(hwnd, &preWindowRect);
			GetClientRect(hwnd, &preClinetRect);
			if (wParam != SIZE_MINIMIZED)
				NotifyEvent(guid, J_WINDOW_EVENT::WINDOW_RESIZE);
		}
		void JWindowImpl::RegisterWindowClass()
		{
			wc = { 0 };
			HICON icon;

			icon = (HICON)::LoadImage(hInst,
				MAKEINTRESOURCE(IDI_ICON1),
				IMAGE_ICON, 128, 128,
				LR_DEFAULTCOLOR);

			wc.cbSize = sizeof(wc);
			wc.style = CS_OWNDC;
			wc.lpfnWndProc = HandleMsgSetup;
			wc.cbClsExtra = 0;
			wc.cbWndExtra = 0;
			wc.hInstance = hInst;
			wc.hIcon = icon;
			wc.hIconSm = icon;
			wc.hCursor = nullptr;
			wc.hbrBackground = (HBRUSH)3;
			wc.lpszMenuName = nullptr;
			wc.lpszClassName = windowClassName.c_str();

			if (!RegisterClassEx(&wc))
			{
				MessageBox(NULL, L"JWindowImpl Registration Failed!", L"Error!",
					MB_ICONEXCLAMATION | MB_OK);
			}
		}
		void JWindowImpl::RegistEvCallable()
		{
			auto lam = [](const size_t& a, const size_t& b) {return a == b; };
			RegistIdenCompareCallable(lam);
		}
		JWindowImpl::JWindowImpl()
			:guid(Core::MakeGuid()),
			minWidth((int)(GetSystemMetrics(SM_CXSCREEN) * 0.4f)),
			minHeight((int)(GetSystemMetrics(SM_CYSCREEN) * 0.4f)),
			hwnd(0)
		{
			RegistEvCallable();
		}
		JWindowImpl::~JWindowImpl()
		{
			//DestroyWindow(hwnd);
		}
	}
}