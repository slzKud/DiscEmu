#include <algorithm>
#include <any>
#include <cstdarg>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <thread>
#include <future>
#ifndef I2C_DISPLAY
#include <u8g2/u8g2.h>
#else
#include <libu8g2arm/U8g2Controller.h>
#include <libu8g2arm/U8g2lib.h>
#include <libu8g2arm/u8g2.h>
#include <libu8g2arm/u8g2arm.h>
#include <libu8g2arm/u8x8.h>
#endif
#include "ver.h"
#include "fonts.h"
#include "input.h"
#include "menu.h"
#include "i18n.h"
#include "i18ntools.h"
#include "config_manager.h"
#include "util.h"
#include <boost/filesystem.hpp>
u8g2_t u8g2 = {};
ConfigManager configManager("config.json");
namespace fs = boost::filesystem;
fs::path iso_root = fs::path("isos");
int action_do_nothing(std::any arg);
int action_errmsg(std::any arg);
int action_settings(std::any arg);
int action_genrnal_settings(std::any arg);
int action_storage(std::any arg);
int action_display_lang(std::any arg);
int action_status(std::any arg);
int action_file_browser(std::any arg);
int action_file_mamager(std::any arg);
int action_disk_emu(std::any arg);
int action_usb_rndis(std::any arg);
int action_mtp(std::any arg);
int action_file_delete_confirm(std::any arg);
int action_file_delete(std::any arg);
int action_create_image_select(std::any arg);
int action_shutdown(std::any arg);
int action_reset(std::any arg);
void display_fonts();

std::vector<MenuItem> main_menu = {
    MenuItem{.name = "Image Browser",
             .action = action_file_browser,
             .action_arg = iso_root},
    MenuItem{.name = "File Transfer", .action = action_mtp},
    MenuItem{.name = "USB RNDIS", .action = action_usb_rndis},
    MenuItem{.name = "Settings", .action = action_settings},
    MenuItem{.name = "Shutdown", .action = action_shutdown},
    MenuItem{.name = "Restart", .action = action_reset}
};

std::vector<ImageSizesList> imageSizeList = {
  ImageSizesList{"512 MB",512.0 * 1024.0 * 1024.0},
  ImageSizesList{"1 GB",1024.0 * 1024.0 * 1024.0},
  ImageSizesList{"2 GB",2.0 * 1024.0 * 1024.0 * 1024.0},
};

// 通用Prompt

int action_prompt(std::any arg) {
    try {
        PromptParams params = std::any_cast<PromptParams>(arg);
        
        Menu prompt_menu { .title = params.title };
        std::vector<MenuItem> menu_items;
        
        // 返回项
        if(params.need_back==1){
          menu_items.push_back(MenuItem{
              .name = "Back",
              .action = [](std::any) { return -1; }
          });
        }
        // 添加选项
        for (size_t i = 0; i < params.options.size(); ++i) {
            std::string name = params.options[i];
            if (static_cast<int>(i) == params.active_index) {
                name = "*" + name;
            }
            
            menu_items.push_back(MenuItem{
                .name = name,
                .action = [params, i](std::any) { 
                    // 调用该选项的回调，传入索引和额外数据
                    return params.actions[i](static_cast<int>(i), params.extra_data);
                }
            });
        }
        
        menu_init(&prompt_menu, &menu_items);
        menu_run(&prompt_menu, &u8g2);
        return 0;
    } catch (const std::bad_any_cast&) {
        return -1;
    }
}

int action_errmsg(std::any arg) {
  std::string msg = std::any_cast<std::string>(arg);
  show_message(msg,1);
  return 0;
}

int action_do_nothing(std::any arg) { return 0; }

int action_main_menu(std::any arg) {
  Menu menu { .title = "DiscEmu" };
  menu_init(&menu, &main_menu);
  menu_run(&menu, &u8g2);
  return 0;
}

