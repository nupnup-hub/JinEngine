#include"JReflectionGuiWidgetHelper.h"
#include"JEditorSearchBarHelper.h"
#include"../GuiLibEx/ImGuiEx/JImGuiImpl.h"
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
#include"../../Core/Reflection/JPropertyInfo.h"
#include"../../Utility/JVector.h" 
#include"../../Utility/JCommonUtility.h"

namespace JinEngine
{
	namespace Editor
	{
		bool JGuiPropertyWidgetHandle::IsPrintTitle()const noexcept
		{
			return isPrintTitle;
		}
		void JGuiPropertyWidgetHandle::SetPrintTitle(bool value)noexcept
		{
			isPrintTitle = value;
		}

		static bool IsEditableObject(Core::JIdentifier* obj)
		{
			if (obj->GetTypeInfo().IsChildOf<JObject>() && !static_cast<JObject*>(obj)->HasFlag(OBJECT_FLAG_UNEDITABLE))
				return true;
			else
				return false;
		}
		static std::string GetUniqueLabel(Core::JIdentifier* obj, Core::JPropertyInfo* pInfo)
		{
			return pInfo->Name() + std::to_string(obj->GetGuid());
		}
#pragma region Property
		 
		class JGuiPropertySetInterface : public JEditorObjectHandlerInterface
		{
		public: 
			template<typename T>
			void SetPropertyValue(Core::JIdentifier* obj, Core::JPropertyInfo* pInfo, const T& value)
			{
				if (!IsEditableObject(obj))
					return;

				pInfo->Set<T>(obj, value);
				SetModifiedBit(Core::GetUserPtr(obj), true);
			}
			template<typename T>
			void UnsafeSetPropertyValue(Core::JIdentifier* obj, Core::JPropertyInfo* pInfo, const T& value)
			{
				if (!IsEditableObject(obj))
					return;

				pInfo->UnsafeSet<T>(obj, value);
				SetModifiedBit(Core::GetUserPtr(obj), true);
			}
		};

