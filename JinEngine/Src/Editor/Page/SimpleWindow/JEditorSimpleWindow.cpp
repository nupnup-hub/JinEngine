#include"JEditorSimpleWindow.h"

namespace JinEngine
{
	namespace Editor
	{
		bool* JEditorSimpleWindow::GetOpenPtr()noexcept
		{
			return &isOpen;
		}
		bool JEditorSimpleWindow::IsOpen()const noexcept
		{
			return isOpen;
		}
	}
}