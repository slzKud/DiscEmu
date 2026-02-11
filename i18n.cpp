#include "i18n.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cstring>
#include "i18nstr.h"

I18N::I18N() : currentLang_(DEFAULT_LANG), initialized_(false) {
}

I18N::~I18N() {
}

I18N& I18N::getInstance() {
    static I18N instance;
    return instance;
}

void I18N::initialize(bool useFile, const char* filename) {
    if (initialized_) return;
    
    if (useFile && filename) {
        loadFromFile(filename);
    } else {
        loadBuiltinData();
    }
    
    initialized_ = true;
    
    // 如果当前语言不在支持的语言列表中，尝试使用默认语言
    if (languages_.find(currentLang_) == languages_.end()) {
        if (!languages_.empty()) {
            currentLang_ = languages_.begin()->first;
        }
    }
}

void I18N::loadBuiltinData() {
    for (size_t i = 0; i < BuiltinLanguages::language_count; ++i) {
        const LanguageData& data = BuiltinLanguages::languages[i];
        
        LanguageInfo info;
        info.name = data.name;
        info.engName = data.engName;
        info.font.fontName = data.fontName;
        // 添加字符串映射
        for (size_t j = 0; j < data.count; ++j) {
            info.strings[data.strings[j].original] = data.strings[j].translated;
        }
        
        languages_[data.id] = info;
    }
}

const char* I18N::translate(const char* key) const {
    if (!key || !initialized_) return key;
    
    auto langIt = languages_.find(currentLang_);
    if (langIt == languages_.end()) {
        return key; // 当前语言不存在
    }
    
    auto strIt = langIt->second.strings.find(key);
    if (strIt == langIt->second.strings.end()) {
        return key; // 没有找到翻译
    }
    
    return strIt->second.c_str();
}

std::string I18N::translateStr(const std::string& key) const {
    if (!initialized_) return key;
    
    auto langIt = languages_.find(currentLang_);
    if (langIt == languages_.end()) {
        return key; // 当前语言不存在
    }
    
    auto strIt = langIt->second.strings.find(key);
    if (strIt == langIt->second.strings.end()) {
        return key; // 没有找到翻译
    }
    
    return strIt->second;
}

std::string I18N::translateFormat(const char* key, ...) const {
    // 先获取翻译后的格式字符串
    const char* format = translate(key);
    
    // 如果没有翻译或格式字符串就是key本身，且不包含格式说明符，直接返回
    if (format == key && strchr(key, '%') == nullptr) {
        return std::string(key);
    }
    
    // 使用可变参数进行格式化
    va_list args;
    va_start(args, key);
    std::string result = translateFormatV(key, args);
    va_end(args);
    
    return result;
}

std::string I18N::translateFormatV(const char* key, va_list args) const {
    // 获取翻译后的格式字符串
    const char* format = translate(key);
    
    // 如果没有翻译或格式字符串就是key本身，且不包含格式说明符，直接返回
    if (format == key && strchr(key, '%') == nullptr) {
        return std::string(key);
    }
    
    return internalFormat(format, args);
}

std::string I18N::internalFormat(const std::string& format, va_list args) const {
    // 复制va_list，因为vsnprintf会修改它
    va_list args_copy;
    va_copy(args_copy, args);
    
    // 第一次调用，获取需要的缓冲区大小
    int size = vsnprintf(nullptr, 0, format.c_str(), args_copy);
    va_end(args_copy);
    
    if (size < 0) {
        return format; // 格式化失败，返回原始格式字符串
    }
    
    // 分配缓冲区（包括null终止符）
    std::vector<char> buffer(size + 1);
    
    // 第二次调用，实际格式化
    vsnprintf(buffer.data(), buffer.size(), format.c_str(), args);
    
    return std::string(buffer.data(), size);
}

bool I18N::setLanguage(const char* langId) {
    if (!langId) return false;
    
    if (languages_.find(langId) != languages_.end()) {
        currentLang_ = langId;
        return true;
    }
    return false;
}

const char* I18N::getCurrentLanguage() const {
    return currentLang_.c_str();
}

std::vector<const char*> I18N::getSupportedLanguages() const {
    std::vector<const char*> result;
    result.reserve(languages_.size());
    
    for (const auto& pair : languages_) {
        result.push_back(pair.first.c_str());
    }
    
    return result;
}

