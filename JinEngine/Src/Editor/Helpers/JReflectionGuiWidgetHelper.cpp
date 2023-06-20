#include"JReflectionGuiWidgetHelper.h"
#include"JEditorSearchBarHelper.h"
#include"../Event/JEditorEvent.h"
#include"../GuiLibEx/ImGuiEx/JImGuiImpl.h"
#include"../Page/JEditorPageShareData.h"
#include"../Page/WindowInterface/JEditorPreviewInterface.h"
#include"../Page/JEditorWindow.h"

#include"../../Object/Resource/AnimationClip/JAnimationClip.h"
#include"../../Object/Resource/AnimationController/JAnimationController.h"
#include"../../Object/Resource/Material/JMaterial.h"
#include"../../Object/Resource/Mesh/JMeshGeometry.h"
#include"../../Object/Resource/Mesh/JStaticMeshGeometry.h"
#include"../../Object/Resource/Mesh/JSkinnedMeshGeometry.h"
#include"../../Object/Resource/Scene/JScene.h"
#include"../../Object/Resource/Shader/JShader.h"
#include"../../Object/Resource/Skeleton/JSkeletonAsset.h"
#include"../../Object/Resource/Texture/JTexture.h"

#include"../../Object/Resource/Scene/Preview/JPreviewScene.h"
#include"../../Object/Resource/JResourceManager.h"
#include"../../Object/Resource/JResourceObject.h" 
#include"../../Object/Component/Camera/JCamera.h"
#include"../../Object/Component/RenderItem/JRenderItem.h"

#include"../../Core/Identity/JIdentifier.h"
#include"../../Core/Reflection/JTypeImplBase.h"
#include"../../Core/FSM/JFSMinterface.h"
#include"../../Core/FSM/JFSMtransition.h"
#include"../../Core/FSM/JFSMparameter.h"
#include"../../Core/FSM/JFSMcondition.h"
#include"../../Core/Reflection/JPropertyInfo.h"
#include"../../Utility/JVector.h" 
#include"../../Utility/JCommonUtility.h"

namespace JinEngine
{
	namespace Editor
	{
		class JGuiWidgetDisplayHandle;
		class JGuiWidgetGroupHandle;
		class JGuiConditionHandle;
		namespace
		{
			//update data
			struct UpdateData
			{
			public:
				Core::JUserPtr<Core::JIdentifier> obj = nullptr; 
				Core::JGuiWidgetInfoHandleBase* handleBase = nullptr;
				Core::JTypeInfo* updateTypeInfo = nullptr;
			public:
				int widgetIndex; 
			public:
				Core::JGuiWidgetInfo* GetWidgetInfo()const noexcept
				{
					return handleBase->GetWidgetInfo(widgetIndex).Get();
				}
				Core::JTypeImplBase* GetImplBase()const noexcept
				{
					return updateTypeInfo->ConvertImplBase(obj.Get());
				}
			public:
				bool HasImplType() const noexcept
				{
					return GetImplBase() != nullptr;
				}
			};

			//user private data
			struct UserData
			{
			public:
				std::unordered_map<std::string, std::unique_ptr<JGuiWidgetDisplayHandle>> guiWidgetHandleMap;
				std::unordered_map<std::string, std::unique_ptr<JGuiWidgetGroupHandle>> guiGroupHandleMap;
			public:
				JEditorWindow* editorWnd;
			public:
				bool allowDisplayParent = true;
				bool allowDisplayName = true;
				bool allowSameLine = false;
			public:
				//Inner
				bool isActivatedTable;
			public:
				UserData(JEditorWindow* editorWnd)
					:editorWnd(editorWnd)
				{}
			public:
				void InitOptionValue()
				{
					allowDisplayParent = allowDisplayName = true;
					allowSameLine = isActivatedTable = false;
				}
			public:
				void Clear()
				{
					guiWidgetHandleMap.clear();
					guiGroupHandleMap.clear();
				}
			};

			static struct PrivateDataMap
			{
			public:
				static std::unordered_map<size_t, std::unique_ptr<UserData>>& Data()
				{
					static std::unordered_map<size_t, std::unique_ptr<UserData>> privateDataMap;
					return privateDataMap;
				}
			}PrivateDataMap;

			static bool IsEditableObject(Core::JIdentifier* obj)
			{
				if (obj->GetTypeInfo().IsChildOf<JObject>() && !static_cast<JObject*>(obj)->HasFlag(OBJECT_FLAG_UNEDITABLE))
					return true;
				else if (obj->GetTypeInfo().IsChildOf<Core::JFSMinterface>())
					return true;
				else
					return false;
			}
			static std::string MakeUniqueLabel(UpdateData& updateData)
			{
				return updateData.handleBase->GetName() +
					std::to_string(updateData.widgetIndex) +
					"_" +
					std::to_string(updateData.obj->GetGuid());
			}
			static Core::JGuiExtraFunctionUserInfo* GetExtraUserInfo(UpdateData& updateData, const Core::J_GUI_EXTRA_FUNCTION_TYPE type) noexcept
			{
				Core::JGuiWidgetInfo* widgetInfo = updateData.GetWidgetInfo();
				if (!widgetInfo->IsExtraFunctionUser())
					return nullptr;

				return widgetInfo->GetExtraFunctionUserInfo(type).Get();
			}
			//forward declaration 
			static std::unique_ptr<JGuiWidgetDisplayHandle> MakeGuiHandle(Core::JParameterHint pHint, Core::JGuiWidgetInfo* widgetInfo);
			static std::unique_ptr<JGuiWidgetGroupHandle> MakeExtraGroupHandle(Core::JGuiWidgetInfo* widgetInfo);
			static std::unique_ptr<JGuiConditionHandle> MakeExtraConditionHandle(Core::JGuiWidgetInfo* widgetInfo);
			static void SettingDisplayTypeInfo(const Core::JUserPtr<Core::JIdentifier>& obj, Core::JTypeInfo* typeInfo, UserData* userData);
			static void SettingUpdateData(const Core::JUserPtr<Core::JIdentifier>& obj, Core::JTypeInfo* typeInfo, UserData* userData);
			static void DisplayWidget(UpdateData& updateData, UserData* userData);
		}

#pragma region WidgetHandle

		//Widget
		class JGuiWidgetDisplayHandle
		{
		public:
			virtual ~JGuiWidgetDisplayHandle() = default;
		public:
			virtual void Initialize(UpdateData& updateData, UserData* userData) = 0;
			virtual void Update(UpdateData& updateData, UserData* userData) = 0;
		public:
			std::string GetDisplayName(const std::string& label, UpdateData& updateData, UserData* userData)const noexcept
			{
				if (userData->allowDisplayName)
					return updateData.handleBase->GetName() + "##" + std::to_string(updateData.widgetIndex) + label;
				else
					return "##" + updateData.handleBase->GetName() + std::to_string(updateData.widgetIndex) + label;
			}
			void TrySameLine(UserData* userData)const noexcept
			{
				if (userData->allowSameLine)
					ImGui::SameLine();
			}
			void DisplayName(UpdateData& updateData, UserData* userData)const noexcept
			{
				if (userData->allowDisplayName)
					JImGuiImpl::Text(updateData.handleBase->GetName());
			}
		};
		class JGuiObjectValueInterface : public JEditorObjectHandlerInterface
		{
		public:
			template<typename T>
			static T GetValue(UpdateData& updateData)
			{
				if (updateData.HasImplType())
					return static_cast<Core::JGuiWidgetInfoHandle<T>*>(updateData.handleBase)->Get(updateData.GetImplBase());
				else
					return static_cast<Core::JGuiWidgetInfoHandle<T>*>(updateData.handleBase)->Get(updateData.obj.Get());
			}
			template<typename T, typename U>
			static T GetValue(U* obj, Core::JPropertyInfo* pInfo)
			{
				return pInfo->Get<T>(obj);
			}
			template<typename T>
			static T UnsafeGetValue(UpdateData& updateData)
			{
				if (updateData.HasImplType())
					return static_cast<Core::JGuiWidgetInfoHandle<T>*>(updateData.handleBase)->UnsafeGet(updateData.GetImplBase());
				else
					return static_cast<Core::JGuiWidgetInfoHandle<T>*>(updateData.handleBase)->UnsafeGet(updateData.obj.Get());
			}
			template<typename T, typename U>
			static T UnsafeGetValue(U* obj, Core::JPropertyInfo* pInfo)
			{
				return pInfo->UnsafeGet<T>(obj);
			}
		public:
			template<typename T>
			static void SetValue(Core::JGuiWidgetInfoHandle<T>* handle,
				Core::JTypeInstanceSearchHint objHint,
				Core::JTypeInstanceSearchHint modObjHint,
				T value)
			{
				Core::JIdentifier* iden = Core::GetRawPtr<Core::JIdentifier>(objHint);
				Core::JIdentifier* modifiedIden = Core::GetRawPtr<Core::JIdentifier>(modObjHint);
				if (iden == nullptr || modifiedIden == nullptr)
					return;

				if (!IsEditableObject(iden))
					return;

				if (objHint.hasImplType)
					handle->Set(_JReflectionInfo::Instance().GetTypeInfo(objHint.typeGuid)->ConvertImplBase(iden), std::forward<T>(value));
				else
					handle->Set(iden, std::forward<T>(value));

				//JObject is selectable object
				//others is subresource for JObject 
				//ex) FSM object for animationController and it is pageOpenResource
				SetModifiedBit(Core::GetUserPtr(modifiedIden), true);
			}
			template<typename T>
			static void UnsafeSetValue(Core::JGuiWidgetInfoHandle<T>* handle,
				Core::JTypeInstanceSearchHint objHint,
				Core::JTypeInstanceSearchHint modObjHint,
				T value)
			{
				Core::JIdentifier* iden = Core::GetRawPtr<Core::JIdentifier>(objHint);
				Core::JIdentifier* modifiedIden = Core::GetRawPtr<Core::JIdentifier>(modObjHint);
				if (iden == nullptr || modifiedIden == nullptr)
					return;

				if (!IsEditableObject(iden))
					return;

				if (objHint.hasImplType)
					handle->UnsafeSet(_JReflectionInfo::Instance().GetTypeInfo(objHint.typeGuid)->ConvertImplBase(iden), std::forward<T>(value));
				else
					handle->UnsafeSet(iden, std::forward<T>(value)); 

				//JObject is selectable object
				//others is subresource for JObject 
				//ex) FSM object for animationController and it is pageOpenResource
				SetModifiedBit(Core::GetUserPtr(modifiedIden), true);
			}
		protected:
			template<typename T>
			void RequestSetValue(UpdateData& updateData, UserData* userData, T value, bool isUnsafe = false)
			{
				using FType = typename Core::JSFunctorType<void,
					Core::JGuiWidgetInfoHandle<T>*,
					Core::JTypeInstanceSearchHint,
					Core::JTypeInstanceSearchHint,
					T>;

				typename FType::Ptr fPtr;
				if (isUnsafe)
					fPtr = &UnsafeSetValue;
				else
					fPtr = &SetValue;

				std::unique_ptr<FType::Functor> doFunctor = std::make_unique<FType::Functor>(fPtr);
				std::unique_ptr<FType::Functor> undoFunctor = std::make_unique<FType::Functor>(fPtr);

				JUserPtr<Core::JIdentifier> modObj;
				if (!updateData.obj->GetTypeInfo().IsChildOf<JObject>())
					modObj = JEditorPageShareData::GetOpendPageData(userData->editorWnd->GetOwnerPageType()).GetOpenSeleted();
				else
					modObj = updateData.obj;
 
				Core::JGuiWidgetInfoHandle<T>* doHandle = static_cast<Core::JGuiWidgetInfoHandle<T>*>(updateData.handleBase);
				Core::JGuiWidgetInfoHandle<T>* undoHandle = static_cast<Core::JGuiWidgetInfoHandle<T>*>(updateData.handleBase);

				std::unique_ptr<typename FType::CompletelyBind> doBind = std::make_unique<typename FType::CompletelyBind>(std::move(doFunctor),
					std::move(doHandle),
					Core::JTypeInstanceSearchHint(updateData.obj),
					Core::JTypeInstanceSearchHint(modObj),
					std::move(value));

				std::unique_ptr<typename FType::CompletelyBind> undoBind = std::make_unique<typename FType::CompletelyBind>(std::move(undoFunctor),
					std::move(undoHandle),
					Core::JTypeInstanceSearchHint(updateData.obj),
					Core::JTypeInstanceSearchHint(modObj),
					GetValue<T>(updateData));

				std::string taskName = "Set " + updateData.handleBase->GetName() + " value";
				std::string taskDesc = "type name: " + updateData.obj->GetTypeInfo().NameWithOutModifier();
				if (!updateData.obj->GetTypeInfo().IsChildOf<JComponent>())
					taskDesc += "\nobject name: " + JCUtil::WstrToU8Str(updateData.obj->GetName());

				size_t guid;
				auto evStruct = JEditorEvent::RegisterEvStruct(std::make_unique<JEditorTSetBindFuncEvStruct>
					(taskName, taskDesc, userData->editorWnd->GetOwnerPageType(), std::move(doBind), std::move(undoBind)), guid);

				(*userData->editorWnd->GetEvFunctor())(*userData->editorWnd, J_EDITOR_EVENT::T_BIND_FUNC, *evStruct);
			}
		};

