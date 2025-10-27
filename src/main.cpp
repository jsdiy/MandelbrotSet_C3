//マンデルブロ集合
//『昼夜逆転』工作室	@jsdiy	https://github.com/jsdiy
//	2025/10

/*	ピンアサイン
			ESP32-C3
SPI-SS		GPIO_NUM_10
SPI-MOSI	GPIO_NUM_7
SPI-MISO	GPIO_NUM_2	未使用
SPI-SCK		GPIO_NUM_6
LCD-DC		GPIO_NUM_4
LCD-RESET	GPIO_NUM_NC
SW-A		GPIO_NUM_1
SW-B		GPIO_NUM_3
*/

#include <Arduino.h>
#include "MandelbrotSet.hpp"
#include "LcdST77xx.hpp"
#include "HwSwitch.hpp"

enum	class	EOpMode
{
	DrawingReady,
	Drawing,
	ZoomingReady,
	Zooming,
	PointingReady,
	Pointing,
};

//オブジェクト
MandelbrotSet	mandelbrot;
LcdST77xx	lcd;
HwSwitch	swA, swB;
HwSwitch	&swV = swA, &swH = swB;	//Vertical, Horizon
HwSwitch	&swZmIn = swA, &swZmOut = swB;	//Zoom-in/out

//色
static	Color	foreColor = Color(0xFF, 0xFF, 0xFF);
static	Color	bgColor = Color(0x00, 0x00, 0x00);
static	Color	colBreak = Color(0xFF, 0xFF, 0x00);

//定数
static	constexpr	gpio_num_t	GpioSwA = GPIO_NUM_1;
static	constexpr	gpio_num_t	GpioSwB = GPIO_NUM_3;
static	constexpr	int8_t	GridSize = 8;	//カーソル移動格子の大きさ

//変数
static	volatile	EOpMode	opMode;	//OperationMode
static	volatile	bool	isDrawBreak;
static	int16_t	cursorX, cursorY;	//画面座標
static	bool	isCursorMove;

//関数
static	void	DrawFrame(Color& color);
static	void	CenterCursor(void);
static	void	DrawCursor(Color& color);
static	void	MoveCursorH(void);
static	void	MoveCursorV(void);

//割り込み
void	IRAM_ATTR	OnButton(void)
{
	if (opMode != EOpMode::Drawing) { return; }
	isDrawBreak = true;
	mandelbrot.StopDrawing();
}

void	setup(void)
{
	Serial.begin(115200);
	delay(1000);

	swA.Initialize(GpioSwA);
	swB.Initialize(GpioSwB);
	attachInterrupt(GpioSwA, OnButton, RISING);
	attachInterrupt(GpioSwB, OnButton, RISING);

	lcd.Initialize();
	lcd.RotateFlip(ERotFlip::Rot90 | ERotFlip::XFlip);
	lcd.ClearScreen(bgColor);

	mandelbrot.Initialize(&lcd);

	opMode = EOpMode::DrawingReady;
}

void	loop(void)
{
	swA.State();
	swB.State();

	switch (opMode)
	{
	case	EOpMode::DrawingReady:
		if (swA.IsOff() && swB.IsOff())
		{
			opMode = EOpMode::Drawing;
		}
		break;

	case	EOpMode::Drawing:
		isDrawBreak = false;
		mandelbrot.Draw();
		if (isDrawBreak) { DrawFrame(colBreak); }
		opMode = EOpMode::ZoomingReady;
		break;

	case	EOpMode::ZoomingReady:
		if (swA.IsOff() && swB.IsOff()) { opMode = EOpMode::Zooming; }
		break;

	case	EOpMode::Zooming:
		if (swZmIn.Clicked()) { mandelbrot.ZoomIn();	opMode = EOpMode::DrawingReady; }
		if (swZmOut.Clicked()) { mandelbrot.ZoomOut();	opMode = EOpMode::DrawingReady; }
		if (swZmIn.IsOn() && swZmOut.IsOn())	//同時押し
		{
			DrawFrame(foreColor);
			opMode = EOpMode::PointingReady;
		}
		break;

	case	EOpMode::PointingReady:
		if (swA.IsOff() && swB.IsOff())
		{
			CenterCursor();
			DrawCursor(foreColor);
			isCursorMove = false;
			opMode = EOpMode::Pointing;
		}
		break;

	case	EOpMode::Pointing:
		if (swH.Clicked()) { MoveCursorH();	isCursorMove = true; }
		if (swV.Clicked()) { MoveCursorV();	isCursorMove = true; }
		if (swH.IsOn() && swV.IsOn())	//同時押し
		{
			if (isCursorMove) { mandelbrot.SetFocus(cursorX, cursorY); }
			opMode = EOpMode::DrawingReady;
		}
		break;
	}
}

//枠線を描く
static	void	DrawFrame(Color& color)
{
	uint8_t bold = 2;
	lcd.FillRect(0, 0, bold, lcd.Height(), color);	//左縦
	lcd.FillRect(lcd.Width() - bold, 0, bold, lcd.Height(), color);	//右縦
	lcd.FillRect(0, 0, lcd.Width(), bold, color);	//上横
	lcd.FillRect(0, lcd.Height() - bold, lcd.Width(), bold, color);	//下横
}

//カーソル座標を画面中央位置にセットする
static	void	CenterCursor(void)
{
	cursorX = lcd.Width() / 2;
	cursorY = lcd.Height() / 2;
}

//カーソルを描く
static	void	DrawCursor(Color& color)
{
	int8_t bold = 2;
	lcd.FillRect(cursorX, cursorY, bold, bold, color);
}

//カーソルを水平移動する
static	void	MoveCursorH(void)
{
	DrawCursor(bgColor);
	cursorX += GridSize;
	if (lcd.Width() - GridSize < cursorX) { cursorX = GridSize; }
	DrawCursor(foreColor);
}

//カーソルを垂直移動する
static	void	MoveCursorV(void)
{
	DrawCursor(bgColor);
	cursorY += GridSize;
	if (lcd.Height() - GridSize < cursorY) { cursorY = GridSize; }
	DrawCursor(foreColor);
}
