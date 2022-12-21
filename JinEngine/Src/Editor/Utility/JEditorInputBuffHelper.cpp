#include "JEditorInputBuffHelper.h"
#include"../../Utility/JCommonUtility.h"

namespace JinEngine
{
	namespace Editor
	{
		JEditorInputBuffHelper::JEditorInputBuffHelper(const int capacity)
			:capacity(capacity)
		{}
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
		void JEditorInputBuffHelper::SetBuff(const std::string& value)noexcept
		{
			buff = value;
			buff.resize(capacity);
		}
	}
}