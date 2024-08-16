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

#include"JFrameResourceManager.h" 
#include"../../Object/Component/JComponent.h"

//#include"../../Develop/Debug/JDevelopDebug.h"
namespace JinEngine::Graphic
{ 
	JFrameUpdateDataSet::JFrameUpdateDataSet(const ObjectDataSetVec* objDataVec, const JObjectDataSetMetadata& metadata, const JFrameUpdateOption& option)
		:objDataVec(objDataVec), compVec(nullptr), metadata(metadata), option(option)
	{
	}
	JFrameUpdateDataSet::JFrameUpdateDataSet(const CompVec* compVec, const JObjectDataSetMetadata& metadata, const JFrameUpdateOption& option)
		: objDataVec(nullptr), compVec(compVec), metadata(metadata), option(option)
	{
	}
	uint JFrameUpdateDataSet::GetDataStorageCount()const noexcept
	{
		return objDataVec != nullptr ? objDataVec->Count() : (uint)compVec->size();
	}
	JGraphicObjectDataSetBase* JFrameUpdateDataSet::GetDataSet(const uint index)const noexcept
	{
		return objDataVec != nullptr ? objDataVec->Get(index)->Get() : static_cast<JGraphicObjectDataSetBase*>((*compVec)[index]->ModuleManagedData());
	}

	JFrameResourceManager::~JFrameResourceManager()
	{
		ClearResource();
	}
	void JFrameResourceManager::UpdateHint::Initialize(const JGraphicInfo& info)
	{
		movedAccumulation = 0;
		AllocMovedrecord(info.minCapacity);
		ClearRecordValue(0, moveRecordRange);
	}
	void JFrameResourceManager::UpdateHint::Clear()
	{ 
		DeAllocMovedrecord(); 
		//delete[] movedRecord;
		//movedRecord = nullptr;
	}
	void JFrameResourceManager::UpdateHint::ClearRecordValue(const uint index, const uint count)
	{
		memset(&movedRecord[index], invalidRecord, sizeof(MovedRecordElementType) * count);
	}
	void JFrameResourceManager::UpdateHint::AllocMovedrecord(const uint count)
	{
		moveRecordRange = count;
		movedRecord = new MovedRecordElementType[moveRecordRange]();
	}
	void JFrameResourceManager::UpdateHint::DeAllocMovedrecord()
	{
		delete[] movedRecord;
		moveRecordRange = 0;
		movedAccumulation = 0;
	}
	void JFrameResourceManager::UpdateHint::ReflectMovedNumber(const int number)
	{
		if (moveRecordRange > movedAccumulation)
		{ 
			movedRecord[movedAccumulation] = number;
			++movedAccumulation;
		} 
	}
	void JFrameResourceManager::UpdateHint::ResizeMovedIndexArray(const uint beforeCount, const uint newCount)
	{ 
		//유효한 범위에 movedRecord 값을 보존하기위해 정렬한뒤에 resize
		if (beforeCount > newCount)
		{
			//유효한 범위에 record를 보존하기위해 정렬한다.
			Sort();

			//Develop::JDevelopDebug::PushLog("ResizeMovedIndexArray: " + std::to_string(movedAccumulation) + " " + 
			//	std::to_string(beforeCount) + " " +
			//	std::to_string(newCount));

			//movedAccumulation 만큼 record vector를 참조하므로
			//잘못된 메모리참조를 방지하기위해 값을 조정
			if (movedAccumulation > newCount)
				movedAccumulation = newCount;
		}
		MovedRecordElementType* temp = movedRecord;
		AllocMovedrecord(newCount); 
		
		memcpy(movedRecord, temp, beforeCount > newCount ? newCount : beforeCount);
		if (beforeCount < newCount)
			ClearRecordValue(beforeCount, newCount - beforeCount);
	}
	void JFrameResourceManager::UpdateHint::Sort()
	{ 
		const uint sortEnd = moveRecordRange > movedAccumulation ? movedAccumulation + 1 : moveRecordRange;
		std::sort(movedRecord, &movedRecord[sortEnd]); 
		//std::sort(movedRecord.begin(), movedRecord.end());
	}

