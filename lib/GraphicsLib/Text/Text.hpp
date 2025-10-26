//テキスト描画（LCDクラスの拡張クラス）
//『昼夜逆転』工作室	@jsdiy	https://github.com/jsdiy
//	2025/10

#pragma	once

#include <Arduino.h>
#include "Font.hpp"
#include "Color.hpp"

//テキスト描画
class	Text
{
private:
	Font	font;
	Color	foreColor = Color(0xA0, 0xA0, 0xA0);	//文字の色
	Color	bgColor = Color(0x00, 0x00, 0x00);	//文字の背景色
	int16_t	screenWidth, screenHeight;
	uint8_t	widthScale, heightScale;	//縦横n倍
	int16_t	scaledCharW, scaledCharH;	//スケーリングした文字の大きさ（文字の余白を含む）
	uint8_t*	imageBuffer = nullptr;	//1文字を描画する画像バッファ
	size_t	imageBufferLength;
	size_t	WriteColorToImageBuffer(const Color* color, int16_t repeat, size_t bufIndex);
	void	DrawCharToImageBuffer(const uint8_t* fontDatas);
	char	ToHexChar(uint8_t n);

public:
	virtual	~Text(void) {}
	virtual	void	DrawImage(int16_t x, int16_t y, int16_t w, int16_t h, const uint8_t* imgData, size_t dataLength) = 0;
	void	Initialize(int16_t screenWidth, int16_t screenHeight);	//LCDクラスのInitialize()で呼び出す
	void	SetTextScreen(int16_t screenWidth, int16_t screenHeight);	//LCDクラスで画面が90度回転するごとに呼び出す
	void	SetTextColor(const Color* color);	//文字の前景色
	void	SetTextBgColor(const Color* color);	//文字の背景色
	bool	SetScale(uint8_t xW, uint8_t xH, bool realloc = false);	//描画の拡大率を設定する（文字、画像）
	void	GetCharSize(int16_t* charWidth, int16_t* charHeight);	//拡大率と文字間隔を考慮した1文字の大きさ
	int16_t	PointX(uint8_t column);	//桁位置(>=0)のx座標
	int16_t	PointY(uint8_t row);	//行位置(>=0)のy座標
	int16_t	DrawChar(int16_t x, int16_t y, char cc);
	int16_t	DrawString(int16_t x, int16_t y, const char* s);
	int16_t	DrawWord(int16_t x, int16_t y, uint16_t n);
	int16_t	DrawByte(int16_t x, int16_t y, uint8_t n);
	int16_t	DrawInt(int16_t x, int16_t y, int32_t n);
	int16_t	DrawUInt(int16_t x, int16_t y, uint32_t n);
};