		//Property
		//int, float, string, vector
		template<typename T>
		class JGuiInputHandle : public JGuiPropertyWidgetHandle, 
			public JGuiPropertySetInterface
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
			void Initialize(Core::JIdentifier* obj, Core::JPropertyInfo* pInfo)final
			{
				Core::JGuiInputInfo* inputInfo = static_cast<Core::JGuiInputInfo*>(pInfo->GetOptionInfo()->GetWidgetInfo());
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
			void Update(Core::JIdentifier* obj, Core::JPropertyInfo* pInfo)final
			{
				buff = pInfo->Get<T>(obj);
				if (IsPrintTitle())
					JImGuiImpl::Text(pInfo->Name());

				bool res = false; 

				if constexpr (std::is_same_v<T, DirectX::XMINT2> ||
					std::is_same_v<T, DirectX::XMFLOAT2> ||
					std::is_same_v<T, JVector2<ValueType>>)
				{
					if (exeCount == 0)
						res = BuildInput(buff.x, GetUniqueLabel(obj, pInfo) + "00");
					else
						res = BuildInput(buff.y, GetUniqueLabel(obj, pInfo) + "01");
				}
				else if constexpr (std::is_same_v<T, DirectX::XMINT3> ||
					std::is_same_v<T, DirectX::XMFLOAT3> ||
					std::is_same_v<T, JVector3<ValueType>>)
				{
					if (exeCount == 0)
						res = BuildInput(buff.x, GetUniqueLabel(obj, pInfo) + "00");
					else if (exeCount == 1)
						res = BuildInput(buff.y, GetUniqueLabel(obj, pInfo) + "01");
					else
						res = BuildInput(buff.z, GetUniqueLabel(obj, pInfo) + "02");
				}
				else if constexpr (std::is_same_v<T, DirectX::XMINT4> ||
					std::is_same_v<T, DirectX::XMFLOAT4> ||
					std::is_same_v<T, JVector4<ValueType>>)
				{
					if (exeCount == 0)
						res = BuildInput(buff.x, GetUniqueLabel(obj, pInfo) + "00");
					else if (exeCount == 1)
						res = BuildInput(buff.y, GetUniqueLabel(obj, pInfo) + "01");
					else if (exeCount == 2)
						res = BuildInput(buff.z, GetUniqueLabel(obj, pInfo) + "02");
					else
						res = BuildInput(buff.w, GetUniqueLabel(obj, pInfo) + "03");
				}
				else
					res = BuildInput(buff, GetUniqueLabel(obj, pInfo) + "00");
				if (res)
				{
					if constexpr (std::is_same_v<T, std::string >)
						SetPropertyValue(obj, pInfo, JCUtil::EraseSideChar(buff, '\0'));
					else
						SetPropertyValue(obj, pInfo, buff);
				}
				++exeCount;
				if (exeCount == exeMaxCount)
					exeCount = 0;
			}
		private:
			template<typename InputType>
			bool BuildInput(InputType& data, const std::string& uniqSymbol)
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
		class JGuiSelectorHandleHelper : public JGuiPropertyWidgetHandle, public JEditorPreviewInterface
		{ 
		private:
			std::vector<JPreviewScene*> selectorPreviewVec;
			std::unique_ptr<JEditorSearchBarHelper> searchBarHelper;
		private:
			float sizeMin = 0;
			float sizeMax = 0;
			float sizeFactor = 0;
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
			void Initialize(Core::JIdentifier* obj, Core::JPropertyInfo* pInfo) override
			{
				Core::JGuiSelectorInfo* inputInfo = static_cast<Core::JGuiSelectorInfo*>(pInfo->GetOptionInfo()->GetWidgetInfo());
				hasSizeSlider = inputInfo->HasSizeSlider();
				if (inputInfo->IsImageRtTexture())
				{
					isRtTexture = true;
					sizeMin = (JImGuiImpl::GetClientWindowSize().x * 0.035f);
					sizeMax = (JImGuiImpl::GetClientWindowSize().x * 0.07f);
				}
				else
				{
					isRtTexture = false;
					hasSizeSlider = false;
					sizeMin = (JImGuiImpl::GetAlphabetSize().x);
					sizeMax = (JImGuiImpl::GetAlphabetSize().x * 2);
				}
				sizeFactor = sizeMin;
			}
		protected:
			void Begin(Core::JIdentifier* obj, Core::JPropertyInfo* pInfo)
			{
				if (isSelected)
				{ 
					SetSelectObject(obj, pInfo);
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
			void SelectorPreviewOnScreen(JPreviewScene* previewScene, Core::JPropertyInfo* pInfo)
			{
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
			bool SelectorOnScreen(PointerRef selectedObj,
				Core::JIdentifier* obj,
				Core::JPropertyInfo* pInfo, 
				const std::string& uniqueLabel)
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
						Core::JUserPtr<JObject> previewObj = selectorPreviewVec[i]->GetJObject();
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

					ImGui::EndGroup();
					JImGuiImpl::EndCombo();
					return true;
				}
				else
					return false;
			}
		protected:
			virtual void CreateSelectorPreviewList() = 0;
			template<typename ValueType>
			void CreateSelectorList()
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
		protected:
			bool IsFirstOpen()const noexcept
			{
				return isFirstOpen;
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
			virtual void SetSelectObject(Core::JIdentifier* obj, Core::JPropertyInfo* pInfo) = 0;
		};
		//T is JResourceObject derive class
		template<typename T>
		class JGuiSingleSelectorHandle : public JGuiSelectorHandleHelper, public JGuiPropertySetInterface
		{
		private:
			using ValueType = Core::RemoveAll_T<T>;
		private:
			T selectedObj;
			JPreviewScene* selectedPreview = nullptr;
		public:
			void Initialize(Core::JIdentifier* obj, Core::JPropertyInfo* pInfo) final
			{
				JGuiSelectorHandleHelper::Initialize(obj, pInfo);
				selectedObj = pInfo->Get<T>(obj);
				CreateSelectorPreviewList();
			}
			void Update(Core::JIdentifier* obj, Core::JPropertyInfo* pInfo) final
			{
				JImGuiImpl::Text(pInfo->Name());
				Begin(obj, pInfo);
				SelectorPreviewOnScreen(selectedPreview, pInfo);
				bool isOpen = SelectorOnScreen<ValueType>(&selectedObj, obj, pInfo, GetUniqueLabel(obj, pInfo));
				if (!isOpen)
				{
					SetFirstTrigger(true); 
					if (GetPreviewCount() > 0)
						SetCloseTrigger(true);
				}
			}
		protected:
			void SetSelectObject(Core::JIdentifier* obj, Core::JPropertyInfo* pInfo) final
			{ 
				SetPropertyValue(obj, pInfo, selectedObj); 
			}
		private:
			void CreateSelectorPreviewList()
			{
				selectedPreview = CreatePreviewScene(Core::GetUserPtr<JResourceObject>(selectedObj));
			}
		};
		template<typename T>
		class JGuiMultiSelectorHandle : public JGuiSelectorHandleHelper, public JGuiPropertySetInterface
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
			void Initialize(Core::JIdentifier* obj, Core::JPropertyInfo* pInfo) final
			{
				JGuiSelectorHandleHelper::Initialize(obj, pInfo);

				container = pInfo->Get<T>(obj);
				containerCount = (uint)container.size();

				CreateSelectorPreviewList();
			}
			void Update(Core::JIdentifier* obj, Core::JPropertyInfo* pInfo) final
			{
				const bool isRenderItemMaterial = obj->GetTypeInfo().IsA<JRenderItem>() && 
					std::is_base_of_v<JMaterial, ValueType>;

				JImGuiImpl::Text(pInfo->Name());
				Begin(obj, pInfo);
				for (uint i = 0; i < containerCount; ++i)
				{
					const std::string uniqueLabel = GetUniqueLabel(obj, pInfo) + "_" + std::to_string(i);
					if (isRenderItemMaterial)
					{
						JRenderItem* rItem = static_cast<JRenderItem*>(obj); 
						JImGuiImpl::Text(JCUtil::WstrToU8Str(rItem->GetMesh()->GetSubMeshName(i)));
					}
					
					SelectorPreviewOnScreen(selectedPreview[i], pInfo);
					bool res = SelectorOnScreen<ValueType>(&container[i], obj, pInfo, uniqueLabel);
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
			void SetSelectObject(Core::JIdentifier* obj, Core::JPropertyInfo* pInfo) final
			{
				SetPropertyValue(obj, pInfo, container); 
			}
		private:
			void CreateSelectorPreviewList()
			{
				selectedPreview.resize(containerCount);
				for (uint i = 0; i < containerCount; ++i)
					selectedPreview[i] = CreatePreviewScene(Core::GetUserPtr<JResourceObject>(container[i]));
			}
		};
		//bool
		class JGuiCheckBoxHandle : public JGuiPropertyWidgetHandle, public JGuiPropertySetInterface
		{
		private:
			bool value = false;
		public:
			void Initialize(Core::JIdentifier* obj, Core::JPropertyInfo* pInfo)final
			{
				if (pInfo->GetHint().jDataEnum != Core::J_PARAMETER_TYPE::Bool)
					return;

				value = pInfo->Get<bool>(obj);
			}
			void Update(Core::JIdentifier* obj, Core::JPropertyInfo* pInfo)final
			{
				if (pInfo->GetHint().jDataEnum != Core::J_PARAMETER_TYPE::Bool)
					return;

				if (JImGuiImpl::CheckBox(pInfo->Name() + "##GuiCheckbox" + GetUniqueLabel(obj, pInfo), value))
					SetPropertyValue(obj, pInfo, value);
			}
		};
		//int, float
		template<typename T, std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>, int> = 0>
		class JGuiSliderHandle : public JGuiPropertyWidgetHandle, public JGuiPropertySetInterface
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
			void Initialize(Core::JIdentifier* obj, Core::JPropertyInfo* pInfo)final
			{
				Core::JGuiSliderInfo* sliderInfo = static_cast<Core::JGuiSliderInfo*>(pInfo->GetOptionInfo()->GetWidgetInfo());

				minValue = static_cast<T>(sliderInfo->GetMinValue());
				maxValue = static_cast<T>(sliderInfo->GetMaxValue());
				isSupportInput = sliderInfo->IsSupportInput();
				isVertical = sliderInfo->IsVertical();
			}
			void Update(Core::JIdentifier* obj, Core::JPropertyInfo* pInfo)final
			{
				value = pInfo->Get<T>(obj);
				JImGuiImpl::Text(pInfo->Name());
				ImGui::SameLine();
				if (isSupportInput)
				{
					ImGui::SetNextItemWidth(JImGuiImpl::GetSliderWidth());
					if constexpr (std::is_integral_v<T>)
					{
						if (JImGuiImpl::InputInt("##GuiSliderIntInput" + GetUniqueLabel(obj, pInfo), &value))
							SetPropertyValue(obj, pInfo, value);
					}
					else
					{
						if (JImGuiImpl::InputFloat("##GuiSliderFloatInput" + GetUniqueLabel(obj, pInfo), &value, 0, "%.1f"))
							SetPropertyValue(obj, pInfo, value);
					}
					ImGui::SameLine();
				}

				if (isVertical)
				{
					JVector2<float> vSliderSize{ JImGuiImpl::GetClientWindowSize().x * 0.01f, JImGuiImpl::GetClientWindowSize().y * 0.075f };
					if constexpr (std::is_integral_v<T>)
					{
						if (JImGuiImpl::VSliderInt("##GuiIntVSlider" + GetUniqueLabel(obj, pInfo), vSliderSize, &value, minValue, maxValue, "", ImGuiSliderFlags_AlwaysClamp))
							SetPropertyValue(obj, pInfo, value);
					}
					else
					{
						if (JImGuiImpl::VSliderFloat("##GuiFloatVSlider" + GetUniqueLabel(obj, pInfo), vSliderSize, &value, minValue, maxValue, "", ImGuiSliderFlags_AlwaysClamp))
							SetPropertyValue(obj, pInfo, value);
					}
				}
				else
				{
					ImGui::SetNextItemWidth(JImGuiImpl::GetSliderWidth());
					if constexpr (std::is_integral_v<T>)
					{
						if (JImGuiImpl::SliderInt("##GuiIntSlider" + GetUniqueLabel(obj, pInfo), &value, minValue, maxValue, "", ImGuiSliderFlags_AlwaysClamp))
							SetPropertyValue(obj, pInfo, value);
					}
					else
					{
						if (JImGuiImpl::SliderFloat("##GuiFloatSlider" + GetUniqueLabel(obj, pInfo), &value, minValue, maxValue, "", ImGuiSliderFlags_AlwaysClamp))
							SetPropertyValue(obj, pInfo, value);
					}
				}

				if (ImGui::IsItemActive() || ImGui::IsItemHovered())
					ImGui::SetTooltip("%.1f", value);
			}
		};
		//XMFLOAT4, JVector4, XMFLOAT3, JVector3
		template<typename T>
		class JGuiColorPickerHandle : public JGuiPropertyWidgetHandle, public JGuiPropertySetInterface
		{
		private:
			float hasRgbInput = false;
			T colorV;
		private:
			ImGuiColorEditFlags flag = ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_DisplayRGB;
		private:
			void Initialize(Core::JIdentifier* obj, Core::JPropertyInfo* pInfo)final
			{
				Core::JGuiColorPickerInfo* colorPickerInfo = static_cast<Core::JGuiColorPickerInfo*>(pInfo->GetOptionInfo()->GetWidgetInfo());
				hasRgbInput = colorPickerInfo->HasRgbInput();
				if (hasRgbInput)
					flag |= ImGuiColorEditFlags_InputRGB;
			}
			void Update(Core::JIdentifier* obj, Core::JPropertyInfo* pInfo)final
			{
				colorV = pInfo->Get<T>(obj);
				JImGuiImpl::Text(pInfo->Name());

				if constexpr (Core::JVectorDetermine<T>::value)
				{
					if constexpr (T::GetDigitCount() == 3)
					{
						if (ImGui::ColorPicker3(("##GuiColorPicker" + GetUniqueLabel(obj, pInfo)).c_str(), (float*)&colorV, flag))
							SetPropertyValue(obj, pInfo, colorV);
					}
					else if constexpr (T::GetDigitCount() == 4)
					{
						if (ImGui::ColorPicker4(("##GuiColorPicker" + GetUniqueLabel(obj, pInfo)).c_str(), (float*)&colorV, flag))
							SetPropertyValue(obj, pInfo, colorV);
					}

				}
				else
				{
					if constexpr (std::is_same_v<T, DirectX::XMFLOAT3>)
					{
						if (ImGui::ColorPicker3(("##GuiColorPicker" + GetUniqueLabel(obj, pInfo)).c_str(), (float*)&colorV, flag))
						{
							if constexpr (std::is_same_v<T, DirectX::XMFLOAT3>)
								SetPropertyValue(obj, pInfo, DirectX::XMFLOAT3(colorV.x, colorV.y, colorV.z));							 
						}
					}
					else if constexpr (std::is_same_v<T, DirectX::XMFLOAT4>)
					{
						if (ImGui::ColorPicker4(("##GuiColorPicker" + GetUniqueLabel(obj, pInfo)).c_str(), (float*)&colorV, flag))
						{
							if constexpr (std::is_same_v<T, DirectX::XMFLOAT4>)
								SetPropertyValue(obj, pInfo, DirectX::XMFLOAT4(colorV.x, colorV.y, colorV.z, colorV.w));
						}
					}
				}
			}
		};
		//int, float ,string, vector
		template<typename T>
		class JGuiReadOnlyTextHandle : public JGuiPropertyWidgetHandle
		{
		private:
			using ValueType = typename Core::JVectorDetermine<T>::ValueType;
		public:
			void Initialize(Core::JIdentifier* obj, Core::JPropertyInfo* pInfo) final {}
			void Update(Core::JIdentifier* obj, Core::JPropertyInfo* pInfo) final
			{
				JImGuiImpl::Text(pInfo->Name());
				ImGui::SameLine();

				T value = pInfo->Get<T>(obj);

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
		class JGuiEnumComboBoxHandle : public JGuiPropertyWidgetHandle, public JGuiPropertySetInterface
		{
		public:
			void Initialize(Core::JIdentifier* obj, Core::JPropertyInfo* pInfo) final
			{ 
			}
			void Update(Core::JIdentifier* obj, Core::JPropertyInfo* pInfo) final
			{
				if (pInfo->GetHint().jDataEnum == Core::J_PARAMETER_TYPE::Enum)
				{
					Core::JGuiEnumComboBoxInfo* enumComboInfo = static_cast<Core::JGuiEnumComboBoxInfo*>(pInfo->GetOptionInfo()->GetWidgetInfo());
					Core::JEnumInfo* enumInfo = Core::JReflectionInfo::Instance().GetEnumInfo(enumComboInfo->GetEnumFullName());
					if (enumInfo == nullptr)
						return;

					Core::JEnum enumValue = pInfo->UnsafeGet<Core::JEnum>(obj);
					int selectedIndex = enumInfo->GetEnumIndex(enumValue);

					const std::string comboLabel = "##GuiEnumComboBox" + GetUniqueLabel(obj, pInfo);
					if (JImGuiImpl::BeginCombo(comboLabel, enumInfo->ElementName(enumValue).c_str(), ImGuiComboFlags_HeightLarge))
					{
						uint enumCount = enumInfo->GetEnumCount();
						for(uint i = 0; i < enumCount; ++i)
						{
							const bool isSelected = (selectedIndex == i);
							if (JImGuiImpl::Selectable(enumInfo->ElementName(enumInfo->EnumValue(i)), isSelected))
							{
								if (selectedIndex != i)
									UnsafeSetPropertyValue<Core::JEnum>(obj, pInfo, enumInfo->EnumValue(i));
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

		//GuiTable 
		class JGuiTableHandle : public JGuiPropertyGroupHandle
		{
		private:
			bool isOpen = false;
			uint rowIndex = 0;
			Core::JGuiFlag flag = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_BordersV |
				ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_RowBg | ImGuiTableFlags_ContextMenuInBody;
		private:
			static const ImGuiTableColumnFlags_ columnDefaultFlag = ImGuiTableColumnFlags_WidthStretch;
		public:
			void Update(Core::JIdentifier* obj, Core::JPropertyInfo* pInfo, JGuiPropertyWidgetHandle* widgetHandle)final
			{
				Core::JGuiGroupInfo* groupInfo = Core::JGuiGroupMap::GetGuiGroupInfo(pInfo->GetOptionInfo()->GetWidgetInfo()->GetGroupKey());
				if (groupInfo == nullptr || groupInfo->GetGuiGroupType() != Core::J_GUI_GROUP_TYPE::TABLE)
					return;

				Core::JGuiTableInfo* tableInfo = static_cast<Core::JGuiTableInfo*>(groupInfo);
				if (rowIndex == 0)
					Begin(pInfo, tableInfo);

				if (isOpen)
				{
					uint i = 0;
					JImGuiImpl::TableNextRow();
					if (tableInfo->IsFirstColunmGuide())
					{
						JImGuiImpl::TableSetColumnIndex(0);
						JImGuiImpl::Text(pInfo->Name());
						widgetHandle->SetPrintTitle(false);
						++i;
					}

					const uint columnCount = tableInfo->GetColumnCount();
					for (; i < columnCount; ++i)
					{
						JImGuiImpl::TableSetColumnIndex(i);
						widgetHandle->Update(obj, pInfo);
					}
					++rowIndex;
					if (tableInfo->GetRowCount() == rowIndex)
						End();
				}
			}
		private:
			void Begin(Core::JPropertyInfo* pInfo, Core::JGuiTableInfo* tableInfo)
			{
				const uint columnCount = tableInfo->GetColumnCount();
				isOpen = JImGuiImpl::BeginTable("##GuiTable" + tableInfo->GetGroupName(), columnCount, flag);
				if (isOpen)
				{
					for (uint i = 0; i < columnCount; ++i)
						JImGuiImpl::TableSetupColumn(tableInfo->GetColumnGuide(i), columnDefaultFlag);
					JImGuiImpl::TableHeadersRow();
				}
			}
			void End()
			{
				JImGuiImpl::EndTable();
				isOpen = false;
				rowIndex = 0;
			}
		};
		class JGuiEnumTriggerHandle : public JGuiPropertyGroupHandle
		{
		public:
			void Update(Core::JIdentifier* obj, Core::JPropertyInfo* pInfo, JGuiPropertyWidgetHandle* widgetHandle)final
			{
				Core::JGuiWidgetInfo* widgetInfo = pInfo->GetOptionInfo()->GetWidgetInfo();
				Core::JGuiGroupInfo* groupInfo = Core::JGuiGroupMap::GetGuiGroupInfo(widgetInfo->GetGroupKey());
				if (groupInfo == nullptr || groupInfo->GetGuiGroupType() != Core::J_GUI_GROUP_TYPE::ENUM_TRIGGER)
					return;

				Core::JGuiEnumTriggerInfo* enumTriggerInfo = static_cast<Core::JGuiEnumTriggerInfo*>(groupInfo);
				Core::JPropertyInfo* propertyInfo = pInfo->GetTypeInfo()->GetProperty(enumTriggerInfo->GetParamName());
				if (propertyInfo == nullptr || propertyInfo->GetHint().jDataEnum != Core::J_PARAMETER_TYPE::Enum)
					return;
				 
				Core::JGuiEnumTriggerGroupMemberInfoHandle* memberHandle = static_cast<Core::JGuiEnumTriggerGroupMemberInfoHandle*>(widgetInfo->GetGroupMemberInfo());
				if (memberHandle->OnTrigger(propertyInfo->UnsafeGet<Core::JEnum>(obj)))
					widgetHandle->Update(obj, pInfo);
			}
		};
#pragma endregion

#pragma region Method
		template<typename T>
		class JGuiGetMethod : public JGuiMethodWidgetHandle
		{
		private:
			using ValueType = typename Core::JVectorDetermine<T>::ValueType;
		public:
			void Initialize(Core::JIdentifier* obj, Core::JMethodInfo* mInfo) final {}
			void Update(Core::JIdentifier* obj, Core::JMethodInfo* mInfo) final
			{
				//Get method의 접두어는 Get
				JImGuiImpl::Text(mInfo->Name().substr(3));
				ImGui::SameLine();

				T value = mInfo->Invoke<T>(obj);

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
#pragma endregion

		void JReflectionGuiWidgetHelper::UpdatePropertyGuiWidget(Core::JIdentifier* obj, Core::JPropertyInfo* pInfo)
		{
			if (obj == nullptr)
				return;

			auto widgetHandle = guiPWidgetHandleMap.find(pInfo->Name());
			if (widgetHandle == guiPWidgetHandleMap.end())
			{
				Core::JPropertyOptionInfo* opInfo = pInfo->GetOptionInfo();
				Core::JParameterHint pHint = pInfo->GetHint();
				Core::J_PARAMETER_TYPE pType = pHint.jDataEnum;

				switch (opInfo->GetWidgetInfo()->GetSupportWidgetType())
				{
				case J_GUI_WIDGET_INPUT:
				{
					if (pType == Core::J_PARAMETER_TYPE::Int)
						guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiInputHandle<int>>());
					else if (pType == Core::J_PARAMETER_TYPE::Float)
						guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiInputHandle<float>>());
					else if (pType == Core::J_PARAMETER_TYPE::String)
						guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiInputHandle<std::string>>());
					else if (pType == Core::J_PARAMETER_TYPE::JVector2)
					{
						if (typeid(JVector2<int>).name() == pHint.name || typeid(JVector2<uint>).name() == pHint.name)
							guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiInputHandle<JVector2<int>>>());
						else if (typeid(JVector2<float>).name() == pHint.name)
							guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiInputHandle<JVector2<float>>>());
						else if (typeid(JVector2<std::string>).name() == pHint.name)
							guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiInputHandle<JVector2<std::string>>>());
					}
					else if (pType == Core::J_PARAMETER_TYPE::JVector3)
					{
						if (typeid(JVector3<int>).name() == pHint.name || typeid(JVector3<uint>).name() == pHint.name)
							guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiInputHandle<JVector3<int>>>());
						else if (typeid(JVector3<float>).name() == pHint.name)
							guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiInputHandle<JVector3<float>>>());
						else if (typeid(JVector3<std::string>).name() == pHint.name)
							guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiInputHandle<JVector3<std::string>>>());
					}
					else if (pType == Core::J_PARAMETER_TYPE::JVector4)
					{
						if (typeid(JVector4<int>).name() == pHint.name || typeid(JVector4<uint>).name() == pHint.name)
							guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiInputHandle<JVector4<int>>>());
						else if (typeid(JVector4<float>).name() == pHint.name)
							guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiInputHandle<JVector4<float>>>());
						else if (typeid(JVector4<std::string>).name() == pHint.name)
							guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiInputHandle<JVector4<std::string>>>());
					}
					else if (pType == Core::J_PARAMETER_TYPE::XMInt2)
						guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiInputHandle<DirectX::XMINT2>>());
					else if (pType == Core::J_PARAMETER_TYPE::XMInt3)
						guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiInputHandle<DirectX::XMINT3>>());
					else if (pType == Core::J_PARAMETER_TYPE::XMInt4)
						guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiInputHandle<DirectX::XMINT4>>());
					else if (pType == Core::J_PARAMETER_TYPE::XMFloat2)
						guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiInputHandle<DirectX::XMFLOAT2>>());
					else if (pType == Core::J_PARAMETER_TYPE::XMFloat3)
						guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiInputHandle<DirectX::XMFLOAT3>>());
					else if (pType == Core::J_PARAMETER_TYPE::XMFloat4)
						guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiInputHandle<DirectX::XMFLOAT4>>());
					break;
				}
				case J_GUI_WIDGET_CHECKBOX:
				{
					if (pType == Core::J_PARAMETER_TYPE::Bool)
						guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiCheckBoxHandle>());
					break;
				}
				case J_GUI_WIDGET_SLIDER:
				{
					if (pType == Core::J_PARAMETER_TYPE::Int)
						guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiSliderHandle<int>>());
					else if (pType == Core::J_PARAMETER_TYPE::Float)
						guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiSliderHandle<float>>());
					break;
				}
				case J_GUI_WIDGET_COLOR_PICKER:
				{
					if (pType == Core::J_PARAMETER_TYPE::XMFloat4)
						guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiColorPickerHandle<DirectX::XMFLOAT4>>());
					else if (pType == Core::J_PARAMETER_TYPE::JVector4)
						guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiColorPickerHandle<JVector4<float>>>());
					else if (pType == Core::J_PARAMETER_TYPE::XMFloat3)
						guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiColorPickerHandle<DirectX::XMFLOAT3>>());
					else if (pType == Core::J_PARAMETER_TYPE::JVector3)
						guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiColorPickerHandle<JVector3<float>>>());
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
								guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiMultiSelectorHandle<std::vector<JAnimationClip*>>>());
							else
								guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiSingleSelectorHandle<JAnimationClip*>>());
						}
						else if (typeinfo->IsA<JAnimationController>())
						{
							if (isVector)
								guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiMultiSelectorHandle<std::vector<JAnimationController*>>>());
							else
								guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiSingleSelectorHandle<JAnimationController*>>());
						}
						else if (typeinfo->IsA<JMaterial>())
						{
							if (isVector)
								guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiMultiSelectorHandle<std::vector<JMaterial*>>>());
							else
								guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiSingleSelectorHandle<JMaterial*>>());
						}
						else if (typeinfo->IsA<JMeshGeometry>())
						{
							if (isVector)
								guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiMultiSelectorHandle<std::vector<JMeshGeometry*>>>());
							else
								guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiSingleSelectorHandle<JMeshGeometry*>>());
						}
						else if (typeinfo->IsA<JStaticMeshGeometry>())
						{
							if (isVector)
								guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiMultiSelectorHandle<std::vector<JStaticMeshGeometry*>>>());
							else
								guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiSingleSelectorHandle<JStaticMeshGeometry*>>());
						}
						else if (typeinfo->IsA<JSkinnedMeshGeometry>())
						{
							if (isVector)
								guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiMultiSelectorHandle<std::vector<JSkinnedMeshGeometry*>>>());
							else
								guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiSingleSelectorHandle<JSkinnedMeshGeometry*>>());
						}
						else if (typeinfo->IsA<JScene>())
						{
							if (isVector)
								guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiMultiSelectorHandle<std::vector<JScene*>>>());
							else
								guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiSingleSelectorHandle<JScene*>>());
						}
						else if (typeinfo->IsA<JShader>())
						{
							if (isVector)
								guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiMultiSelectorHandle<std::vector<JShader*>>>());
							else
								guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiSingleSelectorHandle<JShader*>>());
						}
						else if (typeinfo->IsA<JSkeletonAsset>())
						{
							if (isVector)
								guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiMultiSelectorHandle<std::vector<JSkeletonAsset*>>>());
							else
								guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiSingleSelectorHandle<JSkeletonAsset*>>());
						}
						else if (typeinfo->IsA<JTexture>())
						{
							if (isVector)
								guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiMultiSelectorHandle<std::vector<JTexture*>>>());
							else
								guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiSingleSelectorHandle<JTexture*>>());
						}
						/*else if (typeinfo->IsA<JResourceObject>())
						{
							if (isVector)
								guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiSelectorHandle<std::vector<JResourceObject*>>>());
							else
								guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiSelectorHandle<JResourceObject*>>());
						}*/
					}
					break;
				}
				case J_GUI_WIDGET_READONLY_TEXT:
				{	if (pType == Core::J_PARAMETER_TYPE::Int)
					guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiReadOnlyTextHandle<int>>());
				else if (pType == Core::J_PARAMETER_TYPE::Float)
					guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiReadOnlyTextHandle<float>>());
				else if (pType == Core::J_PARAMETER_TYPE::String)
					guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiReadOnlyTextHandle<std::string>>());
				else if (pType == Core::J_PARAMETER_TYPE::JVector2)
				{
					if (typeid(JVector2<int>).name() == pHint.name || typeid(JVector2<uint>).name() == pHint.name)
						guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiReadOnlyTextHandle<JVector2<int>>>());
					else if (typeid(JVector2<float>).name() == pHint.name)
						guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiReadOnlyTextHandle<JVector2<float>>>());
					else if (typeid(JVector2<std::string>).name() == pHint.name)
						guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiReadOnlyTextHandle<JVector2<std::string>>>());
				}
				else if (pType == Core::J_PARAMETER_TYPE::JVector3)
				{
					if (typeid(JVector3<int>).name() == pHint.name || typeid(JVector3<uint>).name() == pHint.name)
						guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiReadOnlyTextHandle<JVector3<int>>>());
					else if (typeid(JVector3<float>).name() == pHint.name)
						guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiReadOnlyTextHandle<JVector3<float>>>());
					else if (typeid(JVector3<std::string>).name() == pHint.name)
						guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiReadOnlyTextHandle<JVector3<std::string>>>());
				}
				else if (pType == Core::J_PARAMETER_TYPE::JVector4)
				{
					if (typeid(JVector4<int>).name() == pHint.name || typeid(JVector4<uint>).name() == pHint.name)
						guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiReadOnlyTextHandle<JVector4<int>>>());
					else if (typeid(JVector4<float>).name() == pHint.name)
						guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiReadOnlyTextHandle<JVector4<float>>>());
					else if (typeid(JVector4<std::string>).name() == pHint.name)
						guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiReadOnlyTextHandle<JVector4<std::string>>>());
				}
				else if (pType == Core::J_PARAMETER_TYPE::XMInt2)
					guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiReadOnlyTextHandle<DirectX::XMINT2>>());
				else if (pType == Core::J_PARAMETER_TYPE::XMInt3)
					guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiReadOnlyTextHandle<DirectX::XMINT3>>());
				else if (pType == Core::J_PARAMETER_TYPE::XMInt4)
					guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiReadOnlyTextHandle<DirectX::XMINT4>>());
				else if (pType == Core::J_PARAMETER_TYPE::XMFloat2)
					guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiReadOnlyTextHandle<DirectX::XMFLOAT2>>());
				else if (pType == Core::J_PARAMETER_TYPE::XMFloat3)
					guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiReadOnlyTextHandle<DirectX::XMFLOAT3>>());
				else if (pType == Core::J_PARAMETER_TYPE::XMFloat4)
					guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiReadOnlyTextHandle<DirectX::XMFLOAT4>>());
				break;
				break;
				}
				case J_GUI_WIDGET_ENUM_COMBO:
				{
					if (pType == Core::J_PARAMETER_TYPE::Enum)
						guiPWidgetHandleMap.emplace(pInfo->Name(), std::make_unique<JGuiEnumComboBoxHandle>());
					break;
				}
				default:
					break;
				}
			}
			if (widgetHandle == guiPWidgetHandleMap.end())
			{
				widgetHandle = guiPWidgetHandleMap.find(pInfo->Name());
				if (widgetHandle == guiPWidgetHandleMap.end())
					return;
				else
					widgetHandle->second->Initialize(obj, pInfo);
			}

			const Core::JGuiGroupKey groupKey = pInfo->GetOptionInfo()->GetWidgetInfo()->GetGroupKey();
			if (groupKey != Core::Constants::InvalidGroupKey)
			{
				auto groupHandle = guiPGroupHandleMap.find(groupKey);
				if (groupHandle == guiPGroupHandleMap.end())
				{
					auto groupInfo = Core::JGuiGroupMap::GetGuiGroupInfo(groupKey);
					if (groupInfo != nullptr)
					{
						const Core::J_GUI_GROUP_TYPE gType = groupInfo->GetGuiGroupType();
						if (gType == Core::J_GUI_GROUP_TYPE::TABLE)
							guiPGroupHandleMap.emplace(groupInfo->GetGroupName(), std::make_unique<JGuiTableHandle>());
						else if (gType == Core::J_GUI_GROUP_TYPE::ENUM_TRIGGER)
							guiPGroupHandleMap.emplace(groupInfo->GetGroupName(), std::make_unique<JGuiEnumTriggerHandle>());
						groupHandle = guiPGroupHandleMap.find(groupInfo->GetGroupName());
					}
				}
				if (groupHandle == guiPGroupHandleMap.end())
					widgetHandle->second->Update(obj, pInfo);
				else
					groupHandle->second->Update(obj, pInfo, widgetHandle->second.get());
			}
			else
				widgetHandle->second->Update(obj, pInfo);
		}
		void JReflectionGuiWidgetHelper::UpdateMethodGuiWidget(Core::JIdentifier* obj, Core::JMethodInfo* mInfo)
		{
			if (obj == nullptr)
				return;

			auto widgetHandle = guiMWidgetHandleMap.find(mInfo->Name());
			if (widgetHandle == guiMWidgetHandleMap.end())
			{
				Core::JMethodOptionInfo* opInfo = mInfo->GetOptionInfo();
				Core::JParameterHint pHint = mInfo->ReturnHint();
				Core::J_PARAMETER_TYPE pType = pHint.jDataEnum;
				switch (opInfo->GetWidgetInfo()->GetSupportWidgetType())
				{
				case J_GUI_WIDGET_READONLY_TEXT:
				{	if (pType == Core::J_PARAMETER_TYPE::Int)
					guiMWidgetHandleMap.emplace(mInfo->Name(), std::make_unique<JGuiGetMethod<int>>());
				else if (pType == Core::J_PARAMETER_TYPE::Float)
					guiMWidgetHandleMap.emplace(mInfo->Name(), std::make_unique<JGuiGetMethod<float>>());
				else if (pType == Core::J_PARAMETER_TYPE::String)
					guiMWidgetHandleMap.emplace(mInfo->Name(), std::make_unique<JGuiGetMethod<std::string>>());
				else if (pType == Core::J_PARAMETER_TYPE::JVector2)
				{
					if (typeid(JVector2<int>).name() == pHint.name || typeid(JVector2<uint>).name() == pHint.name)
						guiMWidgetHandleMap.emplace(mInfo->Name(), std::make_unique<JGuiGetMethod<JVector2<int>>>());
					else if (typeid(JVector2<float>).name() == pHint.name)
						guiMWidgetHandleMap.emplace(mInfo->Name(), std::make_unique<JGuiGetMethod<JVector2<float>>>());
					else if (typeid(JVector2<std::string>).name() == pHint.name)
						guiMWidgetHandleMap.emplace(mInfo->Name(), std::make_unique<JGuiGetMethod<JVector2<std::string>>>());
				}
				else if (pType == Core::J_PARAMETER_TYPE::JVector3)
				{
					if (typeid(JVector3<int>).name() == pHint.name || typeid(JVector3<uint>).name() == pHint.name)
						guiMWidgetHandleMap.emplace(mInfo->Name(), std::make_unique<JGuiGetMethod<JVector3<int>>>());
					else if (typeid(JVector3<float>).name() == pHint.name)
						guiMWidgetHandleMap.emplace(mInfo->Name(), std::make_unique<JGuiGetMethod<JVector3<float>>>());
					else if (typeid(JVector3<std::string>).name() == pHint.name)
						guiMWidgetHandleMap.emplace(mInfo->Name(), std::make_unique<JGuiGetMethod<JVector3<std::string>>>());
				}
				else if (pType == Core::J_PARAMETER_TYPE::JVector4)
				{
					if (typeid(JVector4<int>).name() == pHint.name || typeid(JVector4<uint>).name() == pHint.name)
						guiMWidgetHandleMap.emplace(mInfo->Name(), std::make_unique<JGuiGetMethod<JVector4<int>>>());
					else if (typeid(JVector4<float>).name() == pHint.name)
						guiMWidgetHandleMap.emplace(mInfo->Name(), std::make_unique<JGuiGetMethod<JVector4<float>>>());
					else if (typeid(JVector4<std::string>).name() == pHint.name)
						guiMWidgetHandleMap.emplace(mInfo->Name(), std::make_unique<JGuiGetMethod<JVector4<std::string>>>());
				}
				else if (pType == Core::J_PARAMETER_TYPE::XMInt2)
					guiMWidgetHandleMap.emplace(mInfo->Name(), std::make_unique<JGuiGetMethod<DirectX::XMINT2>>());
				else if (pType == Core::J_PARAMETER_TYPE::XMInt3)
					guiMWidgetHandleMap.emplace(mInfo->Name(), std::make_unique<JGuiGetMethod<DirectX::XMINT3>>());
				else if (pType == Core::J_PARAMETER_TYPE::XMInt4)
					guiMWidgetHandleMap.emplace(mInfo->Name(), std::make_unique<JGuiGetMethod<DirectX::XMINT4>>());
				else if (pType == Core::J_PARAMETER_TYPE::XMFloat2)
					guiMWidgetHandleMap.emplace(mInfo->Name(), std::make_unique<JGuiGetMethod<DirectX::XMFLOAT2>>());
				else if (pType == Core::J_PARAMETER_TYPE::XMFloat3)
					guiMWidgetHandleMap.emplace(mInfo->Name(), std::make_unique<JGuiGetMethod<DirectX::XMFLOAT3>>());
				else if (pType == Core::J_PARAMETER_TYPE::XMFloat4)
					guiMWidgetHandleMap.emplace(mInfo->Name(), std::make_unique<JGuiGetMethod<DirectX::XMFLOAT4>>());
				break;
				break;
				}
				default:
					break;
				}
			}
			if (widgetHandle == guiMWidgetHandleMap.end())
			{
				widgetHandle = guiMWidgetHandleMap.find(mInfo->Name());
				if (widgetHandle == guiMWidgetHandleMap.end())
					return;
				else
					widgetHandle->second->Initialize(obj, mInfo);
			}

			widgetHandle->second->Update(obj, mInfo);
		}
		void JReflectionGuiWidgetHelper::Clear()
		{
			guiPWidgetHandleMap.clear();
			guiMWidgetHandleMap.clear();
			guiPGroupHandleMap.clear();
		}
	}
}