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
		using IsAvailableOverlapCallable = Core::JStaticCallable<bool>;
	protected:
		using SetFrameDirtyCallable = Core::JStaticCallable<void, JComponent&>;
		using SetFrameOffsetCallable = Core::JStaticCallable<void, JComponent&, JComponent*, bool>;
	public:
		struct CTypeHint
		{
		public:
			J_COMPONENT_TYPE thisType;  
			bool hasFrameDirty;
			bool hasFrameOffset;
		public:
			CTypeHint() = default;
			CTypeHint(const J_COMPONENT_TYPE thisType, const bool hasFrameDirty = false, const bool hasFrameOffset = false);
			~CTypeHint();
		};
		struct CTypeCommonFunc
		{
		private:
			GetTypeNameCallable* getTypeName = nullptr;
			GetTypeInfoCallable* getTypeInfo = nullptr;
			IsAvailableOverlapCallable* isAvailableOverlapCallable = nullptr;
		public:
			CTypeCommonFunc() = default;
			CTypeCommonFunc(GetTypeNameCallable& getTypeName,
				GetTypeInfoCallable& getTypeInfo,
				IsAvailableOverlapCallable& isAvailableOverlapCallable);
			~CTypeCommonFunc();
		public:
			std::string CallGetTypeName();
			JTypeInfo& CallGetTypeInfo();
			bool CallIsAvailableOverlapCallable();
		};

		class CTypeInterfaceFunc
		{
		private:
			SetFrameDirtyCallable* setFrameDirtyCallable;
			SetFrameOffsetCallable* setFrameOffsetCallable;
		public:
			CTypeInterfaceFunc(SetFrameDirtyCallable* setFrameDirtyCallable, SetFrameOffsetCallable* setFrameOffsetCallable);
			CTypeInterfaceFunc() = default;
			~CTypeInterfaceFunc();
		public:
			void CallSetFrameDirty(JComponent& jComp);
			void CallSetFrameOffset(JComponent& jComp, JComponent* refComp, bool isCreated);
			SetFrameDirtyCallable* GetSetFrameDirtyCallable();
			SetFrameOffsetCallable* GetSetFrameOffsetCallable();
		};
	public:
		static CTypeHint GetCTypeHint(const J_COMPONENT_TYPE cType);
		static std::vector<CTypeHint> GetCTypeHint(const COMPONET_ALIGN_TYPE alignType = COMPONET_ALIGN_TYPE::NONE)noexcept;
	//Common
	public:
		static JTypeInfo& CallGetTypeInfo(const J_COMPONENT_TYPE cType);
		static std::string CallGetTypeName(const J_COMPONENT_TYPE cType); 	
		static bool CallIsAvailableOverlap(const J_COMPONENT_TYPE cType);
	private:
		virtual Core::J_FILE_IO_RESULT CallStoreComponent(std::wofstream& stream) = 0;
	//Interface
	protected:
		static void CallSetFrameDirty(JComponent& jComp);
		static void CallSetFrameOffset(JComponent& jComp, JComponent* refComp, bool isCreated);
		static SetFrameDirtyCallable* GetSetFrameDirtyCallable(const J_COMPONENT_TYPE cType);
		static SetFrameOffsetCallable* GetSetFrameOffsetCallable(const J_COMPONENT_TYPE cType);
	protected:
		static bool NameOrder(const CTypeHint& a, const CTypeHint& b)noexcept;
	protected:
		static void RegisterTypeInfo(const CTypeHint& cTypeHint, const CTypeCommonFunc& cTypeCFunc, const CTypeInterfaceFunc& cTypeInterfaceFunc)noexcept;
	protected:
		JComponentInterface(const std::wstring& cTypeName, const size_t guid, J_OBJECT_FLAG flag);
	};

	using JCI = JComponentInterface;
}