#pragma once
#include"JGraphicOption.h"
#include"../Core/Interface/JValidInterface.h"

namespace JinEngine
{
	namespace Graphic
	{
		enum class J_GRAPHIC_EVENT_TYPE
		{
			OPTION_CHANGED,   
		};

		class JGraphicResourceInfo;
		class JGraphicEventStruct : public Core::JValidInterface
		{
		public:
			virtual J_GRAPHIC_EVENT_TYPE GetEventType()const noexcept = 0;
		};

		class JGraphicOptionChangedEvStruct : public JGraphicEventStruct
		{
		public:
			JGraphicOption preOption;
			JGraphicOption newOption;
		public:
			JGraphicOptionChangedEvStruct(const JGraphicOption& preOption, const JGraphicOption& newOption);
		public:
			J_GRAPHIC_EVENT_TYPE GetEventType()const noexcept final;
		};		 
	}
}