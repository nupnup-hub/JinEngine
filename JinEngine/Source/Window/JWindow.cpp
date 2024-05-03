#include"JWindow.h"  
#include"JWindowPrivate.h"   
#include"../Core/JCoreEssential.h"
#include"../Core/JEngineInfo.h"
#include"../Core/Guid/JGuidCreator.h"
#include"../Core/Exception/JExceptionMacro.h"
#include"../Core/Exception/JException.h"
#include"../Core/Singleton/JSingletonHolder.h" 
#include"../Core/Utility/JCommonUtility.h" 
#include"../Core/Log/JLogMacro.h"
#include"../../resource.h"
#include"../../ThirdParty/imgui/imgui.h"
#include<tchar.h>
#include<fstream>
#include<shellapi.h>
#include<ShlObj_core.h> 
#include<functional>
#include<optional> 

#define EXE_FILEDIG(cond, contents) cond = cond ? SUCCEEDED(contents) : false

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
namespace JinEngine
{
	namespace Window
	{
#define WIND_CLASS_NAME L"JinEngineClass"
#define UM_CHECKSTATECHANGE (WM_USER + 100)
		 
		using WindowCloseConfirmF = JWindowPrivate::MainAccess::CloseConfirmF;
		class JWindowImpl : public Core::JEventManager<size_t, J_WINDOW_EVENT>
		{
		private:
			struct JRECT : public RECT
			{
			public:
				JRECT() = default;
				JRECT(const RECT& rhs) 
				{ 
					left = rhs.left;
					right = rhs.right;
					top = rhs.top;
					bottom = rhs.bottom;
				}
				RECT& operator=(const RECT& rhs)
				{
					left = rhs.left;
					right = rhs.right;
					top = rhs.top;
					bottom = rhs.bottom;
					return *this;
				}
				bool operator==(const RECT& rhs)
				{ 
					return left == rhs.left && top == rhs.top && right == rhs.right && bottom == rhs.bottom;
				}
			};
		public:
			const size_t guid;
			//JInputManager inputManager;
			WNDCLASSEX wc;
			HINSTANCE hInst;
			JRECT displayRect;
			JRECT preWindowRect;
			JRECT preClinetRect;
			HWND hwnd;
			bool enableCursor = true;
		private: 
			bool isActivated = false;
			bool isMinimize = false;
			static constexpr float minWindowRate = 0.55f;
		public:
			std::unique_ptr<WindowCloseConfirmF> closeConfirmF;
		public:
			JWindowImpl()
				:JEventManager([](const size_t& a, const size_t& b) {return a == b; }),
				guid(Core::MakeGuid()),
				hwnd(0)
			{}
			~JWindowImpl() {}
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
			JVector2<float> GetDisplayPosition()const noexcept
			{
				return JVector2F(displayRect.left, displayRect.top);
			}
			JVector2<float> GetDisplaySize()const noexcept
			{
				return JVector2F(displayRect.right - displayRect.left, displayRect.bottom - displayRect.top);
			}
			JVector2<float> GetMinSize()const noexcept
			{
				return GetDisplaySize() * minWindowRate;
			}
			JVector2<float> GetWindowPosition()const noexcept
			{
				return JVector2F(preWindowRect.left, preWindowRect.top);
			}
			JVector2<float> GetWindowSize()const noexcept
			{
				return JVector2F(preWindowRect.right - preWindowRect.left, preWindowRect.bottom - preWindowRect.top);
			}
			JVector2<float> GetClientPosition()const noexcept
			{
				return JVector2F(preClinetRect.left, preClinetRect.top);
			}
			JVector2<float> GetClientSize()const noexcept
			{
				return JVector2F(preClinetRect.right - preClinetRect.left, preClinetRect.bottom - preClinetRect.top);
			}
			uint GetMaxDisplayFrequency()const noexcept
			{
				DEVMODE dm;
				ZeroMemory(&dm, sizeof(dm));
				dm.dmSize = sizeof(dm);

				static constexpr uint defaultFrequency = 60;

				// 현재 디스플레이 디바이스의 설정 가져오기
				if (EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm))
					return dm.dmDisplayFrequency;
				else
					return defaultFrequency;
			}
		public:
			void SetDisplayInfo()
			{
				MONITORINFOEX monitorInfo;
				monitorInfo.cbSize = sizeof(MONITORINFOEX);

				// 현재 프라이머리 모니터의 정보 가져오기
				if (GetMonitorInfoW(MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY), &monitorInfo))
					displayRect = monitorInfo.rcWork;
				NotifyEvent(guid, J_WINDOW_EVENT::DISPLAY_RESOULTION_CHANGE);
			}
		public:
			bool IsFullScreen()const noexcept
			{
				return IsZoomed(hwnd);
			}
			bool IsActivated()const noexcept
			{
				return isActivated;
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
			bool SelectFile(_Out_ std::vector<std::wstring>& filePath,
				const std::wstring& guide,
				const bool allowMultiSelect,
				const bool selectFolder)noexcept
			{
				IFileOpenDialog* pfd;
				// CoCreate the dialog object.
				HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog,
					NULL,
					CLSCTX_INPROC_SERVER,
					IID_PPV_ARGS(&pfd));
				 
				bool isValid = SUCCEEDED(hr); 

				EXE_FILEDIG(isValid, pfd->SetTitle(guide.c_str()));
				DWORD dwOptions; 
				EXE_FILEDIG(isValid, pfd->GetOptions(&dwOptions));
				if (selectFolder)
					EXE_FILEDIG(isValid, pfd->SetOptions(dwOptions | FOS_PICKFOLDERS));
				else
				{
					COMDLG_FILTERSPEC fileFilter[] = { { L"All Files", L"*.*" } };
					EXE_FILEDIG(isValid, pfd->SetFileTypes(1, fileFilter));
				}

				if (isValid && allowMultiSelect)
					EXE_FILEDIG(isValid, pfd->SetOptions(dwOptions | FOS_ALLOWMULTISELECT));
				  
				EXE_FILEDIG(isValid, pfd->Show(NULL));
				if (isValid)
				{
					IShellItemArray* pSelectedItems = nullptr;
					EXE_FILEDIG(isValid, pfd->GetResults(&pSelectedItems));

					// 선택된 파일 항목 개수 가져오기
					DWORD dwItemCount = 0;
					EXE_FILEDIG(isValid, pSelectedItems->GetCount(&dwItemCount));

					if (isValid)
					{
						filePath.resize(dwItemCount);
						for (DWORD i = 0; i < dwItemCount; ++i)
						{
							// 개별 파일 항목 가져오기
							IShellItem* pItem = nullptr;
							pSelectedItems->GetItemAt(i, &pItem);

							// 파일 경로 가져오기
							PWSTR pszFilePath = nullptr;
							pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
							filePath[i] = pszFilePath;

							// 메모리 해제
							CoTaskMemFree(pszFilePath);
							pItem->Release();
						}
						// 메모리 해제
						pSelectedItems->Release();
						 
					}
				}			 
				pfd->Release();
				return isValid;
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
					WS_OVERLAPPED | WS_CAPTION | WS_THICKFRAME | WS_SYSMENU | WS_MINIMIZEBOX,
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

				SetDisplayInfo();
				ThrowIfFailedW(RegisterRawInputDevices(&rid, 1, sizeof(rid)));
				isActivated = true;
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

				SetDisplayInfo();
				ThrowIfFailedW(RegisterRawInputDevices(&rid, 1, sizeof(rid)));
				isActivated = true;
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
				wc.hIcon = LoadIcon(wc.hInstance, MAKEINTRESOURCE(IDI_ICON1));
				wc.hIconSm = LoadIcon(wc.hInstance, MAKEINTRESOURCE(IDI_ICON1));
				wc.hCursor = nullptr;
				wc.hbrBackground = (HBRUSH)3;
				wc.lpszMenuName = nullptr;
				wc.lpszClassName = WIND_CLASS_NAME;

				if (!RegisterClassEx(&wc))
					J_LOG_PRINT_OUT("JWindow Registration Failed", "");
			}
		public: 
			void Resize(WPARAM wParam)
			{
				JRECT preWindowRectCahce = preWindowRect;
				JRECT preClinetRectCache = preClinetRect;

				GetWindowRect(hwnd, &preWindowRect);
				GetClientRect(hwnd, &preClinetRect);
				 
				//if (preWindowRectCahce == preWindowRect && preClinetRectCache == preClinetRect)
				//	return;

				if (wParam != SIZE_MINIMIZED)
				{
					OutputDebugString(L"Resize\n");
					NotifyEvent(guid, J_WINDOW_EVENT::WINDOW_RESIZE);
				}
			}
			void Move()
			{
				GetWindowRect(hwnd, &preWindowRect);
				GetClientRect(hwnd, &preClinetRect);

				NotifyEvent(guid, J_WINDOW_EVENT::WINDOW_MOVE);
			}
		public:
			void Activate()
			{
				//window가 minimize상태가 아닌 상태에 Acivate메세지를 받았을시 활성화시킨다.
				if (isActivated || isMinimize)
					return;

				//OutputDebugString(L"Act\n");
				isActivated = true;
				NotifyEvent(guid, J_WINDOW_EVENT::WINDOW_ACTIVATE);  
			}
			void DeActivate()
			{
				//window가 minimize메세지를 받거나 DeActivate메세지를 받았을시 비활성화시킨다.
				if (!isActivated)
					return;

				//OutputDebugString(L"DeAct\n");
				isActivated = false;
				NotifyEvent(guid, J_WINDOW_EVENT::WINDOW_DEACTIVATE);
			}
			void Restore()
			{
				//Caching state
				isMinimize = false;
				Activate();
			}
			void Minimize()
			{	
				//Caching state
				isMinimize = true;
				DeActivate();
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
			case WM_SYSCOMMAND:
			{ 
				if (wParam == SC_RESTORE)
					Impl::Instance().Restore();
				else if (wParam == SC_MINIMIZE)
					Impl::Instance().Minimize();
				break;
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
				JVector2F minSize = Impl::Instance().GetMinSize(); 

				if ((newWidth <= minSize.x || newHeight <= minSize.y) && Impl::Instance().isActivated)
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
			}
			case WM_SIZE:
			{  
				Impl::Instance().Resize(wParam);
				break;
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
			case WM_SETFOCUS:
			{  		 
				break;
			}
			case WM_KILLFOCUS:
			{ 	
				break;
			}
			case WM_ENABLE:
			{
				break;
			} 
			case WM_DISPLAYCHANGE:
			{
				Impl::Instance().SetDisplayInfo();
				Impl::Instance().Resize(wParam);
				break;
			}
			case WM_ACTIVATEAPP:
			{
				if (wParam)
					Impl::Instance().Activate();
				else  
					Impl::Instance().DeActivate();
				break;
			}
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
		JVector2<float> JWindow::GetDisplayPosition()noexcept
		{
			return Impl::Instance().GetDisplayPosition();
		}
		JVector2<float> JWindow::GetDisplaySize()noexcept
		{
			return Impl::Instance().GetDisplaySize();
		}
		JVector2<float> JWindow::GetMinSize()noexcept
		{
			return Impl::Instance().GetMinSize();
		}
		JVector2<float> JWindow::GetWindowPosition()noexcept
		{
			return Impl::Instance().GetWindowPosition();
		}
		JVector2<float> JWindow::GetWindowSize()noexcept
		{
			return Impl::Instance().GetWindowSize();
		} 
		JVector2<float> JWindow::GetClientPosition()noexcept
		{
			return Impl::Instance().GetClientPosition();
		}
		JVector2<float> JWindow::GetClientSize()noexcept
		{
			return Impl::Instance().GetClientSize();
		}
		uint JWindow::GetMaxDisplayFrequency()noexcept
		{
			return Impl::Instance().GetMaxDisplayFrequency();
		}
		bool JWindow::IsFullScreen()noexcept
		{
			return Impl::Instance().IsFullScreen();
		}
		bool JWindow::IsActivated()noexcept
		{
			return Impl::Instance().IsActivated();
		}
		bool JWindow::HasStorageSpace(const std::wstring& dirPath, size_t capacity)noexcept
		{
			return Impl::Instance().HasStorageSpace(dirPath, capacity);
		}
		bool JWindow::SelectDirectory(_Out_ std::wstring& dirPath, const std::wstring& guide)noexcept
		{
			std::vector<std::wstring> path;
			const bool result = Impl::Instance().SelectFile(path, guide, false, true);
			if (result)
				dirPath = path[0];
			return result;
		}
		bool JWindow::SelectFile(_Out_ std::wstring& filePath, const std::wstring& guide)noexcept
		{
			std::vector<std::wstring> path;
			const bool result = Impl::Instance().SelectFile(path, guide, false, false);
			if (result)
				filePath = path[0];
			return result;
		}
		bool JWindow::SelectMultiFile(_Out_ std::vector<std::wstring>& filePath, const std::wstring& guide)noexcept
		{
			return Impl::Instance().SelectFile(filePath, guide, true, false);
		}
		WindowEvInterface* JWindow::EvInterface()noexcept
		{
			return Impl::Instance().EvInterface();
		}

		using MainAccess = JWindowPrivate::MainAccess;
		using HandleInterface = JWindowPrivate::HandleInterface;

		void MainAccess::Initialize(HINSTANCE hInstance, std::unique_ptr<CloseConfirmF>&& closeConfirmF)
		{
			Impl::Instance().Initialize(hInstance, std::move(closeConfirmF));
		}
		void MainAccess::OpenProjecSelectorWindow()
		{
			Impl::Instance().OpenProjecSelectorWindow();
		}
		void MainAccess::OpenEngineWindow()
		{
			Impl::Instance().OpenEngineWindow();
		}
		void MainAccess::CloseWindow()
		{
			Impl::Instance().CloseWindow();
		}
		std::optional<int> MainAccess::ProcessMessages()
		{
			return Impl::Instance().ProcessMessages();
		}

		HWND HandleInterface::GetHandle()noexcept
		{
			return Impl::Instance().GetHandle();
		}
	}
}