	void JFrameResourceManager::Initialize(JGraphicDevice* device)
	{ 
		for (uint i = 0; i < (uint)J_FRAME_RESOURCE_UPLOAD_TYPE::COUNT; ++i)
		{
			for (uint j = 0; j < Constants::gNumFrameResources; ++j)
				hint[i][j].Initialize(GetGraphicInfo());
		}
	}
	void JFrameResourceManager::Clear()
	{
		ClearResource();
	}
	JFrameResourceManager::UpdateHint* JFrameResourceManager::GetFrameHint(const J_FRAME_RESOURCE_UPLOAD_TYPE type, const uint frameIndex)noexcept
	{
		return &hint[(uint)type][frameIndex];
	}
	bool JFrameResourceManager::IsForcedUpdate(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept
	{
		return hint[(uint)type][GetCurrentFrameIndex()].forcedUpdateTrigger;
	}
	bool JFrameResourceManager::DeRegister(JFrameUpdateInfo* info)
	{
		for (uint i = 0; i < Constants::gNumFrameResources; ++i)
			GetFrameHint(info->GetType(), i)->ReflectMovedNumber(info->GetNumber());
		return true;
	}
	void JFrameResourceManager::ReBuild(JGraphicDevice* device, const J_FRAME_RESOURCE_UPLOAD_TYPE type, const uint newCount)
	{
		//ReBuild by newCount
		//expect all frame resource has same capacity
		for (uint i = 0; i < Constants::gNumFrameResources; ++i)
		{
			GetFrameHint(type, i)->forcedUpdateTrigger = true;
			GetFrameHint(type, i)->ResizeMovedIndexArray(GetFrameResource(i)->GetElementCount(type), newCount);
			GetFrameResource(i)->ReBuild(device, type, newCount);
		}
	}
	void JFrameResourceManager::BeginUpdate()
	{  
		const uint frameIndex = GetCurrentFrameIndex();
		auto currentResource = GetCurrentFrameResource();
		for (uint i = 0; i < (uint)J_FRAME_RESOURCE_UPLOAD_TYPE::COUNT; ++i)
		{
			const J_FRAME_RESOURCE_UPLOAD_TYPE type = (J_FRAME_RESOURCE_UPLOAD_TYPE)i;
			auto currentHint = GetFrameHint(type, frameIndex);
			 
			if (currentHint->movedAccumulation == 0)
				continue;

			//currentHint->Sort();
			/*
			Develop::JDevelopDebug::PushLog(Core::GetName(type));
			Develop::JDevelopDebug::PushLog("Acc: " + std::to_string(currentHint->movedAccumulation) + " Range: " +
				std::to_string(currentHint->moveRecordRange));
			Develop::JDevelopDebug::PushLog("After  sort");
			for (uint j = 0; j < currentHint->moveRecordRange; ++j)
			{
				if(currentHint->movedRecord[j] != invalidIndex)
					Develop::JDevelopDebug::PushLog("Index: " + std::to_string(j) + " Record: " + std::to_string(currentHint->movedRecord[j]));
			}

			*/ 
			const uint bufferCount = currentResource->GetElementCount(type);
			for (uint j = 0; j <= currentHint->movedAccumulation; ++j)
			{
				const uint recordValue = currentHint->movedRecord[j];
				//정렬된 상태이므로 Invalid index 이 후에 값들을 얻기위해 순회할 필요가 없다.
				if (recordValue == invalidIndex)
					break;

				//Determin chunk
				uint chunkLength = 1;
				for (uint k = j + 1; k <= currentHint->movedAccumulation; ++k)
				{ 
					//Develop::JDevelopDebug::PushLog(std::to_string(currentHint->movedRecord[k]) + "==" + std::to_string(recordValue) + "+" + std::to_string(chunkLength));
					if (recordValue + chunkLength == currentHint->movedRecord[k])
					{
						//연속된 chunck일시
						++chunkLength;
						++j;
					}
					else if (recordValue == currentHint->movedRecord[k])
					{
						//연속되나 이전과 같은 번호로 Skip대상인 경우.
						++j;
					}
					else
						break;
				}
				//Develop::JDevelopDebug::PushLog("J: " + std::to_string(j) + " Index: " + std::to_string(recordValue) + " chunck: " + std::to_string(chunkLength));
				currentResource->MoveData(type, recordValue, chunkLength);
			}  
			currentHint->ClearRecordValue(0, currentHint->movedAccumulation); 
			currentHint->movedAccumulation = 0;
		}
	//	Develop::JDevelopDebug::Write();
	}
	void JFrameResourceManager::EndUpdate()
	{
		const uint currFrameIndex = GetCurrentFrameIndex();
		for (uint i = 0; i < (uint)J_FRAME_RESOURCE_UPLOAD_TYPE::COUNT; ++i)
			hint[i][currFrameIndex].forcedUpdateTrigger = false;
	}
	void JFrameResourceManager::ClearResource()
	{ 
		for (uint i = 0; i < (uint)J_FRAME_RESOURCE_UPLOAD_TYPE::COUNT; ++i)
		{
			for (uint j = 0; j < Constants::gNumFrameResources; ++j)
				hint[i][j].Clear();
		}
	}
}

/*
Moved senario test result

buffer count: 1024
Type Count: 24
Started half line: 512

Copy one by one
272300.ns
272.3.mics
0.2723.ms

Use moved algorithm and memmove
41600.ns
41.6.mics
0.0416.ms

Use moved algorithm and memcpy
57900.ns
57.9.mics
0.0579.ms

Use moved algorithm and memcpy and last moved indexing(half count is zero)
17800.ns
17.8.mics
0.0178.ms

begin update and end update
100.ns
0.1.mics
0.0001.ms
*/