#include <algorithm>
#include <iostream>
#include <ostream>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#ifndef I2C_DISPLAY
#include <u8g2/u8g2.h>
#else
#include <libu8g2arm/u8g2.h>
#include <libu8g2arm/u8g2arm.h>
#endif
#include "input.h"
#include "menu.h"
#include "i18n.h"
#include "i18ntools.h"
#include "fonts.h"
#include <cstdio>
#include <cstring>

constexpr int TITLE_HEIGHT = 16;
constexpr int PAGE_SIZE = 3;
constexpr int ITEM_Y[PAGE_SIZE] = {16, 32, 48};
constexpr int SCREEN_WIDTH = 128;
constexpr int SCREEN_HEIGHT = 64;
constexpr int ITEM_HEIGHT = (SCREEN_HEIGHT - TITLE_HEIGHT) / PAGE_SIZE;
constexpr int SCROLL_SPEED = 80;
time_t current_time = time(nullptr);
bool forceDisplay = false;
bool screen_clear = false;
bool redraw = true;
int draw_flag = 0; 
int bye_flag = 0;
int menu_system_running = 1;
std::string message_string = "";
char fontNameU8G2[255]="";
void update_viewport(Menu *menu);
// 菜单系统类
class MenuSystem {
private:
    Menu* current_menu;
    u8g2_t* display;
    bool need_redraw;
    std::chrono::steady_clock::time_point last_input_time;
    std::atomic<bool> running;
    
public:
    void initialize(Menu* menu, u8g2_t* disp) {
        current_menu = menu;
        display = disp;
        need_redraw = true;
        running = true;
        last_input_time = std::chrono::steady_clock::now();
        current_time = time(nullptr);
    }
    
    // 非阻塞输入检测
    bool poll_input() {
      #ifndef KEYPAD_INPUT
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_input_time).count();
        
        // 输入冷却，防止过快处理
        if (elapsed < 50) { // 50ms 冷却
            return false;
        }
        
        int k = u8g_sdl_get_key();
        if (k < 0) {
            return false;
        }
        
        last_input_time = now;
        process_input(k);
        return true;
      #else
        process_input(0);
        return true;
      #endif
    }
    
    void process_input(int key) {
        InputValue c = InputValue::Unknown;
        #ifndef KEYPAD_INPUT
        if (key == 'w') c = InputValue::Up;
        else if (key == 's') c = InputValue::Down;
        else if (key == 'f') c = InputValue::Enter;
        else if (key == 'g') c = InputValue::LongEnter;
        else if (key == 'q') { 
            bye_flag = 1;
            running = false;
            return; 
        }
        #else
        c=input_get();
        #endif
        if (c != InputValue::Unknown) {
            if(draw_flag == 1){ // 1: display str,press any key to continue.
              draw_flag = 0;
              return;
            }
            if(draw_flag == 0){
              handle_navigation(c);
              need_redraw = true;
            }
            return;
        }
    }
    
    void handle_navigation(InputValue input) {
        size_t item_count = current_menu->items->size();
        int ret = 0;
        switch (input) {
        case InputValue::Up:
            if (current_menu->curr_index == 0) {
                current_menu->curr_index = item_count - 1;
            } else {
                current_menu->curr_index--;
            }
            update_viewport(current_menu);
            reset_scroll_state();
            break;
            
        case InputValue::Down:
            current_menu->curr_index = (current_menu->curr_index + 1) % item_count;
            update_viewport(current_menu);
            reset_scroll_state();
            break;
            
        case InputValue::Enter:
            ret = execute_menu_action();
            printf("handle_navigation->enter:ret=%d\n",ret);
            if (ret < 0) {
                bye_flag = 0;
                running = false;
            }
            break;
        case InputValue::LongEnter:
            ret = execute_menu_long_action();
            printf("handle_navigation->longenter:ret=%d\n",ret);
            break;
        }
    }
    
    void reset_scroll_state() {
        current_menu->active_scroll = false;
        current_menu->active_x = 0;
        current_time = time(nullptr);
    }
    
    int execute_menu_action() {
        MenuItem curr = current_menu->items->at(current_menu->curr_index);
        if (curr.action) {
            int result = curr.action(curr.action_arg);
            need_redraw = true;
            return result;
        }
        return -1;
    }
    
    int execute_menu_long_action() {
        MenuItem curr = current_menu->items->at(current_menu->curr_index);
        if (curr.long_action) {
          int result = curr.long_action(curr.action_arg);
          need_redraw = true;
          return result;
        }else{
          show_message("Function Disabled.",1);
          return 0;
        }
        return -1;
    }

    // 更新逻辑（滚动等）
    void update_logic() {
        time_t now = time(nullptr);
        if (now - current_time > 5 && !current_menu->active_scroll && current_menu->active_need_scroll) {
            current_menu->active_scroll = true;
            need_redraw = true;
        }
        
        // 更新滚动位置
        if (current_menu->active_scroll && current_menu->active_need_scroll) {
            current_menu->active_x = (current_menu->active_x + 1) % 200; // 简单的滚动位置更新
            need_redraw = true;
        }
    }
    
    // 渲染
    void render() {
      if(draw_flag==0){
        if (!need_redraw) return;
      }  
        u8g2_FirstPage(display);
        do {
          if(draw_flag == 0){  
            menu_draw(current_menu, display);
          }else if (draw_flag == 1 || draw_flag == 2){
            message_draw(display);
          }
        } while (u8g2_NextPage(display));
      if(draw_flag==0){  
        need_redraw = false;
      }
    }
    void goodbye(int bye) {
      bye_flag = bye;
      running=false;
    }
    bool is_running() const {
        return running;
    }
};

