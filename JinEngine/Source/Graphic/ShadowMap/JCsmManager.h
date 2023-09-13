#pragma once 
#include"../../Core/Reflection/JTypeImplBase.h"
#include"JCsmType.h"
#include<unordered_map>

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
		class JCsmManager
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