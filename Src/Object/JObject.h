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
		std::wstring name;
		size_t guid;
		const J_OBJECT_FLAG flag;
		bool isActivated = false;
	public: 
		std::wstring GetName() const noexcept;
		size_t GetGuid()const noexcept;
		J_OBJECT_FLAG GetFlag()const noexcept;
		virtual J_OBJECT_TYPE GetObjectType()const noexcept = 0; 
		void SetName(const std::wstring& name)noexcept;
		bool HasFlag(const J_OBJECT_FLAG flag)const noexcept;
	public:
		virtual bool Copy(JObject* ori) = 0; 
	protected:
		bool IsActivated() const noexcept;
		void Activate() noexcept;
		void DeActivate()noexcept;
	protected:
		virtual void DoActivate() noexcept;
		virtual void DoDeActivate() noexcept;
	public:
		void BeginDestroy();
	protected: 
		bool IsIgnoreUndestroyableFlag()const noexcept;
		void SetIgnoreUndestroyableFlag(const bool value)noexcept;
	protected:
		virtual void Destroy() = 0;
	private:
		void EndDestroy();
	protected:
		template<typename T, std::enable_if_t<std::is_base_of_v<JObject, T>, int> = 0>
		static bool AddInstance(Core::JOwnerPtr<T> ptr)noexcept
		{
			if (ptr.IsValid())
			{
				const size_t guid = ptr->GetGuid();
				T::StaticTypeInfo().AddInstance(guid, std::move(ptr));
				return true;
			}
			else
				return false;
		}
	private:
		bool RemoveInstance(const size_t guid)noexcept;
	protected:
		static std::wstring ConvertMetafilePath(const std::wstring& path)noexcept;
		static Core::J_FILE_IO_RESULT StoreMetadata(std::wofstream& stream, JObject* object);
		static Core::J_FILE_IO_RESULT LoadMetadata(std::wifstream& stream, ObjectMetadata& metadata);
	protected:
		JObject(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag = OBJECT_FLAG_NONE);
		virtual ~JObject();
	protected:
		template<typename T>
		static std::wstring GetDefaultName()noexcept
		{
			return L"New" + std::wstring(T::TypeWName());
		}
	};
}