std::string I18N::getLanguageName(const char* langId,const bool engFlag) const {
    if (!langId) return nullptr;
    
    auto it = languages_.find(langId);
    if (it != languages_.end()) {
        if(engFlag)
            return it->second.engName;
        return it->second.name;
    }
    
    return nullptr;
}

std::string I18N::getFontName(const char* langId) const {
    if (!langId) return nullptr;
    
    auto it = languages_.find(langId);
    if (it != languages_.end()) {
        return it->second.font.fontName;
    }
    
    return nullptr;
}

void I18N::addLanguageData(const LanguageData& data) {
    LanguageInfo info;
    info.name = data.name;
    info.engName = data.engName;
    info.font.fontName = data.fontName;
    // 添加字符串映射
    for (size_t i = 0; i < data.count; ++i) {
        info.strings[data.strings[i].original] = data.strings[i].translated;
    }
    
    languages_[data.id] = info;
    
    // 如果没有设置当前语言，设置为添加的第一个语言
    if (currentLang_.empty() && !languages_.empty()) {
        currentLang_ = data.id;
    }
}

bool I18N::loadFromFile(const char* filename) {
    if (!filename) return false;
    
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return false;
    }
    
    try {
        // 读取语言数量
        size_t langCount;
        file.read(reinterpret_cast<char*>(&langCount), sizeof(langCount));
        
        for (size_t i = 0; i < langCount; ++i) {
            // 读取语言ID和名称长度
            size_t idLen, nameLen;
            file.read(reinterpret_cast<char*>(&idLen), sizeof(idLen));
            std::string langId(idLen, '\0');
            file.read(&langId[0], idLen);
            
            file.read(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));
            std::string langName(nameLen, '\0');
            file.read(&langName[0], nameLen);
            
            // 读取字符串数量
            size_t strCount;
            file.read(reinterpret_cast<char*>(&strCount), sizeof(strCount));
            
            LanguageInfo info;
            info.name = langName;
            
            for (size_t j = 0; j < strCount; ++j) {
                // 读取键值对
                size_t keyLen, valueLen;
                file.read(reinterpret_cast<char*>(&keyLen), sizeof(keyLen));
                std::string key(keyLen, '\0');
                file.read(&key[0], keyLen);
                
                file.read(reinterpret_cast<char*>(&valueLen), sizeof(valueLen));
                std::string value(valueLen, '\0');
                file.read(&value[0], valueLen);
                
                info.strings[key] = value;
            }
            
            languages_[langId] = info;
        }
        
        file.close();
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error loading language file: " << e.what() << std::endl;
        file.close();
        return false;
    }
}

bool I18N::saveToFile(const char* filename) const {
    if (!filename) return false;
    
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to create file: " << filename << std::endl;
        return false;
    }
    
    try {
        // 写入语言数量
        size_t langCount = languages_.size();
        file.write(reinterpret_cast<const char*>(&langCount), sizeof(langCount));
        
        for (const auto& langPair : languages_) {
            const std::string& langId = langPair.first;
            const LanguageInfo& info = langPair.second;
            
            // 写入语言ID
            size_t idLen = langId.size();
            file.write(reinterpret_cast<const char*>(&idLen), sizeof(idLen));
            file.write(langId.c_str(), idLen);
            
            // 写入语言名称
            size_t nameLen = info.name.size();
            file.write(reinterpret_cast<const char*>(&nameLen), sizeof(nameLen));
            file.write(info.name.c_str(), nameLen);
            
            // 写入字符串数量
            size_t strCount = info.strings.size();
            file.write(reinterpret_cast<const char*>(&strCount), sizeof(strCount));
            
            // 写入所有字符串
            for (const auto& strPair : info.strings) {
                const std::string& key = strPair.first;
                const std::string& value = strPair.second;
                
                size_t keyLen = key.size();
                file.write(reinterpret_cast<const char*>(&keyLen), sizeof(keyLen));
                file.write(key.c_str(), keyLen);
                
                size_t valueLen = value.size();
                file.write(reinterpret_cast<const char*>(&valueLen), sizeof(valueLen));
                file.write(value.c_str(), valueLen);
            }
        }
        
        file.close();
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error saving language file: " << e.what() << std::endl;
        file.close();
        return false;
    }
}