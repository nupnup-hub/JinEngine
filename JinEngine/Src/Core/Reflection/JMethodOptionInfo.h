#pragma once
#include"JReflectionGuiInfo.h" 

namespace JinEngine
{
	namespace Core
	{
		template<typename Type, typename Pointer, Pointer ptr> class JMethodInfoRegisterHelper;
		class JMethodInfo;

		class JMethodOptionInfo
		{
		private:
			friend class JMethodInfo;
			template<typename Type, typename Pointer, Pointer ptr> friend class JMethodInfoRegisterHelper;
			friend std::unique_ptr<JMethodOptionInfo>::deleter_type;
		private:
			std::unique_ptr<JGuiWidgetInfo> widgetInfo = nullptr;
		private:
			JMethodOptionInfo() = default;
			JMethodOptionInfo(std::unique_ptr<JGuiWidgetInfo>&& widgetInfo);
		public:
			JGuiWidgetInfo* GetWidgetInfo()const noexcept;
		public:
			bool HasWidgetInfo()const noexcept;
			bool IsGroupMember()const noexcept;
		};
	}
}