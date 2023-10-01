#pragma once
#include<stdio.h>
#include<string> 
#include"JObjectFlag.h"
#include"JObjectType.h" 
#include"../Core/File/JFileIOResult.h"
#include"../Core/Identity/JIdentifier.h"
#include"../Core/Interface/JActivatedInterface.h"

namespace JinEngine
{ 
	class JObjectPrivate;
	class JObject : public Core::JIdentifier, public Core::JActivatedInterface 
	{
		REGISTER_CLASS_IDENTIFIER_LINE(JObject)
	public: 
		class InitData : public Core::JIdentifier::InitData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData)
		public: 
			J_OBJECT_FLAG flag = OBJECT_FLAG_NONE;
		public:
			InitData(const JTypeInfo& initTypeInfo);
			InitData(const JTypeInfo& initTypeInfo, const size_t guid);
			InitData(const JTypeInfo& initTypeInfo, const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag); 
		};
	protected: 
		class StoreData : public Core::JDITypeDataBase
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(StoreData)
		public: 
			JUserPtr<JObject> obj = nullptr;
		public:
			StoreData(JUserPtr<JObject> obj);
		public:
			bool IsValidData()const noexcept override;
			bool HasCorrectType(const JTypeInfo& correctType)const noexcept;
			bool HasCorrectChildType(const JTypeInfo& correctType)const noexcept;
		};
	private:
		friend class JObjectPrivate;
		class JObjectImpl;
	private:
		std::unique_ptr<JObjectImpl> impl;
	public:
		J_OBJECT_FLAG GetFlag()const noexcept;
		virtual J_OBJECT_TYPE GetObjectType()const noexcept = 0;
	public:
		bool CanControlIdentifiable()const noexcept final;
		bool HasFlag(const J_OBJECT_FLAG flag)const noexcept;
	protected:
		JObject(const InitData& initData);
		~JObject();
	};
}