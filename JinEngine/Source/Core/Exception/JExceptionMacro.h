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
#include"JException.h"
#include"JHresultException.h"  
#include"JNormalException.h"
#include"JCudaException.h"
#include <stdio.h>

namespace JinEngine
{
#pragma once
#ifndef ThrowIfFailed
#define ThrowIfFailedHr(hr)                                                      \
{                                                                                \
    HRESULT hr__ = (hr);                                                         \
    if(hr__ != S_OK) {throw Core::JHresultException(__LINE__, __FILE__, hr__); }       \
}
#define ThrowIfFailedNM(b, msg)                                                  \
{                                                                                \
    bool b_ = (b);                                                               \
    if(!b_) {throw Core::JNormalException(__LINE__, __FILE__, msg); }                   \
}
#define ThrowIfFailedN(b)                                                        \
{                                                                                \
    bool b_ = (b);                                                               \
    if(!b_) {throw Core::JNormalException(__LINE__, __FILE__); }                       \
}
#define ThrowIfFailedC(err)                                                       \
{                                                                                 \
    bool b_ = (err);                                                              \
    if(b_) {throw Core::JCudaException(__LINE__, __FILE__, err); }              \
}
#endif

#pragma once
#ifndef ReturnIfFailed
#define ReturnIfFailedHr(hr, ret)                                                \
{                                                                                \
    HRESULT hr__ = (hr);                                                         \
    if(hr__ != S_OK) {HrException(__LINE__, __FILE__, hr__); return ret;}        \
}

#define ReturnIfFailedNM(b, msg, ret)                                            \
{                                                                                \
    bool b_ = (b);                                                               \
    if(!b_) {throw JNormalException(__LINE__, __FILE__, msg); return ret;}        \
}
#define ReturnIfFailedN(b, ret)                                                  \
{                                                                                \
    bool b_ = (b);                                                               \
    if(!b_) {JNormalException(__LINE__, __FILE__); return ret;}                   \
}
#endif
   
#define JCodeAddress (__FILE__ + '(' + std::to_string(__LINE__) + ')') 
#define JErrorMessage(msg) (JCodeAddress + #msg)


}

/*
#ifndef ThrowIfFailedHr
#define ThrowIfFailedHr(x)                                              \
{                                                                     \
	HRESULT hr__ = (x);                                               \
	std::wstring wfn = AnsiToWString(__FILE__);                       \
	if(FAILED(hr__)) { throw DxException(hr__, L#x, wfn, __LINE__); } \
}
#endif
*/
