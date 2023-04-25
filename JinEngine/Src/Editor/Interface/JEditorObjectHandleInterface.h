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
			static void SetModifiedBit(Core::JUserPtr<Core::JIdentifier> obj, const bool value)noexcept; 
		private:
			static Core::JUserPtr<Core::JIdentifier> GetValidModifiedUser(Core::JUserPtr<Core::JIdentifier> obj)noexcept;
		};
	}
	using JModifiedObjectInfoVector = Core::JVectorPointerStorage<JModifiedObjectInfo>;
	using JModifiedObjectInfoMap = Core::JMapStorage<JModifiedObjectInfo, size_t>;
	class JEditorModifedObjectInterface
	{
	public:
		bool IsModified(const size_t guid)noexcept;
	public:
		JModifiedObjectInfoVector::ObjectVector& GetModifiedObjectInfoVec()noexcept;
	public:
		void ClearModifiedInfoStructure()noexcept;
	};
}