// 保留原有的辅助函数
void menu_clear_off_on(bool clear)
{
  if (clear)
  {
    screen_clear = true;
    redraw = true;
  }
  forceDisplay = !clear;
}

void menu_init(Menu *menu, std::vector<MenuItem> *items)
{
  menu->items = items;
  menu->view_start_index = 0;
  menu->curr_index = 0;
  menu->page_size = PAGE_SIZE;
  menu->active_x = 0;
  menu->active_need_scroll = false;
  menu->active_scroll = false;
}

void update_viewport(Menu *menu)
{
  if (menu->curr_index < menu->view_start_index)
  {
    menu->view_start_index = menu->curr_index;
  }
  if (menu->curr_index >= menu->view_start_index + menu->page_size)
  {
    menu->view_start_index =
        menu->curr_index + 1 - std::min(menu->curr_index, menu->page_size);
  }
}
void font_select(const char* fontName,u8g2_t *display){
  // 全局字体管理器实例
  FontManager::getInstance().setFont(display,fontName);
}
void menu_font_select(MenuItem item,u8g2_t *display){
  if(item.fontName.length()==0){
    font_select("",display);
  }else{
    font_select(item.fontName.c_str(),display);
  }
}

// 改进的 menu_draw 函数，解决撕裂问题
void menu_draw(Menu *menu, u8g2_t *display)
{
  size_t start = menu->view_start_index;
  size_t item_count = menu->items->size();

  if (start >= item_count)
  {
    start = item_count - std::min(item_count, menu->page_size);
  }

  size_t end = start + menu->page_size;
  if (end >= item_count)
  {
    end = item_count;
  }

  size_t draw_count = end - start;

  u8g2_ClearBuffer(display);

  // 先绘制所有非活动项
  for (size_t i = 0; i < draw_count; i++)
  {
    size_t index = start + i;
    MenuItem menu_item = menu->items->at(index);
    menu_font_select(menu_item,display);
    if (index != menu->curr_index)
    {
      u8g2_SetDrawColor(display, 1);
      u8g2_DrawUTF8(display, 2, ITEM_Y[i] + 12, _(menu_item.name.c_str()));
    }
  }

  // 最后绘制活动项，避免重叠
  for (size_t i = 0; i < draw_count; i++)
  {
    size_t index = start + i;
    MenuItem menu_item = menu->items->at(index);
    menu_font_select(menu_item,display);
    if (index == menu->curr_index)
    {
      // 绘制背景高亮
      u8g2_SetDrawColor(display, 1);
      u8g2_DrawBox(display, 0, ITEM_Y[i], SCREEN_WIDTH, ITEM_HEIGHT);
      u8g2_SetDrawColor(display, 0);

      int name_width = u8g2_GetUTF8Width(display,menu_item.name.c_str());
      menu->active_need_scroll = (name_width > SCREEN_WIDTH - 4);
      
      if (menu->active_need_scroll && menu->active_scroll)
      {
          int name_width = u8g2_GetUTF8Width(display, menu_item.name.c_str());
          int scroll_width = name_width + 16; // 减少间隔
          
          menu->scroll_counter++;

          // 更新滚动位置
          if (menu->scroll_counter >= SCROLL_SPEED){
            menu->active_x = (menu->active_x + 1) % scroll_width;
            menu->scroll_counter = 0;
          }
          // 计算三个副本的位置
          int base_x = 2 - menu->active_x;
          
          // 始终绘制三个副本，确保无缝
          for (int j = 0; j < 3; j++)
          {
              int x_pos = base_x + (scroll_width * j);
              
              // 只绘制在屏幕范围内的副本
              if (x_pos + name_width > 0 && x_pos < SCREEN_WIDTH)
              {
                  u8g2_DrawUTF8(display, x_pos, ITEM_Y[i] + 12, 
                              _(menu_item.name.c_str()));
              }
          }
      }
      else
      {
        // 不滚动时正常显示
        u8g2_DrawUTF8(display, 2, ITEM_Y[i] + 12, _(menu_item.name.c_str()));
      }
    }
  }

  // 绘制标题栏
  if (TITLE_HEIGHT > 0)
  {
    u8g2_SetDrawColor(display, 1);
    if(containsChinese(menu->title))
      font_select(I18N::getInstance().getFontName("zh-cn").c_str(),display);
    else
      font_select("",display);
    u8g2_DrawUTF8(display, 2, 12, _(menu->title.c_str()));
    u8g2_DrawLine(display, 0, TITLE_HEIGHT - 1, SCREEN_WIDTH, TITLE_HEIGHT - 1);
  }

  // 绘制滚动条
  if (item_count > PAGE_SIZE)
  {
    u8g2_SetDrawColor(display, 0);
    u8g2_DrawBox(display, SCREEN_WIDTH - 4, TITLE_HEIGHT, 4, SCREEN_HEIGHT);
    u8g2_SetDrawColor(display, 1);
    float scrollbar_block_height = (float)(SCREEN_HEIGHT - TITLE_HEIGHT) / item_count;
    u8g2_DrawBox(display, SCREEN_WIDTH - 4 + 2,
                 TITLE_HEIGHT + scrollbar_block_height * (menu->curr_index), 2,
                 scrollbar_block_height);
  }

  u8g2_SendBuffer(display);
}

