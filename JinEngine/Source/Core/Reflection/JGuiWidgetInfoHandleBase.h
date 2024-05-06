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
#include"../Pointer/JOwnerPtr.h" 

namespace JinEngine
{
	namespace Core
	{
		class JGuiWidgetInfo; 
		class JTypeInfo;
		struct JParameterHint;

		//하나의 property or method에 등록된 widgetinfo들의 handle 
		class JGuiWidgetInfoHandleBase
		{
		private:
			template<typename T> friend class JOwnerPtr;
		private:
			std::vector<JOwnerPtr<JGuiWidgetInfo>> widgetInfo;
		public:
			template<typename ...Widget>
			JGuiWidgetInfoHandleBase(Widget&&... var)
			{
				auto pushDataLam = [](JGuiWidgetInfoHandleBase* base, JOwnerPtr<JGuiWidgetInfo>&& info)
				{
					if (info != nullptr)
						base->widgetInfo.push_back(std::move(info));
				};
				(pushDataLam(this, std::move(var)), ...);
				widgetInfo.shrink_to_fit();
			}
		protected:
			virtual ~JGuiWidgetInfoHandleBase();
		public:
			uint GetWidgetInfoCount()const noexcept;
			JUserPtr<JGuiWidgetInfo> GetWidgetInfo(const uint index)const noexcept;
			//Return display name 
			virtual std::string GetName()const noexcept = 0;
			//somtime it is not correctly 
			//property is safe
			//but method get set can declared on another class
			virtual JTypeInfo* GetTypeInfo()const noexcept = 0;
			virtual JParameterHint GetFieldHint()const noexcept = 0;
		};
	}
}