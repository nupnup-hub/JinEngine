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

namespace JinEngine
{
    namespace Core
    {

#define J_IDENTITY(x) x
#define J_TO_STR(x) #x
#define J_TO_STR_ADD_COMMA(x) #x    J_TO_STR(@)
#define J_FRIEND(x) friend x; 
#define J_DATA_INDEX(x, offset, n) int x = offset - n;
#define J_STATIC_CONSTEXPR_DATA_INDEX(x, offset, n) static constexpr int x = offset - n;

#define J_APPLY(macro, ...) J_IDENTITY(macro(__VA_ARGS__))

#define J_EVALUATE_COUNT(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10,                                         \
    _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,                                                     \
	_21, _22, _23, _24, _25, _26, _27, _28, _29, _30,													  \
    _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, count, ...)                                         \
    count                                                                                                 \
																										  \
//__VA_ARGS__에 크기만큼 할당된 번호를 오른쪽으로 밀어서 count항에 __VA_ARGS__크기가 위치하게 한다.
//J_EVALUATE_COUNT 상수범위와 J_COUNT  상수범위가 같아야한다.
#define J_COUNT(...) \
    J_IDENTITY(J_EVALUATE_COUNT(__VA_ARGS__, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, \
	30, 29, 28, 27, 26, 25, 24, 23, 22, 21,    \
	20, 19, 18, 17, 16, 15, 14, 13, 12, 11,	   \
    10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))         \
 

