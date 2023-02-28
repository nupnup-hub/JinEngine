#include"JReflectionGuiWidgetHelper.h"
#include"JEditorSearchBarHelper.h"
#include"../GuiLibEx/ImGuiEx/JImGuiImpl.h"
#include"../Page/JEditorPageShareData.h"
#include"../Page/WindowInterface/JEditorPreviewInterface.h"

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
#include"../../Core/FSM/JFSMInterface.h"
#include"../../Core/FSM/JFSMtransition.h"
#include"../../Core/FSM/JFSMparameter.h"
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
				Core::JIdentifier* obj;
				Core::JGuiWidgetInfoHandleBase* handleBase;
			public:
				int widgetIndex;
			public:
				Core::JGuiWidgetInfo* GetWidgetInfo()
				{
					return handleBase->GetWidgetInfo(widgetIndex);
				}
			};

			//user private data
			struct UserData
			{
			public:
				std::unordered_map<std::string, std::unique_ptr<JGuiWidgetDisplayHandle>> guiWidgetHandleMap;
				std::unordered_map<std::string, std::unique_ptr<JGuiWidgetGroupHandle>> guiGroupHandleMap;
			public:
				const J_EDITOR_PAGE_TYPE ownerPageType;
			public:
				bool allowDisplayParent = true;
				bool allowDisplayName = true;
				bool allowSameLine = false;
			public:
				//Inner
				bool isActivatedTable;
			public:
				UserData(const J_EDITOR_PAGE_TYPE ownerPageType)
					:ownerPageType(ownerPageType)
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
			};
			 
			static bool IsEditableObject(Core::JIdentifier* obj)
			{
				if (obj->GetTypeInfo().IsChildOf<JObject>() && !static_cast<JObject*>(obj)->HasFlag(OBJECT_FLAG_UNEDITABLE))
					return true;
				else if (obj->GetTypeInfo().IsChildOf<Core::JFSMInterface>())
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

				return widgetInfo->GetExtraFunctionUserInfo(type);
			}
			//forward declaration 
			static std::unique_ptr<JGuiWidgetDisplayHandle> MakeDisplayHandle(Core::JParameterHint pHint, Core::JGuiWidgetInfo* widgetInfo);
			static std::unique_ptr<JGuiWidgetGroupHandle> MakeExtraGroupHandle(Core::JGuiWidgetInfo* widgetInfo);
			static std::unique_ptr<JGuiConditionHandle> MakeExtraConditionHandle(Core::JGuiWidgetInfo* widgetInfo);
			static void SettingDisplayTypeInfo(Core::JIdentifier* obj, Core::JTypeInfo* typeInfo, UserData* userData);
			static void SettingUserData(Core::JIdentifier* obj, Core::JTypeInfo* typeInfo, UserData* userData);
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
			void SetValue(UpdateData& updateData, UserData* userData, T&& value)
			{
				if (!IsEditableObject(updateData.obj))
					return;

				static_cast<Core::JGuiWidgetInfoHandle<T>*>(updateData.handleBase)->Set(updateData.obj, std::forward<T>(value));
				
				//JObject is selectable object
				//others is subresource for JObject 
				//ex) FSM object for animationController and it is pageOpenResource
				if (!updateData.obj->GetTypeInfo().IsChildOf<JObject>())
					SetModifiedBit(JEditorPageShareData::GetOpendPageData(userData->ownerPageType).GetOpenSeleted(), true);
				else
					SetModifiedBit(Core::GetUserPtr(updateData.obj), true);
			}
			template<typename T>
			void UnsafeSetValue(UpdateData& updateData, UserData* userData, T&& value)
			{
				if (!IsEditableObject(updateData.obj))
					return;

				static_cast<Core::JGuiWidgetInfoHandle<T>*>(updateData.handleBase)->UnsafeSet(updateData.obj, std::forward<T>(value));
				if (!updateData.obj->GetTypeInfo().IsChildOf<JObject>())
					SetModifiedBit(JEditorPageShareData::GetOpendPageData(userData->ownerPageType).GetOpenSeleted(), true);
				else
					SetModifiedBit(Core::GetUserPtr(updateData.obj), true);
			}
			template<typename T>
			T GetValue(UpdateData& updateData)
			{
				return static_cast<Core::JGuiWidgetInfoHandle<T>*>(updateData.handleBase)->Get(updateData.obj);
			}
			template<typename T>
			T UnsafeGetValue(UpdateData& updateData)
			{
				return 	static_cast<Core::JGuiWidgetInfoHandle<T>*>(updateData.handleBase)->UnsafeGet(updateData.obj);
			}
			template<typename T>
			T UnsafeGetValue(Core::JIdentifier* obj, Core::JPropertyInfo* pInfo)
			{
				return pInfo->UnsafeGet<T>(obj);
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
				Core::JGuiExtraFunctionUserInfo* extraUserInfo = GetExtraUserInfo(updateData, Core::J_GUI_EXTRA_FUNCTION_TYPE::GROUP);
				if (extraUserInfo == nullptr)
					return;

				Core::JGuiExtraFunctionInfo* extraInfo = Core::JGuiExtraFunctionInfoMap::GetExtraFunctionInfo(extraUserInfo->GetRefInfoMapName());
				if (extraInfo == nullptr || static_cast<Core::JGuiGroupInfo*>(extraInfo)->GetGroupType() != Core::J_GUI_EXTRA_GROUP_TYPE::TABLE)
					return;
				
				Core::JGuiTableUserInfo* tableUserInfo = static_cast<Core::JGuiTableUserInfo*>(extraUserInfo);
				Core::JGuiTableInfo* tableInfo = static_cast<Core::JGuiTableInfo*>(extraInfo);

				auto result = handleResult.find(updateData.handleBase->GetName());
				if (result == handleResult.end())
				{
					handleResult.emplace(updateData.handleBase->GetName(), 0);
					result = handleResult.find(updateData.handleBase->GetName());
				}

				const bool canUpdate = result->second < tableUserInfo->GetUseColumnCount();
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

				Core::JGuiExtraFunctionInfo* extraInfo = Core::JGuiExtraFunctionInfoMap::GetExtraFunctionInfo(extraUser->GetRefInfoMapName());
				Core::JGuiConditionInfo* conditionInfo = static_cast<Core::JGuiConditionInfo*>(extraInfo);
				if (conditionInfo->GetConditionType() == Core::J_GUI_EXTRA_CONDITION_TYPE::ENUM)
					return PassEnumCondition(updateData, extraUser, conditionInfo);
				else
				{
					MessageBox(0, L"Not Enum", 0, 0);
					return false;
				}
			}
		private:
			static bool PassEnumCondition(UpdateData& updateData,
				Core::JGuiExtraFunctionUserInfo* extraUser,
				Core::JGuiConditionInfo* conditionInfo)
			{ 
				Core::JIdentifier* enumOwner = nullptr;
				Core::JPropertyInfo* enumPropertyInfo = nullptr;

				Core::JGuiEnumConditionInfo* enumCondInfo = static_cast<Core::JGuiEnumConditionInfo*>(conditionInfo);
				Core::JGuiEnumConditionUserInfoBase* enumCondUser = static_cast<Core::JGuiEnumConditionUserInfoBase*>(extraUser);
				if (enumCondUser->IsRefUser())
				{
					Core::JPropertyInfo* refPropertyInfo = updateData.handleBase->GetTypeInfo()->GetProperty(enumCondUser->GetOwnerTypeParameterNameInRefClass());
					enumOwner = JGuiObjectValueInterface{}.UnsafeGetValue<Core::JIdentifier*>(updateData.obj, refPropertyInfo);
					if (enumOwner == nullptr)
						return false;
					  
					enumPropertyInfo = enumOwner->GetTypeInfo().GetProperty(enumCondInfo->GetParamName());
				}
				else
				{
					enumOwner = updateData.obj;			 
					enumPropertyInfo = updateData.handleBase->GetTypeInfo()->GetProperty(enumCondInfo->GetParamName());				 
				}
				if (enumOwner == nullptr || enumPropertyInfo == nullptr)
					return false;

				if (enumPropertyInfo->GetHint().jDataEnum != Core::J_PARAMETER_TYPE::Enum)
					return false;

				if (enumCondUser->OnTrigger(JGuiObjectValueInterface{}.UnsafeGetValue<Core::JEnum>(enumOwner, enumPropertyInfo)))		 
					return true;
				else
					return false;
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
					if constexpr (Core::JVectorDetermine<T>::value && std::is_same_v<ValueType, std::string>)
					{
						buff.x.resize(JImGuiImpl::GetTextBuffRange());
						buff.y.resize(JImGuiImpl::GetTextBuffRange());
					}
				}
				else if constexpr (std::is_same_v<T, DirectX::XMINT3> ||
					std::is_same_v<T, DirectX::XMFLOAT3> ||
					std::is_same_v<T, JVector3<ValueType>>)
				{
					exeMaxCount = 3;
					if constexpr (Core::JVectorDetermine<T>::value && std::is_same_v<ValueType, std::string>)
					{
						buff.x.resize(JImGuiImpl::GetTextBuffRange());
						buff.y.resize(JImGuiImpl::GetTextBuffRange());
						buff.z.resize(JImGuiImpl::GetTextBuffRange());
					}
				}
				else if constexpr (std::is_same_v<T, DirectX::XMINT4> ||
					std::is_same_v<T, DirectX::XMFLOAT4> ||
					std::is_same_v<T, JVector4<ValueType>>)
				{
					exeMaxCount = 4;
					if constexpr (Core::JVectorDetermine<T>::value && std::is_same_v<ValueType, std::string>)
					{
						buff.x.resize(JImGuiImpl::GetTextBuffRange());
						buff.y.resize(JImGuiImpl::GetTextBuffRange());
						buff.z.resize(JImGuiImpl::GetTextBuffRange());
						buff.w.resize(JImGuiImpl::GetTextBuffRange());
					}
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
					ClassifyInputType(updateData);
				else if (!inputInfo->HasValidFixedParameter())
					ClassifyInputType(updateData);
				else
				{
					Core::J_PARAMETER_TYPE fixedParam = inputInfo->GetFixedParameter();
					switch (fixedParam)
					{
					case JinEngine::Core::J_PARAMETER_TYPE::Bool:
					{
						int intBuff = (int)buff;
						res = JImGuiImpl::InputInt("##GuiInputIntHandle" + MakeUniqueLabel(updateData), &intBuff, flag);
						buff = std::clamp(intBuff, 0, 1);
						break;
					}
					case JinEngine::Core::J_PARAMETER_TYPE::Int:
					{
						int intBuff = (int)buff;
						res = JImGuiImpl::InputInt("##GuiInputIntHandle" + MakeUniqueLabel(updateData), &intBuff, flag);
						buff = intBuff;
						break;
					}
					case JinEngine::Core::J_PARAMETER_TYPE::Float:
					{
						float floatBuff = (float)buff;
						res = JImGuiImpl::InputFloat("##GuiInputFloatHandle" + MakeUniqueLabel(updateData), &floatBuff, flag);
						buff = floatBuff;
						break;
					}
					default:
						break;
					}
				}
				if (res)
				{
					if constexpr (std::is_same_v<T, std::string >)
						SetValue(updateData, userData, JCUtil::EraseSideChar(buff, '\0'));
					else
						SetValue(updateData, userData, buff);
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
			using GetElemntVecF = Core::JSFunctorType<std::vector<Core::JIdentifier*>, Core::JIdentifier*>;
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
				if (!updateData.handleBase->GetTypeInfo()->IsChildOf<JObject>())
					imageType = Core::J_GUI_SELECTOR_IMAGE::NONE;

				getElemenVecPtr = inputInfo->GetElementVecPtr();
				if (imageType == Core::J_GUI_SELECTOR_IMAGE::NONE)
				{
					sizeMin = JImGuiImpl::GetAlphabetSize().y;
					sizeMax = JImGuiImpl::GetAlphabetSize().y;
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
				if (isSelected)
				{
					SetSelectObject(updateData, userData);
					ClearPreviewGroup();
					selectorPreviewVec.clear();
					CreateSelectorPreviewList();
					isSelected = false;
				}

				if (isClosePopup)
				{
					ClearPreviewGroup();
					selectorPreviewVec.clear();
					CreateSelectorPreviewList();
					isClosePopup = false;
				}
			}
		protected:
			void SelectedPreviewOnScreen(JPreviewScene* previewScene)
			{
				if (!CanCreatePreviewImage())
					return;

				if (previewScene != nullptr)
				{
					JImGuiImpl::Image(*previewScene->GetPreviewCamera().Get(), JVector2<float>(sizeMin, sizeMin));
					ImGui::SameLine();
				}
				else
				{
					JImGuiImpl::Image(*JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::NONE), JVector2<float>(sizeMin, sizeMin));
					ImGui::SameLine();
				}
			}
			template<typename ValueType, typename PointerRef>
			bool SelectorOnScreen(PointerRef selectedObj, UpdateData& updateData, const std::string& uniqueLabel)
			{
				std::string name = "None";
				if (selectedObj != nullptr && (*selectedObj) != nullptr)
					name = JCUtil::WstrToU8Str((*selectedObj)->GetName());

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
				JImGuiImpl::Image(*JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::NONE), JVector2<float>(sizeMin, sizeMin));
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
					Core::JUserPtr<Core::JIdentifier> previewObj = selectorPreviewVec[i]->GetJObject();
					if (!searchBarHelper->CanSrcNameOnScreen(previewObj->GetName()))
						continue;

					JImGuiImpl::Image(*selectorPreviewVec[i]->GetPreviewCamera().Get(), JVector2<float>(sizeFactor, sizeFactor));
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
					uint count = 0;
					std::vector<JResourceObject*>::const_iterator cBegin = JResourceManager::Instance().GetResourceVectorHandle<ValueType>(count);
					for (uint i = 0; i < count; ++i)
					{
						JResourceObject* rObj = *(cBegin + i);
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

					std::vector<Core::JIdentifier*> elementVec = (*getElemenVecPtr)(updateData.obj);
					const uint elementCount = (uint)elementVec.size();
					for (uint i = 0; i < elementCount; ++i)
					{
						if (elementVec[i]->GetTypeInfo().IsChildOf<JObject>() && static_cast<JObject*>(elementVec[i])->HasFlag(OBJECT_FLAG_HIDDEN))
							continue;

						if (!searchBarHelper->CanSrcNameOnScreen(elementVec[i]->GetName()))
							continue;

						if (JImGuiImpl::Selectable(JCUtil::WstrToU8Str(elementVec[i]->GetName()) + "##" + uniqueLabel))
						{
							(*selectedObj) = static_cast<ValueType*>(elementVec[i]);
							isSelected = true;
							ImGui::CloseCurrentPopup();
						}
					}
				}
			}
		protected:
			virtual void CreateSelectorPreviewList() = 0;
			template<typename ValueType>
			void CreateSelectorList()
			{
				if (!CanCreatePreviewImage())
					return;

				if constexpr (std::is_base_of_v<JResourceObject, ValueType>)
				{
					uint count = 0;
					std::vector<JResourceObject*>::const_iterator cBegin = JResourceManager::Instance().GetResourceVectorHandle<ValueType>(count);
					for (uint i = 0; i < count; ++i)
					{
						JResourceObject* rObj = *(cBegin + i);
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
			void SetFirstTrigger(bool value)noexcept
			{
				isFirstOpen = value;
			}
			void SetCloseTrigger(bool value)noexcept
			{
				isClosePopup = value;
			}
		protected:
			virtual void SetSelectObject(UpdateData& updateData, UserData* userData) = 0;
		};
		//T is JResourceObject derive class
		template<typename T>
		class JGuiSingleSelectorHandle : public JGuiSelectorHandleHelper, public JGuiObjectValueInterface
		{
		private:
			using ValueType = Core::RemoveAll_T<T>;
		private:
			size_t guid = Core::MakeGuid();
		private:
			T selectedObj;
			JPreviewScene* selectedPreview = nullptr;
		public:
			void Initialize(UpdateData& updateData, UserData* userData) final
			{
				JGuiSelectorHandleHelper::Initialize(updateData, userData);
				selectedObj = GetValue<T>(updateData);
				CreateSelectorPreviewList();
			}
			void Update(UpdateData& updateData, UserData* userData) final
			{
				TrySameLine(userData);
				DisplayName(updateData, userData);
				Begin(updateData, userData);
				if (CanCreatePreviewImage())
					SelectedPreviewOnScreen(selectedPreview);
				TrySameLine(userData);
				bool isOpen = SelectorOnScreen<ValueType>(&selectedObj, updateData, MakeUniqueLabel(updateData));
				if (!isOpen)
				{
					SetFirstTrigger(true);
					if (GetPreviewCount() > 0)
						SetCloseTrigger(true);
				}
			}
		protected:
			void SetSelectObject(UpdateData& updateData, UserData* userData) final
			{
				SetValue(updateData, userData, selectedObj);
			}
		private:
			void CreateSelectorPreviewList()
			{
				if (!CanCreatePreviewImage())
					return;
				if constexpr (std::is_base_of_v<JObject, ValueType>)
					selectedPreview = CreatePreviewScene(Core::GetUserPtr(selectedObj));
			}
		};
		template<typename T>
		class JGuiMultiSelectorHandle : public JGuiSelectorHandleHelper, public JGuiObjectValueInterface
		{
		private:
			using ValueType = Core::RemoveAll_T<typename Core::StdArrayContainerDetermine<T>::ValueType>;
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

				container = GetValue<T>(updateData);
				containerCount = (uint)container.size();

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
						JRenderItem* rItem = static_cast<JRenderItem*>(updateData.obj);
						JImGuiImpl::Text(JCUtil::WstrToU8Str(rItem->GetMesh()->GetSubMeshName(i)));
					}

					if (CanCreatePreviewImage())
						SelectedPreviewOnScreen(selectedPreview[i]);
					TrySameLine(userData);
					bool res = SelectorOnScreen<ValueType>(&container[i], updateData, uniqueLabel);
					if (res)
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
			}
		protected:
			void SetSelectObject(UpdateData& updateData, UserData* userData) final
			{
				SetValue(updateData, userData, container);
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
						selectedPreview[i] = CreatePreviewScene(Core::GetUserPtr<JResourceObject>(container[i]));
				}
			}
		};
		//bool
		class JGuiCheckBoxHandle : public JGuiWidgetDisplayHandle, public JGuiObjectValueInterface
		{
		private:
			bool value = false;
		public:
			void Initialize(UpdateData& updateData, UserData* userData)final{}
			void Update(UpdateData& updateData, UserData* userData)final
			{ 
				if (!IsConveribleParam(updateData.handleBase->GetFieldHint()))
					return;

				value = GetValue<bool>(updateData);
				TrySameLine(userData);
				if (JImGuiImpl::CheckBox(GetDisplayName("GuiCheckbox" + MakeUniqueLabel(updateData), updateData, userData), value))
					SetValue(updateData, userData, value);
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
							SetValue(updateData, userData, value);
					}
					else
					{
						if (JImGuiImpl::InputFloat("##GuiSliderFloatInput" + MakeUniqueLabel(updateData), &value, 0, "%.1f"))
							SetValue(updateData, userData, value);
					}
					ImGui::SameLine();
				}

				if (isVertical)
				{
					JVector2<float> vSliderSize{ JImGuiImpl::GetClientWindowSize().x * 0.01f, JImGuiImpl::GetClientWindowSize().y * 0.075f };
					if constexpr (std::is_integral_v<T>)
					{
						if (JImGuiImpl::VSliderInt("##GuiIntVSlider" + MakeUniqueLabel(updateData), vSliderSize, &value, minValue, maxValue, "", ImGuiSliderFlags_AlwaysClamp))
							SetValue(updateData, userData, value);
					}
					else
					{
						if (JImGuiImpl::VSliderFloat("##GuiFloatVSlider" + MakeUniqueLabel(updateData), vSliderSize, &value, minValue, maxValue, "", ImGuiSliderFlags_AlwaysClamp))
							SetValue(updateData, userData, value);
					}
				}
				else
				{
					ImGui::SetNextItemWidth(JImGuiImpl::GetSliderWidth());
					if constexpr (std::is_integral_v<T>)
					{
						if (JImGuiImpl::SliderInt("##GuiIntSlider" + MakeUniqueLabel(updateData), &value, minValue, maxValue, "", ImGuiSliderFlags_AlwaysClamp))
							SetValue(updateData, userData, value);
					}
					else
					{
						if (JImGuiImpl::SliderFloat("##GuiFloatSlider" + MakeUniqueLabel(updateData), &value, minValue, maxValue, "", ImGuiSliderFlags_AlwaysClamp))
							SetValue(updateData, userData, value);
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
							SetValue(updateData, userData, colorV);
					}
					else if constexpr (T::GetDigitCount() == 4)
					{
						if (ImGui::ColorPicker4(("##GuiColorPicker" + MakeUniqueLabel(updateData)).c_str(), (float*)&colorV, flag))
							SetValue(updateData, userData, colorV);
					}

				}
				else
				{
					if constexpr (std::is_same_v<T, DirectX::XMFLOAT3>)
					{
						if (ImGui::ColorPicker3(("##GuiColorPicker" + MakeUniqueLabel(updateData)).c_str(), (float*)&colorV, flag))
						{
							if constexpr (std::is_same_v<T, DirectX::XMFLOAT3>)
								SetValue(updateData, userData, DirectX::XMFLOAT3(colorV.x, colorV.y, colorV.z));
						}
					}
					else if constexpr (std::is_same_v<T, DirectX::XMFLOAT4>)
					{
						if (ImGui::ColorPicker4(("##GuiColorPicker" + MakeUniqueLabel(updateData)).c_str(), (float*)&colorV, flag))
						{
							if constexpr (std::is_same_v<T, DirectX::XMFLOAT4>)
								SetValue(updateData, userData, DirectX::XMFLOAT4(colorV.x, colorV.y, colorV.z, colorV.w));
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
					Core::JEnumInfo* enumInfo = Core::JReflectionInfo::Instance().GetEnumInfo(enumComboInfo->GetEnumFullName());
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
									UnsafeSetValue<Core::JEnum>(updateData, userData, enumInfo->EnumValue(i));
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
		private:
			using ValueType = Core::RemoveAll_T<typename Core::StdArrayContainerDetermine<T>::ValueType>;
		private:
			T container;
		private:
			int openIndex = -1;
		private:
			//모든 JIdenfier를 상속받는 object는 Factory에서 생성되며
			//Instance는 ownerPtr로 typeInfo class에 저장되고 pointer를 반환한다
			using CreateElementF = Core::JSFunctorType<Core::JIdentifier*, Core::JIdentifier*>;
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
					Core::JTypeInfoOption* option = typeInfo->GetOption();
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
								SettingDisplayTypeInfo(container[i], &container[i]->GetTypeInfo(), userData);
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
			static std::unique_ptr<JGuiWidgetDisplayHandle> MakeDisplayHandle(Core::JParameterHint pHint, Core::JGuiWidgetInfo* widgetInfo)
			{
				Core::J_PARAMETER_TYPE pType = pHint.jDataEnum;
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
						else if (typeid(JVector2<std::string>).name() == pHint.name)
							return std::make_unique<JGuiInputHandle<JVector2<std::string>>>();
					}
					else if (pType == Core::J_PARAMETER_TYPE::JVector3)
					{
						if (typeid(JVector3<int>).name() == pHint.name || typeid(JVector3<uint>).name() == pHint.name)
							return std::make_unique<JGuiInputHandle<JVector3<int>>>();
						else if (typeid(JVector3<float>).name() == pHint.name)
							return std::make_unique<JGuiInputHandle<JVector3<float>>>();
						else if (typeid(JVector3<std::string>).name() == pHint.name)
							return std::make_unique<JGuiInputHandle<JVector3<std::string>>>();
					}
					else if (pType == Core::J_PARAMETER_TYPE::JVector4)
					{
						if (typeid(JVector4<int>).name() == pHint.name || typeid(JVector4<uint>).name() == pHint.name)
							return std::make_unique<JGuiInputHandle<JVector4<int>>>();
						else if (typeid(JVector4<float>).name() == pHint.name)
							return std::make_unique<JGuiInputHandle<JVector4<float>>>();
						else if (typeid(JVector4<std::string>).name() == pHint.name)
							return std::make_unique<JGuiInputHandle<JVector4<std::string>>>();
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
					bool isVector = false;
					if (pType == Core::J_PARAMETER_TYPE::STD_VECTOR)
						isVector = true;

					//if success isA casting
					//exe downcast
					typeinfo = Core::JReflectionInfo::Instance().FindTypeInfo(pHint.valueTypeFullName);
					if (typeinfo != nullptr)
					{
						if (typeinfo->IsA<JAnimationClip>())
						{
							if (isVector)
								return std::make_unique<JGuiMultiSelectorHandle<std::vector<JAnimationClip*>>>();
							else
								return std::make_unique<JGuiSingleSelectorHandle<JAnimationClip*>>();
						}
						else if (typeinfo->IsA<JAnimationController>())
						{
							if (isVector)
								return std::make_unique<JGuiMultiSelectorHandle<std::vector<JAnimationController*>>>();
							else
								return std::make_unique<JGuiSingleSelectorHandle<JAnimationController*>>();
						}
						else if (typeinfo->IsA<JMaterial>())
						{
							if (isVector)
								return std::make_unique<JGuiMultiSelectorHandle<std::vector<JMaterial*>>>();
							else
								return std::make_unique<JGuiSingleSelectorHandle<JMaterial*>>();
						}
						else if (typeinfo->IsA<JMeshGeometry>())
						{
							if (isVector)
								return std::make_unique<JGuiMultiSelectorHandle<std::vector<JMeshGeometry*>>>();
							else
								return std::make_unique<JGuiSingleSelectorHandle<JMeshGeometry*>>();
						}
						else if (typeinfo->IsA<JStaticMeshGeometry>())
						{
							if (isVector)
								return std::make_unique<JGuiMultiSelectorHandle<std::vector<JStaticMeshGeometry*>>>();
							else
								return std::make_unique<JGuiSingleSelectorHandle<JStaticMeshGeometry*>>();
						}
						else if (typeinfo->IsA<JSkinnedMeshGeometry>())
						{
							if (isVector)
								return std::make_unique<JGuiMultiSelectorHandle<std::vector<JSkinnedMeshGeometry*>>>();
							else
								return std::make_unique<JGuiSingleSelectorHandle<JSkinnedMeshGeometry*>>();
						}
						else if (typeinfo->IsA<JScene>())
						{
							if (isVector)
								return std::make_unique<JGuiMultiSelectorHandle<std::vector<JScene*>>>();
							else
								return std::make_unique<JGuiSingleSelectorHandle<JScene*>>();
						}
						else if (typeinfo->IsA<JShader>())
						{
							if (isVector)
								return std::make_unique<JGuiMultiSelectorHandle<std::vector<JShader*>>>();
							else
								return std::make_unique<JGuiSingleSelectorHandle<JShader*>>();
						}
						else if (typeinfo->IsA<JSkeletonAsset>())
						{
							if (isVector)
								return std::make_unique<JGuiMultiSelectorHandle<std::vector<JSkeletonAsset*>>>();
							else
								return std::make_unique<JGuiSingleSelectorHandle<JSkeletonAsset*>>();
						}
						else if (typeinfo->IsA<JTexture>())
						{
							if (isVector)
								return std::make_unique<JGuiMultiSelectorHandle<std::vector<JTexture*>>>();
							else
								return std::make_unique<JGuiSingleSelectorHandle<JTexture*>>();
						}
						else if (typeinfo->IsA<Core::JFSMparameter>())
						{
							if (isVector)
								return std::make_unique<JGuiMultiSelectorHandle<std::vector<Core::JFSMparameter*>>>();
							else
								return std::make_unique<JGuiSingleSelectorHandle<Core::JFSMparameter*>>();
						}
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
					else if (typeid(JVector2<std::string>).name() == pHint.name)
						return std::make_unique<JGuiReadOnlyTextHandle<JVector2<std::string>>>();
				}
				else if (pType == Core::J_PARAMETER_TYPE::JVector3)
				{
					if (typeid(JVector3<int>).name() == pHint.name || typeid(JVector3<uint>).name() == pHint.name)
						return std::make_unique<JGuiReadOnlyTextHandle<JVector3<int>>>();
					else if (typeid(JVector3<float>).name() == pHint.name)
						return std::make_unique<JGuiReadOnlyTextHandle<JVector3<float>>>();
					else if (typeid(JVector3<std::string>).name() == pHint.name)
						return std::make_unique<JGuiReadOnlyTextHandle<JVector3<std::string>>>();
				}
				else if (pType == Core::J_PARAMETER_TYPE::JVector4)
				{
					if (typeid(JVector4<int>).name() == pHint.name || typeid(JVector4<uint>).name() == pHint.name)
						return std::make_unique<JGuiReadOnlyTextHandle<JVector4<int>>>();
					else if (typeid(JVector4<float>).name() == pHint.name)
						return std::make_unique<JGuiReadOnlyTextHandle<JVector4<float>>>();
					else if (typeid(JVector4<std::string>).name() == pHint.name)
						return std::make_unique<JGuiReadOnlyTextHandle<JVector4<std::string>>>();
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
					typeinfo = Core::JReflectionInfo::Instance().FindTypeInfo(pHint.valueTypeFullName);
					if (typeinfo != nullptr)
					{
						if (typeinfo->IsA<JAnimationClip>())
							return std::make_unique<JGuiListHandle<std::vector<JAnimationClip*>>>();
						else if (typeinfo->IsA<JAnimationController>())
							return std::make_unique<JGuiListHandle<std::vector<JAnimationController*>>>();
						else if (typeinfo->IsA<JMaterial>())
							return std::make_unique<JGuiListHandle<std::vector<JMaterial*>>>();
						else if (typeinfo->IsA<JMeshGeometry>())
							return std::make_unique<JGuiListHandle<std::vector<JMeshGeometry*>>>();
						else if (typeinfo->IsA<JStaticMeshGeometry>())
							return std::make_unique<JGuiListHandle<std::vector<JStaticMeshGeometry*>>>();
						else if (typeinfo->IsA<JSkinnedMeshGeometry>())
							return std::make_unique<JGuiListHandle<std::vector<JSkinnedMeshGeometry*>>>();
						else if (typeinfo->IsA<JScene>())
							return std::make_unique<JGuiListHandle<std::vector<JScene*>>>();
						else if (typeinfo->IsA<JShader>())
							return std::make_unique<JGuiListHandle<std::vector<JShader*>>>();
						else if (typeinfo->IsA<JSkeletonAsset>())
							return std::make_unique<JGuiListHandle<std::vector<JSkeletonAsset*>>>();
						else if (typeinfo->IsA<JTexture>())
							return std::make_unique<JGuiListHandle<std::vector<JTexture*>>>();
						else if (typeinfo->IsA<Core::JFSMcondition>())
							return std::make_unique<JGuiListHandle<std::vector<Core::JFSMcondition*>>>();

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
			}
			static std::unique_ptr<JGuiWidgetGroupHandle> MakeExtraPropertyGroupHandle(Core::JGuiWidgetInfo* widgetInfo)
			{
				Core::JGuiExtraFunctionUserInfo* groupUserInfo = widgetInfo->GetExtraFunctionUserInfo(Core::J_GUI_EXTRA_FUNCTION_TYPE::GROUP);
				if (groupUserInfo != nullptr)
				{
					auto info = Core::JGuiExtraFunctionInfoMap::GetExtraFunctionInfo(groupUserInfo->GetRefInfoMapName());
					auto groupInfo = static_cast<Core::JGuiGroupInfo*>(info);
					if (groupInfo->GetGroupType() == Core::J_GUI_EXTRA_GROUP_TYPE::TABLE)
						return std::make_unique<JGuiTableHandle>();
				}
				return nullptr;
			}
			static std::unique_ptr<JGuiConditionHandle> MakeExtraConditionHandle(Core::JGuiWidgetInfo* widgetInfo)
			{
				Core::JGuiExtraFunctionUserInfo* condUserInfo = widgetInfo->GetExtraFunctionUserInfo(Core::J_GUI_EXTRA_FUNCTION_TYPE::CONDITION);
				if (condUserInfo != nullptr)
					return std::make_unique<JGuiConditionHandle>();
				return nullptr;
			}
			static void SettingDisplayTypeInfo(Core::JIdentifier* obj, Core::JTypeInfo* typeInfo, UserData* userData)
			{
				if (obj == nullptr)
					return;

				Core::JTypeInfoOption* typeOption = typeInfo->GetOption();
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
							SettingUserData(obj, infoVec[i], userData);
					}
					else
					{
						Core::JTypeInfo* nowInfo = typeInfo;
						while (nowInfo != nullptr)
						{
							SettingUserData(obj, nowInfo, userData);
							nowInfo = nowInfo->GetParent();
						}
					}
				}
				else
					SettingUserData(obj, typeInfo, userData);
			}
			static void SettingUserData(Core::JIdentifier* obj, Core::JTypeInfo* typeInfo, UserData* userData)
			{
				Core::JTypeInfoOption* typeOption = typeInfo->GetOption();
				const uint widgetHandleCount = typeOption->GetGuiWidgetInfoHandleCount();
				for (uint i = 0; i < widgetHandleCount; ++i)
				{
					UpdateData updateData;
					updateData.obj = obj;
					updateData.handleBase = typeOption->GetGuiWidgetInfoHandle(i);

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
					std::unique_ptr<JGuiWidgetDisplayHandle> newHandle = MakeDisplayHandle(updateData.handleBase->GetFieldHint(), updateData.GetWidgetInfo());
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

				Core::JGuiExtraFunctionUserInfo* groupUserInfo = updateData.GetWidgetInfo()->GetExtraFunctionUserInfo(Core::J_GUI_EXTRA_FUNCTION_TYPE::GROUP);
				if (groupUserInfo != nullptr)
				{
					const std::string groupMapKey = groupUserInfo->GetRefInfoMapName() + std::to_string(updateData.obj->GetGuid());
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

		JReflectionGuiWidgetHelper::JReflectionGuiWidgetHelper(const J_EDITOR_PAGE_TYPE ownerPageType)
			:guid(Core::MakeGuid()), ownerPageType(ownerPageType)
		{
			PrivateDataMap::Data().emplace(guid, std::make_unique<UserData>(ownerPageType));
		}
		JReflectionGuiWidgetHelper::~JReflectionGuiWidgetHelper()
		{
			PrivateDataMap::Data().erase(guid);
		}
		void JReflectionGuiWidgetHelper::UpdateGuiWidget(Core::JIdentifier* obj, Core::JTypeInfo* typeInfo)
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