int action_settings(std::any arg) {
  Menu status_menu { .title = "Settings" };
  std::vector<MenuItem> status_menu_items = {  
      MenuItem{.name = "Back"},
      MenuItem{.name = "Genrnal",.action = action_genrnal_settings},    
      MenuItem{.name = "Storage",.action = action_storage},
      MenuItem{.name = "Language",.action = action_display_lang},
      MenuItem{.name = "Version",.action = action_status}
  };
  menu_init(&status_menu, &status_menu_items);
  menu_run(&status_menu, &u8g2);
  return 0;
}
int action_read_only_set(std::any arg){
  PromptParams params;
  AppConfig& config = configManager.getConfig();
  params.title = "Read-only Settings";
  params.options = {"On", "Off"};
  params.active_index = (configManager.getConfig().readOnlyMode)?0:1;
  params.need_back = 1;
  params.actions = {
        [](int index,std::any arg) { 
            std::cout << "Read-only ON" << std::endl;
            AppConfig& config = configManager.getConfig();
            config.readOnlyMode = true;
            if(configManager.saveConfig(config)){
              show_message("Config Saved.",1);
            }else{
              show_message("Config not effected.",1);
            }
            #ifdef I2C_DISPLAY
            system("sync");
            #endif
            return -1;
        },
        [](int index,std::any arg) { 
            std::cout << "Read-only OFF" << std::endl;
            AppConfig& config = configManager.getConfig();
            config.readOnlyMode = false;
            if(configManager.saveConfig(config)){
              show_message("Config Saved.",1);
            }else{
              show_message("Config not effected.",1);
            }
            #ifdef I2C_DISPLAY
            system("sync");
            #endif
            return -1;
        }
    };
  return action_prompt(params);
}
int action_floppy_mode(std::any arg){
  PromptParams params;
  AppConfig& config = configManager.getConfig();
  params.title = "Floppy Mode";
  params.options = {"On", "Off"};
  params.need_back = 1;
  params.active_index = (configManager.getConfig().floppySupport)?0:1;
  params.actions = {
        [](int index,std::any arg) { 
            std::cout << "Floppy ON" << std::endl;
            AppConfig& config = configManager.getConfig();
            config.floppySupport = true;
            if(configManager.saveConfig(config)){
              show_message("Config Saved.",1);
            }else{
              show_message("Config not effected.",1);
            }
            #ifdef I2C_DISPLAY
            system("sync");
            #endif
            return -1;
        },
        [](int index, std::any arg) { 
            std::cout << "Floppy OFF" << std::endl;
            AppConfig& config = configManager.getConfig();
            config.floppySupport = false;
            if(configManager.saveConfig(config)){
              show_message("Config Saved.",1);
            }else{
              show_message("Config not effected.",1);
            }
            #ifdef I2C_DISPLAY
            system("sync");
            #endif
            return -1;
        }
    };
  return action_prompt(params);
}
int action_screen_rotation(std::any arg){
  PromptParams params;
  AppConfig& config = configManager.getConfig();
  params.title = "Screen Rotation";
  params.options = {"0", "180"};
  params.need_back = 1;
  if(configManager.getConfig().screenRotation==0){
    params.active_index = 0;
  }else if(configManager.getConfig().screenRotation==1){
    params.active_index = 1;
  }
  params.actions = {
        [](int index, std::any arg) { 
            std::cout << "Screen Rotation 0" << std::endl;
            AppConfig& config = configManager.getConfig();
            config.screenRotation = 0;
            if(configManager.saveConfig(config)){
              show_message("Config Saved.",1);
            }else{
              show_message("Config not effected.",1);
            }
            #ifdef I2C_DISPLAY
            system("sync");
            #endif
            return -1;
        },
        [](int index, std::any arg) { 
            std::cout << "Screen Rotation 180" << std::endl;
            AppConfig& config = configManager.getConfig();
            config.screenRotation = 1;
            if(configManager.saveConfig(config)){
              show_message("Config Saved.",1);
            }else{
              show_message("Config not effected.",1);
            }
            #ifdef I2C_DISPLAY
            system("sync");
            #endif
            return -1;
        }
    };
  return action_prompt(params);
}
int action_genrnal_settings(std::any arg) {
  Menu status_menu { .title = "Genrnal" };
  std::vector<MenuItem> status_menu_items = { 
      MenuItem{.name = "Back"},   
      MenuItem{.name = _("Screen Rotation"),.action = action_screen_rotation},    
      MenuItem{.name = _("Read-only Settings"),.action = action_read_only_set},
      MenuItem{.name = _("Floppy Mode"),.action = action_floppy_mode}
  };
  menu_init(&status_menu, &status_menu_items);
  menu_run(&status_menu, &u8g2);
  return 0;
}

int action_storage(std::any arg) {
  DiskSpaceInfo disk_space;
  bool ret;
  #ifdef I2C_DISPLAY
  ret=getDiskSpace("/mnt/sdcard",disk_space);
  #else
  ret=getDiskSpace("./isos",disk_space);
  #endif
  if(!ret){
    show_message("Error.",1);
    return 0;
  }
  Menu status_menu { .title = "Storage" };
  std::vector<MenuItem> status_menu_items = {      
      MenuItem{.name = _f("Total: %sGB",roundNumber(disk_space.totalGB,2).c_str())},
      MenuItem{.name = _f("Free: %sGB",roundNumber(disk_space.freeGB,2).c_str())},
      MenuItem{.name = _("Format Drive")}
  };
  menu_init(&status_menu, &status_menu_items);
  menu_run(&status_menu, &u8g2);
  return 0;
}