        namespace
        {

//매크로 func mapping helper
//parameter에 순서에 맞는 매크로 호출
#define MAP1(m, x)      m(x)
#define MAP2(m, x, ...) m(x) J_IDENTITY(MAP1(m, __VA_ARGS__))
#define MAP3(m, x, ...) m(x) J_IDENTITY(MAP2(m, __VA_ARGS__))
#define MAP4(m, x, ...) m(x) J_IDENTITY(MAP3(m, __VA_ARGS__))
#define MAP5(m, x, ...) m(x) J_IDENTITY(MAP4(m, __VA_ARGS__))
#define MAP6(m, x, ...) m(x) J_IDENTITY(MAP5(m, __VA_ARGS__))
#define MAP7(m, x, ...) m(x) J_IDENTITY(MAP6(m, __VA_ARGS__))
#define MAP8(m, x, ...) m(x) J_IDENTITY(MAP7(m, __VA_ARGS__))
#define MAP9(m, x, ...) m(x) J_IDENTITY(MAP8(m, __VA_ARGS__))
#define MAP10(m, x, ...) m(x) J_IDENTITY(MAP9(m, __VA_ARGS__))
#define MAP11(m, x, ...) m(x) J_IDENTITY(MAP10(m, __VA_ARGS__))
#define MAP12(m, x, ...) m(x) J_IDENTITY(MAP11(m, __VA_ARGS__))
#define MAP13(m, x, ...) m(x) J_IDENTITY(MAP12(m, __VA_ARGS__))
#define MAP14(m, x, ...) m(x) J_IDENTITY(MAP13(m, __VA_ARGS__))
#define MAP15(m, x, ...) m(x) J_IDENTITY(MAP14(m, __VA_ARGS__)) 
#define MAP16(m, x, ...) m(x) J_IDENTITY(MAP15(m, __VA_ARGS__))
#define MAP17(m, x, ...) m(x) J_IDENTITY(MAP16(m, __VA_ARGS__))
#define MAP18(m, x, ...) m(x) J_IDENTITY(MAP17(m, __VA_ARGS__))
#define MAP19(m, x, ...) m(x) J_IDENTITY(MAP18(m, __VA_ARGS__))
#define MAP20(m, x, ...) m(x) J_IDENTITY(MAP19(m, __VA_ARGS__)) 
#define MAP21(m, x, ...) m(x) J_IDENTITY(MAP20(m, __VA_ARGS__))
#define MAP22(m, x, ...) m(x) J_IDENTITY(MAP21(m, __VA_ARGS__))
#define MAP23(m, x, ...) m(x) J_IDENTITY(MAP22(m, __VA_ARGS__))
#define MAP24(m, x, ...) m(x) J_IDENTITY(MAP23(m, __VA_ARGS__))
#define MAP25(m, x, ...) m(x) J_IDENTITY(MAP24(m, __VA_ARGS__)) 
#define MAP26(m, x, ...) m(x) J_IDENTITY(MAP25(m, __VA_ARGS__))
#define MAP27(m, x, ...) m(x) J_IDENTITY(MAP26(m, __VA_ARGS__))
#define MAP28(m, x, ...) m(x) J_IDENTITY(MAP27(m, __VA_ARGS__))
#define MAP29(m, x, ...) m(x) J_IDENTITY(MAP28(m, __VA_ARGS__))
#define MAP30(m, x, ...) m(x) J_IDENTITY(MAP29(m, __VA_ARGS__))
#define MAP31(m, x, ...) m(x) J_IDENTITY(MAP30(m, __VA_ARGS__)) 
#define MAP32(m, x, ...) m(x) J_IDENTITY(MAP31(m, __VA_ARGS__)) 
#define MAP33(m, x, ...) m(x) J_IDENTITY(MAP32(m, __VA_ARGS__)) 
#define MAP34(m, x, ...) m(x) J_IDENTITY(MAP33(m, __VA_ARGS__)) 
#define MAP35(m, x, ...) m(x) J_IDENTITY(MAP34(m, __VA_ARGS__)) 
#define MAP36(m, x, ...) m(x) J_IDENTITY(MAP35(m, __VA_ARGS__)) 
#define MAP37(m, x, ...) m(x) J_IDENTITY(MAP36(m, __VA_ARGS__)) 
#define MAP38(m, x, ...) m(x) J_IDENTITY(MAP37(m, __VA_ARGS__)) 
#define MAP39(m, x, ...) m(x) J_IDENTITY(MAP38(m, __VA_ARGS__)) 
#define MAP40(m, x, ...) m(x) J_IDENTITY(MAP39(m, __VA_ARGS__)) 

#define MAP_MERGE_1(m, x)      m(x)
#define MAP_MERGE_2(m, x, ...) m(x) ## J_IDENTITY(MAP_MERGE_1(m, __VA_ARGS__))
#define MAP_MERGE_3(m, x, ...) m(x) ## J_IDENTITY(MAP_MERGE_2(m, __VA_ARGS__))
#define MAP_MERGE_4(m, x, ...) m(x) ## J_IDENTITY(MAP_MERGE_3(m, __VA_ARGS__))
#define MAP_MERGE_5(m, x, ...) m(x) ## J_IDENTITY(MAP_MERGE_4(m, __VA_ARGS__))
#define MAP_MERGE_6(m, x, ...) m(x) ## J_IDENTITY(MAP_MERGE_5(m, __VA_ARGS__))
#define MAP_MERGE_7(m, x, ...) m(x) ## J_IDENTITY(MAP_MERGE_6(m, __VA_ARGS__))
#define MAP_MERGE_8(m, x, ...) m(x) ## J_IDENTITY(MAP_MERGE_7(m, __VA_ARGS__))
#define MAP_MERGE_9(m, x, ...) m(x) ## J_IDENTITY(MAP_MERGE_8(m, __VA_ARGS__))
#define MAP_MERGE_10(m, x, ...) m(x) ## J_IDENTITY(MAP_MERGE_9(m, __VA_ARGS__))
#define MAP_MERGE_11(m, x, ...) m(x) ## J_IDENTITY(MAP_MERGE_10(m, __VA_ARGS__))
#define MAP_MERGE_12(m, x, ...) m(x) ## J_IDENTITY(MAP_MERGE_11(m, __VA_ARGS__))
#define MAP_MERGE_13(m, x, ...) m(x) ## J_IDENTITY(MAP_MERGE_12(m, __VA_ARGS__))
#define MAP_MERGE_14(m, x, ...) m(x) ## J_IDENTITY(MAP_MERGE_13(m, __VA_ARGS__))
#define MAP_MERGE_15(m, x, ...) m(x) ## J_IDENTITY(MAP_MERGE_14(m, __VA_ARGS__))
#define MAP_MERGE_16(m, x, ...) m(x) ## J_IDENTITY(MAP_MERGE_15(m, __VA_ARGS__))
#define MAP_MERGE_17(m, x, ...) m(x) ## J_IDENTITY(MAP_MERGE_16(m, __VA_ARGS__))
#define MAP_MERGE_18(m, x, ...) m(x) ## J_IDENTITY(MAP_MERGE_17(m, __VA_ARGS__))
#define MAP_MERGE_19(m, x, ...) m(x) ## J_IDENTITY(MAP_MERGE_18(m, __VA_ARGS__))
#define MAP_MERGE_20(m, x, ...) m(x) ## J_IDENTITY(MAP_MERGE_19(m, __VA_ARGS__))             
#define MAP_MERGE_21(m, x, ...) m(x) ## J_IDENTITY(MAP_MERGE_20(m, __VA_ARGS__))
#define MAP_MERGE_22(m, x, ...) m(x) ## J_IDENTITY(MAP_MERGE_21(m, __VA_ARGS__))
#define MAP_MERGE_23(m, x, ...) m(x) ## J_IDENTITY(MAP_MERGE_22(m, __VA_ARGS__))
#define MAP_MERGE_24(m, x, ...) m(x) ## J_IDENTITY(MAP_MERGE_23(m, __VA_ARGS__))
#define MAP_MERGE_25(m, x, ...) m(x) ## J_IDENTITY(MAP_MERGE_24(m, __VA_ARGS__))
#define MAP_MERGE_26(m, x, ...) m(x) ## J_IDENTITY(MAP_MERGE_25(m, __VA_ARGS__))
#define MAP_MERGE_27(m, x, ...) m(x) ## J_IDENTITY(MAP_MERGE_26(m, __VA_ARGS__))
#define MAP_MERGE_28(m, x, ...) m(x) ## J_IDENTITY(MAP_MERGE_27(m, __VA_ARGS__))
#define MAP_MERGE_29(m, x, ...) m(x) ## J_IDENTITY(MAP_MERGE_28(m, __VA_ARGS__))
#define MAP_MERGE_30(m, x, ...) m(x) ## J_IDENTITY(MAP_MERGE_29(m, __VA_ARGS__))            
#define MAP_MERGE_31(m, x, ...) m(x) ## J_IDENTITY(MAP_MERGE_30(m, __VA_ARGS__))
#define MAP_MERGE_32(m, x, ...) m(x) ## J_IDENTITY(MAP_MERGE_31(m, __VA_ARGS__))
#define MAP_MERGE_33(m, x, ...) m(x) ## J_IDENTITY(MAP_MERGE_32(m, __VA_ARGS__))
#define MAP_MERGE_34(m, x, ...) m(x) ## J_IDENTITY(MAP_MERGE_33(m, __VA_ARGS__))
#define MAP_MERGE_35(m, x, ...) m(x) ## J_IDENTITY(MAP_MERGE_34(m, __VA_ARGS__))
#define MAP_MERGE_36(m, x, ...) m(x) ## J_IDENTITY(MAP_MERGE_35(m, __VA_ARGS__))
#define MAP_MERGE_37(m, x, ...) m(x) ## J_IDENTITY(MAP_MERGE_36(m, __VA_ARGS__))
#define MAP_MERGE_38(m, x, ...) m(x) ## J_IDENTITY(MAP_MERGE_37(m, __VA_ARGS__))
#define MAP_MERGE_39(m, x, ...) m(x) ## J_IDENTITY(MAP_MERGE_38(m, __VA_ARGS__))
#define MAP_MERGE_40(m, x, ...) m(x) ## J_IDENTITY(MAP_MERGE_39(m, __VA_ARGS__)) 

#define MAP_INDEX_1(m, c, x)      m(x, c, 1)
#define MAP_INDEX_2(m, c, x, ...) m(x, c, 2) J_IDENTITY(MAP_INDEX_1(m, c,__VA_ARGS__))
#define MAP_INDEX_3(m, c, x, ...) m(x, c, 3) J_IDENTITY(MAP_INDEX_2(m, c,__VA_ARGS__))
#define MAP_INDEX_4(m, c, x, ...) m(x, c, 4) J_IDENTITY(MAP_INDEX_3(m, c,__VA_ARGS__))
#define MAP_INDEX_5(m, c, x, ...) m(x, c, 5) J_IDENTITY(MAP_INDEX_4(m, c, __VA_ARGS__))
#define MAP_INDEX_6(m, c, x, ...) m(x, c, 6) J_IDENTITY(MAP_INDEX_5(m, c,__VA_ARGS__))
#define MAP_INDEX_7(m, c, x, ...) m(x, c, 7) J_IDENTITY(MAP_INDEX_6(m, c,__VA_ARGS__))
#define MAP_INDEX_8(m, c, x, ...) m(x, c, 8) J_IDENTITY(MAP_INDEX_7(m, c,__VA_ARGS__))
#define MAP_INDEX_9(m, c, x, ...) m(x, c, 9) J_IDENTITY(MAP_INDEX_8(m, c,__VA_ARGS__))
#define MAP_INDEX_10(m, c, x, ...) m(x, c, 10) J_IDENTITY(MAP_INDEX_9(m, c,__VA_ARGS__))
#define MAP_INDEX_11(m, c, x, ...) m(x, c, 11) J_IDENTITY(MAP_INDEX_10(m, c,__VA_ARGS__))
#define MAP_INDEX_12(m, c, x, ...) m(x, c, 12) J_IDENTITY(MAP_INDEX_11(m, c,__VA_ARGS__))
#define MAP_INDEX_13(m, c, x, ...) m(x, c, 13) J_IDENTITY(MAP_INDEX_12(m, c,__VA_ARGS__))
#define MAP_INDEX_14(m, c, x, ...) m(x, c, 14) J_IDENTITY(MAP_INDEX_13(m, c,__VA_ARGS__))
#define MAP_INDEX_15(m, c, x, ...) m(x, c, 15) J_IDENTITY(MAP_INDEX_14(m, c,__VA_ARGS__)) 
#define MAP_INDEX_16(m, c, x, ...) m(x, c, 16) J_IDENTITY(MAP_INDEX_15(m, c,__VA_ARGS__))
#define MAP_INDEX_17(m, c, x, ...) m(x, c, 17) J_IDENTITY(MAP_INDEX_16(m, c,__VA_ARGS__))
#define MAP_INDEX_18(m, c, x, ...) m(x, c, 18) J_IDENTITY(MAP_INDEX_17(m, c,__VA_ARGS__))
#define MAP_INDEX_19(m, c, x, ...) m(x, c, 19) J_IDENTITY(MAP_INDEX_18(m, c,__VA_ARGS__))
#define MAP_INDEX_20(m, c, x, ...) m(x, c, 20) J_IDENTITY(MAP_INDEX_19(m, c,__VA_ARGS__)) 
#define MAP_INDEX_21(m, c, x, ...) m(x, c ,21) J_IDENTITY(MAP_INDEX_20(m, c,__VA_ARGS__))
#define MAP_INDEX_22(m, c, x, ...) m(x, c ,22) J_IDENTITY(MAP_INDEX_21(m, c,__VA_ARGS__))
#define MAP_INDEX_23(m, c, x, ...) m(x, c ,23) J_IDENTITY(MAP_INDEX_22(m, c,__VA_ARGS__))
#define MAP_INDEX_24(m, c, x, ...) m(x, c ,24) J_IDENTITY(MAP_INDEX_23(m, c,__VA_ARGS__))
#define MAP_INDEX_25(m, c, x, ...) m(x, c ,25) J_IDENTITY(MAP_INDEX_24(m, c,__VA_ARGS__)) 
#define MAP_INDEX_26(m, c, x, ...) m(x, c ,26) J_IDENTITY(MAP_INDEX_25(m, c,__VA_ARGS__))
#define MAP_INDEX_27(m, c, x, ...) m(x, c ,27) J_IDENTITY(MAP_INDEX_26(m, c,__VA_ARGS__))
#define MAP_INDEX_28(m, c, x, ...) m(x, c ,28) J_IDENTITY(MAP_INDEX_27(m, c,__VA_ARGS__))
#define MAP_INDEX_29(m, c, x, ...) m(x, c ,29) J_IDENTITY(MAP_INDEX_28(m, c,__VA_ARGS__))
#define MAP_INDEX_30(m, c, x, ...) m(x, c ,30) J_IDENTITY(MAP_INDEX_29(m, c,__VA_ARGS__))
#define MAP_INDEX_31(m, c, x, ...) m(x, c, 31) J_IDENTITY(MAP_INDEX_30(m, c,__VA_ARGS__)) 
#define MAP_INDEX_32(m, c, x, ...) m(x, c, 32) J_IDENTITY(MAP_INDEX_31(m, c,__VA_ARGS__)) 
#define MAP_INDEX_33(m, c, x, ...) m(x, c, 33) J_IDENTITY(MAP_INDEX_32(m, c,__VA_ARGS__)) 
#define MAP_INDEX_34(m, c, x, ...) m(x, c, 34) J_IDENTITY(MAP_INDEX_33(m, c,__VA_ARGS__)) 
#define MAP_INDEX_35(m, c, x, ...) m(x, c, 35) J_IDENTITY(MAP_INDEX_34(m, c,__VA_ARGS__)) 
#define MAP_INDEX_36(m, c, x, ...) m(x, c, 36) J_IDENTITY(MAP_INDEX_35(m, c,__VA_ARGS__)) 
#define MAP_INDEX_37(m, c, x, ...) m(x, c, 37) J_IDENTITY(MAP_INDEX_36(m, c,__VA_ARGS__)) 
#define MAP_INDEX_38(m, c, x, ...) m(x, c, 38) J_IDENTITY(MAP_INDEX_37(m, c,__VA_ARGS__)) 
#define MAP_INDEX_39(m, c, x, ...) m(x, c, 39) J_IDENTITY(MAP_INDEX_38(m, c,__VA_ARGS__)) 
#define MAP_INDEX_40(m, c, x, ...) m(x, c, 40) J_IDENTITY(MAP_INDEX_39(m, c,__VA_ARGS__)) 


#define MAP_HELPER(count) MAP##count 
#define MAP_MERGE_HELPER(count) MAP_MERGE_##count 
#define MAP_INDEX_HELPER(count) MAP_INDEX_##count 
 
//J_APPLY(mapHelper, J_COUNT(__VA_ARGS__)) -> MAP_NUMBER_N
//MAP_NUMBER_N(funcMacro, __VA_ARGS__) -> funcMacro(first __VA_ARGS__) and call MAP_NUMBER_N -1(funcMacro, __VA_ARGS__ - 1)

#define CALL_DEFINE(funcMacro, mapHelper, ...)   J_IDENTITY(J_APPLY(mapHelper, J_COUNT(__VA_ARGS__))(funcMacro, __VA_ARGS__)) 
#define CALL_DEFINE_EX(funcMacro, mapHelper, paramMacro, ...)   J_IDENTITY(J_APPLY(mapHelper, J_COUNT(__VA_ARGS__))(funcMacro, paramMacro(__VA_ARGS__)))
#define CALL_DEFINE_COUNT(funcMacro, mapHelper, ...)   J_IDENTITY(J_APPLY(mapHelper, J_COUNT(__VA_ARGS__))(funcMacro, J_COUNT(__VA_ARGS__), __VA_ARGS__)) 
		}
		 
#define J_MERGE_NAME(...) J_IDENTITY(CALL_DEFINE(J_IDENTITY, MAP_MERGE_HELPER, __VA_ARGS__))
#define J_STRINGIZE(...) J_IDENTITY(CALL_DEFINE(J_TO_STR, MAP_HELPER, __VA_ARGS__)) 
#define J_STRINGIZE_ADD_COMMA(...) J_IDENTITY(CALL_DEFINE(J_TO_STR_ADD_COMMA, MAP_HELPER, __VA_ARGS__))

#define J_MAKE_DATA_INDEX_LIST(...) J_IDENTITY(CALL_DEFINE_COUNT(J_DATA_INDEX, MAP_INDEX_HELPER, __VA_ARGS__))
#define J_MAKE_STATIC_CONSTEXPR_DATA_INDEX_LIST(...) J_IDENTITY(CALL_DEFINE_COUNT(J_STATIC_CONSTEXPR_DATA_INDEX, MAP_INDEX_HELPER, __VA_ARGS__))
#define J_MAKE_ENUM_ELEMENT(...) __VA_ARGS__  
#define J_MAKE_FRIEND(...) J_IDENTITY(CALL_DEFINE(J_FRIEND, MAP_HELPER, __VA_ARGS__))  


#define J_SIMPLE_GET_SET(typeName, dataName, methodName)		\
                                                                \
		typeName Get##methodName()const noexcept			    \
		{												\
			return dataName;							\
		}												\
														\
		void Set##methodName(const typeName& newData)noexcept\
		{												\
			dataName = newData;							\
		}                                               \
                                                        \