// 消息绘制函数
void show_message(std::string string,int draw_flag_value){
  message_string=string;
  draw_flag = draw_flag_value;
}
void message_draw(u8g2_t* display){
  FontManager::getInstance().setFont(display,I18N::getInstance().getFontName(I18N::getInstance().getCurrentLanguage()).c_str());
  u8g2_DrawUTF8(display, 2, 28, _(message_string.c_str()));
}
void message_draw_standalone(u8g2_t* display,std::string string){
  FontManager::getInstance().setFont(display,I18N::getInstance().getFontName(I18N::getInstance().getCurrentLanguage()).c_str());
  u8g2_FirstPage(display);
  do {
    u8g2_DrawUTF8(display, 2, 28, _(string.c_str()));
  } while (u8g2_NextPage(display));
}
// 新的 menu_run 函数，使用 MenuSystem 类
int menu_run(Menu *menu, u8g2_t *display)
{
    MenuSystem menu_system;
    menu_system.initialize(menu, display);
    
    while (menu_system.is_running() && menu_system_running == 1) {
        auto frame_start = std::chrono::steady_clock::now();
        
        // 1. 处理输入（非阻塞）
        menu_system.poll_input();
        
        // 2. 更新逻辑
        menu_system.update_logic();
        
        // 3. 渲染
        menu_system.render();
        
        // 4. 帧率控制
        auto frame_end = std::chrono::steady_clock::now();
        auto frame_time = std::chrono::duration_cast<std::chrono::milliseconds>(frame_end - frame_start);
        
        if (frame_time < std::chrono::milliseconds(100)) { // 60fps
            std::this_thread::sleep_for(std::chrono::milliseconds(100) - frame_time);
        }
    }
    printf("menu_run done\n");
    if(bye_flag!=1)
      return 0;
    FontManager::getInstance().setFont(display,I18N::getInstance().getFontName(I18N::getInstance().getCurrentLanguage()).c_str());
    #ifndef I2C_DISPLAY
    u8g2_FirstPage(display);
    do {
      u8g2_DrawUTF8(display, 0, 28, _("Please Wait..."));
    } while (u8g2_NextPage(display));
    sleep(1);
    #else
    u8g2_ClearBuffer(display);
    u8g2_SetDrawColor(display, 1);
    u8g2_DrawUTF8(display, 0, 28, _("Please Wait..."));
    u8g2_SendBuffer(display);
    system("reboot");
    sleep(1);
    #endif
    return 0;
}

void menu_off(int bye){
  //menu_system.goodbye(bye);
  menu_system_running = 0;
  bye_flag = bye;
}