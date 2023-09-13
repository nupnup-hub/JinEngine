#pragma once
#include"../JGraphicResourceHolder.h"
namespace JinEngine
{
	namespace Graphic
	{
		class JDxGraphicResourceHolderInterface : public JGraphicResourceHolder
		{
		public:
			virtual void SetPrivateName(const std::wstring& name)noexcept = 0;
		};
	}
}