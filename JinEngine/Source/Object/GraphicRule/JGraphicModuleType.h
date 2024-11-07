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
#include"../../Core/Reflection/JReflection.h"
#include"../JObjectType.h"

namespace JinEngine
{  
	//data type경우 값을 void*로 캐스팅가능(비권장) void* 주소값에 값이채워짐
	//사용자지정 데이터타입(클래스, 구조체)경우는 주소값만 전달해야함
	using ResourceHandle = void*;

	//contain draw & compute process 
	REGISTER_ENUM_CLASS(J_GRAPHIC_TASK_TYPE, int, 
		SCENE_DRAW,
		SHADOW_MAP_DRAW,
		OUT_LINE_DRAW,
		DEPTH_MAP_DRAW,
		DEPTH_MAP_VISUALIZE,
		ALBEDO_MAP_VISUALIZE,
		SPECULAR_MAP_VISUALIZE,
		NORMAL_MAP_VISUALIZE,
		TANGENT_MAP_VISUALIZE,
		VELOCITY_MAP_VISUALIZE,
		APPLY_BLUR,
		APPLY_DOWN_SAMPLING,
		APPLY_SSAO,
		APPLY_SSR,
		APPLY_TONE_MAPPING,
		APPLY_BLOOM,
		MANAGE_POST_PROCESS_EXPOSURE,
		MANAGE_POST_PROCESS_HISTOGRAM,
		APPLY_ANTIALISE,
		APPLY_POST_PROCESS_RESULT,
		APPLY_CONVERT_COLOR,
		CONTROLL_POST_PROCESS_PIPELINE,
		SSAO_VISUALIZE,
		SSR_VISUALIZE,
		HZB_CULLING,
		HD_CULLING,
		LIGHT_CULLING,
		LIGHT_LIST_DRAW,	 
		VELOCITY_MAP_COMPUTE,
		DEPTH_RELATIVE_COMPUTE,
		RAYTRACING_AMBIENT_OCCLUSION,
		RAYTRACING_GI,
		RAYTRACING_DENOISE,
		RAYTRACING_SHADOW,
		STORE_PREVIOUS_FRAME_DATA, 
		MANAGE_SHARE_DATA,
		UNKNOWN)
	/*
	enum class J_GRAPHIC_TASK_TYPE
	{
		SCENE_DRAW = 0,
		SHADOW_MAP_DRAW,
		OUT_LINE_DRAW,
		DEPTH_MAP_DRAW,
		DEPTH_MAP_VISUALIZE,		//convert non linear to linear depth map (debug)
		ALBEDO_MAP_VISUALIZE,
		SPECULAR_MAP_VISUALIZE,
		NORMAL_MAP_VISUALIZE,
		TANGENT_MAP_VISUALIZE,
		VELOCITY_MAP_VISUALIZE,
		APPLY_BLUR,
		APPLY_DOWN_SAMPLING,
		APPLY_SSAO,
		APPLY_TONE_MAPPING,
		APPLY_BLOOM,
		MANAGE_POST_PROCESS_EXPOSURE,
		MANAGE_POST_PROCESS_HISTOGRAM,
		APPLY_ANTIALISE,
		APPLY_POST_PROCESS_RESULT,
		APPLY_CONVERT_COLOR,
		CONTROLL_POST_PROCESS_PIPELINE,
		SSAO_VISUALIZE,
		//FRUSTUM_CULLING,		//mostly execute on cpu
		HZB_CULLING,
		HD_CULLING,
		LIGHT_CULLING,
		LIGHT_LIST_DRAW,		//visualize light list (debug),
		RAYTRACING_AMBIENT_OCCLUSION,
		RAYTRACING_GI,
		RAYTRACING_DENOISE,
		RAYTRACING_SHADOW,
		STORE_PREVIOUS_FRAME_DATA,
		//COMPLEX,  
		UNKNOWN,
		COUNT
	};
	*/
	 
	struct JGraphicSceneRegisterDesc
	{

	};

	//The rendering features that a user can request.
	//Owner scene had to register before request
	enum class J_GRAPHIC_REQUEST_TYPE
	{
		DRAW_SCENE,
		DRAW_SHADOW_MAP,
		FRUSTUM_CULLING,
		HZB_OCCLUSION_CULLING,
		HARD_WARE_OCCLUSION_CULLING					//using api feature
	};

	enum class J_GRAPHIC_REQUEST_EXECUTE_FREQUENCY
	{
		ALWAYS,
		UPDATED
	};

	struct JGraphicRequestCreationDesc
	{
	public:
		J_GRAPHIC_REQUEST_TYPE type; 
		J_GRAPHIC_REQUEST_EXECUTE_FREQUENCY frequency;				//option
	public:
		JGraphicRequestCreationDesc(const J_GRAPHIC_REQUEST_TYPE type,
			const J_GRAPHIC_REQUEST_EXECUTE_FREQUENCY frequency = J_GRAPHIC_REQUEST_EXECUTE_FREQUENCY::ALWAYS)
			:type(type), frequency(frequency)
		{}
	};

	enum class J_GRAPHIC_OPTIONAL_FEATURE
	{
		DEFERRED_RENDERING = 0,
		RAYTRACING,
		RAYTRACING_GI,
		POST_PROCESSING,
		GPU_ACCELERATOR, 
		COUNT,
	};

	class JObject;
	using NotifyGraphicFeatureChangedPtr = Core::JSFunctorType<void, JObject*, const bool>::Ptr;
	struct JGraphicOptionalFeatureObserverDesc
	{
	public:
		UniqueIndex uniqueIndex;
		J_GRAPHIC_OPTIONAL_FEATURE type;
		NotifyGraphicFeatureChangedPtr ptr;
	};

}