 #define J_SIMPLE_P_GET_SET(typeName, dataName, methodName)		\
                                                                \
		typeName* Get##methodName()const noexcept			    \
		{												\
			return dataName;							\
		}												\
														\
		void Set##methodName(typeName* newData)noexcept\
		{												\
			dataName = newData;							\
		}                                               \

#define  J_SIMPLE_GET_NO_PREFIX_GET(typeName, dataName, methodName)	\
		typeName methodName()const noexcept				\
		{												\
			return dataName;							\
		}												\

#define  J_SIMPLE_GET(typeName, dataName, methodName)	\
		typeName Get##methodName()const noexcept		\
		{												\
			return dataName;							\
		}												\

#define  J_SIMPLE_P_GET(typeName, dataName, methodName)	\
		typeName* Get##methodName()const noexcept		\
		{												\
			return dataName;							\
		}												\

#define  J_SIMPLE_UNIQUE_P_GET(typeName, dataName, methodName)	\
		typeName* Get##methodName()const noexcept		\
		{												\
			return dataName.get();							\
		}												\

#define  J_SIMPLE_CLAMP_SET(typeName, dataName, methodName, minData, maxData)	\
		void Set##methodName(typeName newData)noexcept\
		{												\
			dataName = std::clamp(newData, minData, maxData);							\
		}                                               \

#define J_SIMPLE_GET_CLAMP_SET(typeName, dataName, methodName, minData, maxData)		\
                                                                \
		typeName Get##methodName()const noexcept			    \
		{														\
			return dataName;									\
		}														\
																\
		J_SIMPLE_CLAMP_SET(typeName, dataName, methodName, minData, maxData)	\
 

#define J_SIMPLE_GET_SET_EX(typeName, dataName, methodName, afterSet)\
                                                        \
		typeName Get##methodName()const noexcept		\
		{												\
			return dataName;							\
		}												\
														\
		void Set##methodName(const typeName& newData)noexcept\
		{												\
			dataName = newData;							\
			afterSet;									\
		}                                               \
                                                        \


#define J_COMPILE_ERROR_MESSAGE(msg)  message(msg)			
 
#define SIZE_OF_ARRAY(a) (sizeof(a)/sizeof((a)[0]))
		  

    }
}