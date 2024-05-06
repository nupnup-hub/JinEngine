/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


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