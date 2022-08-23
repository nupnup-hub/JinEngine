#pragma once
#include"../JObject.h"
#include"../../Core/Func/Callable/JCallable.h"
#include"JValidInterface.h"
#include"JReferenceInterface.h"
#include"JResourceType.h"
#include<vector>
#include<unordered_map>
#
namespace JinEngine
{
	class JResourceIO;
	class JResourceManagerImpl;
	class JResourceObject; 

	class JResourceObjectInterface : public JObject , public JReferenceInterface, public JValidInterface
	{
	private:
		friend class JResourceIO;
		friend class JResourceManagerImpl; 
	protected:
		using GetTypeNameCallable = Core::JStaticCallable<std::string>;
		using GetAvailableFormatCallable = Core::JStaticCallable<std::vector<std::string>>;
		using GetFormatIndexCallable = Core::JStaticCallable<int, const std::string&>; 
	protected:
		using SetFrameDirtyCallable = Core::JStaticCallable<void, JResourceObject&>;
		using SetBuffIndexCallable = Core::JStaticCallable<void, JResourceObject&, const uint&>;
	public:
		//Resource Hint
		struct RTypeHint
		{
		public:
			J_RESOURCE_TYPE thisType;
			std::vector<J_RESOURCE_TYPE> hasType;
			bool hasGraphicResource;
			bool isFrameResource;
			bool isGraphicBuffResource;
		public:
			RTypeHint(const J_RESOURCE_TYPE thisType, const std::vector<J_RESOURCE_TYPE>& hasType,
				bool hasGraphicResource,
				bool isFrameResource,
				bool isGraphicBuffResource);
			RTypeHint() = default;
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
			std::vector<std::string> CallGetAvailableFormat();
			int CallFormatIndex(const std::string& format); 
		};

		class RTypeInterfaceFunc
		{
		private:
			SetFrameDirtyCallable* setFrameDirtyCallable = nullptr;
			SetBuffIndexCallable* setBuffIndexCallable = nullptr;
		public:
			RTypeInterfaceFunc(SetFrameDirtyCallable* setFrameDirtyCallable, SetBuffIndexCallable* setBuffIndexCallable);
			RTypeInterfaceFunc() = default;
			~RTypeInterfaceFunc(); 
		public:
			void CallSetFrameDirty(JResourceObject& jRobj);
			void CallSetBuffIndex(JResourceObject& jRobj, const uint index);
		public:
			SetFrameDirtyCallable GetSetFrameDirtyCallable();
			SetBuffIndexCallable GetSetBuffIndexCallable();
		};
	//Common
	public:
		static std::vector<std::string> CallGetAvailableFormat(const J_RESOURCE_TYPE type);
		static std::string CallGetTypeName(const J_RESOURCE_TYPE type);
		static int CallFormatIndex(const J_RESOURCE_TYPE type, const std::string& format);
		static bool CallIsValidFormat(const J_RESOURCE_TYPE type, const std::string& format);
	//Interface
	protected:		
		static void CallSetFrameDirty(JResourceObject& jRobj);
		static void CallSetBuffIndex(JResourceObject& jRobj, const uint index);
	protected:
		static SetFrameDirtyCallable GetSetFrameDirtyCallable(const J_RESOURCE_TYPE type);
		static SetBuffIndexCallable GetSetBuffIndexCallable(const J_RESOURCE_TYPE type);
	private:
		virtual Core::J_FILE_IO_RESULT CallStoreResource() = 0;

	public:
		static int GetFormatIndex(const J_RESOURCE_TYPE type, const std::string& format);
		static const RTypeHint GetRTypeHint(const J_RESOURCE_TYPE type)noexcept;
		static const std::vector<RTypeHint> GetRTypeHintVec(const RESOURCE_ALIGN_TYPE alignType)noexcept;
	private:
		static std::vector<RTypeHint> GetDDSortedType(const RTypeHint& info)noexcept;
		static bool NameOrder(const RTypeHint& a, const RTypeHint& b)noexcept;
	protected:
		static void RegisterTypeInfo(const RTypeHint& rTypeHint, const RTypeCommonFunc& rTypeCFunc, const RTypeInterfaceFunc& rTypeIFunc);
	protected:
		JResourceObjectInterface(const std::string& name, const size_t guid, const J_OBJECT_FLAG flag);
	};

	using JRI = JResourceObjectInterface; 
}
