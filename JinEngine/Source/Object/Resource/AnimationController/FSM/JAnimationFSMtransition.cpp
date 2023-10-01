#include"JAnimationFSMtransition.h" 
#include"JAnimationFSMtransitionPrivate.h" 
#include"../../../JObjectFileIOHelper.h"
#include"../../../../Core/FSM/JFSMcondition.h"
#include"../../../../Core/FSM/JFSMparameter.h"
#include"../../../../Core/FSM/JFSMparameterStorageAccess.h" 
#include"../../../../Core/Identity/JIdenCreator.h"
#include"../../../../Core/Reflection/JTypeImplBase.h"
#include"../../../../Core/File/JFileConstant.h"  
#include<fstream>

namespace JinEngine
{
	namespace
	{
		static JAnimationFSMtransitionPrivate aPrivate;
	}

	class JAnimationFSMtransition::JAnimationFSMtransitionImpl : public Core::JTypeImplBase
	{
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JAnimationFSMtransitionImpl)
	public:
		JWeakPtr<JAnimationFSMtransition> thisPointer = nullptr;
	public:
		REGISTER_PROPERTY_EX(exitTimeRate, GetExitTimeRate, SetExitTimeRate, GUI_SLIDER(0.0f, 1.0f))
		float exitTimeRate = 0.75f;
		REGISTER_PROPERTY_EX(durationTime, GetDurationTime, SetDurationTime, GUI_SLIDER(0.0f, 100.0f))
		float durationTime = 0.25f;
		REGISTER_PROPERTY_EX(targetStartTimeRate, GetTargetStartTimeRate, SetTargetStartTimeRate, GUI_SLIDER(0.0f, 1.0f))
		float targetStartTimeRate = 0;
		REGISTER_PROPERTY_EX(isWaitExitTime, IsWaitExitTime, SetIsWaitExitTime, GUI_CHECKBOX())
		bool isWaitExitTime = true;
		REGISTER_PROPERTY_EX(isFrozen, IsFrozen, SetIsFrozen, GUI_CHECKBOX())
		bool isFrozen = false;
	public:
		JAnimationFSMtransitionImpl(const InitData& initData, JAnimationFSMtransition* thisTransRaw)
		{}
		~JAnimationFSMtransitionImpl()
		{}
	public:
		float GetExitTimeRate()const noexcept
		{
			return exitTimeRate;
		}
		float GetDurationTime()const noexcept
		{
			return durationTime;
		}
		float GetTargetStartTimeRate()const noexcept
		{
			return targetStartTimeRate;
		}
	public:
		void SetIsWaitExitTime(const bool value)noexcept
		{
			isWaitExitTime = value;
		}
		void SetIsFrozen(const bool value)noexcept
		{
			isFrozen = value;
		}
		void SetExitTimeRate(const float value)noexcept
		{
			exitTimeRate = value;
		}
		void SetDurationTime(const float value)noexcept
		{
			durationTime = value;
		}
		void SetTargetStartTimeRate(const float value)noexcept
		{
			targetStartTimeRate = value;
		}
	public:
		bool IsWaitExitTime()const noexcept
		{
			return isWaitExitTime;
		}
		bool IsFrozen()const noexcept
		{
			return isFrozen;
		}
		bool IsSatisfiedOption(const float normalizedTime)const noexcept
		{
			if (isWaitExitTime)
			{
				if (normalizedTime >= exitTimeRate)
					return true;
				else
					return false;
			}
			else
				return true;
		}
	public:
		void Initialize()noexcept
		{
			thisPointer->Initialize();
		}
	public:
		void RegisterThisPointer(JAnimationFSMtransition* trans)
		{
			thisPointer = GetWeakPtr(trans);
		}
		static void RegisterTypeData()
		{
			SET_GUI_FLAG(Core::J_GUI_OPTION_DISPLAY_PARENT);
			Core::JIdentifier::RegisterPrivateInterface(JAnimationFSMtransition::StaticTypeInfo(), aPrivate);
			IMPL_REALLOC_BIND(JAnimationFSMtransition::JAnimationFSMtransitionImpl, thisPointer)
		}
	};

	JAnimationFSMtransition::InitData::InitData(const JUserPtr<Core::JFSMstate>& inState, const JUserPtr<Core::JFSMstate>& outState)
		:JFSMtransition::InitData(JAnimationFSMtransition::StaticTypeInfo(), inState, outState)
	{}
	JAnimationFSMtransition::InitData::InitData(const std::wstring& name,
		const size_t guid,
		const JUserPtr<Core::JFSMstate>& inState,
		const JUserPtr<Core::JFSMstate>& outState)
		: JFSMtransition::InitData(JAnimationFSMtransition::StaticTypeInfo(), name, guid, inState, outState)
	{}

	Core::JIdentifierPrivate& JAnimationFSMtransition::PrivateInterface()const noexcept
	{
		return aPrivate;
	}
	float JAnimationFSMtransition::GetExitTimeRate()const noexcept
	{
		return impl->GetExitTimeRate();
	}
	float JAnimationFSMtransition::GetDurationTime()const noexcept
	{
		return impl->GetDurationTime();
	}
	float JAnimationFSMtransition::GetTargetStartTimeRate()const noexcept
	{
		return impl->GetTargetStartTimeRate();
	}
	void JAnimationFSMtransition::SetIsWaitExitTime(const bool value)noexcept
	{
		impl->SetIsWaitExitTime(value);
	}
	void JAnimationFSMtransition::SetIsFrozen(const bool value)noexcept
	{
		impl->SetIsFrozen(value);
	}
	void JAnimationFSMtransition::SetExitTimeRate(const float value)noexcept
	{
		impl->SetExitTimeRate(value);
	}
	void JAnimationFSMtransition::SetDurationTime(const float value)noexcept
	{
		impl->SetDurationTime(value);
	}
	void JAnimationFSMtransition::SetTargetStartTimeRate(const float value)noexcept
	{
		impl->SetTargetStartTimeRate(value);
	}
	bool JAnimationFSMtransition::IsWaitExitTime()const noexcept
	{
		return impl->IsWaitExitTime();
	}
	bool JAnimationFSMtransition::IsFrozen()const noexcept
	{
		return impl->IsFrozen();
	}
	bool JAnimationFSMtransition::IsSatisfiedOption(const float normalizedTime)const noexcept
	{
		return impl->IsSatisfiedOption(normalizedTime);
	}
	JAnimationFSMtransition::JAnimationFSMtransition(const InitData& initData)
		:JFSMtransition(initData), impl(std::make_unique<JAnimationFSMtransitionImpl>(initData, this))
	{}
	JAnimationFSMtransition::~JAnimationFSMtransition()
	{
		impl.reset();
	}

	using CreateInstanceInterface = JAnimationFSMtransitionPrivate::CreateInstanceInterface;
	using AssetDataIOInterface = JAnimationFSMtransitionPrivate::AssetDataIOInterface;
	using UpdateInterface = JAnimationFSMtransitionPrivate::UpdateInterface;

	JOwnerPtr<Core::JIdentifier> CreateInstanceInterface::Create(Core::JDITypeDataBase* initData)
	{
		return Core::JPtrUtil::MakeOwnerPtr<JAnimationFSMtransition>(*static_cast<JAnimationFSMtransition::InitData*>(initData));
	}
	void CreateInstanceInterface::Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept
	{
		JFSMtransitionPrivate::CreateInstanceInterface::Initialize(createdPtr, initData);
		JAnimationFSMtransition* trans = static_cast<JAnimationFSMtransition*>(createdPtr);
		trans->impl->RegisterThisPointer(trans);
	}
	bool CreateInstanceInterface::CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept
	{
		const bool isValidPtr = initData != nullptr && initData->GetTypeInfo().IsChildOf(JAnimationFSMtransition::InitData::StaticTypeInfo());
		return isValidPtr && initData->IsValidData();
	}

	Core::J_FILE_IO_RESULT AssetDataIOInterface::LoadAssetData(std::wifstream& stream, const JUserPtr<JAnimationFSMtransition>& trans)
	{
		if (!stream.is_open() || !trans->GetTypeInfo().IsChildOf<JAnimationFSMtransition>())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		uint conditionCount = 0;
		JObjectFileIOHelper::LoadAtomicData(stream, conditionCount);

		for (uint i = 0; i < conditionCount; ++i)
		{
			std::wstring condName;
			size_t condGuid;
			Core::J_FSM_OBJECT_TYPE fsmType;
			float onValue;

			JObjectFileIOHelper::LoadFsmIden(stream, condName, condGuid, fsmType);
			JUserPtr<Core::JIdentifier> param = JObjectFileIOHelper::_LoadHasIden(stream);
			JObjectFileIOHelper::LoadAtomicData(stream, onValue);

			JUserPtr<Core::JFSMcondition> newCondition = JICI::Create<Core::JFSMcondition>(condName, condGuid, trans);
			if (param.IsValid())
				newCondition->SetParameter(Core::ConnectChildUserPtr<Core::JFSMparameter>(param));
			newCondition->SetOnValue(onValue);
		}

		bool isWaitExitTime = false;
		bool isFrozen = false;
		float exitTimeRate = 0;
		float durationTime = 0;
		float targetStartTimeRate = 0;

		JObjectFileIOHelper::LoadAtomicData(stream, isWaitExitTime);
		JObjectFileIOHelper::LoadAtomicData(stream, isFrozen);
		JObjectFileIOHelper::LoadAtomicData(stream, exitTimeRate);
		JObjectFileIOHelper::LoadAtomicData(stream, durationTime);
		JObjectFileIOHelper::LoadAtomicData(stream, targetStartTimeRate);

		trans->SetIsWaitExitTime(isWaitExitTime);
		trans->SetIsFrozen(isFrozen);
		trans->SetExitTimeRate(exitTimeRate);
		trans->SetDurationTime(durationTime);
		trans->SetTargetStartTimeRate(targetStartTimeRate);

		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreAssetData(std::wofstream& stream, const JUserPtr<JAnimationFSMtransition>& trans)
	{
		if (!stream.is_open() || !trans->GetTypeInfo().IsChildOf<JAnimationFSMtransition>())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		JObjectFileIOHelper::StoreAtomicData(stream, L"ConditionCount:", trans->GetConditioCount());
		const uint conditionCount = trans->GetConditioCount();
		for (uint i = 0; i < conditionCount; ++i)
		{
			JUserPtr<Core::JFSMcondition> cond = trans->GetConditionByIndex(i);
			JObjectFileIOHelper::StoreFsmIden(stream, cond.Get());
			JObjectFileIOHelper::_StoreHasIden(stream, cond->GetParameter().Get(), Core::JFileConstant::StreamUncopiableGuidSymbol());
			JObjectFileIOHelper::StoreAtomicData(stream, L"ConditionValue:", trans->GetConditionOnValue(i));
		}
		JObjectFileIOHelper::StoreAtomicData(stream, L"IsWaitExitTime:", trans->IsWaitExitTime());
		JObjectFileIOHelper::StoreAtomicData(stream, L"IsFrozen:", trans->IsFrozen());
		JObjectFileIOHelper::StoreAtomicData(stream, L"ExitGameTimerate:", trans->GetExitTimeRate());
		JObjectFileIOHelper::StoreAtomicData(stream, L"DurationTime:", trans->GetDurationTime());
		JObjectFileIOHelper::StoreAtomicData(stream, L"TargetStateOffset:", trans->GetTargetStartTimeRate());

		return Core::J_FILE_IO_RESULT::SUCCESS;
	}

	void UpdateInterface::Initialize(const JUserPtr<Core::JFSMtransition>& trans)noexcept
	{
		static_cast<JAnimationFSMtransition*>(trans.Get())->impl->Initialize();
	}

	Core::JIdentifierPrivate::CreateInstanceInterface& JAnimationFSMtransitionPrivate::GetCreateInstanceInterface()const noexcept
	{
		static CreateInstanceInterface pI;
		return pI;
	}
	JAnimationFSMtransitionPrivate::AssetDataIOInterface& JAnimationFSMtransitionPrivate::GetAssetDataIOInterface()const noexcept
	{
		static AssetDataIOInterface pI;
		return pI;
	}
	Core::JFSMtransitionPrivate::UpdateInterface& JAnimationFSMtransitionPrivate::GetUpdateInterface()const noexcept
	{
		static UpdateInterface pI;
		return pI;
	}
}
