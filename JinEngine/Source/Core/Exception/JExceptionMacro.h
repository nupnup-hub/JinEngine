#pragma once 
#include"JException.h"
#include"JHresultException.h"
#include"JGraphicException.h"
#include"JWindowException.h"
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
#define ThrowIfFailedW(b)                                                        \
{                                                                                \
    BOOL b_ = (b);                                                               \
    if(!b_) {throw Core::JWindowException(__LINE__, __FILE__); }                       \
}
#define ThrowIfFailedNM(b, msg)                                                  \
{                                                                                \
    BOOL b_ = (b);                                                               \
    if(!b_) {throw Core::JNormalException(__LINE__, __FILE__, msg); }                   \
}
#define ThrowIfFailedN(b)                                                        \
{                                                                                \
    BOOL b_ = (b);                                                               \
    if(!b_) {throw Core::JNormalException(__LINE__, __FILE__); }                       \
}
#define ThrowIfFailedG(hr)                                                       \
{                                                                                \
    HRESULT hr__ = (hr);                                                         \
    if(hr__ != S_OK) {throw Core::JGraphicException(__LINE__, __FILE__, hr__); }       \
}
#define ThrowIfFailedC(err)                                                       \
{                                                                                 \
    BOOL b_ = (err);                                                              \
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

#define ReturnIfFailedW(b, ret)                                                  \
{                                                                                \
    BOOL b_ = (b);                                                               \
    if(!b_) {WindowException(__LINE__, __FILE__); return ret;}                        \
}
#define ReturnIfFailedNM(b, msg, ret)                                            \
{                                                                                \
    BOOL b_ = (b);                                                               \
    if(!b_) {throw JNormalException(__LINE__, __FILE__, msg); return ret;}        \
}
#define ReturnIfFailedN(b, ret)                                                  \
{                                                                                \
    BOOL b_ = (b);                                                               \
    if(!b_) {JNormalException(__LINE__, __FILE__); return ret;}                   \
}
#define ReturnIfFailedG(hr, ret)                                                 \
{                                                                                \
    HRESULT hr__ = (hr);                                                         \
    if(hr__ != S_OK) {GfxException(__LINE__, __FILE__, hr__); return ret;}       \
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
