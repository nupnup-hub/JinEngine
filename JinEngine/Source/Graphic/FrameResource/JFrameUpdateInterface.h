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

#pragma once
#include"JFrameUpdateInfo.h"
#include"../../Core/Utility/JTypeSequence.h"
#include"../../Object/GraphicRule/FrameResource/JGraphicModuleFrameResourceUserAccess.h"

namespace JinEngine
{
	namespace Graphic
	{ 
		class JFrameDirtyBase
		{
		protected:
			virtual ~JFrameDirtyBase() = default; 
		public:
			virtual int GetFrameDirty()const noexcept = 0;
			virtual int GetFrameDirtyMax()const noexcept = 0;
		public:
			virtual void SetFrameDirty()noexcept = 0;
		public:
			virtual bool IsFrameDirted()const noexcept = 0;
			bool IsFrameHotDirted()const noexcept;
			bool IsLastFrameHotUpdated()const noexcept;
		public:
			virtual void MinusFrameDirty()noexcept = 0;
			virtual void OffFrameDirty()noexcept = 0;
		};
		 
		//just call empty func
		class JFrameDirtyTrigger : public JFrameDirtyBase
		{
		public:
			int GetFrameDirty()const noexcept override;
			int GetFrameDirtyMax()const noexcept override;
		public:
			void SetFrameDirty()noexcept override;
		public:
			bool IsFrameDirted()const noexcept override;
		public:
			void MinusFrameDirty()noexcept override;
			void OffFrameDirty()noexcept override;
		};
		class JFrameDirty : public JFrameDirtyBase
		{
		private:
			int frameDirty = 0; 
		public:
			int GetFrameDirty()const noexcept override;
			int GetFrameDirtyMax()const noexcept override;
		public:
			void SetFrameDirty()noexcept override; 
		public:
			bool IsFrameDirted()const noexcept override; 
		public:
			void MinusFrameDirty()noexcept override;
			void OffFrameDirty()noexcept override;
		};
		 
		template<typename FrameDirty>
		class JFrameDirtyChain : public FrameDirty
		{
		private:
			std::vector<JFrameDirtyListener> listener;
		public:
			void SetFrameDirty()noexcept final
			{
				FrameDirty::SetFrameDirty();
				const uint listenerCount = (uint)listener.size();
				for (uint i = 0; i < listenerCount; ++i)
					static_cast<JFrameUpdateInterface*>((*listener[i].getFrameUserBind)())->GetDirtyBase()->SetFrameDirty();
			}
		public:
			void AddFrameDirtyListener(const JFrameDirtyListener& listener)noexcept
			{
				listener.push_back(listener);
			}
			void RemoveFrameDirtyListener(const size_t guid)noexcept
			{
				uint listenerCount = (uint)listener.size();
				for (uint i = 0; i < listenerCount; ++i)
				{
					if (listener[i].guid == guid)
					{
						listener.erase(listener.begin() + i);
						break;
					}
				}
			}
		};
		 
