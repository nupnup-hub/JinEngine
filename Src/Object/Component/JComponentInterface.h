#pragma once
#include"../JObject.h"
#include"JComponentType.h"
#include<vector>

namespace JinEngine
{
	class JComponent;
	class JComponentInterface : public JObject
	{
	private:
		friend class JGameObject;
		friend class JScene;
	protected:
		using GetTypeNameCallable = Core::JStaticCallable<std::string>;
		using GetTypeInfoCallable = Core::JStaticCallable<JTypeInfo&>; 
	protected:
		using SetFrameDirtyCallable = Core::JStaticCallable<void, JComponent&>;
	public:
		struct CTypeHint
		{
		public:
			J_COMPONENT_TYPE thisType;  
			bool isFrameInterface;
		public:
			CTypeHint() = default;
			CTypeHint(const J_COMPONENT_TYPE thisType, const bool isFrameInterface);
			~CTypeHint();
		};
		struct CTypeCommonFunc
		{
		private:
			GetTypeNameCallable* getTypeName = nullptr;
			GetTypeInfoCallable* getTypeInfo = nullptr;
		public:
			CTypeCommonFunc() = default;
			CTypeCommonFunc(GetTypeNameCallable& getTypeName, GetTypeInfoCallable& getTypeInfo);
			~CTypeCommonFunc();
		public:
			std::string CallGetTypeName();
			JTypeInfo& CallGetTypeInfo();
		};

		class CTypeInterfaceFunc
		{
		private:
			SetFrameDirtyCallable* setFrameDirtyCallable;
		public:
			CTypeInterfaceFunc(SetFrameDirtyCallable* setFrameDirtyCallable);
			CTypeInterfaceFunc() = default;
			~CTypeInterfaceFunc();
		public:
			void CallSetFrameDirty(JComponent& jComp);
			SetFrameDirtyCallable GetSetFrameDirtyCallable();
		};
	public:
		static CTypeHint GetCTypeHint(const J_COMPONENT_TYPE cType);
		static std::vector<CTypeHint> GetCTypeHint(const COMPONET_ALIGN_TYPE alignType = COMPONET_ALIGN_TYPE::NONE)noexcept;
	//Common
	public:
		static JTypeInfo& CallGetTypeInfo(const J_COMPONENT_TYPE cType);
	protected:
		static std::string CallGetTypeName(const J_COMPONENT_TYPE cType); 	
	private:
		virtual Core::J_FILE_IO_RESULT CallStoreComponent(std::wofstream& stream) = 0;
	//Interface
	protected:
		static void CallSetFrameDirty(JComponent& jComp);
		static SetFrameDirtyCallable GetSetFrameDirtyCallable(const J_COMPONENT_TYPE cType);
	protected:
		static bool NameOrder(const CTypeHint& a, const CTypeHint& b)noexcept;
	protected:
		static void RegisterTypeInfo(const CTypeHint& cTypeHint, const CTypeCommonFunc& cTypeCFunc, const CTypeInterfaceFunc& cTypeInterfaceFunc)noexcept;
	protected:
		JComponentInterface(const std::string& cTypeName, const size_t guid, JOBJECT_FLAG flag);
	};

	using JCI = JComponentInterface;
}