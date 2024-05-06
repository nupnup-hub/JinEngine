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


#include"JAnimationFSMtransition.h" 
#include"JAnimationFSMtransitionPrivate.h" 
#include"../../../JObjectFileIOHelper.h"
#include"../../../../Core/FSM/JFSMparameter.h"
#include"../../../../Core/FSM/JFSMcondition.h"
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

	Core::J_FILE_IO_RESULT AssetDataIOInterface::LoadAssetData(JFileIOTool& tool, const JUserPtr<JAnimationFSMtransition>& trans)
	{
		if (!tool.CanLoad())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		uint conditionCount = 0;
		JObjectFileIOHelper::LoadAtomicData(tool, conditionCount, "ConditionCount:");

		tool.PushExistStack("ConditionData");
		for (uint i = 0; i < conditionCount; ++i)
		{
			std::wstring condName;
			size_t condGuid;
			Core::J_FSM_OBJECT_TYPE fsmType;
			float onValue;

			tool.PushExistStack();
			JObjectFileIOHelper::LoadFsmIden(tool, condName, condGuid, fsmType);
			JUserPtr<Core::JIdentifier> param = JObjectFileIOHelper::_LoadHasIden(tool, "FsmParameter");
			JObjectFileIOHelper::LoadAtomicData(tool, onValue, "ConditionValue");
			tool.PopStack();

			JUserPtr<Core::JFSMcondition> newCondition = JICI::Create<Core::JFSMcondition>(condName, condGuid, trans);
			if (param.IsValid())
				newCondition->SetParameter(Core::ConnectChildUserPtr<Core::JFSMparameter>(param));
			newCondition->SetOnValue(onValue);
		}
		tool.PopStack();

		bool isWaitExitTime = false;
		bool isFrozen = false;
		float exitTimeRate = 0;
		float durationTime = 0;
		float targetStartTimeRate = 0;

		JObjectFileIOHelper::LoadAtomicData(tool, isWaitExitTime, "IsWaitExitTime:");
		JObjectFileIOHelper::LoadAtomicData(tool, isFrozen, "IsFrozen:");
		JObjectFileIOHelper::LoadAtomicData(tool, exitTimeRate, "ExitGameTimerate:");
		JObjectFileIOHelper::LoadAtomicData(tool, durationTime, "DurationTime:");
		JObjectFileIOHelper::LoadAtomicData(tool, targetStartTimeRate, "TargetStateOffset:");
 
		trans->SetIsWaitExitTime(isWaitExitTime);
		trans->SetIsFrozen(isFrozen);
		trans->SetExitTimeRate(exitTimeRate);
		trans->SetDurationTime(durationTime);
		trans->SetTargetStartTimeRate(targetStartTimeRate);

		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreAssetData(JFileIOTool& tool, const JUserPtr<JAnimationFSMtransition>& trans)
	{
		if (!tool.CanStore())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		JObjectFileIOHelper::StoreAtomicData(tool, trans->GetConditioCount(), "ConditionCount:");
		const uint conditionCount = trans->GetConditioCount();
		tool.PushArrayOwner("ConditionData");
		for (uint i = 0; i < conditionCount; ++i)
		{
			tool.PushArrayMember();
			JUserPtr<Core::JFSMcondition> cond = trans->GetConditionByIndex(i);
			JObjectFileIOHelper::StoreFsmIden(tool, cond.Get());
			JObjectFileIOHelper::_StoreHasIden(tool, cond->GetParameter().Get(), "FsmParameter", true);
			JObjectFileIOHelper::StoreAtomicData(tool, trans->GetConditionOnValue(i), "ConditionValue");
			tool.PopStack();
		}
		tool.PopStack();
		JObjectFileIOHelper::StoreAtomicData(tool, trans->IsWaitExitTime(), "IsWaitExitTime:");
		JObjectFileIOHelper::StoreAtomicData(tool, trans->IsFrozen(), "IsFrozen:");
		JObjectFileIOHelper::StoreAtomicData(tool, trans->GetExitTimeRate(), "ExitGameTimerate:");
		JObjectFileIOHelper::StoreAtomicData(tool, trans->GetDurationTime(), "DurationTime:");
		JObjectFileIOHelper::StoreAtomicData(tool, trans->GetTargetStartTimeRate(), "TargetStateOffset:");

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
