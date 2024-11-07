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
#include"GraphicResource/JGraphicResourceType.h"
#include"JGraphicOption.h" 
#include"JGraphicInfo.h"  
#include"../Core/Event/JEventManager.h"   
#include"Command/JCommandContextLog.h"

namespace JinEngine
{
	namespace Core
	{
		template<typename T>class JCreateUsingNew;
	}
	namespace Graphic
	{
		class JGraphicPrivate;  
		class JGraphic
		{
		private:
			template<typename T>friend class Core::JCreateUsingNew; 
		private:
			friend class JGraphicPrivate;
			class JGraphicImpl;
			class JGraphicModuleImpl;
		private:
			std::unique_ptr<JGraphicImpl> impl;
			std::unique_ptr<JGraphicModuleImpl> moduleImpl;
		public:
			JGraphicInfo GetGraphicInfo()const noexcept; 
			const JGraphicInfo& GetGraphicInfoRef()const noexcept;
			JGraphicOption GetGraphicOption()const noexcept;  
			const JGraphicOption& GetGraphicOptionRef()const noexcept;
			void GetLastDeviceErrorInfo(_Out_ std::wstring& errorCode, _Out_ std::wstring& errorMsg)const noexcept;
			std::vector<JCommandContextLog> GetCpuDrawingLog()const noexcept;
		public:
			void SetGraphicOption(JGraphicOption newGraphicOption)noexcept;  
		private:
			JGraphic();
			~JGraphic();
		};
		  
		using _JGraphic = JinEngine::Core::JSingletonHolder<Graphic::JGraphic>;
	}
	using JGraphic = Graphic::_JGraphic;
}