int action_display_lang(std::any arg) {
  AppConfig& config = configManager.getConfig();
  if(arg.has_value()){
    auto langid = std::any_cast<const char *>(arg);
    if(langid!=nullptr){
      printf("user select lang:%s\n",langid);
      config.langID = langid;
      configManager.saveConfig(config);
      #ifdef I2C_DISPLAY
      system("sync");
      #endif
      show_message("Language changed.",1);
      I18N::getInstance().setLanguage(langid);
      return -1;
    }
  }
  Menu status_menu { .title = "Language" };
  auto langList=I18N::getInstance().getSupportedLanguages();
  std::vector<MenuItem> status_menu_items = {  
      MenuItem{.name = "Back"},    
  };
  for(int i=0;i<langList.size();i++){
    //printf("support lang:%d,%s,%s\n",i,langList[i],I18N::getInstance().getLanguageName(langList[i],true).c_str());
    status_menu_items.push_back(MenuItem{
      .name = std::move(I18N::getInstance().getLanguageName(langList[i],false)),
      .fontName = std::move(I18N::getInstance().getFontName(langList[i])),
      .action = action_display_lang,
      .action_arg = langList[i]
    });
  }
  menu_init(&status_menu, &status_menu_items);
  menu_run(&status_menu, &u8g2);
  return 0;
}

int action_status(std::any arg) {
  Menu status_menu { .title = "Version" };
  std::vector<MenuItem> status_menu_items = {
      MenuItem{.name = _f("%s",DEVICE_TYPE)},      
      MenuItem{.name = _f("Version: %s",DISCEMU_VER)},
      MenuItem{.name = _f("Build Date: %s",BUILD_DATE)}
  };
  menu_init(&status_menu, &status_menu_items);
  menu_run(&status_menu, &u8g2);
  return 0;
}
int action_create_image(std::any arg){
  auto create_aborted = std::any_cast<bool>(arg);
  if(create_aborted){
    show_message(_("Create Aborted."),1);
    return action_main_menu(NULL);
  }
  /*
  show_message("",2); //屏蔽输入
  for (int i = 0; i < 5; ++i) {
            std::cout << "Wait " << i+1 << "s\n";
            message_draw_standalone(&u8g2,_f("Wait %ds...",i+1));
            std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  */
  show_message(_("Create Success."),1);
  return action_main_menu(NULL);
}
int action_disk_image(int index,std::any arg){
  auto path = std::any_cast<fs::path>(arg);
  std::cout << "user select " << imageSizeList[index].friendlyName << std::endl;
  std::string image_size = imageSizeList[index].friendlyName;
  std::string name=getDiskImageFilename("./"+path.string(),image_size);
  Menu dir_menu { .title = "Confirm image info" };
  std::vector<MenuItem> dir_menu_items;
  dir_menu_items.push_back(
      MenuItem{ .name = _f("%s",name.c_str())});
  dir_menu_items.push_back(
      MenuItem{.name = "No",
                    .action = action_create_image,
                   .action_arg = true});
  dir_menu_items.push_back(
      MenuItem{.name = "Yes",
                    .action = action_create_image,
                    .action_arg = false});
  menu_init(&dir_menu, &dir_menu_items);
  menu_run(&dir_menu, &u8g2);
  return 0;
}

