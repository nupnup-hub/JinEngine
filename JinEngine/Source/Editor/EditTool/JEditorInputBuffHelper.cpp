#include "JEditorInputBuffHelper.h"
#include"../../Core/Utility/JCommonUtility.h"

namespace JinEngine
{
	namespace Editor
	{
		JEditorInputBuffHelper::JEditorInputBuffHelper(const int capacity)
			:capacity(capacity)
		{
			Clear();
		}
		void JEditorInputBuffHelper::Clear()
		{
			result.clear();
			buff.clear();
			buff.resize(capacity);
		}
		std::string JEditorInputBuffHelper::GetResult()const noexcept
		{
			return JCUtil::EraseSideChar(result, ' ');
		}
		int JEditorInputBuffHelper::GetCapactiy()const noexcept
		{
			return capacity;
		}
		void JEditorInputBuffHelper::SetCapacity(const int newCapacity)noexcept
		{ 
			capacity = newCapacity;
			buff.resize(newCapacity);
			if (result.size() > newCapacity)
				result.resize(newCapacity);
		}
		void JEditorInputBuffHelper::SetBuff(const std::string& value)noexcept
		{
			buff = value;
			buff.resize(capacity);
		}
	}
}