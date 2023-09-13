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
		* JCsmHandlerInterface������ ���������� Info
		* Handler�� ������ �����ϸ� ���� �����ϴ� Target�� �ٸ���
		* Target�� ���� ������ TargetInface�� ���� �̷������ Handler�� Target�� Frustum�� ������
		* ����� �����Ѵ�.
		* �׻� ��ȿ�� ��ü�̴�.
		* JCsmTargetInfo�� �۰� ���󵵵� ���������Ƿ� Custom Allcator ����� �����ϰ�
		* Deafult Heap���� �Ҵ�޴´�.
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