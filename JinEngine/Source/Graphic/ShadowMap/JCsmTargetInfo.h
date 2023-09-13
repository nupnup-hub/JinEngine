#pragma once 
#include"JCsmType.h"
#include"../../Core/Func/Functor/JFunctor.h"
#include"../../Core/Reflection/JReflection.h"
#include<DirectXCollision.h>

namespace JinEngine
{
	namespace Graphic
	{		
		/*
		* JCsmHandlerInterface에서만 생성가능한 Info
		* Handler는 복수가 존재하며 각각 참조하는 Target도 다르다
		* Target에 대한 접근은 TargetInface를 통해 이루어지며 Handler는 Target에 Frustum을 참조해
		* 계산을 수행한다.
		* 항상 유효한 객체이다.
		* JCsmTargetInfo는 작고 사용빈도도 많지않으므로 Custom Allcator 사용을 지양하고
		* Deafult Heap에서 할당받는다.
		*/
		class JCsmHandlerInterface;
		class JCsmTargetInfo final
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(JCsmTargetInfo)
		private:
			friend class JCsmHandlerInterface;
		private:
			size_t handlerGuid; 
			mutable GetCsmTargetBoundingFrustumF::Functor getFrustumF;
		public:
			size_t GetHandlerGuid()const noexcept;  
			DirectX::BoundingFrustum GetFrustum()const noexcept;
		public: 
			//void SetFrustumPtr(GetCsmTargetBoundingFrustumF::CPtr ptr, JCsmTargetInterface* iTar);
			void SetFrustumPtr(GetCsmTargetBoundingFrustumF::Functor&& newFunc);
		public:
			bool IsValid()const noexcept;
		private:
			JCsmTargetInfo(const size_t handlerGuid);
			~JCsmTargetInfo();
		};
	}
}