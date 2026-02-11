#ifndef MENU_H
#define MENU_H

#include <string>
#include <vector>
#include <any>
#include <functional>
#include <unistd.h>
#ifndef I2C_DISPLAY
#include <u8g2/u8g2.h>
#else
#include <libu8g2arm/u8g2.h>
#endif

struct PromptParams {
    std::string title;
    std::vector<std::string> options;
    std::vector<std::function<int(int,std::any)>> actions;
    int active_index = -1;
    int need_back = 0;
    std::any extra_data;
};

typedef struct {
    std::string name;
    std::string fontName = "";
    std::function<int(std::any)> action;
    std::function<int(std::any)> long_action; //长按处理
    std::any action_arg;
} MenuItem;

typedef struct {
    std::vector<MenuItem>* items;
    std::string title;
    size_t curr_index;
    size_t view_start_index;
    size_t page_size;
    bool active_scroll;
    bool active_need_scroll;
    int active_x;
    int scroll_counter;
} Menu;
void menu_clear_off_on(bool clear);
void menu_init(Menu* menu, std::vector<MenuItem>* items);
void menu_draw(Menu* menu, u8g2_t* display);
void menu_off(int bye);
void message_draw(u8g2_t* display);
void message_draw_standalone(u8g2_t* display,std::string string);
void show_message(std::string string,int draw_flag_value);
void font_select(const char* fontName,u8g2_t *display);
int menu_run(Menu* menu, u8g2_t* display);
#endif