int action_create_image_select(std::any arg) {
  auto path = std::any_cast<fs::path>(arg);
  DiskSpaceInfo disk_space;
  bool ret;
  #ifdef I2C_DISPLAY
  ret=getDiskSpace("/mnt/sdcard",disk_space);
  #else
  ret=getDiskSpace("./isos",disk_space);
  #endif
  if(!ret){
    show_message("Error.",1);
    return 0;
  }
  PromptParams params;
  AppConfig& config = configManager.getConfig();
  params.title = "Choose Disk Size";
  params.need_back = 1;
  params.extra_data = arg;
  for(int i=0;i<imageSizeList.size();i++){
    if(disk_space.freeBytes<imageSizeList[i].usedBytes)
      continue;
    params.options.push_back(imageSizeList[i].friendlyName);
    params.actions.push_back([](int index,std::any arg) { 
      return action_disk_image(index,arg);
    });
  }
  return action_prompt(params);
}
int action_file_delete(std::any arg) {
  auto path = std::any_cast<fs::path>(arg);
  bool result = false;
  bool file_or_folder=fs::is_directory(path);
  message_draw_standalone(&u8g2,_f("Deleting..."));
  if(!fs::is_directory(path)){
    if(fs::remove(path)){
      result=true;
    }
  }else{
    if(fs::remove_all(path)>0){
      result=true;
    }
  }
  if(result){
    show_message(_f(file_or_folder?"Delete folder OK.":"Delete file OK."),1);
  }else{
    show_message(_f("Delete Failed."),1);
  }
  if(result){
    return action_main_menu(NULL);
  }else{
    return -1;
  }
}
int action_file_delete_confirm(std::any arg) {
  auto path = std::any_cast<fs::path>(arg);
  Menu dir_menu { .title = fs::is_directory(path)?"Delele folder?":"Delete File?" };
  std::vector<MenuItem> dir_menu_items;
  dir_menu_items.push_back(
      MenuItem{ .name = path.filename().string()});
  dir_menu_items.push_back(
      MenuItem{.name = "Yes",
                    .action = action_file_delete,
                    .action_arg = path});
  dir_menu_items.push_back(
      MenuItem{.name = "No",
                    .action = nullptr});
  menu_init(&dir_menu, &dir_menu_items);
  menu_run(&dir_menu, &u8g2);
  return 0;
}
int action_file_mamager(std::any arg){
  auto path = std::any_cast<fs::path>(arg);
  Menu dir_menu { .title = fs::is_directory(path)?"Folder action":"File action" };
  std::vector<MenuItem> dir_menu_items;
  dir_menu_items.push_back(
      MenuItem{ .name = "Back",
                .action = nullptr});
  dir_menu_items.push_back(
      MenuItem{.name = "Delete",
               .action = action_file_delete_confirm,
               .action_arg = path});
  dir_menu_items.push_back(
      MenuItem{.name = "Create disk image",
               .action = action_create_image_select,
               .action_arg = fs::is_directory(path)?path:path.parent_path()});
  menu_init(&dir_menu, &dir_menu_items);
  menu_run(&dir_menu, &u8g2);
  return 0;
}
int action_file_browser(std::any arg) {
  auto path = std::any_cast<fs::path>(arg);
  if (!fs::exists(path) || !fs::is_directory(path)) {
    action_errmsg(std::string("Failed to open dir."));
    return 0;
  }

  std::vector<MenuItem> dir_menu_items;
  std::vector<MenuItem> iso_menu_items;
  Menu dir_menu { .title = path.filename().string() };

  dir_menu_items.push_back(MenuItem{.name = "[..]", .action = nullptr});
  for (const auto &entry : fs::directory_iterator(path)) {
    std::string fontName ="";
    fs::path entry_path = entry.path();
    if(containsChinese(entry_path.filename().string())){
      fontName=I18N::getInstance().getFontName("zh-cn");
    }
    if (fs::is_directory(entry)) {
      dir_menu_items.push_back(
          MenuItem{.name = '[' + entry_path.filename().string() + ']',
                   .fontName = fontName,
                   .action = action_file_browser,
                   .long_action = action_file_mamager,
                   .action_arg = entry.path()});
    } else {
      std::string ext = entry_path.extension().string();
      std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
      if (ext == ".iso" || ext == ".img" || ext == ".flp") {
        iso_menu_items.push_back(MenuItem{
          .name = entry_path.filename().string(),
          .fontName = fontName,
          .action = action_disk_emu,
          .long_action = action_file_mamager,
          .action_arg = entry.path()});
      }
    }
  }

  std::sort(dir_menu_items.begin(), dir_menu_items.end(),
            [](MenuItem a, MenuItem b) { return a.name < b.name; });
  std::sort(iso_menu_items.begin(), iso_menu_items.end(),
            [](MenuItem a, MenuItem b) { return a.name < b.name; });

  for (auto const &items : iso_menu_items) {
    dir_menu_items.push_back(items);
  }
  menu_init(&dir_menu, &dir_menu_items);
  menu_run(&dir_menu, &u8g2);
  return 0;
}

