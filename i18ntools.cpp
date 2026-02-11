#include <iostream>
#include <string>
// 判断字符是否为中文字符（UTF-8）
bool isChineseCharUTF8(const std::string& str, size_t pos) {
    if (pos >= str.length()) return false;
    
    unsigned char c = static_cast<unsigned char>(str[pos]);
    
    // 中文字符在UTF-8中占3个字节，首字节范围是0xE4-0xE9
    if ((c & 0xF0) == 0xE0) {
        // 确保有足够的字节
        if (pos + 2 < str.length()) {
            return true;
        }
    }
    return false;
}

// 判断字符是否为ASCII字符（英文）
bool isASCIIChar(char c) {
    return static_cast<unsigned char>(c) <= 127;
}

// 检查是否包含中文字符
bool containsChinese(const std::string& str) {
    for (size_t i = 0; i < str.length(); ) {
        unsigned char c = static_cast<unsigned char>(str[i]);
        
        // UTF-8中文字符的标志
        if (c >= 0xE4 && c <= 0xE9 && i + 2 < str.length()) {
            // 检查后续字节是否在正确范围
            unsigned char c2 = static_cast<unsigned char>(str[i + 1]);
            unsigned char c3 = static_cast<unsigned char>(str[i + 2]);
            
            if (c2 >= 0x80 && c2 <= 0xBF && c3 >= 0x80 && c3 <= 0xBF) {
                return true;
            }
        }
        i++;
    }
    return false;
}