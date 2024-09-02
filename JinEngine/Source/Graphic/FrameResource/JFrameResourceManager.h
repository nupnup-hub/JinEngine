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
#include"JFrameResource.h" 
#include"JFrameUpdateInfo.h" 
#include"../JGraphicConstants.h"
#include"../JGraphicSubClassInterface.h"
#include"../JGraphicUpdateLog.h"
#include"../DataSet/JGraphicObjectDataSet.h"
#include"../../Core/Pointer/JOwnerPtr.h"
#include"../../Core/Threading/JThreadInfo.h"
  
#ifdef _USE_FRAME_MOVE_DIRTY_OPTIMIZATION
#define USE_FRAME_MOVE_DIRTY_OPTIMIZATION
#endif
namespace JinEngine
{
	class JObject;
	namespace Graphic
	{ 
		using SetUpdateThreadTaskPtr = Core::JSFunctorType<Core::JThreadUserHandle, const Core::JThreadInitInfo&, std::unique_ptr<Core::JBindHandleBase>&&>::Ptr;
		struct JFrameUpdateOption
		{ 
		public:
			SetUpdateThreadTaskPtr setUpdateThreadTask = nullptr;
		public: 
			bool isActivatedSceneTimer = false;
		}; 
		struct JFrameUpdateDataSet
		{ 
		public:
			using CompVec = std::vector<JUserPtr<JComponent>>;   
		public:
			const ObjectDataSetVec* objDataVec = nullptr;
			const CompVec* compVec = nullptr;
		public:
			JObjectDataSetMetadata metadata;
			JFrameUpdateOption option;  
		public:
			JGraphicUpdateLog updateLog;							//Out
		public:
			JFrameUpdateDataSet() = default;
			JFrameUpdateDataSet(const ObjectDataSetVec* objDataVec, const JObjectDataSetMetadata& metadata, const JFrameUpdateOption& option);
			JFrameUpdateDataSet(const CompVec* compVec, const JObjectDataSetMetadata& metadata, const JFrameUpdateOption& option);
			JFrameUpdateDataSet(const JFrameUpdateDataSet& rhs);
			JFrameUpdateDataSet& operator=(const JFrameUpdateDataSet& rhs);
		public:
			uint GetDataStorageCount()const noexcept; 
			JGraphicObjectDataSetBase* GetDataSet(const uint index)const noexcept;
		};

