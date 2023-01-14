#pragma once
#include"../../Core/Pointer/JOwnerPtr.h"
#include"../../Core/Storage/JStorage.h"
#include"../../Object/JModifiedObjectInfo.h"
#include<unordered_map>
#include<string>

namespace JinEngine
{ 
	namespace Core
	{
		class JIdentifier;
	}
	namespace Editor
	{		
		class JEditorObjectHandlerInterface
		{
		protected:
			void SetModifiedBit(Core::JUserPtr<Core::JIdentifier> obj, const bool value)noexcept;
			void RemoveModifiedInfo(Core::JUserPtr<Core::JIdentifier> obj)noexcept;
		private:
			Core::JUserPtr<Core::JIdentifier> GetValidModifiedUser(Core::JUserPtr<Core::JIdentifier> obj)const noexcept;
		};

		using JModifiedObjectInfoVector = Core::JVectorPointerStorage<JModifiedObjectInfo>;
		using JModifiedObjectInfoMap = Core::JMapStorage<JModifiedObjectInfo, size_t>;

		class JEditorModifedObjectStructureInterface
		{
		public:
			JModifiedObjectInfoVector::ObjectVector& GetModifiedObjectInfoVec()noexcept;
		public:
			void ClearModifiedInfoStructure()noexcept;
		};
	}
}