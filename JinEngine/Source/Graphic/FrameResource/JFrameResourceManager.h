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
#include"../DataSet/JGraphicObjectDataSet.h"
#include"../../Core/Pointer/JOwnerPtr.h"
 
namespace JinEngine
{
	class JObject;
	namespace Graphic
	{ 
		struct JFrameUpdateOption
		{ 
		}; 
		struct JFrameUpdateDataSet
		{ 
		public:
			using CompVec = std::vector<JUserPtr<JComponent>>;  
		public:
			struct Log
			{
			public:
				uint updatedCount = 0;
				uint hotUpdatedCount = 0;
			};
		public:
			const ObjectDataSetVec* objDataVec;
			const CompVec* compVec;
		public:
			const JObjectDataSetMetadata metadata;
			const JFrameUpdateOption option;  
		public:
			Log updateLog;							//Out
		public:
			JFrameUpdateDataSet(const ObjectDataSetVec* objDataVec, const JObjectDataSetMetadata& metadata, const JFrameUpdateOption& option);
			JFrameUpdateDataSet(const CompVec* compVec, const JObjectDataSetMetadata& metadata, const JFrameUpdateOption& option);
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
			struct UpdateHint
			{
			public:
				using MovedRecordElementType = int;
				using MovedAccumulationType = int;
			public:
				MovedRecordElementType* movedRecord;
				//MovedRecordElementType* movedRecord = nullptr;				//Accumulation left to right
				MovedAccumulationType movedAccumulation;
			public:
				uint moveRecordRange = 0;
			public:
				bool forcedUpdateTrigger = false;
			private:
				static constexpr MovedRecordElementType invalidRecord = invalidIndex;
			public:
				void Initialize(const JGraphicInfo& info);
				void Clear(); 
				void ClearRecordValue(const uint index, const uint count);
			private:
				void AllocMovedrecord(const uint count);
				void DeAllocMovedrecord();
			public:
				void ReflectMovedNumber(const int number);
				void ResizeMovedIndexArray(const uint beforeCount, const uint newCount);
			public:
				void Sort(); 
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
		public:
			virtual void SetNextFrameResource() = 0;
		public:
			bool IsForcedUpdate(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept;
		public:
			virtual JUserPtr<JFrameUpdateInfo> Register(const JFrameUploadDataCreationDesc& desc) = 0;
			virtual bool DeRegister(JFrameUpdateInfo* info);
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