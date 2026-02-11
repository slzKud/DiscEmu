#include <unordered_map>
#include <string>
#include <cstring>
#include "i18n.h"
#ifndef I2C_DISPLAY
#include <u8g2/u8g2.h>
#else
#include <libu8g2arm/U8g2Controller.h>
#include <libu8g2arm/U8g2lib.h>
#include <libu8g2arm/u8g2.h>
#include <libu8g2arm/u8g2arm.h>
#include <libu8g2arm/u8x8.h>
#include <libu8g2arm/u8g2_fonts_gplcopyleft.h>
#endif
// 在你的代码基础上添加字体映射功能
class FontManager {
private:
    std::unordered_map<std::string, const uint8_t*> u8g2FontMap;
    char currentFontName[64];
    
    // 私有构造函数
    FontManager() {
        currentFontName[0] = '\0';
        initializeFontMap();
    }
    
public:
    // 删除拷贝构造函数和赋值运算符
    FontManager(const FontManager&) = delete;
    FontManager& operator=(const FontManager&) = delete;
    
    // 获取单例实例
    static FontManager& getInstance() {
        static FontManager instance;
        return instance;
    }
    
    void initializeFontMap() {
        // 初始化字体映射
        u8g2FontMap["u8g2_font_6x13_tf"] = u8g2_font_6x13_tf;
        #ifndef LUCKFOX_NO_GB2312
        u8g2FontMap["u8g2_font_wqy13_t_gb2312"] = u8g2_font_wqy13_t_gb2312;
        #endif
        u8g2FontMap["u8g2_font_wqy13_t_chinese3"] = u8g2_font_wqy13_t_chinese3;
        // 添加更多字体...
    }
    
    void setFont(u8g2_t* u8g2, const char* fontName) {
        // 如果当前字体不为空且与目标字体相同，直接返回
        if (currentFontName[0] != '\0' && strcmp(currentFontName, fontName) == 0) {
            return;
        }
        
        const uint8_t* fontPtr = u8g2_font_6x13_tf; // 默认字体
        
        if (fontName != nullptr && fontName[0] != '\0') {
            // 查找对应的U8g2字体
            std::string fontStr(fontName);
            auto it = u8g2FontMap.find(fontStr);
            if (it != u8g2FontMap.end()) {
                fontPtr = it->second;
                strcpy(currentFontName, fontName);
                //printf("font_select:%s\n", currentFontName);
            } else {
                // 使用默认字体
                std::string normalFontName = I18N::getInstance().getFontName(
                    I18N::getInstance().getCurrentLanguage()
                );
                strcpy(currentFontName, normalFontName.c_str());
                
                // 尝试查找默认字体
                auto defaultIt = u8g2FontMap.find(normalFontName);
                if (defaultIt != u8g2FontMap.end()) {
                    fontPtr = defaultIt->second;
                }
            }
        } else {
            // 使用默认字体
            std::string normalFontName = I18N::getInstance().getFontName(
                I18N::getInstance().getCurrentLanguage()
            );
            strcpy(currentFontName, normalFontName.c_str());
            
            auto it = u8g2FontMap.find(normalFontName);
            if (it != u8g2FontMap.end()) {
                fontPtr = it->second;
            }
        }
        
        // 设置字体
        u8g2_SetFont(u8g2, fontPtr);
    }
    
    // 使用单例
    // FontManager::getInstance().setFont(&u8g2, "u8g2_font_6x13_tf");
};
