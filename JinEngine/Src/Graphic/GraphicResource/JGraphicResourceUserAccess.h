#pragma once  
namespace JinEngine
{ 
	namespace Graphic
	{
		class JGraphicSingleResourceUserInterface;
		class JGraphicMultiResourceUserInterface;
		class JGraphicSingleResourceUserAccess
		{
		protected:
			JGraphicSingleResourceUserAccess() = default;
			virtual ~JGraphicSingleResourceUserAccess() = default;
		public:
			virtual const JGraphicSingleResourceUserInterface GraphicResourceUserInterface()const noexcept = 0;
		};
		class JGraphicMultiResourceUserAccess
		{
		protected:
			JGraphicMultiResourceUserAccess() = default;
			virtual ~JGraphicMultiResourceUserAccess() = default;
		public:
			virtual const JGraphicMultiResourceUserInterface GraphicResourceUserInterface()const noexcept = 0;
		};
	}
}
