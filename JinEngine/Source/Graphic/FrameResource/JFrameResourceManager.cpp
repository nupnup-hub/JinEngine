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

//#define USE_FRAME_MOVE_DIRTY_OPTIMIZATION 0

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
	JFrameUpdateDataSet::JFrameUpdateDataSet(const JFrameUpdateDataSet& rhs)
		: objDataVec(rhs.objDataVec), compVec(rhs.compVec), metadata(rhs.metadata), option(rhs.option), updateLog(rhs.updateLog)
	{
	}
	JFrameUpdateDataSet& JFrameUpdateDataSet::operator=(const JFrameUpdateDataSet& rhs)
	{
		objDataVec =rhs.objDataVec;
		compVec =rhs.compVec;
		metadata = rhs.metadata;
		option = rhs.option;
		updateLog = rhs.updateLog;
		return *this;
	}
	uint JFrameUpdateDataSet::GetDataStorageCount()const noexcept
	{
		return objDataVec != nullptr ? objDataVec->Count() : (uint)compVec->size();
	}
	JGraphicObjectDataSetBase* JFrameUpdateDataSet::GetDataSet(const uint index)const noexcept
	{
		return objDataVec != nullptr ? objDataVec->Get(index)->Get() : static_cast<JGraphicObjectDataSetBase*>((*compVec)[index]->ModuleManagedData());
	}

	void JFrameResourceManager::UpdateHint::Initialize(const JGraphicInfo& info)
	{
#ifdef USE_FRAME_MOVE_DIRTY_OPTIMIZATION
		AllocMovedrecord(info.minCapacity);
		ClearRecordValue(0, moveRecordRange);
#else
		dirtyMinIndex = invalidIndex;
		moveDirty = 0;
#endif
	}
	void JFrameResourceManager::UpdateHint::Clear()
	{
#ifdef USE_FRAME_MOVE_DIRTY_OPTIMIZATION
		DeAllocMovedrecord(); 
#endif
	}
	void JFrameResourceManager::UpdateHint::ClearRecordValue(const uint index, const uint count)
	{
#ifdef USE_FRAME_MOVE_DIRTY_OPTIMIZATION
		JCUtil::Fill<MovedRecordElementType, 0>(&movedRecord[index], count);
		//memset(&movedRecord[index], 0, sizeof(MovedRecordElementType) * count);
#endif
	}
#ifdef USE_FRAME_MOVE_DIRTY_OPTIMIZATION
	void JFrameResourceManager::UpdateHint::AllocMovedrecord(const uint count)
	{  
		hasMoveDirty = false;
		moveRecordRange = count;
		movedRecord = new MovedRecordElementType[moveRecordRange]();
	}
	void JFrameResourceManager::UpdateHint::DeAllocMovedrecord()
	{ 
		delete[] movedRecord;
		moveRecordRange = 0;
		hasMoveDirty = false;
	}