		class JGuiWidgetGroupHandle
		{
		public:
			virtual void Update(JGuiWidgetDisplayHandle* widgetHandle, UpdateData& updateData, UserData* userData, const bool displayWidget) = 0;
		};
		//GuiTable 
		//For gui widget grouping
		class JGuiTableHandle : public JGuiWidgetGroupHandle
		{
		private:
			using SuccessUpdateCount = int;
		private:
			std::unordered_map<std::string, SuccessUpdateCount> handleResult;
		private:
			bool isOpen = false;
			uint rowIndex = 0;
			uint columnIndex = 0;
			Core::JGuiFlag flag = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_BordersV |
				ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_RowBg | ImGuiTableFlags_ContextMenuInBody;
		private:
			static const ImGuiTableColumnFlags_ columnDefaultFlag = ImGuiTableColumnFlags_WidthStretch;
		public:
			void Update(JGuiWidgetDisplayHandle* widgetHandle, UpdateData& updateData, UserData* userData, const bool canDisplayWidget)final
			{
				Core::JGuiExtraFunctionUserInfo* extraUserInfo = GetExtraUserInfo(updateData, Core::J_GUI_EXTRA_FUNCTION_TYPE::TABLE);
				if (extraUserInfo == nullptr)
					return;

				Core::JGuiExtraFunctionInfo* extraInfo = extraUserInfo->GetExtraFunctionInfo().Get();
				if (extraInfo == nullptr)
					return;

				Core::JGuiTableUserInfo* tableUserInfo = static_cast<Core::JGuiTableUserInfo*>(extraUserInfo);
				Core::JGuiTableInfo* tableInfo = static_cast<Core::JGuiTableInfo*>(extraInfo);

				auto result = handleResult.find(updateData.handleBase->GetName());
				if (result == handleResult.end())
				{
					handleResult.emplace(updateData.handleBase->GetName(), 0);
					result = handleResult.find(updateData.handleBase->GetName());
				}

				const bool canUpdate = result->second < (INT64)tableUserInfo->GetUseColumnCount();
				if (canUpdate && canDisplayWidget)
				{
					DoUpdate(tableInfo, tableUserInfo, widgetHandle, updateData, userData);
					++result->second;
				}
				if (updateData.handleBase->GetWidgetInfoCount() == updateData.widgetIndex + 1)
				{
					if (result->second == 0)
					{
						++columnIndex;
						if (columnIndex >= tableInfo->GetColumnCount())
						{
							columnIndex = 0;
							++rowIndex;
							if (tableInfo->GetRowCount() <= rowIndex)
								End();
							else
								JImGuiImpl::TableNextRow();
						}
					}
					result->second = 0;
				}
			}
		private:
			void Begin(Core::JGuiTableInfo* tableInfo)
			{
				const uint columnCount = tableInfo->GetColumnCount();
				isOpen = JImGuiImpl::BeginTable("##GuiTable" + tableInfo->GetName(), columnCount, flag);
				if (isOpen)
				{
					for (uint i = 0; i < columnCount; ++i)
						JImGuiImpl::TableSetupColumn(tableInfo->GetColumnGuide(i), columnDefaultFlag);
					JImGuiImpl::TableHeadersRow();
					JImGuiImpl::TableNextRow();
					rowIndex = 0;
					columnIndex = 0;
				}
			}
			void DoUpdate(Core::JGuiTableInfo* tableInfo,
				Core::JGuiTableUserInfo* tableUserInfo,
				JGuiWidgetDisplayHandle* widgetHandle,
				UpdateData& updateData,
				UserData* userData)
			{
				if (rowIndex == 0 && columnIndex == 0)
					Begin(tableInfo);

				if (isOpen)
				{
					userData->allowDisplayName = false;
					if (tableInfo->IsFirstColunmGuide())
					{
						JImGuiImpl::TableSetColumnIndex(0);
						JImGuiImpl::Text(updateData.handleBase->GetName());
						++columnIndex;
					}

					const uint useColumnCount = tableUserInfo->GetUseColumnCount();
					const uint columnMax = tableInfo->GetColumnCount();

					for (uint i = 0; i < useColumnCount; ++i)
					{
						JImGuiImpl::TableSetColumnIndex(columnIndex + i);
						widgetHandle->Update(updateData, userData);
					}
					columnIndex += useColumnCount;

					if (columnIndex >= columnMax)
					{
						columnIndex = 0;
						++rowIndex;
						if (tableInfo->GetRowCount() <= rowIndex)
							End();
						else
							JImGuiImpl::TableNextRow();
					}
					userData->allowDisplayName = true;
				}
			}
			void End()
			{
				JImGuiImpl::EndTable();
				isOpen = false;
				rowIndex = 0;
				columnIndex = 0;
			}
		};
		class JGuiConditionHandle
		{
		public:
			static bool PassCondition(UpdateData& updateData)
			{
				Core::JGuiExtraFunctionUserInfo* extraUser = GetExtraUserInfo(updateData, Core::J_GUI_EXTRA_FUNCTION_TYPE::CONDITION);
				if (extraUser == nullptr)
					return true;

				Core::JGuiExtraFunctionInfo* extraInfo = extraUser->GetExtraFunctionInfo().Get();
				Core::JGuiConditionInfo* conditionInfo = static_cast<Core::JGuiConditionInfo*>(extraInfo);
				if (conditionInfo->GetConditionType() == Core::J_GUI_EXTRA_CONDITION_TYPE::BOOLEAN_PARAM)
					return updateData.HasImplType() ? PassImplBoolCondition(updateData, extraUser, conditionInfo) : PassBoolCondition(updateData, extraUser, conditionInfo);
				else if (conditionInfo->GetConditionType() == Core::J_GUI_EXTRA_CONDITION_TYPE::ENUM_PARAM)
					return updateData.HasImplType() ? PassImplEnumCondition(updateData, extraUser, conditionInfo) : PassEnumCondition(updateData, extraUser, conditionInfo);
				else
				{
					MessageBox(0, L"Not Enum", 0, 0);
					return false;
				}
			}
		private:
			//IsOwnRefParam
			static bool GetReferenceParameterInfo(UpdateData& updateData,
				const std::string& refParamName,
				_Out_ Core::JIdentifier** owner,
				_Out_ Core::JPropertyInfo** pInfo)
			{
				*owner = updateData.obj.Get();
				*pInfo = updateData.handleBase->GetTypeInfo()->GetProperty(refParamName);
				return true;
			}
			//IsOwnRefParam
			static bool GetReferenceParameterInfo(UpdateData& updateData,
				const std::string& refParamName,
				_Out_ Core::JTypeImplBase** owner,
				_Out_ Core::JPropertyInfo** pInfo)
			{
				*owner = updateData.GetImplBase();
				*pInfo = updateData.handleBase->GetTypeInfo()->GetProperty(refParamName);
				return true;
			}
			static bool GetReferenceParameterInfo(UpdateData& updateData,
				const std::string& refParamName,
				const std::string& refParamOwnerName,
				_Out_ Core::JIdentifier** owner,
				_Out_ Core::JPropertyInfo** pInfo)
			{
				Core::JPropertyInfo* refPropertyInfo = updateData.handleBase->GetTypeInfo()->GetProperty(refParamOwnerName);
				*owner = JGuiObjectValueInterface{}.UnsafeGetValue<Core::JIdentifier*>(updateData.obj.Get(), refPropertyInfo);

				if (*owner == nullptr)
					return false;

				*pInfo = (*owner)->GetTypeInfo().GetProperty(refParamName);
				return true;
			}
			static bool GetReferenceParameterInfo(UpdateData& updateData,
				const std::string& refParamName,
				const std::string& refParamOwnerName,
				_Out_ Core::JTypeImplBase** owner,
				_Out_ Core::JPropertyInfo** pInfo)
			{
				Core::JPropertyInfo* refPropertyInfo = updateData.handleBase->GetTypeInfo()->GetProperty(refParamOwnerName);
				*owner = JGuiObjectValueInterface{}.UnsafeGetValue<Core::JTypeImplBase*>(updateData.obj.Get(), refPropertyInfo);

				if (*owner == nullptr)
					return false;

				*pInfo = updateData.obj->GetTypeInfo().GetImplTypeInfo()->GetProperty(refParamName);
				return true;
			} 
			static bool PassBoolCondition(UpdateData& updateData,
				Core::JGuiExtraFunctionUserInfo* extraUser,
				Core::JGuiConditionInfo* conditionInfo)
			{
				Core::JIdentifier* boolOwner = nullptr;
				Core::JPropertyInfo* boolPropertyInfo = nullptr;

				Core::JGuiBoolParamConditionInfo* condInfo = static_cast<Core::JGuiBoolParamConditionInfo*>(conditionInfo);
				Core::JGuiBoolParmConditionUserInfo* condUser = static_cast<Core::JGuiBoolParmConditionUserInfo*>(extraUser);

				if (condUser->IsOwnRefParam())
					GetReferenceParameterInfo(updateData, condInfo->GetRefParamName(), &boolOwner, &boolPropertyInfo);
				else
					GetReferenceParameterInfo(updateData, condInfo->GetRefParamName(), condUser->GetRefParamOwnerName(), &boolOwner, &boolPropertyInfo);

				if (boolOwner == nullptr || boolPropertyInfo == nullptr || boolPropertyInfo->GetHint().jDataEnum != Core::J_PARAMETER_TYPE::Bool)
					return false;

				return condUser->OnTrigger(JGuiObjectValueInterface{}.UnsafeGetValue<bool>(boolOwner, boolPropertyInfo));
			}
			static bool PassImplBoolCondition(UpdateData& updateData,
				Core::JGuiExtraFunctionUserInfo* extraUser,
				Core::JGuiConditionInfo* conditionInfo)
			{
				Core::JTypeImplBase* boolOwner = nullptr;
				Core::JPropertyInfo* boolPropertyInfo = nullptr;

				Core::JGuiBoolParamConditionInfo* condInfo = static_cast<Core::JGuiBoolParamConditionInfo*>(conditionInfo);
				Core::JGuiBoolParmConditionUserInfo* condUser = static_cast<Core::JGuiBoolParmConditionUserInfo*>(extraUser);

				if (condUser->IsOwnRefParam())
					GetReferenceParameterInfo(updateData, condInfo->GetRefParamName(), &boolOwner, &boolPropertyInfo);
				else
					GetReferenceParameterInfo(updateData, condInfo->GetRefParamName(), condUser->GetRefParamOwnerName(), &boolOwner, &boolPropertyInfo);

				if (boolOwner == nullptr || boolPropertyInfo == nullptr || boolPropertyInfo->GetHint().jDataEnum != Core::J_PARAMETER_TYPE::Bool)
					return false;

				return condUser->OnTrigger(JGuiObjectValueInterface{}.UnsafeGetValue<bool>(boolOwner, boolPropertyInfo));
			}
			static bool PassEnumCondition(UpdateData& updateData,
				Core::JGuiExtraFunctionUserInfo* extraUser,
				Core::JGuiConditionInfo* conditionInfo)
			{
				Core::JIdentifier* enumOwner = nullptr;
				Core::JPropertyInfo* enumPropertyInfo = nullptr;

				Core::JGuiEnumParamConditionInfo* condInfo = static_cast<Core::JGuiEnumParamConditionInfo*>(conditionInfo);
				Core::JGuiEnumParamConditionUserInfoInterface* condUser = static_cast<Core::JGuiEnumParamConditionUserInfoInterface*>(extraUser);
				
				if (condUser->IsOwnRefParam())
					GetReferenceParameterInfo(updateData, condInfo->GetRefParamName(), &enumOwner, &enumPropertyInfo);
				else
					GetReferenceParameterInfo(updateData, condInfo->GetRefParamName(), condUser->GetRefParamOwnerName(), &enumOwner, &enumPropertyInfo);
				
				if (enumOwner == nullptr || enumPropertyInfo == nullptr || enumPropertyInfo->GetHint().jDataEnum != Core::J_PARAMETER_TYPE::Enum)
					return false;

				return condUser->OnTrigger(JGuiObjectValueInterface{}.UnsafeGetValue<Core::JEnum>(enumOwner, enumPropertyInfo));
			}
			static bool PassImplEnumCondition(UpdateData& updateData,
				Core::JGuiExtraFunctionUserInfo* extraUser,
				Core::JGuiConditionInfo* conditionInfo)
			{
				Core::JTypeImplBase* enumOwner = nullptr;
				Core::JPropertyInfo* enumPropertyInfo = nullptr;

				Core::JGuiEnumParamConditionInfo* condInfo = static_cast<Core::JGuiEnumParamConditionInfo*>(conditionInfo);
				Core::JGuiEnumParamConditionUserInfoInterface* condUser = static_cast<Core::JGuiEnumParamConditionUserInfoInterface*>(extraUser);
				
				if (condUser->IsOwnRefParam())
					GetReferenceParameterInfo(updateData, condInfo->GetRefParamName(), &enumOwner, &enumPropertyInfo);
				else
					GetReferenceParameterInfo(updateData, condInfo->GetRefParamName(), condUser->GetRefParamOwnerName(), &enumOwner, &enumPropertyInfo);

				if (enumOwner == nullptr || enumPropertyInfo == nullptr || enumPropertyInfo->GetHint().jDataEnum != Core::J_PARAMETER_TYPE::Enum)
					return false;

				return condUser->OnTrigger(JGuiObjectValueInterface{}.UnsafeGetValue<Core::JEnum>(enumOwner, enumPropertyInfo));
			}
		};

