#pragma once
#include"JReflectionGuiInfo.h" 

namespace JinEngine
{
	namespace Core
	{
		template<typename Type, typename Field, typename Pointer, Pointer ptr> class JPropertyInfoRegister;
		class JPropertyInfo;

		class JPropertyOptionInfo
		{ 
		private:
			friend class JPropertyInfo;
			template<typename Type, typename Field, typename Pointer, Pointer ptr> friend class JPropertyInfoRegister;
			friend std::unique_ptr<JPropertyOptionInfo>::deleter_type;
		private:
			std::unique_ptr<JGuiWidgetInfo> widgetInfo = nullptr;
		private:
			JPropertyOptionInfo() = default;
			JPropertyOptionInfo(std::unique_ptr<JGuiWidgetInfo>&& widgetInfo);
		public:
			JGuiWidgetInfo* GetWidgetInfo()const noexcept;
		public:
			bool HasWidgetInfo()const noexcept;
			bool IsGroupMember()const noexcept;		
		};
	}
}