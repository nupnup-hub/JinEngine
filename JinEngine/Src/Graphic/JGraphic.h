#pragma once  
#include"GraphicResource/JGraphicResourceType.h"
#include"JGraphicOption.h" 
#include"JGraphicInfo.h" 
#include<d3d12.h>
 
namespace JinEngine
{
	namespace Core
	{
		template<typename T>class JCreateUsingNew;
	}
	namespace Graphic
	{
		class JGraphicPrivate; 
		class JGraphic
		{
		private:
			template<typename T>friend class Core::JCreateUsingNew; 
		private:
			friend class JGraphicPrivate;
			class JGraphicImpl;
		private:
			std::unique_ptr<JGraphicImpl> impl;
		public:
			JGraphicInfo GetGraphicInfo()const noexcept;
			JGraphicOption GetGraphicOption()const noexcept;
			void SetGraphicOption(JGraphicOption newGraphicOption)noexcept;
		private:
			JGraphic();
			~JGraphic();
		};
		using _JGraphic = JinEngine::Core::JSingletonHolder<Graphic::JGraphic>;
	}
	using JGraphic = Graphic::_JGraphic;
}