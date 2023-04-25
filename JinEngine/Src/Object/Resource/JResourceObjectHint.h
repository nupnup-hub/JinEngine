#include"../../Core/Func/Callable/JCallable.h" 
#include"JResourceObjectType.h"
#include<vector> 
#include<string>

namespace JinEngine
{
	class JResourceObject;
	class JResourceObjectPrivate;
	using GetRTypeInfoCallable = Core::JStaticCallable<Core::JTypeInfo&>;
	using GetAvailableFormatCallable = Core::JStaticCallable<std::vector<std::wstring>>;
	using GetFormatIndexCallable = Core::JStaticCallable<uint8, const std::wstring&>;
 
	using SetRFrameDirtyCallable = Core::JStaticCallable<void, JResourceObject*>;
	using SetRFrameBuffIndexCallable = Core::JStaticCallable<void, JResourceObject*, uint>;

	struct RTypeHint
	{
	public:
		J_RESOURCE_TYPE thisType;
		std::vector<J_RESOURCE_TYPE> hasType;
		bool hasGraphicResource;
		bool isFrameResource;
		//inner object cache = meta + asset .. extract object cache = meta
		bool isInnerResource;
		bool isFixedAssetFile;	//ex) mesh, animation clip
		bool canKeepJFileLife;
		bool canKeepDiskFileLife;
	public:
		RTypeHint(const J_RESOURCE_TYPE thisType,
			const std::vector<J_RESOURCE_TYPE>& hasType,
			const bool hasGraphicResource,
			const bool isFrameResource,		//has frame update
			const bool isInnerResource,
			const bool isFixedAssetFile,
			const bool canKeepJFileLife = true,
			const bool canKeepDiskFileLife = true);
		RTypeHint() = default;
		RTypeHint(const RTypeHint& rhs) = default;
		RTypeHint& operator=(const RTypeHint& rhs) = default;
		~RTypeHint();
	};

	//Resource Func
	struct RTypeCommonFunc
	{
	private:
		GetRTypeInfoCallable* getTypeInfo;
		GetAvailableFormatCallable* getAvailableFormat;
		GetFormatIndexCallable* getFormatIndex;
	public:
		RTypeCommonFunc(GetRTypeInfoCallable& getTypeInfo,
			GetAvailableFormatCallable& getAvailableFormat,
			GetFormatIndexCallable& getFormatIndex);
		RTypeCommonFunc() = default;
		~RTypeCommonFunc();
	public:
		Core::JTypeInfo& CallGetTypeInfo();
		std::vector<std::wstring> CallGetAvailableFormat();
		uint8 CallFormatIndex(const std::wstring& format);
	};

	struct RTypePrivateFunc
	{
	private:
		SetRFrameDirtyCallable* setFrameDirtyCallable = nullptr;
		SetRFrameBuffIndexCallable* setFrameBuffIndexCallable = nullptr;
	public:
		RTypePrivateFunc(SetRFrameDirtyCallable* setFrameDirtyCallable, SetRFrameBuffIndexCallable* setFrameBuffIndexCallable);
		RTypePrivateFunc() = default;
		~RTypePrivateFunc();
	public:
		SetRFrameDirtyCallable GetSetFrameDirtyCallable();
		SetRFrameBuffIndexCallable GetSetFrameBuffIndexCallable();
	public:
		void CallSetFrameDirty(JResourceObject* jRobj);
		void CallSetFrameBuffIndex(JResourceObject* jRobj, const uint value);
	};

	class RTypeRegister
	{
	private:
		friend class JResourceObject;
	private:
		static void RegisterRTypeInfo(const RTypeHint& rTypeHint, const RTypeCommonFunc& rTypeCFunc, const RTypePrivateFunc& rTypeIFunc)noexcept;
	};

	class RTypeCommonCall
	{
	public:
		static std::wstring GetFormat(const J_RESOURCE_TYPE type, const uint8 index); 
		static const RTypeHint GetRTypeHint(const J_RESOURCE_TYPE type)noexcept;
		static const std::vector<RTypeHint> GetRTypeHintVec(const J_RESOURCE_ALIGN_TYPE alignType)noexcept;
	public:
		static std::vector<std::wstring> CallGetAvailableFormat(const J_RESOURCE_TYPE type);
		static Core::JTypeInfo& CallGetTypeInfo(const J_RESOURCE_TYPE type);
		static uint8 CallFormatIndex(const J_RESOURCE_TYPE type, const std::wstring& format);
		static bool CallIsValidFormat(const J_RESOURCE_TYPE type, const std::wstring& format);
	};

	class RTypePrivateCall
	{
	private: 
		friend class JResourceObjectPrivate; 
	private:
		static SetRFrameDirtyCallable GetSetFrameDirtyCallable(const J_RESOURCE_TYPE type);
		static SetRFrameBuffIndexCallable GetSetFrameBuffIndexCallable(const J_RESOURCE_TYPE type);
	private:
		static void CallSetFrameDirty(JResourceObject* jRobj);
		static void CallSetFrameBuffIndex(JResourceObject* jRobj, const uint value);
	};
}