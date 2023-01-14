#pragma once
#include"../../Core/Pointer/JOwnerPtr.h"
#include"../../Core/Storage/JStorage.h"
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
		struct JModifiedObjectInfo
		{
		public:
			const size_t guid;
			const std::string typeName;
			bool isModified = false;
			bool isStore = true;
		public:
			JModifiedObjectInfo(const size_t guid, const std::string& typeName); 
		};		 

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
		protected:
			JModifiedObjectInfoVector::ObjectVector& GetModifiedObjectInfoVec()noexcept;
		protected:
			void ClearModifiedInfoStructure()noexcept;
		};
	}
}