		class JFrameUpdateInterface;
		class JFrameResourceManager : public JGraphicDeviceUser, public JGraphicSubClassInterface
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(JFrameResourceManager)
		private: 
			//Update Cache 
			//DeRegister시 발생하는 버퍼의 빈공간을 memmove로 update하는 작업은 Update에 한번 수행한다.
			//memmove를 수행할시 가능하면 많은 데이터를 대상으로 수행해 호출 횟수를 줄이는게 중요하므로
			//작업이 필요한 메모리 덩어리를 식별하기 위해 접두사 Moved가 붙은 데이터들 활용한다.
			//덩어리는 DeRegister호출 횟수만큼 생기므로 그 횟수와 인덱스를 기록한 다음
			//Update시 DeRegister이 발생한 인덱스 Array를 정렬한 뒤 memmove을 수행한다.
			//정렬은 22, 44, 66이 있을시 22에서 한칸, 44에서 두칸, 66에서 세칸을 당겨와야하므로
			//각 DeRegister 인덱스마다 카운팅을 옳바르게 하는데 필요하다.
			//2024-08-18 
			//Insert시 ReBuid->Reflect 순으로 진행되야 element를 뒤쪽으로 Push가능하다
			//반대로 Pop시에 ReBuid를 먼저하면 만약 buffer가 축소될시 data를 잃어버리므로
			//해결방법은 버퍼를 복사하고 Reflect를 실행해야하며 종합해서 실시간 update마다
			//타입당 loop 수행과 move search 그리고 buffer 복사와 삭제을 수행하게 되며 현재보다
			//크게 나아지지않고 최악에 경우는 더 않좋을 것 으로 판단되어 적용을 보류한다. 
			struct UpdateHint
			{
				//int minMoveDirtyIndex = invalidIndex;
#ifdef _USE_FRAME_MOVE_DIRTY_OPTIMIZATION
			public:
				using MovedRecordElementType = int;
				using MovedAccumulationType = int;
			public:
				MovedRecordElementType* movedRecord; 
				uint moveRecordRange = 0;
			public: 
				bool hasMoveDirty = false;
#else
			public:
				int dirtyMinIndex = INT_MAX;
				int moveDirty = 0;
#endif
			public: 
				bool forcedUpdateTrigger = false; 
			public:
				void Initialize(const JGraphicInfo& info);
				void Clear(); 
				void ClearRecordValue(const uint index, const uint count);
			public:
#ifdef _USE_FRAME_MOVE_DIRTY_OPTIMIZATION
				void AllocMovedrecord(const uint count);
				void DeAllocMovedrecord();
#endif
			public:
				void ReflectInsertNumber(const int number); 
				void ReflectPopNumber(const int number);  
			}; 
		private:   
			UpdateHint hint[(uint)J_FRAME_RESOURCE_UPLOAD_TYPE::COUNT][Constants::gNumFrameResources];
		public:
			~JFrameResourceManager();
		public:
			virtual void Initialize(JGraphicDevice* device);
			virtual void Clear(); 
		public:
			virtual JFrameResource* GetCurrentFrameResource() noexcept = 0;
			virtual JFrameResource* GetFrameResource(const uint index) noexcept = 0;
			virtual uint GetCurrentFrameIndex() const noexcept = 0; 
			virtual uint GetNextFrameIndex()const noexcept = 0;
			virtual uint GetTotalRegistedCount(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept = 0;
			virtual uint GetTotalFrameCount(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept = 0;
			virtual uint GetAreaRegistedCount(const J_FRAME_RESOURCE_UPLOAD_TYPE type, const size_t areaGuid)const noexcept = 0;
			//count 0 ~ areaStart
			virtual uint GetAreaRegistedOffset(const J_FRAME_RESOURCE_UPLOAD_TYPE type, const size_t areaGuid)const noexcept = 0;
			virtual uint GetFrameResourceCapacity(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept = 0;
		private: 
			UpdateHint* GetFrameHint(const J_FRAME_RESOURCE_UPLOAD_TYPE type, const uint frameIndex)noexcept;
		protected:
			int GetMoveDirtyMinIndex(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept;
		public:
			virtual void SetNextFrameResource() = 0;
		public:
			bool IsForcedUpdate(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept;
		public:
			virtual JUserPtr<JFrameUpdateInfo> Register(const JFrameUploadDataCreationDesc& desc) = 0;
			virtual bool DeRegister(JFrameUpdateInfo* info) = 0;
		protected: 
			void ReflectInsertNumber(const J_FRAME_RESOURCE_UPLOAD_TYPE type, const uint number);
			void ReflectPopNumber(const J_FRAME_RESOURCE_UPLOAD_TYPE type, const uint number);
		private:
#ifdef _USE_FRAME_MOVE_DIRTY_OPTIMIZATION
			void ReflectMoveDirty(const J_FRAME_RESOURCE_UPLOAD_TYPE type);
#endif
		public:
			virtual void ReBuild(JGraphicDevice* device, const J_FRAME_RESOURCE_UPLOAD_TYPE type, const uint newCount);
		public:
			virtual void BeginUpdate();
			/**
			* Update senario
			* 1. classify can update class
			* 2. move data position in array if has moved dirty(occurred remove data)
			*	2.1 copy data from last dirty pos to last element of array
			* 3. frame update if dirted
			* 4. copy data per dirted object
			*/
			virtual void Update(JFrameUpdateDataSet& set) = 0;
			virtual void EndUpdate();   
		private: 
			void ClearResource();
		};
	}
}