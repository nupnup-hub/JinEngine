#pragma once

namespace JinEngine
{
    namespace Core
    {

#define J_IDENTITY(x) x
#define J_TO_STR(x) #x
#define J_TO_STR_ADD_COMMA(x) #x    J_TO_STR(@)
#define J_FRIEND(x) friend x; 

#define J_APPLY(macro, ...) J_IDENTITY(macro(__VA_ARGS__))

#define J_EVALUATE_COUNT(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10,                                           \
    _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,                                                     \
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, count, ...)                                         \
    count                                                                                                 \

#define J_COUNT(...) \
    J_IDENTITY(J_EVALUATE_COUNT(__VA_ARGS__, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21,    \
    20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1))         \

        namespace
        {

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

#define MAP_HELPER(count) MAP##count 
#define MAP_MERGE_HELPER(count) MAP_MERGE_##count 

#define CALL_DEFINE(funcMacro, mapHelper, ...)   J_IDENTITY(J_APPLY(mapHelper, J_COUNT(__VA_ARGS__))(funcMacro, __VA_ARGS__)) 
#define CALL_DEFINE_EX(funcMacro, mapHelper, paramMacro, ...)   J_IDENTITY(J_APPLY(mapHelper, J_COUNT(__VA_ARGS__))(funcMacro, paramMacro(__VA_ARGS__)))

        }

#define J_MERGE_NAME(...) J_IDENTITY(CALL_DEFINE(J_IDENTITY, MAP_MERGE_HELPER, __VA_ARGS__))
#define J_STRINGIZE(...) J_IDENTITY(CALL_DEFINE(J_TO_STR, MAP_HELPER, __VA_ARGS__))
#define J_STRINGIZE_ADD_COMMA(...) J_IDENTITY(CALL_DEFINE(J_TO_STR_ADD_COMMA, MAP_HELPER, __VA_ARGS__))
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
		typeName Get##methodName()const noexcept			    \
		{												\
			return dataName;							\
		}												\
														\
		void Set##methodName(typeName newData)noexcept\
		{												\
			dataName = newData;							\
		}                                               \

    }
}