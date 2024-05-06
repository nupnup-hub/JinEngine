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
#include"JObjectFlag.h"
#include"JObjectType.h" 
#include"../Core/File/JFileIOResult.h"
#include"../Core/Identity/JIdentifier.h"
#include"../Core/Interface/JActivatedInterface.h"
#include<stdio.h> 

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