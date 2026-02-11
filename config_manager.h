#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H
#include "cJSON/cJSON.h"
#include <string>

struct AppConfig {
    int screenRotation = 0;
    bool mscOnStart = false;
    bool readOnlyMode = false;
    bool floppySupport = false;
    std::string langID = "zh-cn";
    
    // 默认构造函数
    AppConfig() = default;
    
    // 带参数的构造函数
    AppConfig(int rotation, bool msc, bool readOnly, bool floppy, const std::string& lang)
        : screenRotation(rotation), mscOnStart(msc), 
          readOnlyMode(readOnly), floppySupport(floppy), langID(lang) {}
};

class ConfigManager {
public:
    // 构造函数，传入配置文件名
    ConfigManager(const std::string& configFile = "config.json");
    
    // 读取配置
    bool loadConfig(AppConfig& config);
    
    // 保存配置
    bool saveConfig(const AppConfig& config);
    
    // 获取配置对象引用
    AppConfig& getConfig() { return currentConfig; }
    
    // 设置配置并保存
    bool updateConfig(const AppConfig& newConfig);
    
private:
    std::string configFilePath;
    AppConfig currentConfig;
    
    // 内部辅助方法
    bool createDefaultConfig();
    bool parseConfig(cJSON* root, AppConfig& config);
    cJSON* createJsonFromConfig(const AppConfig& config);
};

#endif // CONFIG_MANAGER_H