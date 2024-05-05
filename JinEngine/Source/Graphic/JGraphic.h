#pragma once  
#include"GraphicResource/JGraphicResourceType.h"
#include"JGraphicOption.h" 
#include"JGraphicInfo.h" 
#include"JGraphicEvent.h" 
#include"../Core/Event/JEventManager.h"   
#include"Command/JCommandContextLog.h"

namespace JinEngine
{
	namespace Core
	{
		template<typename T>class JCreateUsingNew;
	}
	namespace Graphic
	{
		class JGraphicPrivate; 
		 
		using GraphicEventManager = Core::JEventManager<size_t, J_GRAPHIC_EVENT_TYPE, JGraphicEventStruct*>;
		using GraphicEventInterface = GraphicEventManager::Interface;
		using GraphicEventPtr = GraphicEventInterface::OnEventPtr;
		using GraphicEventListener = GraphicEventInterface::Listener;
	 
		class JGraphic
		{
		private:
			template<typename T>friend class Core::JCreateUsingNew; 
		private:
			struct UpdateHelper;
			struct DrawHelper;
			struct DrawCondition;
		private:
			friend class JGraphicPrivate;
			class JGraphicImpl;
		private:
			std::unique_ptr<JGraphicImpl> impl;
		public:
			JGraphicInfo GetGraphicInfo()const noexcept; 
			const JGraphicInfo& GetGraphicInfoRef()const noexcept;
			JGraphicOption GetGraphicOption()const noexcept;  
			const JGraphicOption& GetGraphicOptionRef()const noexcept;
			void GetLastDeviceErrorInfo(_Out_ std::wstring& errorCode, _Out_ std::wstring& errorMsg)const noexcept;
			std::vector<JCommandContextLog> GetCpuDrawingLog()const noexcept;
		public:
			void SetGraphicOption(JGraphicOption newGraphicOption)noexcept;
		public:
			bool IsRaytracingSupported()const noexcept;
			bool CanBuildGpuAccelerator()const noexcept;
		public: 
			GraphicEventInterface* EventInterface()noexcept; 
		private:
			JGraphic();
			~JGraphic();
		};
		using _JGraphic = JinEngine::Core::JSingletonHolder<Graphic::JGraphic>;
	}
	using JGraphic = Graphic::_JGraphic;
}