		class JFrameUpdateInterface : public JFrameUpdateUserInterface
		{
		protected:
			template<typename T, typename = void>
			struct DirtyChainDetermine
			{
			public:
				static constexpr bool value = false;
			};
			template<typename T>
			struct DirtyChainDetermine<T, std::void_t<decltype(&T::AddFrameDirtyListener)>>
			{
			public:
				static constexpr bool value = true;
			};
		private:
			using ObjectUpdateBind = JFrameObjectUpdateB;
			ObjectUpdateBind objectUpdateB;
		public:
			bool Register(const JFrameUploadDataCreationDesc& desc);
			void DeRegister();
		public:
			virtual bool Add(const JUserPtr<JFrameUpdateInfo>& newInfo) = 0;
		public: 
			virtual JFrameUpdateInfo* GetFrameInfo(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept = 0;
			virtual JFrameDirtyBase* GetDirtyBase()const noexcept = 0;
			int GetNumber(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept override;
			int GetFrameIndex(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept override;
			int GetFrameIndexSize(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept override;
			int GetLocalFrameCount(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept override;
		public:
			void SetFrameDirty()noexcept final;
		public:
			bool IsDirted()const noexcept final;
		public:
			void OffFrameDirty()noexcept final;
		public:
			void TryExecuteObjectUpdateBind();
		public:
			bool RegisterObjectUpdateB(JFrameObjectUpdateB&& bind)final;
			bool DeRegisterObjectUpdateB()final;
		};

		/*
		* @brief 중복되지 않는 Type들의 Info holder
		*/
		template<typename FrameDirty, typename TypeSequence>
		class JFrameUpdateTypePerSingleHolder : public JFrameUpdateInterface
		{
			static_assert(std::is_same_v<TypeSequence::ValueType, J_FRAME_RESOURCE_UPLOAD_TYPE>, "Invalid TypeSequence");
		public:
			static constexpr bool isSupportedFrameUpload = true;
			static constexpr bool isSupportedFrameDirty = std::is_base_of_v<JFrameDirtyBase, FrameDirty>;
		private:
			static constexpr uint count = TypeSequence::count;
		private:
			static int order[(uint)J_FRAME_RESOURCE_UPLOAD_TYPE::COUNT];
		private:
			JUserPtr<JFrameUpdateInfo> updateInfo[count];
			mutable FrameDirty dirty;
		public:
			JFrameUpdateTypePerSingleHolder()
			{
				static bool initTrigger = false;
				if (!initTrigger)
				{ 
					TypeSequence::StuffTypeSequenceOrder(std::make_index_sequence<count>(), order, (uint)J_FRAME_RESOURCE_UPLOAD_TYPE::COUNT);
					initTrigger = true;
				}
			}
		public:
			bool Add(const JUserPtr<JFrameUpdateInfo>& newInfo)
			{
				if (!HasSpace(newInfo->GetType()))
					return false;

				updateInfo[order[(uint)newInfo->GetType()]] = newInfo;
				return true;
			}
		public:
			JFrameUpdateInfo* GetFrameInfo(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept final
			{
				return order[(uint)type] != invalidIndex ? updateInfo[(uint)type].Get() : nullptr;
			}
			JFrameDirtyBase* GetDirtyBase()const noexcept final
			{
				return isSupportedFrameDirty ? &dirty : nullptr;
			}
		public:
			bool HasSpace(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept final
			{
				return order[(uint)type] != invalidIndex && updateInfo[order[(uint)type]] != nullptr;
			}
		public:
			bool TryRegisterDirtyListener(const JFrameDirtyListener& listener)final
			{
				if constexpr (DirtyChainDetermine<FrameDirty>::value)
					dirty.AddFrameDirtyListener(listener);
			}
			bool TryDeRegisterDirtyListener(const size_t guid)final
			{
				if constexpr (DirtyChainDetermine<FrameDirty>::value)
					dirty.RemoveFrameDirtyListener(guid);
			}
		};
		template<typename FrameDirty, typename TypeSequence>
		int JFrameUpdateTypePerSingleHolder<FrameDirty, TypeSequence>::order[(uint)J_FRAME_RESOURCE_UPLOAD_TYPE::COUNT];

		template<typename FrameDirty>
		class JFrameUpdateOnlyDirty : public JFrameUpdateInterface
		{
		public:
			static constexpr bool isSupportedFrameUpload = false;
			static constexpr bool isSupportedFrameDirty = std::is_base_of_v<JFrameDirtyBase, FrameDirty>;
		private:
			mutable FrameDirty dirty;
		public:
			bool Add(const JUserPtr<JFrameUpdateInfo>& newInfo)
			{  
				return false;
			}
		public:
			int GetNumber(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept final
			{
				return invalidIndex;
			}
			int GetFrameIndex(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept final
			{
				return invalidIndex;
			}
			int GetFrameIndexSize(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept final
			{
				return invalidIndex;
			}
			JFrameUpdateInfo* GetFrameInfo(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept final
			{
				return nullptr;
			}
			JFrameDirtyBase* GetDirtyBase()const noexcept final
			{
				return &dirty;
			}
		public:
			bool HasSpace(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept final
			{
				return false;
			}
		public:
			bool TryRegisterDirtyListener(const JFrameDirtyListener& listener)final
			{
				if constexpr (DirtyChainDetermine<FrameDirty>::value)
					dirty.AddFrameDirtyListener(listener);
			}
			bool TryDeRegisterDirtyListener(const size_t guid)final
			{
				if constexpr (DirtyChainDetermine<FrameDirty>::value)
					dirty.RemoveFrameDirtyListener(guid);
			}
		};

	}
}