int action_usb_rndis(std::any arg){
  /*
  show_message("",2); //屏蔽输入
  for (int i = 0; i < 5; ++i) {
            std::cout << "Wait " << i+1 << "s\n";
            message_draw_standalone(&u8g2,_f("Wait %ds...",i+1));
            std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  show_message("",0); //开启输入
  */
  // 等待结束后创建菜单
  Menu rndis_menu { .title = "USB RNDIS" };
  std::vector<MenuItem> rndis_menu_items = {      
        MenuItem{.name = _f("IP: %s","172.32.0.70"), .action = action_do_nothing},      
        MenuItem{.name = "Exit",
                .action = [](std::any) { return -1; }},
    };
  menu_init(&rndis_menu, &rndis_menu_items);
  return menu_run(&rndis_menu, &u8g2);
}
int action_disk_emu(std::any arg) {
  auto path = std::any_cast<fs::path>(arg);
  std::string ext = path.extension().string();
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
  Menu emu_menu { .title = "Current image" };
  std::vector<MenuItem> emu_menu_items = {      
      MenuItem{.name = path.filename().c_str(), .action = action_do_nothing},
      MenuItem{.name = "Eject",
               .action =
                  [](std::any) {
                    return -1;
                  }},
      MenuItem{.name = "", .action = action_do_nothing},
  };
  menu_init(&emu_menu, &emu_menu_items);
  menu_run(&emu_menu, &u8g2);
  return 0;
}
int action_mtp(std::any arg) {
  Menu mtp_menu { .title = "File Transfer" };
  std::vector<MenuItem> mtp_menu_items = {      
      MenuItem{.name = "Now you can transfer file via USB.", .action = action_do_nothing},      
      MenuItem{.name = "Exit",
               .action =
                   [](std::any) {
                     return -1;
                   }},
  };
  menu_init(&mtp_menu, &mtp_menu_items);
  menu_run(&mtp_menu, &u8g2);
  return 0;
}
int action_shutdown(std::any arg){
  show_message("",2);
  message_draw_standalone(&u8g2,_f("Good bye."));
  sleep(2);
  #ifdef I2C_DISPLAY
  system("halt");
  #else
  menu_off(0);
  #endif
  return 0;
}

int action_reset(std::any arg){
  menu_off(1);
  return 0;
}
void runonce(){
	action_main_menu(NULL);
	return;
}
void display_fonts(){
  // 初始化每个语言默认使用的字体
  FontManager::getInstance().setFont(&u8g2,I18N::getInstance().getFontName(I18N::getInstance().getCurrentLanguage()).c_str());
}
int main(void) {
  AppConfig& config = configManager.getConfig();
  I18N::getInstance().initialize();
  FontManager::getInstance().initializeFontMap();
  I18N::getInstance().setLanguage(config.langID.c_str());
  std::cout << "current lang:" << I18N::getInstance().getCurrentLanguage() << std::endl;
  #ifndef I2C_DISPLAY
  std::cout << "Press [w] to up, [d] to down,[f] to select,[g] to select sub menu." << std::endl;
  if(config.screenRotation==0){
	  u8g2_SetupBuffer_SDL_128x64_4(&u8g2, &u8g2_cb_r0);
  }else{
    u8g2_SetupBuffer_SDL_128x64_4(&u8g2, &u8g2_cb_r2);
  }
  #else
  int success;
  success = input_init();
  if (!success) {
    std::cout << "failed to initialize input" << std::endl;
    return 1;
  }
  u8x8_t *p_u8x8 = u8g2_GetU8x8(&u8g2);
  if(config.screenRotation==0){
    u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2, U8G2_R0, u8x8_byte_arm_linux_hw_i2c,
                                      u8x8_arm_linux_gpio_and_delay);
  }else{
    u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2, U8G2_R2, u8x8_byte_arm_linux_hw_i2c,
                                      u8x8_arm_linux_gpio_and_delay);
  }

  u8x8_SetPin(p_u8x8, U8X8_PIN_I2C_CLOCK, U8X8_PIN_NONE);
  u8x8_SetPin(p_u8x8, U8X8_PIN_I2C_DATA, U8X8_PIN_NONE);
  u8x8_SetPin(p_u8x8, U8X8_PIN_RESET, U8X8_PIN_NONE);

  success = u8g2arm_arm_init_hw_i2c(p_u8x8, 3); // I2C 3
  if (!success) {
    std::cout << "failed to initialize display" << std::endl;
    return 1;
  }
  #endif
	u8x8_InitDisplay(u8g2_GetU8x8(&u8g2));
	u8x8_SetPowerSave(u8g2_GetU8x8(&u8g2), 0);  
	u8g2_ClearBuffer(&u8g2);
  display_fonts();
	runonce();
  #ifdef I2C_DISPLAY
  u8g2_ClearDisplay(&u8g2);
  #endif
	return 0;
}