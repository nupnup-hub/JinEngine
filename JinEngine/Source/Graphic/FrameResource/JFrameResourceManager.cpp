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

namespace JinEngine::Graphic
{
	JFrameUpdateDataSet::JFrameUpdateDataSet(ObjectDataSetVec* objDataVec, const JObjectDataSetMetadata& metadata, const JFrameUpdateOption& option)
		:objDataVec(objDataVec), metadata(metadata), option(option)
	{}

	void JFrameResourceManager::UpdateHint::Initialize(const JGraphicInfo& info)
	{
		movedAccumulation = 0;
		movedRecord = new MovedRecordElementType(info.minCapacity);
	}
	void JFrameResourceManager::UpdateHint::Clear()
	{
		delete[] movedRecord;
	}
	void JFrameResourceManager::UpdateHint::ReflectMovedNumber(const int number)
	{ 
		for (uint i = 0; i < Constants::gNumFrameResources; ++i)
		{
			movedRecord[movedAccumulation] = number;
			++movedAccumulation;
		}
	}
	void JFrameResourceManager::UpdateHint::ResizeMovedIndexArray(const uint beforeCount, const uint newCount)
	{
		int* temp = movedRecord;
		movedRecord = new int(newCount);

		if (beforeCount < newCount)
			memcpy(movedRecord, temp, sizeof(MovedRecordElementType) * beforeCount);
		else
			memcpy(movedRecord, temp, sizeof(MovedRecordElementType) * newCount);
		delete[] temp;
	}

	void JFrameResourceManager::Initialize(JGraphicDevice* device)
	{  
		for (uint i = 0; i < (uint)J_FRAME_RESOURCE_UPLOAD_TYPE::COUNT; ++i)
		{
			for(uint j = 0; j < Constants::gNumFrameResources; ++j)
				hint[i][j].Initialize(GetGraphicInfo());
		}
	}
	void JFrameResourceManager::Clear()
	{
		for (uint i = 0; i < (uint)J_FRAME_RESOURCE_UPLOAD_TYPE::COUNT; ++i)
		{
			for (uint j = 0; j < Constants::gNumFrameResources; ++j)
				hint[i][j].Clear();
		} 
	}  
	JFrameResourceManager::UpdateHint* JFrameResourceManager::GetFrameHint(const J_FRAME_RESOURCE_UPLOAD_TYPE type, const uint frameIndex)noexcept
	{ 
		return &hint[(uint)type][frameIndex];
	} 
	void JFrameResourceManager::DeRegister(const JUserPtr<JFrameUpdateInfo>& info)
	{    
		GetFrameHint(info->GetType(), GetCurrentFrameIndex())->ReflectMovedNumber(info->GetNumber());
	}
	void JFrameResourceManager::ReBuild(JGraphicDevice* device, const J_FRAME_RESOURCE_UPLOAD_TYPE type, const uint newCount)
	{
		//ReBuild by newCount
		//expect all frame resource has same capacity
		for (uint i = 0; i < Constants::gNumFrameResources; ++i)
		{
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
			   
			std::sort(currentHint->movedRecord, &currentHint->movedRecord[currentHint->movedAccumulation]);
			 
			const uint bufferCount = currentResource->GetElementCount(type);
			for (uint j = 0; j < currentHint->movedAccumulation; ++j)
			{
				const uint range = j == currentHint->movedAccumulation - 1 ? bufferCount : currentHint->movedRecord[j];		
				currentResource->MoveData(type, currentHint->movedRecord[j], range, j + 1);
			}
  
			memset(currentHint->movedRecord, 0, sizeof(UpdateHint::MovedRecordElementType) * currentHint->movedAccumulation);
			currentHint->movedAccumulation = 0; 
		}

	}
	void JFrameResourceManager::EndUpdate()
	{
		
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