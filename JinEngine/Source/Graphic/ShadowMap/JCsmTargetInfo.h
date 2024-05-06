/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


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