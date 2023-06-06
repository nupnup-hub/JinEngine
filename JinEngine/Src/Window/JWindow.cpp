#include"JWindow.h"  
#include"JWindowPrivate.h"  
#include"JInputManager.h"  
#include"../Core/JDataType.h"
#include"../Core/JEngineInfo.h"
#include"../Core/Guid/JGuidCreator.h"
#include"../Core/Exception/JExceptionMacro.h"
#include"../Core/Exception/JException.h"
#include"../Core/Singleton/JSingletonHolder.h" 
#include"../Utility/JCommonUtility.h"
#include"../Utility/JD3DUtility.h" 
#include"../../resource.h"
#include"../../../Lib/imgui/imgui.h"
#include<tchar.h>
#include<fstream>
#include<shellapi.h>
#include<ShlObj_core.h> 
#include<functional>
#include<optional>
#pragma comment(lib, "shell32") 

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
namespace JinEngine
{
	namespace Window
	{
#define WIND_CLASS_NAME L"JinEngineClass"
#define UM_CHECKSTATECHANGE (WM_USER + 100)

		using WindowCloseConfirmF = JWindowPrivate::ApplicationAccess::CloseConfirmF;
		class JWindowImpl : public Core::JEventManager<size_t, J_WINDOW_EVENT>
		{
		public: 
			const size_t guid;
			JInputManager inputManager;
			WNDCLASSEX wc;
			HINSTANCE hInst;
			RECT preWindowRect;
			RECT preClinetRect;
			HWND hwnd;
			bool enableCursor = true;
		public:
			std::unique_ptr<WindowCloseConfirmF> closeConfirmF;
		public:
			JWindowImpl()
				:JEventManager([](const size_t& a, const size_t& b) {return a == b; }),
				guid(Core::MakeGuid()),
				hwnd(0)
			{}
			~JWindowImpl(){}
		public:
			HWND GetHandle()const noexcept
			{
				return hwnd;
			}
			RECT GetWindowR()const noexcept
			{
				RECT rect;
				GetWindowRect(hwnd, &rect);
				return rect;
			}
			RECT GetPreWindowR()const noexcept
			{
				return preWindowRect;
			}
			RECT GetClientR()const noexcept
			{
				RECT rect;
				GetClientRect(hwnd, &rect);
				return rect;
			}
			RECT GetPreClientR()const noexcept
			{
				return preClinetRect;
			}
			int GetDisplayWidth()const noexcept
			{
				return GetSystemMetrics(SM_CXFULLSCREEN);
			}
			int GetDisplayHeight()const noexcept
			{
				return GetSystemMetrics(SM_CYFULLSCREEN);
			}
			int GetMinWidth()const noexcept
			{
				return GetDisplayWidth() * 0.4f;
			}
			int GetMinHeight()const noexcept
			{
				return GetDisplayHeight() * 0.4f;
			}
			int GetWindowPositionX()const noexcept
			{
				return preWindowRect.left;
			}
			int GetWindowPositionY()const noexcept
			{
				return preWindowRect.top;
			}
			int GetWindowWidth()const noexcept
			{
				return preWindowRect.right - preWindowRect.left;
			}
			int GetWindowHeight()const noexcept
			{
				return preWindowRect.bottom - preWindowRect.top;
			}
			JVector2<int> GetClientPos()const noexcept
			{
				return JVector2<int>(preClinetRect.left, preClinetRect.top);
			}
			int GetClientPositionX()const noexcept
			{
				return preClinetRect.left;
			}
			int GetClientPositionY()const noexcept
			{
				return preClinetRect.top;
			}
			JVector2<int> GetClientSize()const noexcept
			{
				return JVector2<int>(GetClientWidth(), GetClientHeight());
			}
			int GetClientWidth()const noexcept
			{
				return preClinetRect.right - preClinetRect.left;
			}
			int GetClientHeight()const noexcept
			{
				return preClinetRect.bottom - preClinetRect.top;
			}
		public:
			bool IsFullScreen()const noexcept
			{
				return IsZoomed(hwnd);
			}
			bool HasStorageSpace(const std::wstring& dirPath, size_t capacity)const noexcept
			{
				ULARGE_INTEGER uliAvailable;
				ULARGE_INTEGER uliTotal;
				ULARGE_INTEGER uliFree;

				if (GetDiskFreeSpaceExA(JCUtil::WstrToU8Str(dirPath).c_str(), &uliAvailable, &uliTotal, &uliFree))
				{
					if (uliAvailable.QuadPart > capacity)
						return true;
					else
						return false;
				}
				return false;
			}
		public:
			bool SelectDirectory(std::wstring& dirPath, const std::wstring& guide)noexcept
			{
				BROWSEINFO   browserInfo;
				LPITEMIDLIST  idl;
				TCHAR path[MAX_PATH] = { 0, };
				ZeroMemory(&browserInfo, sizeof(BROWSEINFO));
				browserInfo.hwndOwner = hwnd;
				browserInfo.pszDisplayName = path;
				browserInfo.lpszTitle = guide.c_str();
				browserInfo.ulFlags = BIF_EDITBOX | BIF_USENEWUI | 0x0040;

				idl = SHBrowseForFolder(&browserInfo);
				if (idl)
				{
					SHGetPathFromIDList(idl, path);
					dirPath = path;
					return true;
				}
				else
					return false;
			}
			bool SelectFile(std::wstring& filePath, const std::wstring& guide)noexcept
			{
				BROWSEINFO   browserInfo;
				LPITEMIDLIST  idl;
				TCHAR path[MAX_PATH] = { 0, };
				ZeroMemory(&browserInfo, sizeof(BROWSEINFO));
				browserInfo.hwndOwner = hwnd;
				browserInfo.pszDisplayName = path;
				browserInfo.lpszTitle = guide.c_str();
				browserInfo.ulFlags = BIF_EDITBOX | BIF_USENEWUI | BIF_BROWSEINCLUDEFILES | 0x0040;

				idl = SHBrowseForFolder(&browserInfo);
				if (idl)
				{
					SHGetPathFromIDList(idl, path);
					filePath = path;
					return true;
				}
				else
					return false;

			}
		public:
			JEventInterface* EvInterface()
			{
				return this;
			}
		public:
			void Initialize(HINSTANCE hInstance, std::unique_ptr<WindowCloseConfirmF>&& newCloseConfirm)
			{
				hInst = hInstance;
				closeConfirmF = std::move(newCloseConfirm);
			}
			void OpenProjecSelectorWindow()
			{
				LONG displayWidth = GetSystemMetrics(SM_CXFULLSCREEN);
				LONG displayHeight = GetSystemMetrics(SM_CYFULLSCREEN);

				int widthHalf = (int)(displayWidth * 0.5f);
				int heightHalf = (int)(displayHeight * 0.5f);
				int widthOffset = (int)(displayWidth * 0.35f);
				int heightOffset = (int)(displayHeight * 0.4f);

				int left = widthHalf - widthOffset;
				int right = widthHalf + widthOffset;
				int top = heightHalf - heightOffset;
				int bottom = heightHalf + heightOffset;

				int windowX = left;
				int windowY = top;
				int windowWidth = right - left;
				int windowHeight = bottom - top;

				RegisterWindowClass();

				hwnd = CreateWindowEx(
					WS_EX_ACCEPTFILES,
					WIND_CLASS_NAME, ENGINE_WNAME,
					WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
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
			void OpenEngineWindow()
			{
				RECT workArea;
				SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);

				int left = workArea.left;
				int right = workArea.right;
				int top = workArea.top;
				int bottom = workArea.bottom;

				int windowX = left;
				int windowY = top;
				int windowWidth = right - left;
				int windowHeight = bottom - top;

				RegisterWindowClass();

				hwnd = CreateWindowEx(
					WS_EX_ACCEPTFILES,
					WIND_CLASS_NAME, ENGINE_WNAME,
					WS_CAPTION | WS_THICKFRAME | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_MAXIMIZE,
					windowX, windowY, windowWidth, windowHeight,
					nullptr, nullptr, hInst, this);

				HMONITOR hmon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
				MONITORINFO moninfo;
				moninfo.cbSize = sizeof(moninfo);
				GetMonitorInfo(hmon, &moninfo);

				SetWindowPos(hwnd, 0, moninfo.rcWork.left, moninfo.rcWork.top,
					moninfo.rcWork.right, moninfo.rcWork.bottom, SWP_NOZORDER);

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

				ThrowIfFailedW(RegisterRawInputDevices(&rid, 1, sizeof(rid)));
			}
			void CloseWindow()
			{
				NotifyEvent(guid, J_WINDOW_EVENT::WINDOW_CLOSE);
				ClearEvent();
				DestroyWindow(GetHandle());
				UnregisterClass(WIND_CLASS_NAME, hInst);
			}
		public:
			std::optional<int> ProcessMessages()
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
			void RegisterWindowClass()
			{
				wc = { 0 };
				/*HICON icon;

				icon = (HICON)::LoadImage(hInst,
					MAKEINTRESOURCE(IDI_ICON1),
					IMAGE_ICON, 128, 128,
					LR_DEFAULTCOLOR);
				 */
				wc.cbSize = sizeof(wc);
				wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
				//wc.style = CS_HREDRAW | CS_VREDRAW;
				wc.lpfnWndProc = HandleMsgSetup;
				wc.cbClsExtra = 0;
				wc.cbWndExtra = 0;
				wc.hInstance = hInst;
				wc.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));
				//wc.hIconSm = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON2));
				wc.hCursor = nullptr;
				wc.hbrBackground = (HBRUSH)3;
				wc.lpszMenuName = nullptr;
				wc.lpszClassName = WIND_CLASS_NAME;

