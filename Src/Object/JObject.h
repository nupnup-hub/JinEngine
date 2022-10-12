#pragma once
#include<stdio.h>
#include<string> 
#include"JObjectFlag.h"
#include"JObjectType.h"
#include"../Core/JDataType.h"
#include"../Core/File/JFileIOResult.h"
#include"../Core/Identity/JIdentifier.h"

namespace JinEngine
{ 
	class JObject : public Core::JIdentifier
	{
		REGISTER_CLASS(JObject)
	protected:
		struct JObjectMetaData
		{
		public:
			size_t guid;
			J_OBJECT_FLAG flag;
		};
	private: 
		const J_OBJECT_FLAG flag;
		bool isActivated = false;
	public:  
		J_OBJECT_FLAG GetFlag()const noexcept;
		virtual J_OBJECT_TYPE GetObjectType()const noexcept = 0;  
		bool HasFlag(const J_OBJECT_FLAG flag)const noexcept;
	public:
		virtual bool Copy(JObject* ori); 
	private:
		virtual void DoCopy(JObject* ori) = 0;
	protected:
		bool IsActivated() const noexcept;
		void Activate() noexcept;
		void DeActivate()noexcept;
	protected:
		virtual void DoActivate() noexcept;
		virtual void DoDeActivate() noexcept;
	public:
		bool BeginDestroy();
	protected:
		bool BegineForcedDestroy();
	protected: 
		static bool IsIgnoreUndestroyableFlag()noexcept;
		static void SetIgnoreUndestroyableFlag(const bool value)noexcept;
	protected:
		virtual bool Destroy() = 0;
	private:
		bool EndDestroy();
	protected:
		static Core::J_FILE_IO_RESULT StoreMetadata(std::wofstream& stream, JObject* object);
		static Core::J_FILE_IO_RESULT LoadMetadata(std::wifstream& stream, JObjectMetaData& metadata);
	protected:
		JObject(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag = OBJECT_FLAG_NONE);
		virtual ~JObject();
	};
}