#pragma once
#include<stdio.h>
#include<string> 
#include"JObjectFlag.h"
#include"JObjectType.h"
#include"../Core/JDataType.h"
#include"../Core/Reflection/JReflection.h"
#include"../Core/File/JFileIOResult.h"

namespace JinEngine
{ 
	class JObject
	{
		REGISTER_CLASS(JObject)
	protected:
		struct ObjectMetadata
		{
		public:
			size_t guid;
			J_OBJECT_FLAG flag;
		};
	private:
		std::string name;
		size_t guid;
		const J_OBJECT_FLAG flag;
		bool isActivated = false;
	public:
		std::string GetName() const noexcept;
		std::wstring GetWName() const noexcept;
		size_t GetGuid()const noexcept;
		J_OBJECT_FLAG GetFlag()const noexcept;
		virtual J_OBJECT_TYPE GetObjectType()const noexcept = 0;
		void SetName(const std::string& name)noexcept;		
	protected:
		bool HasFlag(const J_OBJECT_FLAG flag)const noexcept;
	protected:
		bool IsActivated() const noexcept;
		void Activate() noexcept;
		void DeActivate()noexcept;
	protected:
		virtual void DoActivate() noexcept;
		virtual void DoDeActivate() noexcept;
	protected:
		static Core::J_FILE_IO_RESULT StoreMetadata(std::wofstream& stream, JObject* object);
		static Core::J_FILE_IO_RESULT LoadMetadata(std::wifstream& stream, ObjectMetadata& metadata);
	public:
		void BeginDestroy();
	protected: 
		bool IsIgnoreUndestroyableFlag()const noexcept;
		void SetIgnoreUndestroyableFlag(const bool value)noexcept;
	private:
		void EndDestroy();
	protected:
		virtual void Destroy() = 0;
	protected:
		JObject(const std::string& name, const size_t guid, const J_OBJECT_FLAG flag = OBJECT_FLAG_NONE);
		virtual ~JObject();
	protected:
		template<typename T>
		static std::string GetDefaultName()noexcept
		{
			return "New" + std::string(T::TypeName());
		}
	};
}