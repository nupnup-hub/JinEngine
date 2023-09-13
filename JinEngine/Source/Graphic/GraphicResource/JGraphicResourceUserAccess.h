#pragma once  
namespace JinEngine
{ 
	namespace Graphic
	{
		class JGraphicResourceUserInterface; 
		class JGraphicResourceUserAccess
		{
		protected:
			JGraphicResourceUserAccess() = default;
			virtual ~JGraphicResourceUserAccess() = default;
		public:
			virtual const JGraphicResourceUserInterface GraphicResourceUserInterface()const noexcept = 0;
		};
	}
}
