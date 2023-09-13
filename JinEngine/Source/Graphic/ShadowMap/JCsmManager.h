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
		* JCsmTargetInfo에 생성과 파괴는 Manager가 자동으로 관리한다
		* Handler와 Target은 객체가 유효한 상태가되면 포인터를  Manager에 등록하며 Manager는 각각
		* Handler이 등록된경우 해당하는 Area(Scene)에 속하는 Target들을 Handler에 캐싱하며
		* Target이 등록된경우 해당하는 Area에 속하는 Handler들은 Target을 자신의 객체 내부에 캐싱한다
		* 이는 하나의 Scene에서 관리되는 Handler들은 같은 Target들을 Rendering한다는 점에서 채택된 구현사항이다
		* 각각에 Handler  Target들은 수동으로도 객체를 추가/제거가 불가능하다
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