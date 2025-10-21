//マンデルブロー集合
//『昼夜逆転』工作室	@jsdiy	https://github.com/jsdiy
//	2025/10

#pragma once

#include <Arduino.h>
#include "LcdST77xx.hpp"

//集合領域の設定
namespace	MandelbrotSetConfig
{
	//複素平面の範囲
	//・x,y(=Re,Im)とも閉区間[-2.0, 2.0]で指定する。
	//・画面の縦横比と同じにすると見た目がよくなる。
	constexpr	float
		MinX = -2.0f,	MaxX = 1.5f,	//実部(Re)の範囲
		MinY = -1.3f,	MaxY = 1.3f;	//虚部(Im)の範囲

	//中心座標（c=a+bi の a(Re)とb(Im)）
	//・a,bとも閉区間[-2.0, 2.0]で指定する。例: FocusCA = 0.334958f, FocusCB = -0.046732f
	//・「複素平面の範囲」の中心から開始する場合は0xFFを指定する。
	constexpr	float
		FocusCA = 0xFF,
		FocusCB = 0xFF;

	//拡大レベル（初期値は0）
	//・「中心座標」を指定する場合、拡大レベルも指定したい場合がある。例：Lv:8 C(Re:0.113132f,Im:0.644894f)
	//・Zoom=4で「複素平面の範囲」がfloat型の場合、レベル10以上(4^10倍に拡大)は実数精度が追従できず、描画が粗くなる。
	constexpr	uint8_t	Level = 0;
}

class	MandelbrotSet
{
private:
	static	constexpr	int16_t	IterMax = 256;	// 最大反復回数
	static	constexpr	uint8_t	Zoom = 4;	//1回の操作で何倍に拡大するか

private:
	LcdST77xx*	lcd;
	float	minX, minY, maxX, maxY;
	float	centerX, centerY;
	uint8_t	level;
	float	*aBuf, *bBuf;
	volatile	bool	reqDrawStop;
	int16_t	Iterate(float a, float b);
	void	ToRGB565(int16_t iterCount, Color* color);
	void	SetComplexPlane(float centerX, float centerY, uint8_t level);
	float	MappingToRe(int16_t screenX);
	float	MappingToIm(int16_t screenY);

public:
	MandelbrotSet(void) { lcd = nullptr; }
	void	Initialize(LcdST77xx* lcd);
	void	Draw(void);
	void	StopDrawing(void) { reqDrawStop = true; }
	void	SetFocus(int16_t screenX, int16_t screenY);
	void	ZoomIn(void);
	void	ZoomOut(void);
};

/*	マンデルブロー集合
式:
	Z[n+1] = Z[n]^2 + c
ただし、
	Z[n] = x[n] + y[n]i
	c = a + bi	※複素平面上の任意の点
	Z[0] = 0 (x = y = 0)
また、
	発散条件: |Z[n]| > 2

描画:
	式を反復計算し、発散するか、一定回数を超えたら反復を終了する。

計算の過程:
Z[n+1] = Z[n]^2 + c
= (x[n] + y[n]i)^2 + (a + bi)
= (x[n]^2 − y[n]^2 + 2x[n]y[n]i) + (a + bi)
= (x[n]^2 − y[n]^2 + a) + (2x[n]y[n] + b)i
== x[n+1] + y[n+1]i	※恒等的に等しい
よって、
x[n+1] = x[n]^2 − y[n]^2 + a,
y[n+1] = 2x[n]y[n] + b

発散の判断:
|Z[n]| > 2
--> Z[n]^2 > 2^2
--> x[n]^2 + y[n]^2 > 4
よって、反復を繰り返す中で↑この式を満たした時点で発散が確定する。
*/
