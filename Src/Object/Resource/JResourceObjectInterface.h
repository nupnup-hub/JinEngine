#pragma once
#include"../JObject.h"
#include"../../Core/Func/Callable/JCallable.h"
#include"JResourceType.h"
#include<vector>
#include<unordered_map>

namespace JinEngine
{
	class JResourceIO;
	class JResourceManager;  

	class JResourceObjectInterface : public JObject
	{
	private:
		friend class JResourceIO;
		friend class JResourceManager;
	protected:
		using GetTypeNameCallable = Core::JStaticCallable<std::string>;
		using GetAvailableFormatCallable = Core::JStaticCallable<std::vector<std::string>>;
		using GetFormatIndexCallable = Core::JStaticCallable<int, const std::string&>;
	public:
		//Resource Hint
		struct RTypeHint
		{
		public:
			J_RESOURCE_TYPE thisType;
			std::vector<J_RESOURCE_TYPE> hasType;
			bool isGraphicResource;
		public:
			RTypeHint(const J_RESOURCE_TYPE thisType, const std::vector<J_RESOURCE_TYPE>& hasType, bool isGraphicResource);
			~RTypeHint();
		};
		//Resource Func
		class RTypeUtil
		{
		private:
			GetTypeNameCallable* getTypeName;
			GetAvailableFormatCallable* getAvailableFormat;
			GetFormatIndexCallable* getFormatIndex;
		public:
			RTypeUtil(GetTypeNameCallable& getTypeName,
				GetAvailableFormatCallable& getAvailableFormat,
				GetFormatIndexCallable& getFormatIndex);
			~RTypeUtil();
			std::string CallGetTypeName();
			std::vector<std::string> CallGetAvailableFormat();
			int CallFormatIndex(const std::string& format);
		};
	public:
		static std::vector<std::string> CallGetAvailableFormat(const J_RESOURCE_TYPE type);
		static std::string CallGetTypeName(const J_RESOURCE_TYPE type);
		static int CallFormatIndex(const J_RESOURCE_TYPE type, const std::string& format);
		static bool CallIsValidFormat(const J_RESOURCE_TYPE type, const std::string& format);
	public:
		static int GetFormatIndex(const J_RESOURCE_TYPE type, const std::string& format);
		static const std::vector<RTypeHint> GetRInfo(const RESOURCE_ALIGN_TYPE alignType);
	protected:
		JResourceObjectInterface(const std::string& name, const size_t guid, const JOBJECT_FLAG flag);
		static void RegisterTypeInfo(const RTypeHint& rTypeHint, const RTypeUtil& rTypeUtil);
	private:
		virtual Core::J_FILE_IO_RESULT CallStoreResource() = 0;
		static std::vector<RTypeHint> GetDDSortedType(const RTypeHint& info)noexcept;
		static bool NameOrder(const RTypeHint& a, const RTypeHint& b)noexcept;
	};

	using JRI = JResourceObjectInterface;
}
