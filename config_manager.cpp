#include "config_manager.h"
#include "cJSON/cJSON.h"
#include <fstream>
#include <sstream>
#include <iostream>
#ifndef DEFAULT_LANG
#define DEFAULT_LANG "en"
#endif
#ifndef SCREEN_ROTATE
#define SCREEN_ROTATE 0
#endif
ConfigManager::ConfigManager(const std::string& configFile) 
    : configFilePath(configFile) {
    // 尝试加载配置，如果失败则使用默认配置
    if (!loadConfig(currentConfig)) {
        std::cerr << "Failed to load config, using defaults." << std::endl;
        // 可以在这里保存默认配置
        createDefaultConfig();
    }
}

bool ConfigManager::loadConfig(AppConfig& config) {
    // 打开配置文件
    std::ifstream file(configFilePath);
    if (!file.is_open()) {
        std::cerr << "Cannot open config file: " << configFilePath << std::endl;
        return false;
    }
    
    // 读取文件内容
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string jsonStr = buffer.str();
    file.close();
    
    if (jsonStr.empty()) {
        std::cerr << "Config file is empty." << std::endl;
        return false;
    }
    
    // 解析JSON
    cJSON* root = cJSON_Parse(jsonStr.c_str());
    if (!root) {
        const char* error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != nullptr) {
            std::cerr << "JSON parse error: " << error_ptr << std::endl;
        }
        return false;
    }
    
    // 解析配置项
    bool success = parseConfig(root, config);
    
    // 清理cJSON对象
    cJSON_Delete(root);
    
    return success;
}

bool ConfigManager::parseConfig(cJSON* root, AppConfig& config) {
    // 解析 screenRotation
    cJSON* rotationItem = cJSON_GetObjectItem(root, "screenRotation");
    if (cJSON_IsNumber(rotationItem)) {
        config.screenRotation = rotationItem->valueint;
    }
    
    // 解析 mscOnStart
    cJSON* mscItem = cJSON_GetObjectItem(root, "mscOnStart");
    if (cJSON_IsBool(mscItem)) {
        config.mscOnStart = cJSON_IsTrue(mscItem);
    }
    
    // 解析 readOnlyMode
    cJSON* readOnlyItem = cJSON_GetObjectItem(root, "readOnlyMode");
    if (cJSON_IsBool(readOnlyItem)) {
        config.readOnlyMode = cJSON_IsTrue(readOnlyItem);
    }
    
    // 解析 floppySupport
    cJSON* floppyItem = cJSON_GetObjectItem(root, "floppySupport");
    if (cJSON_IsBool(floppyItem)) {
        config.floppySupport = cJSON_IsTrue(floppyItem);
    }
    
    // 解析 langID
    cJSON* langItem = cJSON_GetObjectItem(root, "langID");
    if (cJSON_IsString(langItem) && (langItem->valuestring != nullptr)) {
        config.langID = langItem->valuestring;
    }
    
    return true;
}

bool ConfigManager::saveConfig(const AppConfig& config) {
    // 创建JSON对象
    cJSON* root = createJsonFromConfig(config);
    if (!root) {
        return false;
    }
    
    // 格式化为字符串
    char* jsonStr = cJSON_Print(root);
    if (!jsonStr) {
        cJSON_Delete(root);
        return false;
    }
    
    // 写入文件
    std::ofstream file(configFilePath);
    if (!file.is_open()) {
        free(jsonStr);
        cJSON_Delete(root);
        return false;
    }
    
    file << jsonStr;
    file.close();
    
    // 清理
    free(jsonStr);
    cJSON_Delete(root);
    
    // 更新当前配置
    currentConfig = config;
    
    std::cout << "Config saved successfully." << std::endl;
    return true;
}

cJSON* ConfigManager::createJsonFromConfig(const AppConfig& config) {
    cJSON* root = cJSON_CreateObject();
    if (!root) {
        return nullptr;
    }
    
    // 添加 screenRotation
    cJSON_AddNumberToObject(root, "screenRotation", config.screenRotation);
    
    // 添加 mscOnStart
    cJSON_AddBoolToObject(root, "mscOnStart", config.mscOnStart);
    
    // 添加 readOnlyMode
    cJSON_AddBoolToObject(root, "readOnlyMode", config.readOnlyMode);
    
    // 添加 floppySupport
    cJSON_AddBoolToObject(root, "floppySupport", config.floppySupport);
    
    // 添加 langID
    cJSON_AddStringToObject(root, "langID", config.langID.c_str());
    
    return root;
}

bool ConfigManager::createDefaultConfig() {
    AppConfig defaultConfig(SCREEN_ROTATE, false, false, false, DEFAULT_LANG);
    return saveConfig(defaultConfig);
}

bool ConfigManager::updateConfig(const AppConfig& newConfig) {
    return saveConfig(newConfig);
}