		//int, float, string, vector
		template<typename T>
		class JGuiInputHandle : public JGuiWidgetDisplayHandle,
			public JGuiObjectValueInterface
		{
		private:
			using ValueType = typename Core::JVectorDetermine<T>::ValueType;
		private:
			T buff;
			uint exeMaxCount = 0;
			uint exeCount = 0;
		private:
			ImGuiInputTextFlags flag = ImGuiInputTextFlags_None;
		public:
			void Initialize(UpdateData& updateData, UserData* userData)final
			{
				Core::JGuiInputInfo* inputInfo = static_cast<Core::JGuiInputInfo*>(updateData.GetWidgetInfo());
				if (inputInfo->IsEnterToReturn())
					flag |= ImGuiInputTextFlags_EnterReturnsTrue;

				if constexpr (std::is_same_v<T, DirectX::XMINT2> ||
					std::is_same_v<T, DirectX::XMFLOAT2> ||
					std::is_same_v<T, JVector2<ValueType>>)
				{
					exeMaxCount = 2;
				}
				else if constexpr (std::is_same_v<T, DirectX::XMINT3> ||
					std::is_same_v<T, DirectX::XMFLOAT3> ||
					std::is_same_v<T, JVector3<ValueType>>)
				{
					exeMaxCount = 3;
				}
				else if constexpr (std::is_same_v<T, DirectX::XMINT4> ||
					std::is_same_v<T, DirectX::XMFLOAT4> ||
					std::is_same_v<T, JVector4<ValueType>>)
				{
					exeMaxCount = 4;
				}
				else
				{
					exeMaxCount = 1;
					if constexpr (std::is_same_v<std::string, T>)
						buff.resize(JImGuiImpl::GetTextBuffRange());
				}
			}
			void Update(UpdateData& updateData, UserData* userData)final
			{
				buff = GetValue<T>(updateData);
				TrySameLine(userData);
				DisplayName(updateData, userData);
				TrySameLine(userData);

				Core::JGuiInputInfo* inputInfo = static_cast<Core::JGuiInputInfo*>(updateData.GetWidgetInfo());
				bool res = false;
				constexpr bool canUseFixed = std::is_integral_v<T> || std::is_floating_point_v<T>;
				if constexpr (!canUseFixed)
					res = ClassifyInputType(updateData);
				else if (!inputInfo->HasValidFixedParameter())
					res = ClassifyInputType(updateData);
				else
				{
					Core::J_PARAMETER_TYPE fixedParam = inputInfo->GetFixedParameter();
					switch (fixedParam)
					{
					case JinEngine::Core::J_PARAMETER_TYPE::Bool:
					{
						int intBuff = (int)buff;
						res = JImGuiImpl::InputInt("##GuiInputIntHandle" + MakeUniqueLabel(updateData), &intBuff, flag);
						buff = static_cast<T>(std::clamp(intBuff, 0, 1));
						break;
					}
					case JinEngine::Core::J_PARAMETER_TYPE::Int:
					{
						int intBuff = (int)buff;
						res = JImGuiImpl::InputInt("##GuiInputIntHandle" + MakeUniqueLabel(updateData), &intBuff, flag);
						buff = static_cast<T>(intBuff);
						break;
					}
					case JinEngine::Core::J_PARAMETER_TYPE::Float:
					{
						float floatBuff = (float)buff;
						res = JImGuiImpl::InputFloat("##GuiInputFloatHandle" + MakeUniqueLabel(updateData), &floatBuff, flag);
						buff = static_cast<T>(floatBuff);
						break;
					}
					default:
						break;
					}
				}
				if (res)
				{
					if constexpr (std::is_same_v<T, std::string >)
						RequestSetValue<T>(updateData, userData, JCUtil::EraseSideChar(buff, '\0'));
					else
						RequestSetValue<T>(updateData, userData, buff);
				}
				++exeCount;
				if (exeCount == exeMaxCount)
					exeCount = 0;
			}
		private:
			bool ClassifyInputType(UpdateData& updateData)
			{
				if constexpr (std::is_same_v<T, DirectX::XMINT2> ||
					std::is_same_v<T, DirectX::XMFLOAT2> ||
					std::is_same_v<T, JVector2<ValueType>>)
				{
					if (exeCount == 0)
						return InputOnScreen(buff.x, MakeUniqueLabel(updateData) + "00");
					else
						return InputOnScreen(buff.y, MakeUniqueLabel(updateData) + "01");
				}
				else if constexpr (std::is_same_v<T, DirectX::XMINT3> ||
					std::is_same_v<T, DirectX::XMFLOAT3> ||
					std::is_same_v<T, JVector3<ValueType>>)
				{
					if (exeCount == 0)
						return InputOnScreen(buff.x, MakeUniqueLabel(updateData) + "00");
					else if (exeCount == 1)
						return InputOnScreen(buff.y, MakeUniqueLabel(updateData) + "01");
					else
						return InputOnScreen(buff.z, MakeUniqueLabel(updateData) + "02");
				}
				else if constexpr (std::is_same_v<T, DirectX::XMINT4> ||
					std::is_same_v<T, DirectX::XMFLOAT4> ||
					std::is_same_v<T, JVector4<ValueType>>)
				{
					if (exeCount == 0)
						return InputOnScreen(buff.x, MakeUniqueLabel(updateData) + "00");
					else if (exeCount == 1)
						return InputOnScreen(buff.y, MakeUniqueLabel(updateData) + "01");
					else if (exeCount == 2)
						return InputOnScreen(buff.z, MakeUniqueLabel(updateData) + "02");
					else
						return InputOnScreen(buff.w, MakeUniqueLabel(updateData) + "03");
				}
				else
					return InputOnScreen(buff, MakeUniqueLabel(updateData) + "00");
			}
			template<typename InputType>
			bool InputOnScreen(InputType& data, const std::string& uniqSymbol)
			{
				if constexpr (std::is_integral_v<InputType>)
					return JImGuiImpl::InputInt("##GuiInputIntHandle" + uniqSymbol, &data, flag);
				else if constexpr (std::is_floating_point_v<InputType>)
					return JImGuiImpl::InputFloat("##GuiInputFloatHandle" + uniqSymbol, &data, flag);
				else if constexpr (std::is_same_v <std::string, InputType>)
					return JImGuiImpl::InputText("##GuiInputStringHandle" + uniqSymbol, data, flag);
				else
					return false;
			}
		};
		//Select JObject
		class JGuiSelectorHandleHelper : public JGuiWidgetDisplayHandle, public JEditorPreviewInterface
		{
		private:
			using GetElemntVecF = Core::JSFunctorType<std::vector<JUserPtr<Core::JIdentifier>>, JUserPtr<Core::JIdentifier>>;
		private:
			std::vector<JPreviewScene*> selectorPreviewVec;
			std::unique_ptr<JEditorSearchBarHelper> searchBarHelper;
		private:
			Core::J_GUI_SELECTOR_IMAGE imageType;
			float sizeMin = 0;
			float sizeMax = 0;
			float sizeFactor = 0;
		private:
			GetElemntVecF::Ptr getElemenVecPtr = nullptr;
		private:
			bool isFirstOpen = true;
			bool isSelected = false;
			bool isClosePopup = false;
		private:
			bool isRtTexture = false;
			bool hasSizeSlider = false;
		protected:
			JGuiSelectorHandleHelper()
			{
				//!주의필요
				//Activated되는 Selector는 엔진내에서 하나이므로
				//"GuiSelectorInputText"외에 고유라벨은 불필요하다.
				searchBarHelper = std::make_unique<JEditorSearchBarHelper>(false);
			}
		protected:
			void Initialize(UpdateData& updateData, UserData* userData) override
			{
				Core::JGuiSelectorInfo* inputInfo = static_cast<Core::JGuiSelectorInfo*>(updateData.GetWidgetInfo());
				hasSizeSlider = inputInfo->HasSizeSlider();
				imageType = inputInfo->GetPreviewImageType();

				Core::JTypeInfo* handleType = updateData.handleBase->GetTypeInfo();
				const bool canUseImage = handleType->IsChildOf<JObject>() || (handleType->HasInterfaceTypeInfo() && handleType->GetInterfaceTypeInfo()->IsChildOf<JObject>());
				if (!canUseImage)
					imageType = Core::J_GUI_SELECTOR_IMAGE::NONE;

				getElemenVecPtr = inputInfo->GetElementVecPtr();
				if (imageType == Core::J_GUI_SELECTOR_IMAGE::NONE)
				{
					sizeMin = static_cast<float>(JImGuiImpl::GetAlphabetSize().y);
					sizeMax = static_cast<float>(JImGuiImpl::GetAlphabetSize().y);
					hasSizeSlider = false;
				}
				else if (imageType == Core::J_GUI_SELECTOR_IMAGE::ICON)
				{
					sizeMin = (JImGuiImpl::GetClientWindowSize().x * 0.01f);
					sizeMax = (JImGuiImpl::GetClientWindowSize().x * 0.02f);
				}
				else
				{
					sizeMin = (JImGuiImpl::GetClientWindowSize().x * 0.035f);
					sizeMax = (JImGuiImpl::GetClientWindowSize().x * 0.07f);
				}
				sizeFactor = sizeMin;
			}
		protected:
			void Begin(UpdateData& updateData, UserData* userData)
			{
				UpdateValue(updateData);
				if (isClosePopup)
				{
					ClearPreviewGroup();
					ClearSelectorPreviewList();
					selectorPreviewVec.clear();
					isClosePopup = false;
				}
				CreateSelectorPreviewList();
			}
			void End(UpdateData& updateData, UserData* userData)
			{
				if (isSelected)
				{
					SetSelectObject(updateData, userData);
					isSelected = false;
				}
			}
			virtual void UpdateValue(UpdateData& updateData) = 0;
		protected:
			void SelectedPreviewOnScreen(JPreviewScene** previewScene, JObject* previewObject)
			{
				if (!CanCreatePreviewImage())
					return;

				bool hasPreviewScene = previewScene != nullptr && (*previewScene) != nullptr;
				bool hasPreviewObject = previewObject != nullptr;
				bool isNone = !hasPreviewScene && !hasPreviewObject;

				bool isMatch = (hasPreviewScene && hasPreviewObject) && (*previewScene)->GetJObject()->GetGuid() == previewObject->GetGuid();
				if (!isMatch && !isNone)
				{
					DestroyPreviewScene(*previewScene);
					*previewScene = CreatePreviewScene(Core::GetUserPtr(previewObject));
					hasPreviewScene = previewScene != nullptr && (*previewScene) != nullptr;
				}

				if (hasPreviewScene && hasPreviewObject)
				{
					JImGuiImpl::Image((*previewScene)->GetPreviewCamera().Get(), Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, JVector2<float>(sizeMin, sizeMin));
					ImGui::SameLine();
				}
				else
				{
					JImGuiImpl::Image(_JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::NONE).Get(), JVector2<float>(sizeMin, sizeMin));
					ImGui::SameLine();
				}
			}
			void SelectedPreviewOnScreen(JPreviewScene** previewScene, JUserPtr<JObject> previewObject)
			{
				SelectedPreviewOnScreen(previewScene, previewObject.Get());
			}
			template<typename ValueType, typename PointerRef>
			bool SelectorOnScreen(PointerRef selectedObj, UpdateData& updateData, const std::string& uniqueLabel)
			{
				std::string name = "None";
				if constexpr (Core::JUserPtrDetermine<Core::RemoveAll_T<PointerRef>>::value)
				{
					if ((*selectedObj).IsValid())
						name = JCUtil::WstrToU8Str((*selectedObj)->GetName());
				}
				else
				{
					if (selectedObj != nullptr && (*selectedObj) != nullptr)
						name = JCUtil::WstrToU8Str((*selectedObj)->GetName());
				}


				ImGui::SetNextWindowSize(JImGuiImpl::GetClientWindowSize() * 0.3f);
				const std::string comboLabel = "##PreviewSelector" + uniqueLabel;
				if (JImGuiImpl::BeginCombo(comboLabel, name.c_str(), ImGuiComboFlags_HeightLarge | ImGuiComboFlags_PopupAlignLeft))
				{
					ImGui::BeginGroup();
					if (isFirstOpen)
					{
						isFirstOpen = false;
						CreateSelectorList<ValueType>();
						searchBarHelper->ClearInputBuffer();
					}

					JImGuiImpl::Text("Selector");
					if (hasSizeSlider)
					{
						ImGui::SameLine();

						float preFramePaddingY = ImGui::GetStyle().FramePadding.y;
						ImGui::GetStyle().FramePadding.y = 0;

						float textWidth = ImGui::CalcTextSize("Selector").x;
						float sliderWidth = JImGuiImpl::GetSliderWidth();
						float sliderPosX = JImGuiImpl::GetSliderPosX(true);

						if (sliderPosX < textWidth)
							sliderWidth -= (textWidth - sliderPosX);
						ImGui::SetCursorPosX(sliderPosX);
						ImGui::SetNextItemWidth(sliderWidth);
						JImGuiImpl::SliderFloat("##GuiSelectorSlider" + uniqueLabel, &sizeFactor, sizeMin, sizeMax, "", ImGuiSliderFlags_AlwaysClamp);
						if (ImGui::IsItemActive() || ImGui::IsItemHovered())
							ImGui::SetTooltip("%.1f", sizeFactor);

						ImGui::GetStyle().FramePadding.y = preFramePaddingY;
					}
					ImGui::Separator();
					JImGuiImpl::Text("Search");
					ImGui::SameLine();
					searchBarHelper->UpdateSearchBar();

					if (CanCreatePreviewImage())
						ImageSelectorOnScreen<ValueType>(selectedObj, uniqueLabel);
					else
						SimpleSelectorOnScreen<ValueType>(updateData, selectedObj, uniqueLabel);

					ImGui::EndGroup();
					JImGuiImpl::EndCombo();
					return true;
				}
				else
					return false;
			}
		private:
			template<typename ValueType, typename PointerRef>
			void ImageSelectorOnScreen(PointerRef selectedObj, const std::string& uniqueLabel)
			{
				JImGuiImpl::Image(_JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::NONE).Get(), JVector2<float>(sizeMin, sizeMin));
				ImGui::SameLine();
				if (JImGuiImpl::Selectable("None##" + uniqueLabel, nullptr, 0, JVector2<float>(0, sizeFactor)))
				{
					(*selectedObj) = nullptr;
					isSelected = true;
					ImGui::CloseCurrentPopup();
				}

