#include "color.hpp"

namespace xen {
Color const Color::Clear(0x00000000, Type::RGBA);
Color const Color::Black(0x000000FF, Type::RGBA);
Color const Color::Grey(0x808080);
Color const Color::Silver(0xC0C0C0);
Color const Color::White(0xFFFFFF);
Color const Color::Maroon(0x800000);
Color const Color::Red(0xFF0000);
Color const Color::Olive(0x808000);
Color const Color::Yellow(0xFFFF00);
Color const Color::Green(0x00FF00);
Color const Color::Lime(0x008000);
Color const Color::Teal(0x008080);
Color const Color::Aqua(0x00FFFF);
Color const Color::Navy(0x000080);
Color const Color::Blue(0x0000FF);
Color const Color::Purple(0x800080);
Color const Color::Fuchsia(0xFF00FF);
}