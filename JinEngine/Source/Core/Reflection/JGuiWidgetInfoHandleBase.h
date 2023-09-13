#pragma once
#include"../JCoreEssential.h"  
#include"../Pointer/JOwnerPtr.h"
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