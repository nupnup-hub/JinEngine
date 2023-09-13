#pragma once
#include"../../Core/Func/Functor/JFunctor.h"
#include<DirectXCollision.h>

namespace JinEngine
{
	namespace Graphic
	{
		class JCsmTargetInterface;
		using GetCsmTargetBoundingFrustumF = Core::JMFunctorType<JCsmTargetInterface, DirectX::BoundingFrustum>; 
	}
}