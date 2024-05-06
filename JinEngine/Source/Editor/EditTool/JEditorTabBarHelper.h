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


#pragma once  
#include"../../Core/JCoreEssential.h"

namespace JinEngine
{
	namespace Editor
	{
		template<int itemCount>
		class JEditorTabBarHelper
		{  
		private:
			std::string itemLabel[itemCount];
			bool itemBit[itemCount]; 
		private:
			int opendIndex = invalidIndex; 
			bool isOpenNewTab = true;
		public:
			JEditorTabBarHelper(std::string(&itemLabel)[itemCount])
			{
				for (int i = 0; i < itemCount; ++i)
				{
					JEditorTabBarHelper::itemLabel[i] = itemLabel[i];
					itemBit[i] = false;
				}
			}
			~JEditorTabBarHelper() = default;
			JEditorTabBarHelper(const JEditorTabBarHelper& rhs) = delete;
			JEditorTabBarHelper& operator=(const JEditorTabBarHelper& rhs) = delete;
		public:
			std::string GetLabel(const int index)const noexcept
			{
				return itemLabel[index];
			}
			int GetLabelIndex(const std::string label)const noexcept
			{
				for (int i = 0; i < itemCount; ++i)
				{
					if (itemLabel[i] == label)
						return i;
				}
				return invalidIndex;
			}
			int GetOpenItemBitIndex()const noexcept
			{ 
				return opendIndex; 
			}
			bool* GetItemBitPointer(const int index)noexcept
			{ 
				return &itemBit[index];
			}
		public:
			void SetInitState()noexcept
			{
				ClearItemBit();
			}
		public:
			bool IsActivatedItem(const int index)const noexcept
			{
				return itemBit[index];
			}
			bool IsActivatedItem(const std::string label)const noexcept
			{
				int index = GetLabelIndex(label);
				return index != invalidIndex ? IsActivatedItem(index) : false;
			}
			bool IsOpenNewTab()const noexcept
			{
				return isOpenNewTab;
			}
		public:
			void OpenItemBit(const int index)
			{
				int preOpendIndex = opendIndex;
				ClearItemBit();
				itemBit[index] = true; 
				if (index != preOpendIndex)
					isOpenNewTab = true;
				opendIndex = index;
			}
		private:
			void ClearItemBit()
			{
				for (int i = 0; i < itemCount; ++i)
					itemBit[i] = false; 
				opendIndex = invalidIndex;
				isOpenNewTab = false;
			}
		};
	}
}