				const uint previweSceneCount = (uint)selectorPreviewVec.size();
				for (uint i = 0; i < previweSceneCount; ++i)
				{
					JUserPtr<Core::JIdentifier> previewObj = selectorPreviewVec[i]->GetJObject();
					if (!searchBarHelper->CanSrcNameOnScreen(previewObj->GetName()))
						continue;

					JImGuiImpl::Image(selectorPreviewVec[i]->GetPreviewCamera().Get(), Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, JVector2<float>(sizeFactor, sizeFactor));
					ImGui::SameLine();
					if (JImGuiImpl::Selectable(JCUtil::WstrToU8Str(previewObj.Get()->GetName()) + "##" + uniqueLabel,
						nullptr,
						0,
						JVector2<float>(0, sizeFactor)))
					{
						(*selectedObj) = static_cast<ValueType*>(previewObj.Get());
						isSelected = true;
						ImGui::CloseCurrentPopup();
					}
				}
			}
			template<typename ValueType, typename PointerRef>
			void SimpleSelectorOnScreen(UpdateData& updateData, PointerRef selectedObj, const std::string& uniqueLabel)
			{
				if (JImGuiImpl::Selectable("None##" + uniqueLabel))
				{
					(*selectedObj) = nullptr;
					isSelected = true;
					ImGui::CloseCurrentPopup();
				}
				if constexpr (std::is_base_of_v<JResourceObject, ValueType>)
				{
					std::vector<Core::JTypeBase*> rVec;
					auto derivedTypeVec = _JReflectionInfo::Instance().GetDerivedTypeInfo(ValueType::StaticTypeInfo());
					for (const auto& data : derivedTypeVec)
					{
						auto rawVec = data->GetInstanceRawPtrVec();
						rVec.insert(rVec.end(), rawVec.begin(), rawVec.end());
					}
					uint count = (uint)rVec.size();
					for (uint i = 0; i < count; ++i)
					{
						JResourceObject* rObj = static_cast<JResourceObject*>(rVec[i]);
						if (rObj->HasFlag(OBJECT_FLAG_HIDDEN) || !searchBarHelper->CanSrcNameOnScreen(rObj->GetName()))
							continue;

						if (JImGuiImpl::Selectable(JCUtil::WstrToU8Str(rObj->GetName()) + "##" + uniqueLabel))
						{
							(*selectedObj) = static_cast<ValueType*>(rObj);
							isSelected = true;
							ImGui::CloseCurrentPopup();
						}
					}
				}
				else
				{
					if (getElemenVecPtr == nullptr)
						return;

					std::vector<JUserPtr<Core::JIdentifier>> elementVec = (*getElemenVecPtr)(updateData.obj);
					const uint elementCount = (uint)elementVec.size();
					for (uint i = 0; i < elementCount; ++i)
					{
						if (elementVec[i]->GetTypeInfo().IsChildOf<JObject>() && static_cast<JObject*>(elementVec[i].Get())->HasFlag(OBJECT_FLAG_HIDDEN))
							continue;

						if (!searchBarHelper->CanSrcNameOnScreen(elementVec[i]->GetName()))
							continue;

						if (JImGuiImpl::Selectable(JCUtil::WstrToU8Str(elementVec[i]->GetName()) + "##" + uniqueLabel))
						{
							(*selectedObj) = static_cast<ValueType*>(elementVec[i].Get());
							isSelected = true;
							ImGui::CloseCurrentPopup();
						}
					}
				}
			}
		protected:
			//Create PreviewList and reCreate unmatch preview
			virtual void CreateSelectorPreviewList() = 0;
			virtual void ClearSelectorPreviewList() = 0;
			template<typename ValueType>
			void CreateSelectorList()
			{
				if (!CanCreatePreviewImage())
					return;

				if constexpr (std::is_base_of_v<JResourceObject, ValueType>)
				{
					std::vector<Core::JTypeBase*> rVec; 
					auto derivedTypeVec = _JReflectionInfo::Instance().GetDerivedTypeInfo(ValueType::StaticTypeInfo(), true);
					for (const auto& data : derivedTypeVec)
					{
						auto rawVec = data->GetInstanceRawPtrVec();
						rVec.insert(rVec.end(), rawVec.begin(), rawVec.end());
					}

					for (auto& data : rVec)
					{
						JResourceObject* rObj = static_cast<JResourceObject*>(data);
						if (!rObj->HasFlag(OBJECT_FLAG_HIDDEN))
							selectorPreviewVec.push_back(CreatePreviewScene(Core::GetUserPtr(rObj)));
					}
				}
			}
		protected:
			bool IsFirstOpen()const noexcept
			{
				return isFirstOpen;
			}
			bool CanCreatePreviewImage()const
			{
				return imageType != Core::J_GUI_SELECTOR_IMAGE::NONE;
			}
		protected:
			uint GetPreviewCount()const noexcept
			{
				return (uint)selectorPreviewVec.size();
			}
		protected:
			void SetFirstTrigger(bool value)noexcept
			{
				isFirstOpen = value;
			}
			void SetCloseTrigger(bool value)noexcept
			{
				isClosePopup = value;
			}
			virtual void SetSelectObject(UpdateData& updateData, UserData* userData) = 0;
		};
		//T is JResourceObject derive class
		template<typename T>
		class JGuiSingleSelectorHandle : public JGuiSelectorHandleHelper, public JGuiObjectValueInterface
		{
		private:
			static constexpr bool isUser = Core::JUserPtrDetermine<typename Core::RemoveAll_T<T>>::value;
			using ValueType = typename Core::JUserPtrDetermine<typename Core::RemoveAll_T<T>>::ElementType;
		private:
			size_t guid = Core::MakeGuid();
		private:
			T selectedObj;
			JPreviewScene* selectedPreview = nullptr;
		public:
			void Initialize(UpdateData& updateData, UserData* userData) final
			{
				JGuiSelectorHandleHelper::Initialize(updateData, userData);
				UpdateValue(updateData);
				CreateSelectorPreviewList();
			}
			void Update(UpdateData& updateData, UserData* userData) final
			{
				TrySameLine(userData);
				DisplayName(updateData, userData);
				Begin(updateData, userData);
				if constexpr (std::is_base_of_v<JObject, ValueType>)
					SelectedPreviewOnScreen(&selectedPreview, selectedObj);
				TrySameLine(userData);
				bool isOpen = false;
				if constexpr (isUser)
				{
					ValueType* ptr = selectedObj.Get();
					isOpen = SelectorOnScreen<ValueType>(&ptr, updateData, MakeUniqueLabel(updateData));
					selectedObj = Core::GetUserPtr(ptr);
				}
				else
					isOpen = SelectorOnScreen<ValueType>(&selectedObj, updateData, MakeUniqueLabel(updateData));
				if (!isOpen)
				{
					SetFirstTrigger(true);
					if (GetPreviewCount() > 0)
						SetCloseTrigger(true);
				}
				End(updateData, userData);
			}
			void UpdateValue(UpdateData& updateData)final
			{
				selectedObj = GetValue<T>(updateData);
			}
		protected:
			void SetSelectObject(UpdateData& updateData, UserData* userData) final
			{
				RequestSetValue(updateData, userData, selectedObj);
			}
		private:
			void CreateSelectorPreviewList()
			{
				if (!CanCreatePreviewImage())
					return;

				if constexpr (std::is_base_of_v<JObject, ValueType>)
				{
					if (selectedPreview != nullptr)
					{
						if (selectedObj == nullptr)
						{
							DestroyPreviewScene(selectedPreview);
							selectedPreview = nullptr;
							return;
						}
						else if (selectedObj->GetGuid() != selectedPreview->GetJObject()->GetGuid())
						{
							DestroyPreviewScene(selectedPreview);
							selectedPreview = nullptr;
						}
						else
							return;
					}

					if (selectedObj == nullptr)
						return;

					if constexpr (isUser)
						selectedPreview = CreatePreviewScene(selectedObj);
					else
						selectedPreview = CreatePreviewScene(Core::GetUserPtr(selectedObj));
				}
			}
			void ClearSelectorPreviewList()
			{
				selectedPreview = nullptr;
			}
		};
		template<typename T>
		class JGuiMultiSelectorHandle : public JGuiSelectorHandleHelper, public JGuiObjectValueInterface
		{
		private:
			static constexpr bool isUser = Core::JUserPtrDetermine<Core::RemoveAll_T<typename Core::StdArrayDetermine<T>::ValueType>>::value;
			using ValueType = typename Core::JUserPtrDetermine<Core::RemoveAll_T<typename Core::StdArrayDetermine<T>::ValueType>>::ElementType;
		private:
			T container;
			std::vector<JPreviewScene*> selectedPreview;
		private:
			uint containerCount = 0;
			int selectedIndex = -1;
		public:
			void Initialize(UpdateData& updateData, UserData* userData) final
			{
				JGuiSelectorHandleHelper::Initialize(updateData, userData);
				UpdateValue(updateData);
				CreateSelectorPreviewList();
			}
			void Update(UpdateData& updateData, UserData* userData) final
			{
				const bool isRenderItemMaterial = updateData.obj->GetTypeInfo().IsA<JRenderItem>() && std::is_base_of_v<JMaterial, ValueType>;
				TrySameLine(userData);
				DisplayName(updateData, userData);
				Begin(updateData, userData);
				for (uint i = 0; i < containerCount; ++i)
				{
					const std::string uniqueLabel = MakeUniqueLabel(updateData) + "_" + std::to_string(i);
					if (isRenderItemMaterial)
					{
						JUserPtr<JRenderItem> rItem;
						rItem.ConnnectChild(updateData.obj);
						JImGuiImpl::Text(JCUtil::WstrToU8Str(rItem->GetMesh()->GetSubMeshName(i)));
					}

					if constexpr (std::is_base_of_v<JObject, ValueType>)
						SelectedPreviewOnScreen(&selectedPreview[i], container[i]);
					TrySameLine(userData);

					bool isOpen = false;
					if constexpr (isUser)
					{
						ValueType* ptr = container[i].Get();
						isOpen = SelectorOnScreen<ValueType>(&ptr, updateData, MakeUniqueLabel(updateData));
						container[i] = Core::GetUserPtr(ptr);
					}
					else
						isOpen = SelectorOnScreen<ValueType>(&container[i], updateData, uniqueLabel);

					if (isOpen)
						selectedIndex = i;
					else
					{
						if (i == selectedIndex)
						{
							SetFirstTrigger(true);
							if (GetPreviewCount() > 0)
								SetCloseTrigger(true);
							selectedIndex = -1;
						}
					}
				}
				End(updateData, userData);
			}
			void UpdateValue(UpdateData& updateData)final
			{
				container = GetValue<T>(updateData);
				containerCount = (uint)container.size();
			}
		protected:
			void SetSelectObject(UpdateData& updateData, UserData* userData) final
			{
				RequestSetValue<T>(updateData, userData, container);
			}
		private:
			void CreateSelectorPreviewList()
			{
				if (!CanCreatePreviewImage())
					return;

				if constexpr (std::is_base_of_v<JObject, ValueType>)
				{
					selectedPreview.resize(containerCount);
					for (uint i = 0; i < containerCount; ++i)
					{
						if (selectedPreview[i] != nullptr)
						{
							if (container[i] == nullptr)
							{
								DestroyPreviewScene(selectedPreview[i]);
								selectedPreview[i] = nullptr;
								return;
							}
							else if (container[i]->GetGuid() != selectedPreview[i]->GetJObject()->GetGuid())
							{
								DestroyPreviewScene(selectedPreview[i]);
								selectedPreview[i] = nullptr;
							}
							else
								continue;
						}

						if constexpr (isUser)
							selectedPreview[i] = CreatePreviewScene(container[i]);
						else
							selectedPreview[i] = CreatePreviewScene(Core::GetUserPtr<JResourceObject>(container[i]));
					}
				}
			}
			void ClearSelectorPreviewList()
			{
				selectedPreview.clear();
			}
		};

		//bool
		class JGuiCheckBoxHandle : public JGuiWidgetDisplayHandle, public JGuiObjectValueInterface
		{
		private:
			bool value = false;
		public:
			void Initialize(UpdateData& updateData, UserData* userData)final {}
			void Update(UpdateData& updateData, UserData* userData)final
			{
				if (!IsConveribleParam(updateData.handleBase->GetFieldHint()))
					return;

				value = GetValue<bool>(updateData);
				TrySameLine(userData);
				if (JImGuiImpl::CheckBox(GetDisplayName("GuiCheckbox" + MakeUniqueLabel(updateData), updateData, userData), value))
					RequestSetValue<bool>(updateData, userData, value);
			}
		public:
			static bool IsConveribleParam(const Core::JParameterHint hint)
			{
				return hint.jDataEnum == Core::J_PARAMETER_TYPE::Bool ||
					hint.jDataEnum == Core::J_PARAMETER_TYPE::Int ||
					hint.jDataEnum == Core::J_PARAMETER_TYPE::Float;
			}
		};
		//int, float
		template<typename T, std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>, int> = 0>
		class JGuiSliderHandle : public JGuiWidgetDisplayHandle, public JGuiObjectValueInterface
		{
		private:
			T minValue;
			T maxValue;
			T value;
		private:
			bool isSupportInput = false;
			bool isVertical = false;
		private:
			ImGuiSliderFlags_ sliderFlag;
		public:
			void Initialize(UpdateData& updateData, UserData* userData)final
			{
				Core::JGuiSliderInfo* sliderInfo = static_cast<Core::JGuiSliderInfo*>(updateData.GetWidgetInfo());

				minValue = static_cast<T>(sliderInfo->GetMinValue());
				maxValue = static_cast<T>(sliderInfo->GetMaxValue());
				isSupportInput = sliderInfo->IsSupportInput();
				isVertical = sliderInfo->IsVertical();
			}
			void Update(UpdateData& updateData, UserData* userData)final
			{
				value = GetValue<T>(updateData);
				DisplayName(updateData, userData);
				ImGui::SameLine();
				if (isSupportInput)
				{
					ImGui::SetNextItemWidth(JImGuiImpl::GetSliderWidth());
					if constexpr (std::is_integral_v<T>)
					{
						if (JImGuiImpl::InputInt("##GuiSliderIntInput" + MakeUniqueLabel(updateData), &value))
							RequestSetValue<T>(updateData, userData, value);
					}
					else
					{
						if (JImGuiImpl::InputFloat("##GuiSliderFloatInput" + MakeUniqueLabel(updateData), &value, 0, "%.1f"))
							RequestSetValue<T>(updateData, userData, value);
					}
					ImGui::SameLine();
				}

				if (isVertical)
				{
					JVector2<float> vSliderSize{ JImGuiImpl::GetClientWindowSize().x * 0.01f, JImGuiImpl::GetClientWindowSize().y * 0.075f };
					if constexpr (std::is_integral_v<T>)
					{
						if (JImGuiImpl::VSliderInt("##GuiIntVSlider" + MakeUniqueLabel(updateData), vSliderSize, &value, minValue, maxValue, "", ImGuiSliderFlags_AlwaysClamp))
							RequestSetValue<T>(updateData, userData, value);
					}
					else
					{
						if (JImGuiImpl::VSliderFloat("##GuiFloatVSlider" + MakeUniqueLabel(updateData), vSliderSize, &value, minValue, maxValue, "", ImGuiSliderFlags_AlwaysClamp))
							RequestSetValue<T>(updateData, userData, value);
					}
				}
				else
				{
					ImGui::SetNextItemWidth(JImGuiImpl::GetSliderWidth());
					if constexpr (std::is_integral_v<T>)
					{
						if (JImGuiImpl::SliderInt("##GuiIntSlider" + MakeUniqueLabel(updateData), &value, minValue, maxValue, "", ImGuiSliderFlags_AlwaysClamp))
							RequestSetValue<T>(updateData, userData, value);
					}
					else
					{
						if (JImGuiImpl::SliderFloat("##GuiFloatSlider" + MakeUniqueLabel(updateData), &value, minValue, maxValue, "", ImGuiSliderFlags_AlwaysClamp))
							RequestSetValue<T>(updateData, userData, value);
					}
				}

				if (ImGui::IsItemActive() || ImGui::IsItemHovered())
					ImGui::SetTooltip("%.1f", value);
			}
		};
		//XMFLOAT4, JVector4, XMFLOAT3, JVector3
		template<typename T>
		class JGuiColorPickerHandle : public JGuiWidgetDisplayHandle, public JGuiObjectValueInterface
		{
		private:
			float hasRgbInput = false;
			T colorV;
		private:
			ImGuiColorEditFlags flag = ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_DisplayRGB;
		private:
			void Initialize(UpdateData& updateData, UserData* userData)final
			{
				Core::JGuiColorPickerInfo* colorPickerInfo = static_cast<Core::JGuiColorPickerInfo*>(updateData.GetWidgetInfo());
				hasRgbInput = colorPickerInfo->HasRgbInput();
				if (hasRgbInput)
					flag |= ImGuiColorEditFlags_InputRGB;
			}
			void Update(UpdateData& updateData, UserData* userData)final
			{
				colorV = GetValue<T>(updateData);
				TrySameLine(userData);
				DisplayName(updateData, userData);
				TrySameLine(userData);
				if constexpr (Core::JVectorDetermine<T>::value)
				{
					if constexpr (T::GetDigitCount() == 3)
					{
						if (ImGui::ColorPicker3(("##GuiColorPicker" + MakeUniqueLabel(updateData)).c_str(), (float*)&colorV, flag))
							RequestSetValue<T>(updateData, userData, colorV);
					}
					else if constexpr (T::GetDigitCount() == 4)
					{
						if (ImGui::ColorPicker4(("##GuiColorPicker" + MakeUniqueLabel(updateData)).c_str(), (float*)&colorV, flag))
							RequestSetValue<T>(updateData, userData, colorV);
					}

				}
				else
				{
					if constexpr (std::is_same_v<T, DirectX::XMFLOAT3>)
					{
						if (ImGui::ColorPicker3(("##GuiColorPicker" + MakeUniqueLabel(updateData)).c_str(), (float*)&colorV, flag))
						{
							if constexpr (std::is_same_v<T, DirectX::XMFLOAT3>)
								RequestSetValue<T>(updateData, userData, DirectX::XMFLOAT3(colorV.x, colorV.y, colorV.z));
						}
					}
					else if constexpr (std::is_same_v<T, DirectX::XMFLOAT4>)
					{
						if (ImGui::ColorPicker4(("##GuiColorPicker" + MakeUniqueLabel(updateData)).c_str(), (float*)&colorV, flag))
						{
							if constexpr (std::is_same_v<T, DirectX::XMFLOAT4>)
								RequestSetValue<T>(updateData, userData, DirectX::XMFLOAT4(colorV.x, colorV.y, colorV.z, colorV.w));
						}
					}
				}
			}
		};
		//int, float ,string, vector
		template<typename T>
		class JGuiReadOnlyTextHandle : public JGuiWidgetDisplayHandle, public JGuiObjectValueInterface
		{
		private:
			using ValueType = typename Core::JVectorDetermine<T>::ValueType;
		public:
			void Initialize(UpdateData& updateData, UserData* userData) final {}
			void Update(UpdateData& updateData, UserData* userData) final
			{
				TrySameLine(userData);
				DisplayName(updateData, userData);
				ImGui::SameLine();
				T value = GetValue<T>(updateData);
				if constexpr (std::is_same_v< ValueType, std::string>)
				{
					if constexpr (std::is_same_v<T, JVector2<ValueType>>)
					{
						JImGuiImpl::Text(value.x + " ");
						ImGui::SameLine();
						JImGuiImpl::Text(value.y);
					}
					else if constexpr (std::is_same_v<T, JVector3<ValueType>>)
					{
						JImGuiImpl::Text(value.x + " ");
						ImGui::SameLine();
						JImGuiImpl::Text(value.y + " ");
						ImGui::SameLine();
						JImGuiImpl::Text(value.z);
					}
					else if constexpr (std::is_same_v<T, JVector4<ValueType>>)
					{
						JImGuiImpl::Text(value.x + " ");
						ImGui::SameLine();
						JImGuiImpl::Text(value.y + " ");
						ImGui::SameLine();
						JImGuiImpl::Text(value.z + " ");
						ImGui::SameLine();
						JImGuiImpl::Text(value.w);
					}
					else
						JImGuiImpl::Text(value);
				}
				else if constexpr (std::is_same_v<T, DirectX::XMINT2> ||
					std::is_same_v<T, DirectX::XMFLOAT2> ||
					std::is_same_v<T, JVector2<ValueType>>)
				{
					JImGuiImpl::Text("X: " + std::to_string(value.x) + " ");
					ImGui::SameLine();
					JImGuiImpl::Text("Y: " + std::to_string(value.y));
				}
				else if constexpr (std::is_same_v<T, DirectX::XMINT3> ||
					std::is_same_v<T, DirectX::XMFLOAT3> ||
					std::is_same_v<T, JVector3<ValueType>>)
				{
					JImGuiImpl::Text("X: " + std::to_string(value.x) + " ");
					ImGui::SameLine();
					JImGuiImpl::Text("Y: " + std::to_string(value.y) + " ");
					ImGui::SameLine();
					JImGuiImpl::Text("Z: " + std::to_string(value.z));
				}
				else if constexpr (std::is_same_v<T, DirectX::XMINT4> ||
					std::is_same_v<T, DirectX::XMFLOAT4> ||
					std::is_same_v<T, JVector4<ValueType>>)
				{
					JImGuiImpl::Text("X: " + std::to_string(value.x) + " ");
					ImGui::SameLine();
					JImGuiImpl::Text("Y: " + std::to_string(value.y) + " ");
					ImGui::SameLine();
					JImGuiImpl::Text("Z: " + std::to_string(value.z) + " ");
					ImGui::SameLine();
					JImGuiImpl::Text("Z: " + std::to_string(value.w));
				}
				else
					JImGuiImpl::Text(std::to_string(value));
			}
		};
		class JGuiEnumComboBoxHandle : public JGuiWidgetDisplayHandle, public JGuiObjectValueInterface
		{
		public:
			void Initialize(UpdateData& updateData, UserData* userData) final {}
			void Update(UpdateData& updateData, UserData* userData) final
			{
				if (updateData.handleBase->GetFieldHint().jDataEnum == Core::J_PARAMETER_TYPE::Enum)
				{
					Core::JGuiEnumComboBoxInfo* enumComboInfo = static_cast<Core::JGuiEnumComboBoxInfo*>(updateData.GetWidgetInfo());
					Core::JEnumInfo* enumInfo = _JReflectionInfo::Instance().GetEnumInfo(enumComboInfo->GetEnumFullName());
					if (enumInfo == nullptr)
						return;

					Core::JEnum enumValue = UnsafeGetValue<Core::JEnum>(updateData);
					int selectedIndex = enumInfo->GetEnumIndex(enumValue);

					TrySameLine(userData);
					const std::string comboLabel = "##GuiEnumComboBox" + MakeUniqueLabel(updateData);
					if (JImGuiImpl::BeginCombo(comboLabel, enumInfo->ElementName(enumValue).c_str(), ImGuiComboFlags_HeightLarge))
					{
						uint enumCount = enumInfo->GetEnumCount();
						for (uint i = 0; i < enumCount; ++i)
						{
							const bool isSelected = (selectedIndex == i);
							if (JImGuiImpl::Selectable(enumInfo->ElementName(enumInfo->EnumValue(i)), isSelected))
							{
								if (selectedIndex != i)
									RequestSetValue<Core::JEnum>(updateData, userData, enumInfo->EnumValue(i), true);
								selectedIndex = i;
							}

							// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
							if (isSelected)
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndCombo();
					}
				}
			}
		};
		//list is array  type
		template<typename T>
		class JGuiListHandle : public JGuiWidgetDisplayHandle, public JGuiObjectValueInterface
		{
		public:
			static constexpr bool isUser = Core::JUserPtrDetermine<Core::RemoveAll_T<typename Core::StdArrayDetermine<T>::ValueType>>::value;
			using ValueType = typename Core::JUserPtrDetermine<Core::RemoveAll_T<typename Core::StdArrayDetermine<T>::ValueType>>::ElementType;
		private:
			T container;
		private:
			int openIndex = -1;
		private:
			//모든 JIdenfier를 상속받는 object는 Factory에서 생성되며
			//Instance는 ownerPtr로 typeInfo class에 저장되고 pointer를 반환한다
			using CreateElementF = Core::JSFunctorType<JUserPtr<Core::JIdentifier>, JUserPtr<Core::JIdentifier>>;
		private:
			Core::J_GUI_LIST_TYPE listType;
			bool canDisplayElementGui;
			CreateElementF::Ptr createElementPtr = nullptr;
		public:
			void Initialize(UpdateData& updateData, UserData* userData) final
			{
				Core::JGuiListInfo* lsitInfo = static_cast<Core::JGuiListInfo*>(updateData.GetWidgetInfo());
				listType = lsitInfo->GetListType();
				canDisplayElementGui = lsitInfo->CanDisplayElementGui();
				createElementPtr = lsitInfo->GetCreateElementPtr();
			}
			void Update(UpdateData& updateData, UserData* userData) final
			{
				container = GetValue<T>(updateData);
				TrySameLine(userData);
				DisplayName(updateData, userData);
				TrySameLine(userData);
				ImGui::BeginGroup();
				ImGui::SetNextWindowSize(JImGuiImpl::GetClientWindowSize() * 0.3f);
				const uint containerCount = (uint)container.size();
				if (JImGuiImpl::BeginListBox("##GulList" + MakeUniqueLabel(updateData)))
				{
					Core::JGuiFlag flag = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_BordersV |
						ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_RowBg | ImGuiTableFlags_ContextMenuInBody;
					const ImGuiTableColumnFlags_ columnDefaultFlag = ImGuiTableColumnFlags_WidthStretch;

					Core::JTypeInfo* typeInfo = &ValueType::StaticTypeInfo();
					Core::JTypeInfoGuiOption* option = typeInfo->GetOption();
					uint rowMax = containerCount;
					uint columnMax = option->GetGuiWidgetInfoHandleCount();

					JImGuiImpl::BeginTable("##GuiListTable" + MakeUniqueLabel(updateData), columnMax, flag);
					for (uint i = 0; i < columnMax; ++i)
						JImGuiImpl::TableSetupColumn(option->GetGuiWidgetInfoHandle(i)->GetName(), columnDefaultFlag);
					JImGuiImpl::TableHeadersRow();

					if (rowMax > 0)
					{
						JImGuiImpl::TableNextRow();
						for (uint i = 0; i < containerCount; ++i)
						{
							//JImGuiImpl::Text(JCUtil::WstrToU8Str(container[i]->GetName()));
							if (canDisplayElementGui)
							{
								userData->allowDisplayName = false;
								userData->isActivatedTable = true;
								if constexpr (isUser)
									SettingDisplayTypeInfo(container[i], &container[i]->GetTypeInfo(), userData);
								else
									SettingDisplayTypeInfo(Core::GetUserPtr(container[i]), &container[i]->GetTypeInfo(), userData);
								userData->isActivatedTable = false;
								userData->allowDisplayName = true;
							}
							if (i + 1 < containerCount)
								JImGuiImpl::TableNextRow();
						}
					}
					JImGuiImpl::EndTable();
					JImGuiImpl::EndListBox();
				}
				if (listType == Core::J_GUI_LIST_TYPE::DYNAMIC && createElementPtr != nullptr)
				{
					if (JImGuiImpl::Button("Add New Object"))
						(*createElementPtr)(updateData.obj);
				}
				ImGui::EndGroup();
			}
		};

#pragma endregion

		namespace
		{

			template<typename T, bool isVector, bool isRawPointer, bool isUserPointer>
			struct JGuiSelectorHandleType;
			template<typename T, bool isRawPointer, bool isUserPointer>
			struct JGuiSelectorHandleType<T, false, isRawPointer, isUserPointer>
			{
			public:
				using Type = JGuiSingleSelectorHandle<std::conditional_t<isRawPointer, T*,
					std::conditional_t<isUserPointer,
					JUserPtr<T>,
					T>>>;
			};
			template<typename T, bool isRawPointer, bool isUserPointer>
			struct JGuiSelectorHandleType<T, true, isRawPointer, isUserPointer>
			{
			public:
				using Type = JGuiMultiSelectorHandle<std::conditional_t<isRawPointer, std::vector<T*>,
					std::conditional_t<isUserPointer,
					std::vector<JUserPtr<T>>,
					std::vector<T>>>>;
			};

			template<typename T, bool isVector, bool isRawPointer, bool isUserPointer>
			struct JGuiHandleType
			{
			public:
				using Type = std::conditional_t<isVector,
					std::conditional_t<isRawPointer, std::vector<T*>, std::conditional_t<isUserPointer, std::vector<JUserPtr<T>>, std::vector<T>>>,
					std::conditional_t<isRawPointer, T*, std::conditional_t<isUserPointer, JUserPtr<T>, T>>>;
			};

			template<typename T>
			static std::unique_ptr<JGuiWidgetDisplayHandle> DoMakeGuiSelectorHandle(const bool isVector, const bool isRawPtr, const bool isUser)
			{
				if (isVector)
				{
					if (isRawPtr)
						return std::make_unique<JGuiSelectorHandleType<T, true, true, false>::Type>();
					else
						return std::make_unique<JGuiSelectorHandleType<T, true, false, true>::Type>();
				}
				else
				{
					if (isRawPtr)
						return std::make_unique<JGuiSelectorHandleType<T, false, true, false>::Type>();
					else
						return std::make_unique<JGuiSelectorHandleType<T, false, false, true>::Type>();
				}
			}

			template<typename T>
			static std::unique_ptr<JGuiWidgetDisplayHandle> DoMakeListGuiHandle(const bool isVector, const bool isRawPtr, const bool isUser)
			{
				if (isVector)
				{
					if (isRawPtr)
						return std::make_unique<JGuiListHandle<JGuiHandleType<T, true, true, false>::Type>>();
					else
						return std::make_unique<JGuiListHandle<JGuiHandleType<T, true, false, true>::Type>>();
				}
				else
					return nullptr;
			}

			static std::unique_ptr<JGuiWidgetDisplayHandle> MakeGuiHandle(Core::JParameterHint pHint, Core::JGuiWidgetInfo* widgetInfo)
			{
				Core::J_PARAMETER_TYPE pType = pHint.jDataEnum;
				bool isVector = pType == Core::J_PARAMETER_TYPE::STD_VECTOR;
				bool isUser = pType == Core::J_PARAMETER_TYPE::USER_PTR;
				bool isRawPtr = pHint.isPtr;

				switch (widgetInfo->GetSupportWidgetType())
				{
				case J_GUI_WIDGET_INPUT:
				{
					if (pType == Core::J_PARAMETER_TYPE::Int)
						return std::make_unique<JGuiInputHandle<int>>();
					else if (pType == Core::J_PARAMETER_TYPE::Float)
						return std::make_unique<JGuiInputHandle<float>>();
					else if (pType == Core::J_PARAMETER_TYPE::String)
						return std::make_unique<JGuiInputHandle<std::string>>();
					else if (pType == Core::J_PARAMETER_TYPE::JVector2)
					{
						if (typeid(JVector2<int>).name() == pHint.name || typeid(JVector2<uint>).name() == pHint.name)
							return std::make_unique<JGuiInputHandle<JVector2<int>>>();
						else if (typeid(JVector2<float>).name() == pHint.name)
							return std::make_unique<JGuiInputHandle<JVector2<float>>>(); 
					}
					else if (pType == Core::J_PARAMETER_TYPE::JVector3)
					{
						if (typeid(JVector3<int>).name() == pHint.name || typeid(JVector3<uint>).name() == pHint.name)
							return std::make_unique<JGuiInputHandle<JVector3<int>>>();
						else if (typeid(JVector3<float>).name() == pHint.name)
							return std::make_unique<JGuiInputHandle<JVector3<float>>>(); 
					}
					else if (pType == Core::J_PARAMETER_TYPE::JVector4)
					{
						if (typeid(JVector4<int>).name() == pHint.name || typeid(JVector4<uint>).name() == pHint.name)
							return std::make_unique<JGuiInputHandle<JVector4<int>>>();
						else if (typeid(JVector4<float>).name() == pHint.name)
							return std::make_unique<JGuiInputHandle<JVector4<float>>>(); 
					}
					else if (pType == Core::J_PARAMETER_TYPE::XMInt2)
						return std::make_unique<JGuiInputHandle<DirectX::XMINT2>>();
					else if (pType == Core::J_PARAMETER_TYPE::XMInt3)
						return std::make_unique<JGuiInputHandle<DirectX::XMINT3>>();
					else if (pType == Core::J_PARAMETER_TYPE::XMInt4)
						return std::make_unique<JGuiInputHandle<DirectX::XMINT4>>();
					else if (pType == Core::J_PARAMETER_TYPE::XMFloat2)
						return std::make_unique<JGuiInputHandle<DirectX::XMFLOAT2>>();
					else if (pType == Core::J_PARAMETER_TYPE::XMFloat3)
						return std::make_unique<JGuiInputHandle<DirectX::XMFLOAT3>>();
					else if (pType == Core::J_PARAMETER_TYPE::XMFloat4)
						return std::make_unique<JGuiInputHandle<DirectX::XMFLOAT4>>();
					break;
				}
				case J_GUI_WIDGET_CHECKBOX:
				{
					if (JGuiCheckBoxHandle::IsConveribleParam(pHint))
						return std::make_unique<JGuiCheckBoxHandle>();
					break;
				}
				case J_GUI_WIDGET_SLIDER:
				{
					if (pType == Core::J_PARAMETER_TYPE::Int)
						return std::make_unique<JGuiSliderHandle<int>>();
					else if (pType == Core::J_PARAMETER_TYPE::Float)
						return std::make_unique<JGuiSliderHandle<float>>();
					break;
				}
				case J_GUI_WIDGET_COLOR_PICKER:
				{
					if (pType == Core::J_PARAMETER_TYPE::XMFloat4)
						return std::make_unique<JGuiColorPickerHandle<DirectX::XMFLOAT4>>();
					else if (pType == Core::J_PARAMETER_TYPE::JVector4)
						return std::make_unique<JGuiColorPickerHandle<JVector4<float>>>();
					else if (pType == Core::J_PARAMETER_TYPE::XMFloat3)
						return std::make_unique<JGuiColorPickerHandle<DirectX::XMFLOAT3>>();
					else if (pType == Core::J_PARAMETER_TYPE::JVector3)
						return std::make_unique<JGuiColorPickerHandle<JVector3<float>>>();
					break;
				}
				case J_GUI_WIDGET_SELECTOR:
				{
					Core::JTypeInfo* typeinfo = nullptr;
					//if success isA casting
					//exe downcast
					typeinfo = _JReflectionInfo::Instance().GetTypeInfo(pHint.valueTypeFullName);
					if (typeinfo != nullptr)
					{
						if (typeinfo->IsA<JAnimationClip>())
							return DoMakeGuiSelectorHandle<JAnimationClip>(isVector, isRawPtr, isUser);
						else if (typeinfo->IsA<JAnimationController>())
							return DoMakeGuiSelectorHandle<JAnimationController>(isVector, isRawPtr, isUser);
						else if (typeinfo->IsA<JMaterial>())
							return DoMakeGuiSelectorHandle<JMaterial>(isVector, isRawPtr, isUser);
						else if (typeinfo->IsA<JMeshGeometry>())
							return DoMakeGuiSelectorHandle<JMeshGeometry>(isVector, isRawPtr, isUser);
						else if (typeinfo->IsA<JStaticMeshGeometry>())
							return DoMakeGuiSelectorHandle<JStaticMeshGeometry>(isVector, isRawPtr, isUser);
						else if (typeinfo->IsA<JSkinnedMeshGeometry>())
							return DoMakeGuiSelectorHandle<JSkinnedMeshGeometry>(isVector, isRawPtr, isUser);
						else if (typeinfo->IsA<JScene>())
							return DoMakeGuiSelectorHandle<JScene>(isVector, isRawPtr, isUser);
						else if (typeinfo->IsA<JShader>())
							return DoMakeGuiSelectorHandle<JShader>(isVector, isRawPtr, isUser);
						else if (typeinfo->IsA<JSkeletonAsset>())
							return DoMakeGuiSelectorHandle<JSkeletonAsset>(isVector, isRawPtr, isUser);
						else if (typeinfo->IsA<JTexture>())
							return DoMakeGuiSelectorHandle<JTexture>(isVector, isRawPtr, isUser);
						else if (typeinfo->IsA<Core::JFSMparameter>())
							return DoMakeGuiSelectorHandle<Core::JFSMparameter>(isVector, isRawPtr, isUser);
					}
					break;
				}
				case J_GUI_WIDGET_READONLY_TEXT:
				{	if (pType == Core::J_PARAMETER_TYPE::Int)
					return std::make_unique<JGuiReadOnlyTextHandle<int>>();
				else if (pType == Core::J_PARAMETER_TYPE::Float)
					return std::make_unique<JGuiReadOnlyTextHandle<float>>();
				else if (pType == Core::J_PARAMETER_TYPE::String)
					return std::make_unique<JGuiReadOnlyTextHandle<std::string>>();
				else if (pType == Core::J_PARAMETER_TYPE::JVector2)
				{
					if (typeid(JVector2<int>).name() == pHint.name || typeid(JVector2<uint>).name() == pHint.name)
						return std::make_unique<JGuiReadOnlyTextHandle<JVector2<int>>>();
					else if (typeid(JVector2<float>).name() == pHint.name)
						return std::make_unique<JGuiReadOnlyTextHandle<JVector2<float>>>(); 
				}
				else if (pType == Core::J_PARAMETER_TYPE::JVector3)
				{
					if (typeid(JVector3<int>).name() == pHint.name || typeid(JVector3<uint>).name() == pHint.name)
						return std::make_unique<JGuiReadOnlyTextHandle<JVector3<int>>>();
					else if (typeid(JVector3<float>).name() == pHint.name)
						return std::make_unique<JGuiReadOnlyTextHandle<JVector3<float>>>(); 
				}
				else if (pType == Core::J_PARAMETER_TYPE::JVector4)
				{
					if (typeid(JVector4<int>).name() == pHint.name || typeid(JVector4<uint>).name() == pHint.name)
						return std::make_unique<JGuiReadOnlyTextHandle<JVector4<int>>>();
					else if (typeid(JVector4<float>).name() == pHint.name)
						return std::make_unique<JGuiReadOnlyTextHandle<JVector4<float>>>(); 
				}
				else if (pType == Core::J_PARAMETER_TYPE::XMInt2)
					return std::make_unique<JGuiReadOnlyTextHandle<DirectX::XMINT2>>();
				else if (pType == Core::J_PARAMETER_TYPE::XMInt3)
					return std::make_unique<JGuiReadOnlyTextHandle<DirectX::XMINT3>>();
				else if (pType == Core::J_PARAMETER_TYPE::XMInt4)
					return std::make_unique<JGuiReadOnlyTextHandle<DirectX::XMINT4>>();
				else if (pType == Core::J_PARAMETER_TYPE::XMFloat2)
					return std::make_unique<JGuiReadOnlyTextHandle<DirectX::XMFLOAT2>>();
				else if (pType == Core::J_PARAMETER_TYPE::XMFloat3)
					return std::make_unique<JGuiReadOnlyTextHandle<DirectX::XMFLOAT3>>();
				else if (pType == Core::J_PARAMETER_TYPE::XMFloat4)
					return std::make_unique<JGuiReadOnlyTextHandle<DirectX::XMFLOAT4>>();
				break;
				}
				case J_GUI_WIDGET_ENUM_COMBO:
				{
					if (pType == Core::J_PARAMETER_TYPE::Enum)
						return std::make_unique<JGuiEnumComboBoxHandle>();
					break;
				}
				case J_GUI_WIDGET_LIST:
				{
					Core::JTypeInfo* typeinfo = nullptr;
					if (pType != Core::J_PARAMETER_TYPE::STD_VECTOR)
						break;

					//if success isA casting
					//exe downcast
					typeinfo = _JReflectionInfo::Instance().GetTypeInfo(pHint.valueTypeFullName);
					if (typeinfo != nullptr)
					{
						if (typeinfo->IsA<JAnimationClip>())
							return DoMakeListGuiHandle<JAnimationClip>(isVector, isRawPtr, isUser);
						else if (typeinfo->IsA<JAnimationController>())
							return DoMakeListGuiHandle<JAnimationController>(isVector, isRawPtr, isUser);
						else if (typeinfo->IsA<JMaterial>())
							return DoMakeListGuiHandle<JMaterial>(isVector, isRawPtr, isUser);
						else if (typeinfo->IsA<JMeshGeometry>())
							return DoMakeListGuiHandle<JMeshGeometry>(isVector, isRawPtr, isUser);
						else if (typeinfo->IsA<JStaticMeshGeometry>())
							return DoMakeListGuiHandle<JStaticMeshGeometry>(isVector, isRawPtr, isUser);
						else if (typeinfo->IsA<JSkinnedMeshGeometry>())
							return DoMakeListGuiHandle<JSkinnedMeshGeometry>(isVector, isRawPtr, isUser);
						else if (typeinfo->IsA<JScene>())
							return DoMakeListGuiHandle<JScene>(isVector, isRawPtr, isUser);
						else if (typeinfo->IsA<JShader>())
							return DoMakeListGuiHandle<JShader>(isVector, isRawPtr, isUser);
						else if (typeinfo->IsA<JSkeletonAsset>())
							return DoMakeListGuiHandle<JSkeletonAsset>(isVector, isRawPtr, isUser);
						else if (typeinfo->IsA<JTexture>())
							return DoMakeListGuiHandle<JTexture>(isVector, isRawPtr, isUser);
						else if (typeinfo->IsA<Core::JFSMcondition>())
							return DoMakeListGuiHandle<Core::JFSMcondition>(isVector, isRawPtr, isUser);

						/*else if (typeinfo->IsA<JResourceObject>())
						{
							if (isVector)
								return std::make_unique<JGuiSelectorHandle<std::vector<JResourceObject*>>>();
							else
								return std::make_unique<JGuiSelectorHandle<JResourceObject*>>();
						}*/
					}
					break;
				}
				default:
					return nullptr;
				}
				return nullptr;
			}
			static std::unique_ptr<JGuiWidgetGroupHandle> MakeExtraPropertyGroupHandle(Core::JGuiWidgetInfo* widgetInfo)
			{
				Core::JGuiExtraFunctionUserInfo* groupUserInfo = widgetInfo->GetExtraFunctionUserInfo(Core::J_GUI_EXTRA_FUNCTION_TYPE::TABLE).Get();
				if (groupUserInfo != nullptr)
				{
					auto info = groupUserInfo->GetExtraFunctionInfo(); 
					if (info->GetExtraFunctionType() == Core::J_GUI_EXTRA_FUNCTION_TYPE::TABLE)
						return std::make_unique<JGuiTableHandle>();
				}
				return nullptr;
			}
			static std::unique_ptr<JGuiConditionHandle> MakeExtraConditionHandle(Core::JGuiWidgetInfo* widgetInfo)
			{
				Core::JGuiExtraFunctionUserInfo* condUserInfo = widgetInfo->GetExtraFunctionUserInfo(Core::J_GUI_EXTRA_FUNCTION_TYPE::CONDITION).Get();
				if (condUserInfo != nullptr)
					return std::make_unique<JGuiConditionHandle>();
				return nullptr;
			}
			static void SettingDisplayTypeInfo(const Core::JUserPtr<Core::JIdentifier>& obj, Core::JTypeInfo* typeInfo, UserData* userData)
			{
				if (obj == nullptr)
					return;

				Core::JTypeInfoGuiOption* typeOption = typeInfo->GetOption();
				const bool canDisplayParent = userData->allowDisplayParent && Core::HasSQValueEnum(typeOption->GetGuiWidgetFlag(), Core::J_GUI_OPTION_DISPLAY_PARENT);
				const bool isParentToChild = Core::HasSQValueEnum(typeOption->GetGuiWidgetFlag(), Core::J_GUI_OPTION_DISPLAY_PARENT_TO_CHILD);
				if (canDisplayParent)
				{
					if (isParentToChild)
					{
						std::vector<Core::JTypeInfo*> infoVec;
						Core::JTypeInfo* nowInfo = typeInfo;
						while (nowInfo != nullptr)
						{
							infoVec.push_back(nowInfo);
							nowInfo = nowInfo->GetParent();
						}
						std::reverse(infoVec.begin(), infoVec.end());
						const uint infoCount = (uint)infoVec.size();
						for (uint i = 0; i < infoCount; ++i)
							SettingUpdateData(obj, infoVec[i], userData);
					}
					else
					{
						Core::JTypeInfo* nowInfo = typeInfo;
						while (nowInfo != nullptr)
						{
							SettingUpdateData(obj, nowInfo, userData);
							nowInfo = nowInfo->GetParent();
						}
					}
				}
				else
					SettingUpdateData(obj, typeInfo, userData);
			}
			static void SettingUpdateData(const Core::JUserPtr<Core::JIdentifier>& obj, Core::JTypeInfo* typeInfo, UserData* userData)
			{
				Core::JTypeInfoGuiOption* typeOption = nullptr;
				if (typeInfo->HasImplTypeInfo())
					typeOption = typeInfo->GetImplTypeInfo()->GetOption();
				else
					typeOption = typeInfo->GetOption();

				const uint widgetHandleCount = typeOption->GetGuiWidgetInfoHandleCount();
				for (uint i = 0; i < widgetHandleCount; ++i)
				{
					UpdateData updateData;
					updateData.obj = obj; 

					updateData.updateTypeInfo = typeInfo;
					updateData.handleBase = typeOption->GetGuiWidgetInfoHandle(i).Get();
					if (userData->isActivatedTable)
						JImGuiImpl::TableSetColumnIndex(i);
					const uint innerWidgetInfoCount = updateData.handleBase->GetWidgetInfoCount();
					for (uint j = 0; j < innerWidgetInfoCount; ++j)
					{
						updateData.widgetIndex = j;
						DisplayWidget(updateData, userData);
					}
				}
			}
			static void DisplayWidget(UpdateData& updateData, UserData* userData)
			{
				if (updateData.GetWidgetInfo()->GetSupportWidgetType() == Core::Constants::NotSupportGuiWidget)
					return;

				const std::string widgetMapKey = MakeUniqueLabel(updateData);
				auto widgetHandle = userData->guiWidgetHandleMap.find(widgetMapKey);
				if (widgetHandle == userData->guiWidgetHandleMap.end())
				{
					std::unique_ptr<JGuiWidgetDisplayHandle> newHandle = MakeGuiHandle(updateData.handleBase->GetFieldHint(), updateData.GetWidgetInfo());
					if (newHandle != nullptr)
					{
						userData->guiWidgetHandleMap.emplace(widgetMapKey, std::move(newHandle));
						widgetHandle = userData->guiWidgetHandleMap.find(widgetMapKey);
						widgetHandle->second->Initialize(updateData, userData);
					}
				}

				if (widgetHandle == userData->guiWidgetHandleMap.end())
					return;

				auto condHandle = MakeExtraConditionHandle(updateData.GetWidgetInfo());
				bool failCondition = condHandle != nullptr && !condHandle->PassCondition(updateData);

				Core::JGuiExtraFunctionUserInfo* groupUserInfo = updateData.GetWidgetInfo()->GetExtraFunctionUserInfo(Core::J_GUI_EXTRA_FUNCTION_TYPE::TABLE).Get();
				if (groupUserInfo != nullptr)
				{
					const std::string groupMapKey = groupUserInfo->GetExtraFunctionName() + std::to_string(updateData.obj->GetGuid());
					auto groupData = userData->guiGroupHandleMap.find(groupMapKey);
					if (groupData == userData->guiGroupHandleMap.end())
					{
						std::unique_ptr<JGuiWidgetGroupHandle> groupHandle = MakeExtraPropertyGroupHandle(updateData.GetWidgetInfo());
						if (groupHandle != nullptr)
							userData->guiGroupHandleMap.emplace(groupMapKey, std::move(groupHandle));
						groupData = userData->guiGroupHandleMap.find(groupMapKey);
					}

					if (groupData != userData->guiGroupHandleMap.end())
					{
						if (failCondition)
							groupData->second->Update(widgetHandle->second.get(), updateData, userData, false);
						else
							groupData->second->Update(widgetHandle->second.get(), updateData, userData, true);
					}
					else
					{
						if (failCondition)
							return;
						widgetHandle->second->Update(updateData, userData);
					}
				}
				else
				{
					if (failCondition)
						return;
					widgetHandle->second->Update(updateData, userData);
				}
			}
		}

		JReflectionGuiWidgetHelper::JReflectionGuiWidgetHelper(JEditorWindow* ownerWnd)
			:guid(Core::MakeGuid()), ownerWnd(ownerWnd)
		{
			PrivateDataMap::Data().emplace(guid, std::make_unique<UserData>(ownerWnd));
		}
		JReflectionGuiWidgetHelper::~JReflectionGuiWidgetHelper()
		{
			PrivateDataMap::Data().erase(guid);
		}
		void JReflectionGuiWidgetHelper::UpdateGuiWidget(const Core::JUserPtr<Core::JIdentifier>& obj, Core::JTypeInfo* typeInfo)
		{
			auto userData = PrivateDataMap::Data().find(guid)->second.get();
			userData->InitOptionValue();

			SettingDisplayTypeInfo(obj, typeInfo, userData);
		}
		void JReflectionGuiWidgetHelper::Clear()
		{
			PrivateDataMap::Data().find(guid)->second->Clear();
		}
	}
}