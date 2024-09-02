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

//#include"../../Develop/Debug/JDevelopDebug.h"
namespace JinEngine
{
	namespace Graphic
	{
		enum class J_FRAME_DIRTY_FREQUENCY
		{
			UPDATED,
			ALWAYS
		};

		class JFrameDirtyChain
		{
		private:
			std::vector<JFrameDirtyListener> listenerVec;
		public:
			uint GetListenerCount()const noexcept;
		public:
			void SetFrameDirty()noexcept;
		public:
			bool AddFrameDirtyListener(JFrameDirtyListener&& listener)noexcept;
			bool RemoveFrameDirtyListener(const size_t guid)noexcept;
		};

		class JFrameDirtyBase
		{
		private:
			std::unique_ptr<JFrameDirtyChain> dirtyChain;
		protected:
			virtual ~JFrameDirtyBase() = default;
		public:
			virtual J_FRAME_DIRTY_FREQUENCY GetDirtyFrequency()const noexcept = 0;
			virtual int GetFrameDirty()const noexcept = 0; 
			uint GetListenerCount()const noexcept;
		public:
			virtual void SetFrameDirty()noexcept;
		public:
			virtual bool IsFrameDirted()const noexcept = 0;
			virtual bool IsLastFrameUpdated()const noexcept = 0;
			bool IsFrameHotDirted()const noexcept;
			bool IsLastFrameHotUpdated()const noexcept;
		public:
			virtual void OffFrameDirty()noexcept = 0;
		public:
			virtual void BeginUpdate()noexcept = 0;
			virtual void EndUpdate()noexcept = 0;
		public:
			bool AddFrameDirtyListener(JFrameDirtyListener&& listener)noexcept;
			bool RemoveFrameDirtyListener(const size_t guid)noexcept;
		};

		//just call empty func
		class JFrameDirtyTrigger : public JFrameDirtyBase
		{
		public:
			J_FRAME_DIRTY_FREQUENCY GetDirtyFrequency()const noexcept final;
			int GetFrameDirty()const noexcept override; 
		public:
			void SetFrameDirty()noexcept override;
		public:
			bool IsFrameDirted()const noexcept override;
			bool IsLastFrameUpdated()const noexcept override;
		public:
			void OffFrameDirty()noexcept override;
		public:
			void BeginUpdate()noexcept override;
			void EndUpdate()noexcept override;
		};

		class JFrameAlwaysDirty : public JFrameDirtyBase
		{
		public:
			J_FRAME_DIRTY_FREQUENCY GetDirtyFrequency()const noexcept final;
			int GetFrameDirty()const noexcept override;
		public:
			void SetFrameDirty()noexcept override;
		public:
			bool IsFrameDirted()const noexcept override;
			bool IsLastFrameUpdated()const noexcept override;
		public:
			void OffFrameDirty()noexcept override;
		public:
			void BeginUpdate()noexcept override;
			void EndUpdate()noexcept override;
		};
		class JFrameDirty : public JFrameDirtyBase
		{
		private:
			int frameDirty = 0;
			bool isLastFrameUpdated = false;
		public:
			J_FRAME_DIRTY_FREQUENCY GetDirtyFrequency()const noexcept final;
			int GetFrameDirty()const noexcept override; 
		public:
			void SetFrameDirty()noexcept override;
		public:
			bool IsFrameDirted()const noexcept override;
			bool IsLastFrameUpdated()const noexcept override;
		public:
			void OffFrameDirty()noexcept override;
		public:
			void BeginUpdate()noexcept override;
			void EndUpdate()noexcept override;
		};

		class JFrameUpdateInterface : public JFrameUpdateUserInterface
		{
		private:
			using ObjectUpdateBind = JFrameObjectUpdateB;
			ObjectUpdateBind hotUpdateBind;
			ObjectUpdateBind alwaysUpdateBind;
		public:
			virtual bool Add(const JUserPtr<JFrameUpdateInfo>& newInfo) = 0;
			virtual JFrameUpdateInfo* Release(const J_FRAME_RESOURCE_UPLOAD_TYPE type) = 0;
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
			bool IsLastUpdated()const noexcept final;
		public:
			void OffFrameDirty()noexcept final;
		public:
			void TryExecuteObjectHotUpdateBind();
			void TryExecuteObjectAlwaysUpdateBind();
		public:
			bool TryRegisterDirtyListener(JFrameDirtyListener&& listener)final;
			bool TryRegisterDirtyListener(const JUserPtr<JObject>& obj)final;
			bool TryDeRegisterDirtyListener(const size_t guid)final;
		public:
			bool RegisterObjectUpdateB(JFrameObjectUpdateB&& hotUpdateBind = nullptr, JFrameObjectUpdateB&& alwaysUpdateBind = nullptr)final;
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
			static constexpr bool isNeedToUpdateEveryFrame = std::is_base_of_v<JFrameAlwaysDirty, FrameDirty>;
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
				const J_FRAME_RESOURCE_UPLOAD_TYPE type = newInfo->GetType();
				if (!HasSpace(type))
					return false;

				updateInfo[GetTypeIndex(type)] = newInfo;
				return true;
			}
			JFrameUpdateInfo* Release(const J_FRAME_RESOURCE_UPLOAD_TYPE type)
			{
				int typeIndex = GetTypeIndex(type);
				return typeIndex != invalidIndex ? updateInfo[typeIndex].Release() : nullptr;
			}
		public:
			JFrameUpdateInfo* GetFrameInfo(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept final
			{
				int typeIndex = GetTypeIndex(type);
				return typeIndex != invalidIndex ? updateInfo[typeIndex].Get() : nullptr;
			} 
			JFrameDirtyBase* GetDirtyBase()const noexcept final
			{
				return isSupportedFrameDirty ? &dirty : nullptr;
			}
		public:
			int GetTypeIndex(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept
			{
				return order[(uint)type];
			}
		public:
			static bool IsSupported(const J_FRAME_RESOURCE_UPLOAD_TYPE type)
			{
				return TypeSequence::Index(type) != invalidIndex;
				//return order[(uint)type] != invalidIndex;
			}
			bool HasSpace(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept final
			{
				int typeIndex = GetTypeIndex(type);
				return typeIndex != invalidIndex && updateInfo[typeIndex] == nullptr;
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
			static constexpr bool isNeedToUpdateEveryFrame = std::is_base_of_v<JFrameAlwaysDirty, FrameDirty>;
		private:
			mutable FrameDirty dirty;
		public:
			bool Add(const JUserPtr<JFrameUpdateInfo>& newInfo)
			{
				return false;
			}
			JFrameUpdateInfo* Release(const J_FRAME_RESOURCE_UPLOAD_TYPE type)
			{
				return nullptr;
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
			static bool IsSupported(const J_FRAME_RESOURCE_UPLOAD_TYPE type)
			{
				return false;
			}
			bool HasSpace(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept final
			{
				return false;
			}
		};

	}
}