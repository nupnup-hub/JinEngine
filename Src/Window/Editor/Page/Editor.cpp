#include"Editor.h"
#include"EditorAttribute.h"
#include"../Utility/EditorUtility.h"
#include"../../../Utility/JCommonUtility.h"

namespace JinEngine
{
	Editor::Editor(std::unique_ptr<EditorAttribute> attribute)
		:attribute(std::move(attribute))
	{}
	Editor::~Editor() {}
	bool Editor::Activate(EditorUtility* editorUtility)
	{
		if (!attribute->isActivated && attribute->isOpen && attribute->isFront)
		{
			attribute->isActivated = true;
			return true;
		}
		else
			return false;
	}
	bool Editor::DeActivate(EditorUtility* editorUtility)
	{
		if (attribute->isActivated && attribute->isOpen && attribute->isFront)
		{
			if (attribute->isFocus)
				OffFocus(editorUtility);
			attribute->isActivated = false;
			return true;
		}
		else
			return false;
	}
	bool Editor::OnFocus(EditorUtility* editorUtility)
	{
		if (attribute->isActivated && !attribute->isFocus)
		{
			attribute->isFocus = true;
			return true;
		}
		else
			return false;
	}
	bool Editor::OffFocus(EditorUtility* editorUtility)
	{
		if (attribute->isActivated && attribute->isFocus)
		{
			attribute->isFocus = false;
			return true;
		}
		else
			return false;
	}
	bool Editor::SetFront(EditorUtility* editorUtility)
	{
		if (!attribute->isFront)
		{
			attribute->isFront = true;
			return true;
		}
		else
			return false;
	}
	bool Editor::OffFront(EditorUtility* editorUtility)
	{
		if (attribute->isFront)
		{
			attribute->isFront = false;
			return true;
		}
		else
			return false;
	}
	bool Editor::Open(EditorUtility* editorUtility)
	{
		if (!attribute->isOpen)
		{
			attribute->isOpen = true;
			return true;
		}
		else
			return false;
	}
	bool Editor::Close(EditorUtility* editorUtility)
	{
		if (attribute->isOpen)
		{
			attribute->isOpen = false;
			return true;
		}
		else
			return false;
	}
	float Editor::GetInitPosXRate()const noexcept
	{
		return attribute->initPosXRate;
	}
	float Editor::GetInitPsYRate()const noexcept
	{
		return attribute->initPosYRate;
	}
	float Editor::GetInitWidthRate()const noexcept
	{
		return attribute->initWidthRate;
	}
	float Editor::GetInitHeightRate()const noexcept
	{
		return attribute->initHeightRate;
	}
	bool* Editor::GetOpenPtr()const noexcept
	{
		return &attribute->isOpen;
	}
	bool* Editor::GetActivationPtr()const noexcept
	{
		return	&attribute->isActivated;
	}
	bool Editor::IsOpen()const noexcept
	{
		return attribute->isOpen;
	}
	bool Editor::IsFront()const noexcept
	{
		return attribute->isFront;
	}
	bool Editor::IsActivated()const noexcept
	{
		return attribute->isActivated;
	}
	bool Editor::IsFocus()const noexcept
	{
		return attribute->isFocus;
	}
	void Editor::SetOpen()noexcept
	{
		attribute->isOpen = true;
	}
	void Editor::SetFront()noexcept
	{
		attribute->isFront = true;
	}
	void Editor::SetActivattion()noexcept
	{
		attribute->isActivated = true;
	}
	void Editor::OffOpen()noexcept
	{
		attribute->isOpen = false;
	}
	void Editor::OffFront()noexcept
	{
		attribute->isFront = false;
	}
	void Editor::OffActivattion()noexcept
	{
		attribute->isActivated = false;
	}
	std::string Editor::GetName()const noexcept
	{
		return attribute->name;
	}
	std::wstring Editor::GetWName()const noexcept
	{
		return JCommonUtility::U8StringToWstring(attribute->name);
	}
	size_t Editor::GetGuid()const noexcept
	{
		return attribute->guid;
	}
}
