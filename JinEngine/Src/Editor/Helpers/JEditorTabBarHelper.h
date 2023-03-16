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
		public:
			static constexpr int invalidIndex = -1;
		private:
			std::string itemLabel[itemCount];
			bool itemBit[itemCount];
		private:
			int lastOpenIndex = 0;
		public:
			JEditorTabBarHelper(std::string(&itemLabel)[itemCount])
			{
				for (int i = 0; i < itemCount; ++i)
				{
					JEditorTabBarHelper::itemLabel[i] = itemLabel[i];
					itemBit[i] = 0;
				}
			}
			~JEditorTabBarHelper() = default;
			JEditorTabBarHelper(const JEditorTabBarHelper& rhs) = delete;
			JEditorTabBarHelper& operator=(const JEditorTabBarHelper& rhs) = delete;
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
				for (int i = 0; i < itemCount; ++i)
					if (itemBit[i])
						return i;

				return invalidIndex;
			}
			bool* GetItemBitPointer(const int index)noexcept
			{ 
				return &itemBit[index];
			}
		public:
			void OpenItemBit(const int index)
			{
				ClearItemBit();
				itemBit[index] = true; 
			}
		private:
			void ClearItemBit()
			{
				for (int i = 0; i < itemCount; ++i)
					itemBit[i] = 0;
			}
		};
	}
}
