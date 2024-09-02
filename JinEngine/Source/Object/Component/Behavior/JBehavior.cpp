/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


#include"JBehavior.h"  
#include"JBehaviorPrivate.h"
#include"../JComponentHint.h"
#include"../JComponentPrivate.h"
#include"../../JObjectFileIOHelper.h"
#include"../../GameObject/JGameObject.h"  
#include"../../GraphicRule/JGraphicModuleInterfaceHolder.h" 
#include"../../GraphicRule/JGraphicModuleMacro.h"
#include"../../../Core/Guid/JGuidCreator.h"
#include"../../../Core/Reflection/JTypeImplBase.h"
 
namespace JinEngine
{ 
	namespace Private
	{
		static auto isAvailableoverlapLam = []() {return true; };
		static std::unordered_map<size_t, JBehavior::DerivedTypeData> derivedPrivateMap;
		static JBehaviorPrivate instance;
	}
	
	class JBehavior::JBehaviorImpl : public Core::JTypeImplBase
	{
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JBehaviorImpl)
	public:
		JWeakPtr<JBehavior> thisPointer;
	public:
		JFastPtr<JGraphicModuleManagedDataFrame> graphicData = nullptr;
	public:
		JBehaviorImpl(const InitData& initData, JBehavior* thisBehaviorRaw)
		{}
		~JBehaviorImpl()
		{}
	public:
		static bool DoCopy(JBehavior* from, JBehavior* to)
		{
			//미구현
			return true;
		}
	public:
		void Activate()
		{
		}
		void DeActivate()
		{  
		}
	public: 
		void RegisterThisPointer(JBehavior* behav)
		{
			thisPointer = Core::GetWeakPtr(behav);
		}
		static void RegisterTypeData()
		{
			static GetCTypeInfoCallable getTypeInfoCallable{ &JBehavior::StaticTypeInfo };
			static IsAvailableOverlapCallable isAvailableOverlapCallable{ Private::isAvailableoverlapLam };
			using InitUnq = std::unique_ptr<Core::JDITypeDataBase>;
			auto createInitDataLam = [](const Core::JTypeInfo& typeInfo, JUserPtr<JGameObject> parent, InitUnq&& parentClassInitData) -> InitUnq
			{
				using CorrectType = JComponent::ParentType::InitData;
				const bool isValidUnq = parentClassInitData != nullptr && parentClassInitData->GetTypeInfo().IsChildOf(CorrectType::StaticTypeInfo());
				if (isValidUnq)
				{
					CorrectType* ptr = static_cast<CorrectType*>(parentClassInitData.get());
					return std::make_unique<JBehavior::InitData>(typeInfo, ptr->guid, ptr->flag, parent);
				}
				else
					return std::make_unique<JBehavior::InitData>(typeInfo, parent);
			};
			static CreateInitDataCallable createInitDataCallable{ createInitDataLam };

			static CTypeHint cTypeHint{ GetStaticComponentType(), false };
			static CTypeCommonFunc cTypeCommonFunc{ getTypeInfoCallable, isAvailableOverlapCallable, createInitDataCallable };

			JComponent::RegisterCTypeInfo(JBehavior::StaticTypeInfo(), cTypeHint, cTypeCommonFunc, CTypePrivateFunc{});
			Core::JIdentifier::RegisterPrivateInterface(JBehavior::StaticTypeInfo(), Private::instance);

			IMPL_REALLOC_BIND()
		}
	};

	JBehavior::InitData::InitData(const Core::JTypeInfo& typeInfo, const JUserPtr<JGameObject>& owner)
		:JComponent::InitData(typeInfo, owner)
	{}
	JBehavior::InitData::InitData(const Core::JTypeInfo& typeInfo, const size_t guid, const J_OBJECT_FLAG flag, const JUserPtr<JGameObject>& owner)
		: JComponent::InitData(typeInfo, GetDefaultName(JBehavior::StaticTypeInfo()), guid, flag, owner)
	{}

	Core::JIdentifierPrivate& JBehavior::PrivateInterface()const noexcept
	{
		return Private::instance;
	}
	JGraphicModuleManagedDataFrame* JBehavior::ModuleManagedData()const noexcept
	{
		return impl->graphicData.Get();
	}
	uint JBehavior::GetSubTypeIndex()const noexcept
	{
		return 0;
	}
	J_COMPONENT_TYPE JBehavior::GetComponentType()const noexcept
	{
		return GetStaticComponentType();
	}
	bool JBehavior::IsAvailableOverlap()const noexcept
	{
		return Private::isAvailableoverlapLam();
	}
	bool JBehavior::PassDefectInspection()const noexcept
	{
		if (JComponent::PassDefectInspection())
			return true;
		else
			return false;
	}
	void JBehavior::DoActivate()noexcept
	{
		//Caution 
		//Activate와 RegisterComponent는 순서에 종속성을 가진다.
		//RegisterComponent는 Scene과 가속구조에 Component에 대한 정보를 추가하는 작업으로
		//Activate Process중에 자기자신과 관련된 Scene component vector, Scene As관련 data에 대한 호출은 에러를 일으킬 수 있다.
		INTERFACE_ALLOC_GRAPHIC_MODULE_DATA();
		JComponent::DoActivate();

		impl->Activate();
		RegisterComponent(impl->thisPointer);
		NotifyActivate();
	}
	void JBehavior::DoDeActivate()noexcept
	{
		NotifyDeActivate();
		DeRegisterComponent(impl->thisPointer);
		impl->DeActivate();

		JComponent::DoDeActivate();
		DEALLOC_GRAPHIC_MODULE_DATA()
	}
	void JBehavior::NotifyActivate(){}
	void JBehavior::NotifyDeActivate(){}
	void JBehavior::Initialize(){}
	void JBehavior::Clear(){}
	bool JBehavior::Copy(JUserPtr<Core::JIdentifier> to)
	{ 
		return true;
	}
	void JBehavior::RegisterDerivedData(const Core::JTypeInfo& info, const DerivedTypeData& derivedData)
	{
		auto data = Private::derivedPrivateMap.find(info.TypeGuid());
		if (data != Private::derivedPrivateMap.end())
			return;

		Private::derivedPrivateMap.emplace(info.TypeGuid(), derivedData);
	}
	JBehavior::JBehavior(const InitData& initData)
		:JComponent(initData), impl(std::make_unique<JBehaviorImpl>(initData, this))
	{}
	JBehavior::~JBehavior() 
	{
		impl.reset();
	}

	using CreateInstanceInterface = JBehaviorPrivate::CreateInstanceInterface;
	using DestroyInstanceInterface = JBehaviorPrivate::DestroyInstanceInterface;
	using AssetDataIOInterface = JBehaviorPrivate::AssetDataIOInterface;

	JOwnerPtr<Core::JIdentifier> CreateInstanceInterface::Create(Core::JDITypeDataBase* initData)
	{
		auto initPtr = static_cast<JBehavior::InitData*>(initData);
		auto dPrivate = Private::derivedPrivateMap.find(initPtr->initTypeInfo.TypeGuid());
		return dPrivate != Private::derivedPrivateMap.end() ? dPrivate->second.createPtr(initData) : nullptr;
	}
	void CreateInstanceInterface::Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept
	{
		JComponentPrivate::CreateInstanceInterface::Initialize(createdPtr, initData);
		JBehavior* behav = static_cast<JBehavior*>(createdPtr);
		behav->impl->RegisterThisPointer(behav);
		behav->Initialize();
	}
	bool CreateInstanceInterface::CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept
	{
		const bool isValidPtr = initData != nullptr && initData->GetTypeInfo().IsChildOf(JBehavior::InitData::StaticTypeInfo());
		return isValidPtr && initData->IsValidData();
	}
	bool CreateInstanceInterface::Copy(JUserPtr<Core::JIdentifier> from, JUserPtr<Core::JIdentifier> to) noexcept
	{
		const bool canCopy = CanCopy(from, to) && from->GetTypeInfo().IsA(to->GetTypeInfo());
		if (!canCopy)
			return false;
		 
		JUserPtr<JBehavior> fromB = Core::ConvertChildUserPtr<JBehavior>(from);
		JUserPtr<JBehavior> toB = Core::ConvertChildUserPtr<JBehavior>(to);
		const bool res = JBehavior::JBehaviorImpl::DoCopy(fromB.Get(), toB.Get());
		return res && fromB->Copy(toB);
	}

	void DestroyInstanceInterface::Clear(Core::JIdentifier* ptr, const bool isForced)noexcept
	{
		static_cast<JBehavior*>(ptr)->Clear();
		JComponentPrivate::DestroyInstanceInterface::Clear(ptr, isForced);
	}

	JUserPtr<Core::JIdentifier> AssetDataIOInterface::LoadAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JBehavior::LoadData::StaticTypeInfo()))
			return nullptr;

		auto loadData = static_cast<JBehavior::LoadData*>(data);
		JFileIOTool& tool = loadData->tool;
		JUserPtr<JGameObject> owner = loadData->owner;

		std::wstring guide;
		size_t guid;
		J_OBJECT_FLAG flag; 
		bool isActivated;
		JObjectFileIOHelper::LoadComponentIden(tool, guid, flag, isActivated);
		auto iden = Private::instance.GetCreateInstanceInterface().BeginCreate(std::make_unique<JBehavior::InitData>(*loadData->loadTypeInfo, guid, flag, owner), &Private::instance);
		auto bUser = Core::ConvertChildUserPtr<JBehavior>(std::move(iden));
		if (!isActivated)
			bUser->DeActivate();

		return bUser;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JBehavior::StoreData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		auto storeData = static_cast<JBehavior::StoreData*>(data);
		if (!storeData->HasCorrectChildType(JBehavior::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		JBehavior* bComp = static_cast<JBehavior*>(storeData->obj.Get());
		JFileIOTool& tool = storeData->tool;

		JObjectFileIOHelper::StoreComponentIden(tool, bComp);
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}

	Core::JIdentifierPrivate::CreateInstanceInterface& JBehaviorPrivate::GetCreateInstanceInterface()const noexcept
	{
		static CreateInstanceInterface pI;
		return pI;
	}
	Core::JIdentifierPrivate::DestroyInstanceInterface& JBehaviorPrivate::GetDestroyInstanceInterface()const noexcept
	{
		static DestroyInstanceInterface pI;
		return pI;
	}
	JComponentPrivate::AssetDataIOInterface& JBehaviorPrivate::GetAssetDataIOInterface()const noexcept
	{
		static AssetDataIOInterface pI;
		return pI;
	}

}