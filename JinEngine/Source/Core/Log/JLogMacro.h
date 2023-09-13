#pragma once
#include"JLogHandler.h"

#define J_LOG_PRINT_OUT(title, body) JLogPrintOut::PrintOut(__FILE__, __LINE__, title, body)

