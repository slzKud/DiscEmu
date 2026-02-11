#ifndef I18N_H
#define I18N_H

#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <sstream>
#include <cstdarg>
#include <cstdio>

// 默认语言定义，可以在编译时通过-DDEFAULT_LANG=xxx覆盖
#ifndef DEFAULT_LANG
#define DEFAULT_LANG "en"
#endif

// 本地化字符串条目
struct LocalizedString {
    const char* original;   // 原始字符串
    const char* translated; // 翻译后的字符串
};

// 字体设置
struct FontConfig {
    std::string fontName;
};

// 语言数据
struct LanguageData {
    const char* id;           // 语言ID，如"zh-cn", "en"
    const char* name;         // 语言名称
    const char* engName;      // 用于非本地语言界面的语言名称
    const char* fontName;
    const LocalizedString* strings; // 字符串数组
    size_t count;             // 字符串数量
};

// I18N类
class I18N {
public:
    // 获取单例实例
    static I18N& getInstance();
    
    // 初始化语言系统
    // 如果useFile为true，则从文件加载，否则使用编译时的数据
    void initialize(bool useFile = false, const char* filename = nullptr);
    
    // 查找翻译 - 返回C字符串
    const char* translate(const char* key) const;
    
    // 查找翻译 - 返回std::string
    std::string translateStr(const std::string& key) const;
    
    // 格式化翻译 - 支持可变参数
    std::string translateFormat(const char* key, ...) const;
    
    // 格式化翻译 - 使用va_list
    std::string translateFormatV(const char* key, va_list args) const;
    
    // 设置当前语言
    bool setLanguage(const char* langId);
    
    // 获取当前语言ID
    const char* getCurrentLanguage() const;
    
    // 获取支持的语言列表
    std::vector<const char*> getSupportedLanguages() const;
    
    // 获取语言名称
    std::string getLanguageName(const char* langId,const bool engFlag) const;

    // 获取字体名称
    std::string getFontName(const char* langId) const;
    
    // 添加语言数据（运行时添加）
    void addLanguageData(const LanguageData& data);
    
    // 从文件加载语言数据
    bool loadFromFile(const char* filename);
    
    // 保存语言数据到文件
    bool saveToFile(const char* filename) const;
    
private:
    I18N();
    ~I18N();
    I18N(const I18N&) = delete;
    I18N& operator=(const I18N&) = delete;
    
    // 语言字符串映射类型
    using StringMap = std::unordered_map<std::string, std::string>;
    
    // 语言信息结构
    struct LanguageInfo {
        std::string name;
        std::string engName;
        FontConfig font;
        StringMap strings;
    };
    
    // 编译时的语言数据
    void loadBuiltinData();
    
    // 内部格式化函数
    std::string internalFormat(const std::string& format, va_list args) const;
    
    std::unordered_map<std::string, LanguageInfo> languages_;
    std::string currentLang_;
    bool initialized_;
};

// 便捷宏定义

// 简单翻译宏（返回const char*）
#define _(key) I18N::getInstance().translate(key)

// 字符串翻译宏（返回std::string）
#define _s(key) I18N::getInstance().translateStr(key)

// 格式化翻译宏（支持可变参数）
#define _f(key, ...) I18N::getInstance().translateFormat(key, ##__VA_ARGS__)

// 命名空间别名（可选）
namespace i18n {
    inline const char* tr(const char* key) {
        return I18N::getInstance().translate(key);
    }
    
    inline std::string trStr(const std::string& key) {
        return I18N::getInstance().translateStr(key);
    }
    
    template<typename... Args>
    inline std::string trFormat(const char* key, Args&&... args) {
        return I18N::getInstance().translateFormat(key, std::forward<Args>(args)...);
    }
}

#endif // I18N_H