				if (!RegisterClassEx(&wc))
				{
					MessageBox(NULL, L"JWindow Registration Failed!", L"Error!",
						MB_ICONEXCLAMATION | MB_OK);
				}
			}
		public:
			void Resize(WPARAM wParam)
			{
				GetWindowRect(hwnd, &preWindowRect);
				GetClientRect(hwnd, &preClinetRect);

				if (wParam != SIZE_MINIMIZED)
					NotifyEvent(guid, J_WINDOW_EVENT::WINDOW_RESIZE);
			}
			void Move()
			{
				GetWindowRect(hwnd, &preWindowRect);
				GetClientRect(hwnd, &preClinetRect);

				NotifyEvent(guid, J_WINDOW_EVENT::WINDOW_MOVE);
			}
		public:
			static LRESULT CALLBACK HandleMsgSetup(HWND hwnd, uint msg, WPARAM wParam, LPARAM lParam);
			static LRESULT CALLBACK HandleMsgThunk(HWND hwnd, uint msg, WPARAM wParam, LPARAM lParam);
			static LRESULT HandleMsg(HWND hwnd, uint msg, WPARAM wParam, LPARAM lParam);
		};
 
		using Impl = JinEngine::Core::JSingletonHolder<JWindowImpl>;

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
				SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&HandleMsgThunk));
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
				(*Impl::Instance().closeConfirmF)();
				return 0;
			}
			case WM_DESTROY:
			{
				::PostQuitMessage(0);
				return 0;
			}
			case WM_QUIT:
			{
				break;
				//return 0;
			}
			case WM_MDIMAXIMIZE:
			{
				HMONITOR hmon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
				MONITORINFO moninfo;
				moninfo.cbSize = sizeof(moninfo);
				GetMonitorInfo(hmon, &moninfo);

				SetWindowPos(hwnd, 0, moninfo.rcWork.left, moninfo.rcWork.top,
					moninfo.rcWork.right, moninfo.rcWork.bottom, SWP_NOZORDER);

				Impl::Instance().Resize(wParam);
				break;
			}
			case WM_SIZING:
			{
				int newWidth = ((RECT*)lParam)->right - ((RECT*)lParam)->left;
				int newHeight = ((RECT*)lParam)->bottom - ((RECT*)lParam)->top;
				int minWidth = Impl::Instance().GetMinWidth();
				int minHeight = Impl::Instance().GetMinHeight();

				if (newWidth <= minWidth || newHeight <= minHeight)
				{
					RECT preWindowRect = Impl::Instance().GetPreWindowR();
					((RECT*)lParam)->left = preWindowRect.left;
					((RECT*)lParam)->right = preWindowRect.right;
					((RECT*)lParam)->top = preWindowRect.top;
					((RECT*)lParam)->bottom = preWindowRect.bottom;
				}
				/*if (newWidth > GetSystemMetrics(SM_CXFULLSCREEN) || newHeight > GetSystemMetrics(SM_CYFULLSCREEN))
				{
					RECT preWindowRect = Instance().GetPreClientR();
					((RECT*)lParam)->left = preWindowRect.left;
					((RECT*)lParam)->right = preWindowRect.right;
					((RECT*)lParam)->top = preWindowRect.top;
					((RECT*)lParam)->bottom = preWindowRect.bottom;
				}*/
				break;
				//return TRUE;
			}
			case WM_SIZE:
			{
				Impl::Instance().Resize(wParam);
				break;
				//return 0;
			}
			case WM_MOVE:
			{
				Impl::Instance().Move();
				break;
			}
			case WM_SHOWWINDOW:
			{
				break;
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

		RECT JWindow::GetWindowR()noexcept
		{
			return Impl::Instance().GetWindowR();
		}
		RECT JWindow::GetPreWindowR()noexcept
		{
			return Impl::Instance().GetPreWindowR();
		}
		RECT JWindow::GetClientR()noexcept
		{
			return Impl::Instance().GetClientR();
		}
		RECT JWindow::GetPreClientR()noexcept
		{
			return Impl::Instance().GetPreClientR();
		}
		int JWindow::GetDisplayWidth()noexcept
		{
			return Impl::Instance().GetDisplayWidth();
		}
		int JWindow::GetDisplayHeight()noexcept
		{
			return Impl::Instance().GetDisplayHeight();
		}
		int JWindow::GetMinWidth()noexcept
		{
			return Impl::Instance().GetMinWidth();
		}
		int JWindow::GetMinHeight()noexcept
		{
			return Impl::Instance().GetMinHeight();
		}
		int JWindow::GetWindowPositionX()noexcept
		{
			return Impl::Instance().GetWindowPositionX();
		}
		int JWindow::GetWindowPositionY()noexcept
		{
			return Impl::Instance().GetWindowPositionY();
		}
		int JWindow::GetWindowWidth()noexcept
		{
			return Impl::Instance().GetWindowWidth();
		}
		int JWindow::GetWindowHeight()noexcept
		{
			return Impl::Instance().GetWindowHeight();
		}
		JVector2<int> JWindow::GetClientPos()noexcept
		{
			return Impl::Instance().GetClientPos();
		}
		int JWindow::GetClientPositionX()noexcept
		{
			return Impl::Instance().GetClientPositionX();
		}
		int JWindow::GetClientPositionY()noexcept
		{
			return Impl::Instance().GetClientPositionY();
		}
		JVector2<int> JWindow::GetClientSize()noexcept
		{
			return Impl::Instance().GetClientSize();
		}
		int JWindow::GetClientWidth()noexcept
		{
			return Impl::Instance().GetClientWidth();
		}
		int JWindow::GetClientHeight()noexcept
		{
			return Impl::Instance().GetClientHeight();
		}
		bool JWindow::IsFullScreen()noexcept
		{
			return Impl::Instance().IsFullScreen();
		}
		bool JWindow::HasStorageSpace(const std::wstring& dirPath, size_t capacity)noexcept
		{
			return Impl::Instance().HasStorageSpace(dirPath, capacity);
		}
		bool JWindow::SelectDirectory(std::wstring& dirPath, const std::wstring& guide)noexcept
		{
			return Impl::Instance().SelectDirectory(dirPath, guide);
		}
		bool JWindow::SelectFile(std::wstring& filePath, const std::wstring& guide)noexcept
		{
			return Impl::Instance().SelectFile(filePath, guide);
		}
		WindowEvInterface* JWindow::EvInterface()noexcept
		{
			return Impl::Instance().EvInterface();
		}

		using ApplicationAccess = JWindowPrivate::ApplicationAccess;
		using HandleInterface = JWindowPrivate::HandleInterface;

		void ApplicationAccess::Initialize(HINSTANCE hInstance, std::unique_ptr<CloseConfirmF>&& closeConfirmF)
		{
			Impl::Instance().Initialize(hInstance, std::move(closeConfirmF));
		}
		void ApplicationAccess::OpenProjecSelectorWindow()
		{
			Impl::Instance().OpenProjecSelectorWindow();
		}
		void ApplicationAccess::OpenEngineWindow()
		{
			Impl::Instance().OpenEngineWindow();
		}
		void ApplicationAccess::CloseWindow()
		{
			Impl::Instance().CloseWindow();
		}
		std::optional<int> ApplicationAccess::ProcessMessages()
		{
			return Impl::Instance().ProcessMessages();
		}

		HWND HandleInterface::GetHandle()noexcept
		{
			return Impl::Instance().GetHandle();
		}
	}
}