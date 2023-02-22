#pragma once
#include"../JDataType.h"  
#include<memory>
#include<vector>
#include<string>

namespace JinEngine
{
	namespace Core
	{
		class JGuiWidgetInfo; 
		class JTypeInfo;
		struct JParameterHint;

		class JGuiWidgetInfoHandleBase
		{
		private:
			std::vector<std::unique_ptr<JGuiWidgetInfo>> widgetInfo;
		public:
			template<typename ...Widget>
			JGuiWidgetInfoHandleBase(Widget&&... var)
			{
				auto pushDataLam = [](JGuiWidgetInfoHandleBase* base, std::unique_ptr<JGuiWidgetInfo>&& info)
				{
					if (info != nullptr)
						base->widgetInfo.push_back(std::move(info));
				};
				(pushDataLam(this, std::move(var)), ...);
				widgetInfo.shrink_to_fit();
			}
			virtual ~JGuiWidgetInfoHandleBase();
		public:
			uint GetWidgetInfoCount()const noexcept;
			JGuiWidgetInfo* GetWidgetInfo(const uint index)const noexcept;
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