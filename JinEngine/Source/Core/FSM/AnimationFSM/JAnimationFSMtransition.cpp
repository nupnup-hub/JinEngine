#include"JAnimationFSMtransition.h" 
#include"JAnimationFSMtransitionPrivate.h" 
#include"../JFSMcondition.h"
#include"../JFSMparameter.h"
#include"../JFSMparameterStorageAccess.h" 
#include"../../Identity/JIdenCreator.h"
#include"../../Reflection/JTypeImplBase.h"
#include"../../File/JFileConstant.h" 
#include"../../File/JFileIOHelper.h"
#include<fstream>

namespace JinEngine
{
	namespace Core
	{
		namespace
		{
			static JAnimationFSMtransitionPrivate aPrivate;
		}
		 
		class JAnimationFSMtransition::JAnimationFSMtransitionImpl : public JTypeImplBase
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
				SET_GUI_FLAG(J_GUI_OPTION_DISPLAY_PARENT);
				JIdentifier::RegisterPrivateInterface(JAnimationFSMtransition::StaticTypeInfo(), aPrivate);
				IMPL_REALLOC_BIND(JAnimationFSMtransition::JAnimationFSMtransitionImpl, thisPointer)
			}
		};

		JAnimationFSMtransition::InitData::InitData(const JUserPtr<JFSMstate>& inState, const JUserPtr<JFSMstate>& outState)
			:JFSMtransition::InitData(JAnimationFSMtransition::StaticTypeInfo(), inState, outState)
		{}
		JAnimationFSMtransition::InitData::InitData(const std::wstring& name,
			const size_t guid,
			const JUserPtr<JFSMstate>& inState,
			const JUserPtr<JFSMstate>& outState)
			:JFSMtransition::InitData(JAnimationFSMtransition::StaticTypeInfo(), name, guid, inState, outState)
		{}

		JIdentifierPrivate& JAnimationFSMtransition::PrivateInterface()const noexcept
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

		JOwnerPtr<JIdentifier> CreateInstanceInterface::Create(JDITypeDataBase* initData)
		{
			return JPtrUtil::MakeOwnerPtr<JAnimationFSMtransition>(*static_cast<JAnimationFSMtransition::InitData*>(initData));
		}
		void CreateInstanceInterface::Initialize(JIdentifier* createdPtr, JDITypeDataBase* initData)noexcept
		{
			JFSMtransitionPrivate::CreateInstanceInterface::Initialize(createdPtr, initData);
			JAnimationFSMtransition* trans = static_cast<JAnimationFSMtransition*>(createdPtr);
			trans->impl->RegisterThisPointer(trans); 
		}
		bool CreateInstanceInterface::CanCreateInstance(JDITypeDataBase* initData)const noexcept
		{
			const bool isValidPtr = initData != nullptr && initData->GetTypeInfo().IsChildOf(JAnimationFSMtransition::InitData::StaticTypeInfo());
			return isValidPtr && initData->IsValidData();
		}
 
		J_FILE_IO_RESULT AssetDataIOInterface::LoadAssetData(std::wifstream& stream, const JUserPtr<JAnimationFSMtransition>& trans)
		{
			if (!stream.is_open() || !trans->GetTypeInfo().IsChildOf<JAnimationFSMtransition>())
				return J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

			uint conditionCount = 0;
			JFileIOHelper::LoadAtomicData(stream, conditionCount);

			for (uint i = 0; i < conditionCount; ++i)
			{
				std::wstring condName;
				size_t condGuid;
				J_FSM_OBJECT_TYPE fsmType;
				float onValue;

				JFileIOHelper::LoadFsmObjectIden(stream, condName, condGuid, fsmType);
				JUserPtr<JIdentifier> param = JFileIOHelper::LoadHasObjectIden(stream);
				JFileIOHelper::LoadAtomicData(stream, onValue);

				JUserPtr<JFSMcondition> newCondition = JICI::Create<JFSMcondition>(condName, condGuid, trans);
				if (param.IsValid())
					newCondition->SetParameter(Core::ConnectChildUserPtr<JFSMparameter>(param));
				newCondition->SetOnValue(onValue);
			}

			bool isWaitExitTime = false;
			bool isFrozen = false;
			float exitTimeRate = 0;
			float durationTime = 0;
			float targetStartTimeRate = 0;

			JFileIOHelper::LoadAtomicData(stream, isWaitExitTime);
			JFileIOHelper::LoadAtomicData(stream, isFrozen);
			JFileIOHelper::LoadAtomicData(stream, exitTimeRate);
			JFileIOHelper::LoadAtomicData(stream, durationTime);
			JFileIOHelper::LoadAtomicData(stream, targetStartTimeRate);

			trans->SetIsWaitExitTime(isWaitExitTime);
			trans->SetIsFrozen(isFrozen);
			trans->SetExitTimeRate(exitTimeRate);
			trans->SetDurationTime(durationTime);
			trans->SetTargetStartTimeRate(targetStartTimeRate);
			 
			return J_FILE_IO_RESULT::SUCCESS;
		}
		J_FILE_IO_RESULT AssetDataIOInterface::StoreAssetData(std::wofstream& stream, const JUserPtr<JAnimationFSMtransition>& trans)
		{
			if (!stream.is_open() || !trans->GetTypeInfo().IsChildOf<JAnimationFSMtransition>())
				return J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

			JFileIOHelper::StoreAtomicData(stream, L"ConditionCount:", trans->GetConditioCount());
			const uint conditionCount = trans->GetConditioCount();
			for (uint i = 0; i < conditionCount; ++i)
			{
				JUserPtr<JFSMcondition> cond = trans->GetConditionByIndex(i);
				JFileIOHelper::StoreFsmObjectIden(stream, cond.Get());
				JFileIOHelper::StoreHasObjectIden(stream, cond->GetParameter().Get(), JFileConstant::StreamUncopiableGuidSymbol());
				JFileIOHelper::StoreAtomicData(stream, L"ConditionValue:", trans->GetConditionOnValue(i));
			}
			JFileIOHelper::StoreAtomicData(stream, L"IsWaitExitTime:", trans->IsWaitExitTime());
			JFileIOHelper::StoreAtomicData(stream, L"IsFrozen:", trans->IsFrozen());
			JFileIOHelper::StoreAtomicData(stream, L"ExitGameTimerate:", trans->GetExitTimeRate());
			JFileIOHelper::StoreAtomicData(stream, L"DurationTime:", trans->GetDurationTime());
			JFileIOHelper::StoreAtomicData(stream, L"TargetStateOffset:", trans->GetTargetStartTimeRate());

			return J_FILE_IO_RESULT::SUCCESS;
		}

		void UpdateInterface::Initialize(const JUserPtr<JFSMtransition>& trans)noexcept
		{ 
			static_cast<JAnimationFSMtransition*>(trans.Get())->impl->Initialize();
		}

		JIdentifierPrivate::CreateInstanceInterface& JAnimationFSMtransitionPrivate::GetCreateInstanceInterface()const noexcept
		{
			static CreateInstanceInterface pI;
			return pI;
		}
		JAnimationFSMtransitionPrivate::AssetDataIOInterface& JAnimationFSMtransitionPrivate::GetAssetDataIOInterface()const noexcept
		{
			static AssetDataIOInterface pI;
			return pI;
		}
		JFSMtransitionPrivate::UpdateInterface& JAnimationFSMtransitionPrivate::GetUpdateInterface()const noexcept
		{
			static UpdateInterface pI;
			return pI;
		}
	}
}