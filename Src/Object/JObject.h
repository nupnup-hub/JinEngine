#pragma once
#include<stdio.h>
#include<string>
#include"JObjectInterface.h"
#include"JObjectFlag.h"
#include"JObjectType.h"
#include"../Core/JDataType.h"
#include"../Core/Reflection/JReflection.h"
#include"../Core/File/JFileIOResult.h"

namespace JinEngine
{ 
	class JObject : public JObjectInterface
	{
		REGISTER_CLASS(JObject)
	protected:
		struct ObjectMetadata
		{
		public:
			size_t guid;
			JOBJECT_FLAG flag;
		};
	private:
		std::string name;
		size_t guid;
		const JOBJECT_FLAG flag;
		bool isActivated = false;
	public:
		std::string GetName() const noexcept;
		std::wstring GetWName() const noexcept;
		size_t GetGuid()const noexcept;
		JOBJECT_FLAG GetFlag()const noexcept;
		virtual J_OBJECT_TYPE GetObjectType()const noexcept = 0;
		void SetName(const std::string& name)noexcept;		
	protected:
		bool HasFlag(const JOBJECT_FLAG flag)const noexcept;
	protected:
		bool IsActivated() const noexcept final;
		void Activate() noexcept final;
		void DeActivate()noexcept final;
		virtual void DoActivate() noexcept;
		virtual void DoDeActivate()noexcept; 
		static Core::J_FILE_IO_RESULT StoreMetadata(std::wofstream& stream, JObject* object);
		static Core::J_FILE_IO_RESULT LoadMetadata(std::wifstream& stream, ObjectMetadata& metadata);
	protected:
		JObject(const std::string& name, const size_t guid, const JOBJECT_FLAG flag = OBJECT_FLAG_NONE);
		virtual ~JObject();
	protected:
		template<typename T>
		static std::string GetDefaultName()noexcept
		{
			return "New" + std::string(T::TypeName());
		}
	};
}