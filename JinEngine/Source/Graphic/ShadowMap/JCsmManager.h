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
#include"../../Core/Reflection/JTypeImplBase.h"
#include"../JGraphicSubClassInterface.h"
#include"JCsmType.h" 

namespace JinEngine
{
	namespace Graphic
	{ 
		class JCsmHandlerInterface;
		class JCsmTargetInterface;
		class JCsmTargetInfo;

		using JCsmHandlerPointer = Core::JTypeImplInterfacePointer<JCsmHandlerInterface>;
		using JCsmTargetInterfacePointer = Core::JTypeImplInterfacePointer<JCsmTargetInterface>;

		/**
		* JCsmTargetInfo�� ������ �ı��� Manager�� �ڵ����� �����Ѵ�
		* Handler�� Target�� ��ü�� ��ȿ�� ���°��Ǹ� �����͸�  Manager�� ����ϸ� Manager�� ����
		* Handler�� ��ϵȰ�� �ش��ϴ� Area(Scene)�� ���ϴ� Target���� Handler�� ĳ���ϸ�
		* Target�� ��ϵȰ�� �ش��ϴ� Area�� ���ϴ� Handler���� Target�� �ڽ��� ��ü ���ο� ĳ���Ѵ�
		* �̴� �ϳ��� Scene���� �����Ǵ� Handler���� ���� Target���� Rendering�Ѵٴ� ������ ä�õ� ���������̴�
		* ������ Handler  Target���� �������ε� ��ü�� �߰�/���Ű� �Ұ����ϴ�
		*/
		class JCsmManager : public JGraphicSubClassInterface
		{ 
		private:
			struct AreaData
			{
			public:
				std::vector<JUserPtr<JCsmHandlerPointer>> handler;
				std::vector<JUserPtr<JCsmTargetInterfacePointer>> target;
			};
		private:
			std::unordered_map<size_t, AreaData> areaData;
		public:
			~JCsmManager();
		public:
			bool RegisterHandler(JCsmHandlerInterface* handler);
			bool DeRegisterHandler(JCsmHandlerInterface* handler);
			bool RegisterTarget(JCsmTargetInterface* target);
			bool DeRegisterTarget(JCsmTargetInterface* target);
		private:
			JUserPtr<JCsmHandlerPointer> GetHandler(const size_t areaGuid, const size_t handlerGuid)const noexcept;
			JUserPtr<JCsmTargetInterfacePointer> GetTarget(const size_t areaGuid, const size_t targetGuid)const noexcept;
			int GetHandlerIndex(const size_t areaGuid, const size_t handlerGuid)const;
			int GetTargetIndex(const size_t areaGuid, const size_t targetGuid)const noexcept;
		public:
			void Clear();
		};
	}
}