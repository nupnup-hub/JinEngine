#pragma once
#include"JLogHandler.h"

#define J_LOG_PRINT_OUT(title, body) JinEngine::Core::JLogPrintOut::PrintOut(__FILE__, std::to_string(__LINE__), title, body)
#define J_LOG_PRINT_OUTW(title, body) JinEngine::Core::JLogPrintOut::PrintOut(__FILEW__, std::to_wstring(__LINE__), title, body)


#define J_LOG_PRINT_OUT_IF_FAIL(v, title, body) {bool b = v; if(!b) J_LOG_PRINT_OUT(title, body)}
#define J_LOG_PRINT_OUT_IF_FAIL_AND_RETURN(v, title, body) {bool b = v; if(!b) {J_LOG_PRINT_OUT(title, body); return;}}

