#pragma once
#include"../JObject.h"
#include"../../Core/Func/Callable/JCallable.h"
#include"../../Core/Interface/JValidInterface.h" 
#include"JReferenceInterface.h"
#include"JResourceType.h"
#include<vector>
#include<unordered_map>
#
namespace JinEngine
{ 
	class JResourceManagerImpl;
	class JResourceObject; 
	class JDirectory;

	namespace Editor
	{
		class JWindowDirectory;
	}
	class JResourceObjectInterface : public JObject,  
		public JReferenceInterface, 
		public Core::JValidInterface
	{
	private: 
		friend class Editor::JWindowDirectory;
		friend class JResourceManagerImpl; 
	protected:
		using GetTypeNameCallable = Core::JStaticCallable<std::string>;
		using GetAvailableFormatCallable = Core::JStaticCallable<std::vector<std::wstring>>;
		using GetFormatIndexCallable = Core::JStaticCallable<uint8, const std::wstring&>; 
	protected:
		using SetFrameDirtyCallable = Core::JStaticCallable<void, JResourceObject&>;
		using SetFrameBuffIndexCallable = Core::JStaticCallable<void, JResourceObject&, const uint&>;
	public:
		//Resource Hint
		struct RTypeHint
		{
		public:
			J_RESOURCE_TYPE thisType;
			std::vector<J_RESOURCE_TYPE> hasType;
			bool hasGraphicResource;
			bool isFrameResource; 
		public:
			RTypeHint(const J_RESOURCE_TYPE thisType, const std::vector<J_RESOURCE_TYPE>& hasType,
				bool hasGraphicResource,
				bool isFrameResource);
			RTypeHint() = default;
			RTypeHint(const RTypeHint& rhs) = default;
			RTypeHint& operator=(const RTypeHint& rhs) = default;
			~RTypeHint();
		};
		//Resource Func
		class RTypeCommonFunc
		{
		private:
			GetTypeNameCallable* getTypeName;
			GetAvailableFormatCallable* getAvailableFormat;
			GetFormatIndexCallable* getFormatIndex; 
		public:
			RTypeCommonFunc(GetTypeNameCallable& getTypeName,
				GetAvailableFormatCallable& getAvailableFormat,
				GetFormatIndexCallable& getFormatIndex);
			RTypeCommonFunc() = default;
			~RTypeCommonFunc();
		public:
			std::string CallGetTypeName();
			std::vector<std::wstring> CallGetAvailableFormat();
			uint8 CallFormatIndex(const std::wstring& format);
		};

		class RTypeInterfaceFunc
		{
		private:
			SetFrameDirtyCallable* setFrameDirtyCallable = nullptr;
			SetFrameBuffIndexCallable* setFrameBuffIndexCallable = nullptr;
		public:
			RTypeInterfaceFunc(SetFrameDirtyCallable* setFrameDirtyCallable, SetFrameBuffIndexCallable* setFrameBuffIndexCallable);
			RTypeInterfaceFunc() = default;
			~RTypeInterfaceFunc(); 
		public:
			void CallSetFrameDirty(JResourceObject& jRobj);
			void CallSetFrameBuffIndex(JResourceObject& jRobj, const uint value);
		public:
			SetFrameDirtyCallable GetSetFrameDirtyCallable();
			SetFrameBuffIndexCallable GetSetFrameBuffIndexCallable();
		};
	//Call Resource Object Functor
	public:
		//Common 
		//all resource support
		static std::vector<std::wstring> CallGetAvailableFormat(const J_RESOURCE_TYPE type);
		static std::string CallGetTypeName(const J_RESOURCE_TYPE type);
		static uint8 CallFormatIndex(const J_RESOURCE_TYPE type, const std::wstring& format);
		static bool CallIsValidFormat(const J_RESOURCE_TYPE type, const std::wstring& format);
	protected:	
		//Option
		static void CallSetFrameDirty(JResourceObject& jRobj);
		static void CallSetFrameBuffIndex(JResourceObject& jRobj, const uint value); 
	//Type Data
	protected:
		static SetFrameDirtyCallable GetSetFrameDirtyCallable(const J_RESOURCE_TYPE type);
		static SetFrameBuffIndexCallable GetSetFrameBuffIndexCallable(const J_RESOURCE_TYPE type);
	public:
		static const RTypeHint GetRTypeHint(const J_RESOURCE_TYPE type)noexcept;
		static const std::vector<RTypeHint> GetRTypeHintVec(const J_RESOURCE_ALIGN_TYPE alignType)noexcept;
	private:
		//Get Vector ordered by dependency
		static std::vector<RTypeHint> GetDDHintVector()noexcept;
		static std::vector<RTypeHint> SpreadHasType(const RTypeHint& info)noexcept;
		static bool NameOrder(const RTypeHint& a, const RTypeHint& b)noexcept;
	protected:
		virtual Core::J_FILE_IO_RESULT CallStoreResource() = 0;
	protected:
		virtual JDirectory* GetDirectory()const noexcept = 0;
	private:
		virtual void DeleteRFile() = 0;
		virtual void MoveRFile(JDirectory* newDir) = 0;
	protected:
		static void RegisterTypeInfo(const RTypeHint& rTypeHint, const RTypeCommonFunc& rTypeCFunc, const RTypeInterfaceFunc& rTypeIFunc);
	protected:
		JResourceObjectInterface(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag);
	};

	using JRI = JResourceObjectInterface; 
}
