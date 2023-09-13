#pragma once 
#include<bitset>
#include<string>
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
