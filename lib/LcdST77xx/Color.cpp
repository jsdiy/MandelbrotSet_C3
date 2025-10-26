//色データクラス
//『昼夜逆転』工作室	@jsdiy	https://github.com/jsdiy
//	2024/04 - 2025/08

#include <Arduino.h>
#include "Color.hpp"

/*
//色を作る
Color	Color::CreateRGB565(uint8_t red, uint8_t green, uint8_t blue)
{
	Color color;
	color.SetRGB565(red, green, blue);
	return color;
}
*/

//色をセットする
void	Color::SetRGB565(uint8_t red, uint8_t green, uint8_t blue)
{
	red		= red   * 0x1F / 0xFF;
	green	= green * 0x3F / 0xFF;
	blue	= blue  * 0x1F / 0xFF;
	
	//[R5:G3][G3:B5]
	Bytes[0] = (red   << 3) | (green >> 3);
	Bytes[1] = (green << 5) | blue;
}
