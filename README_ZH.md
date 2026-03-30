# DiscEmu

[镜像 SDK](https://github.com/slzKud/DiscEmu_Build_SDK) | [English](README.md)

基于 [driver1998](https://github.com/driver1998/DiscEmu) 的 CD-ROM 模拟器设备软件。与原始项目相比，本项目进行了一定程度的重构和代码修改。

目前持续开发中。

## 改进与增强
* 添加 Luckfox Pico Mini 支持
* 添加按键和 I2C OLED 支持
* 添加 SDL 界面模拟器
* 支持更改屏幕方向
* 只读模式
* 多语言支持（当前支持简体中文）

## 待办事项
* 使用 U8g2 完全替换 libu8g2arm。（目前只有 SDL 模拟器使用 U8g2。）
* ~~为源项目添加实用的模拟功能。~~
* USB 软盘模拟。
* 支持 SPI OLED 和编码器的原始 DiscEmu。（使用 Milk-V Duo）
* 微控制器支持（例如 CH32）

## 硬件信息

详见 [详细信息](hardware_luckfox.md)。

## 构建

### SDL 界面模拟器

本应用依赖：
- [U8g2](https://github.com/olikraus/u8g2)
- [boost](https://sourceforge.net/projects/boost/files/boost/)
- [SDL2](https://github.com/libsdl-org/SDL/releases/tag/release-2.32.10)

修改 makefile 中的 TOOLCHAIN_PREFIX、U8G2_PREFIX 和 BOOST_PREFIX，然后使用 ``make`` 编译程序。

### Luckfox 及其他平台

本应用依赖 [libu8g2arm](https://github.com/antiprism/libu8g2arm) 库和 [boost](https://sourceforge.net/projects/boost/files/boost/) 库。

修改 makefile 中的 TOOLCHAIN_PREFIX、U8G2_PREFIX 和 BOOST_PREFIX，然后使用 ``make DEVICE_TYPE=luckfox USB_ON=1`` 编译程序。

如果需要禁用 USB 进行调试，使用 ``make DEVICE_TYPE=luckfox USB_ON=0`` 编译程序。

## 运行镜像

### SDL 界面模拟器
![原型](asset/sdl_emu.png)
### Luckfox
![原型](asset/discemu_on_luckfox_pico.jpg)