#endif
	void JFrameResourceManager::UpdateHint::ReflectInsertNumber(const int number)
	{
#ifdef USE_FRAME_MOVE_DIRTY_OPTIMIZATION
		if (number >= moveRecordRange)
			return;

		--movedRecord[number];
		hasMoveDirty = true;
#else
		if (dirtyMinIndex == invalidIndex || dirtyMinIndex > number)
			dirtyMinIndex = number;
		moveDirty = Constants::gNumFrameResources;
#endif
	}
	void JFrameResourceManager::UpdateHint::ReflectPopNumber(const int number)
	{
#ifdef USE_FRAME_MOVE_DIRTY_OPTIMIZATION
		if (number >= moveRecordRange)
			return;

		++movedRecord[number];
		hasMoveDirty = true;
#else
		if (dirtyMinIndex == invalidIndex || dirtyMinIndex > number)
			dirtyMinIndex = number;
		moveDirty = Constants::gNumFrameResources;
#endif
	}
 

	JFrameResourceManager::~JFrameResourceManager()
	{
		ClearResource();
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
	int JFrameResourceManager::GetMoveDirtyMinIndex(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept
	{
		return hint[(uint)type][GetCurrentFrameIndex()].dirtyMinIndex;
	}
	bool JFrameResourceManager::IsForcedUpdate(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept
	{
		return hint[(uint)type][GetCurrentFrameIndex()].forcedUpdateTrigger;
	}
	bool JFrameResourceManager::DeRegister(JFrameUpdateInfo* info)
	{
		return true;
	}
	void JFrameResourceManager::ReflectInsertNumber(const J_FRAME_RESOURCE_UPLOAD_TYPE type, const uint number)
	{ 
		for (uint i = 0; i < Constants::gNumFrameResources; ++i)
			GetFrameHint(type, i)->ReflectInsertNumber(number);
	}
	void JFrameResourceManager::ReflectPopNumber(const J_FRAME_RESOURCE_UPLOAD_TYPE type, const uint number)
	{ 
		for (uint i = 0; i < Constants::gNumFrameResources; ++i)
			GetFrameHint(type, i)->ReflectPopNumber(number);
	}
#ifdef USE_FRAME_MOVE_DIRTY_OPTIMIZATION
	void JFrameResourceManager::ReflectMoveDirty(const J_FRAME_RESOURCE_UPLOAD_TYPE type)
	{ 
		auto currentResource = GetCurrentFrameResource();
		auto currentHint = GetFrameHint(type, GetCurrentFrameIndex());
		if (!currentHint->hasMoveDirty)
			return;

		int accumulation = 0;
		for (uint j = 0; j < currentHint->moveRecordRange; ++j)
		{
			UpdateHint::MovedRecordElementType& record = currentHint->movedRecord[j];
			if (record == 0)
				continue;

			//Determine chunk
			const uint curIndex = j; 
			int additionalMoveCount = 1;
			for (uint k = j + 1; k < currentHint->moveRecordRange; ++k)
			{
				const bool isMovedBlock = currentHint->movedRecord[k] != 0;
				if (isMovedBlock)
					break;
				else
				{ 
					++additionalMoveCount;
					++j;
				}
			}
			accumulation += record;

			const int bufferCount = currentResource->GetElementCount(type);
			if (accumulation > 0)
			{
				//pull
				const int srcIndex = curIndex + accumulation;
				const int destIndex = curIndex;
				int moveCount = additionalMoveCount + accumulation;
				 
				const Core::JRestrictedRangeVar<int> range(srcIndex + moveCount, 0, bufferCount);
				if (srcIndex + moveCount > bufferCount)
					moveCount = (srcIndex + moveCount) - bufferCount;

				currentResource->MoveData(type, srcIndex, destIndex, moveCount);
			}
			else
			{
				//push
				const int absAcc = abs(accumulation);
				const int srcIndex = curIndex;
				const int destIndex = curIndex + absAcc;
				int moveCount = additionalMoveCount + absAcc;

				const Core::JRestrictedRangeVar<int> range(destIndex + moveCount, 0, bufferCount);
				if (destIndex + moveCount > bufferCount)
					moveCount = (destIndex + moveCount) - bufferCount;

				currentResource->MoveData(type, srcIndex, destIndex, moveCount);
			} 
			record = 0;
		}
		currentHint->hasMoveDirty = false;
	}
#endif
	void JFrameResourceManager::ReBuild(JGraphicDevice* device, const J_FRAME_RESOURCE_UPLOAD_TYPE type, const uint newCount)
	{
#ifdef USE_FRAME_MOVE_DIRTY_OPTIMIZATION
		ReflectMoveDirty(type);

		//ReBuild by newCount
		//expect all frame resource has same capacity
		for (uint i = 0; i < Constants::gNumFrameResources; ++i)
		{
			auto hint = GetFrameHint(type, i);
			hint->forcedUpdateTrigger = true;

			hint->DeAllocMovedrecord();
			hint->AllocMovedrecord(newCount);
			hint->ClearRecordValue(0, newCount);
			GetFrameResource(i)->ReBuild(device, type, newCount);
		}
#else  
		for (uint i = 0; i < Constants::gNumFrameResources; ++i)
		{
			auto hint = GetFrameHint(type, i);
			hint->forcedUpdateTrigger = true; 

			GetFrameResource(i)->ReBuild(device, type, newCount);
		}
#endif
	}
	void JFrameResourceManager::BeginUpdate()
	{
#ifdef USE_FRAME_MOVE_DIRTY_OPTIMIZATION 
		for (uint i = 0; i < (uint)J_FRAME_RESOURCE_UPLOAD_TYPE::COUNT; ++i)
			ReflectMoveDirty((J_FRAME_RESOURCE_UPLOAD_TYPE)i);
#endif
	}
	void JFrameResourceManager::EndUpdate()
	{
		const uint currFrameIndex = GetCurrentFrameIndex();
		for (uint i = 0; i < (uint)J_FRAME_RESOURCE_UPLOAD_TYPE::COUNT; ++i)
		{
			auto& curHint = hint[i][currFrameIndex];
			curHint.forcedUpdateTrigger = false;
#ifdef USE_FRAME_MOVE_DIRTY_OPTIMIZATION
#else
			--curHint.moveDirty;
			if (curHint.moveDirty <= 0)
			{
				curHint.moveDirty = 0;
				curHint.dirtyMinIndex = invalidIndex;
			}
#endif
		}
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