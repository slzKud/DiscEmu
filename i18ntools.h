#include <stdbool.h>
#include <string>
#ifndef __I18N_TOOLS_H__
bool isASCIIChar(char c);
bool isChineseCharUTF8(const std::string& str, size_t pos);
bool containsChinese(const std::string& str);
#define __I18N_TOOLS_H__
#endif