#pragma once
#include"JModifiedObjectInfo.h"
#include"../Core/Pointer/JOwnerPtr.h"
#include"../Core/Storage/JStorage.h"
#include<unordered_map> 

namespace JinEngine
{ 
	namespace Core
	{
		class JIdentifier;
	}

	class JObjectModifyInterface
	{
	protected:
		static void SetModifiedBit(JUserPtr<Core::JIdentifier> obj, const bool value)noexcept;
	private:
		static JUserPtr<Core::JIdentifier> GetValidModifiedUser(JUserPtr<Core::JIdentifier> obj)noexcept;
	};

	using JModifiedObjectInfoVector = Core::JVectorPointerStorage<JModifiedObjectInfo>;
	using JModifiedObjectInfoMap = Core::JMapStorage<JModifiedObjectInfo, size_t>;
	class JModifedObjectInterface
	{
	public:
		bool IsModified(const size_t guid)const noexcept; 
		bool IsModifiedAndStoreAble(const size_t guid)const noexcept;
		bool IsModifiedAndStoreAble(const JModifiedObjectInfo* info)const noexcept;
	public:
		JModifiedObjectInfoVector::ObjectVector& GetModifiedObjectInfoVec()const noexcept;
	public:
		void RemoveInfo(const size_t guid);
		void ClearModifiedInfoStructure()noexcept;
	};
}