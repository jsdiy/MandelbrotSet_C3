//マンデルブロー集合
//『昼夜逆転』工作室	@jsdiy	https://github.com/jsdiy
//	2025/10

#include <Arduino.h>
#include <sstream>
#include "MandelbrotSet.hpp"

volatile	bool	reqDrawStop;

//初期化
void	MandelbrotSet::Initialize(LcdST77xx* lcd)
{
	using	namespace	MandelbrotSetConfig;
	this->lcd = lcd;

	//c=a+biの事前計算用バッファ
	aBuf = (float*)heap_caps_malloc(sizeof(float) * lcd->Width(), MALLOC_CAP_32BIT);
	bBuf = (float*)heap_caps_malloc(sizeof(float) * lcd->Height(), MALLOC_CAP_32BIT);

	if (FocusCA == 0xFF || FocusCB == 0xFF)
	{
		centerX = (MinX + MaxX) / 2.0f;
		centerY = (MinY + MaxY) / 2.0f;
	}
	else
	{
		centerX = FocusCA;
		centerY = FocusCB;
	}

	level = Level;
	SetComplexPlane(centerX, centerY, level);
}

//LCDへ描画する
void	MandelbrotSet::Draw(void)
{
	using	namespace	MandelbrotSetConfig;
	Color color;
	size_t bufLength;
	uint8_t* lineBuffer = lcd->GetLineBuffer(&bufLength);

	auto msStart = millis();
	reqDrawStop = false;
	for (int16_t y = 0; y < lcd->Height(); y++)
	{
		float b = bBuf[y];	//c=a+biのb
		for (int16_t x = 0; x < lcd->Width(); x++)
		{
			//マンデルブロー集合の計算
			float a = aBuf[x];	//c=a+biのa
			int16_t iter = Iterate(a, b);

			//反復回数を色に変換する
			ToRGB565(iter, &color);
			memcpy(&lineBuffer[x * Color::Length], color.Bytes, Color::Length);
		}

		lcd->DrawImage(0, y, lcd->Width(), 1, lineBuffer, bufLength);
		if (reqDrawStop) { break; }
	}

	Serial.printf("msec=%lu\n", millis() - msStart);
}

//マンデルブロー集合を反復計算する
int16_t	MandelbrotSet::Iterate(float a, float b)
{
	// マンデルブロー集合の計算
	float x = 0.0f, y = 0.0f;	//z[0] = 0
	int16_t iter;
	for (iter = 0; iter < IterMax; iter++)
	{
		float x2 = x * x;
		float y2 = y * y;
		if (x2 + y2 > 4.0f) { break; }	//発散
		y = 2 * x * y + b;	// z[n+1]の虚部	|こちらの式を下の式より先に計算する必要がある。
		x = x2 - y2 + a;	// z[n+1]の実部	|左辺xは新たなx[n+1]を表すが、上の式のxはx[n]であるので。
	}

	return iter;
}

//反復回数を色に変換する
//・8bit値のビット列で0を00へ、1を11へ置換する。0b11001010 --> 0b1111000011001100
void	MandelbrotSet::ToRGB565(int16_t iterCount, Color* color)
{
	uint16_t colorBits = 0x0000;
	if (iterCount < IterMax)
	{
		for (uint8_t wpos = 0, rpos = 0; rpos < 8; rpos++, wpos += 2)
		{
			if (iterCount & (1 << rpos)) { colorBits |= (0b11 << wpos); }
		}
	}

	color->Bytes[0] = (colorBits >> 8) & 0xFF;
	color->Bytes[1] = colorBits & 0xFF;
}

//計算の中心位置を指定する
void	MandelbrotSet::SetFocus(int16_t screenX, int16_t screenY)
{
	centerX = MappingToRe(screenX);
	centerX = std::min(2.0f, centerX);
	centerX = std::max(-2.0f, centerX);

	centerY = MappingToIm(screenY);
	centerY = std::min(2.0f, centerY);
	centerY = std::max(-2.0f, centerY);

	SetComplexPlane(centerX, centerY, level);
}

//画面座標を複素平面へマッピングする（c=a+bi の a）
float	MandelbrotSet::MappingToRe(int16_t screenX)
{
	return minX + (maxX - minX) * screenX / (lcd->Width() - 1);
}

//画面座標を複素平面へマッピングする（c=a+bi の b）
float	MandelbrotSet::MappingToIm(int16_t screenY)
{
	//Y軸とIm軸の向きが逆であることを考慮する
	return maxY - (maxY - minY) * screenY / (lcd->Height() - 1);
}

//拡大
void	MandelbrotSet::ZoomIn(void)
{
	level++;
	SetComplexPlane(centerX, centerY, level);
}

//縮小
void	MandelbrotSet::ZoomOut(void)
{
	if (0 < level)
	{
		level--;
		SetComplexPlane(centerX, centerY, level);
	}
}

//計算対象となる集合領域を決める
//・拡大／縮小は計算誤差を減らすため、現在の階層から次の階層を計算するのではなく、初期値を起点にして計算する。
void	MandelbrotSet::SetComplexPlane(float centerX, float centerY, uint8_t level)
{
	using	namespace	MandelbrotSetConfig;
	float div = std::pow(Zoom, level);
	float initLength, newLength;

	initLength = (MaxX - MinX);
	newLength = initLength / div;
	minX = centerX - newLength / 2.0f;
	maxX = centerX + newLength / 2.0f;

	initLength = (MaxY - MinY);
	newLength = initLength / div;
	minY = centerY - newLength / 2.0f;
	maxY = centerY + newLength / 2.0f;

	//集合領域が決まったので、画面座標に対応するc=a+biのa,bを事前に計算しておく
	for (int16_t x = 0; x < lcd->Width(); x++) { aBuf[x] = MappingToRe(x); }
	for (int16_t y = 0; y < lcd->Height(); y++) { bBuf[y] = MappingToIm(y); }

	Serial.printf("Lv:%d C(%f,%f) Re[%f,%f] Im[%f,%f]\n", level, centerX, centerY, minX, maxX, minY, maxY);
	PrintInfo();
}

//描画情報を出力する
void	MandelbrotSet::PrintInfo(void)
{
	std::string (*fS)(float) = std::to_string;
	std::string (*iS)(int) = std::to_string;

	auto sLevel = "Lv:" + iS(level);
	auto sCenter = "C:(" + fS(centerX) + "," + fS(centerY) + ")";
	auto sRePart = "Re:[" + fS(minX) + "," + fS(maxX) + "]";
	auto sImPart = "Im:[" + fS(minY) + "," + fS(maxY) + "]";

	std::string infos[] = { sLevel, sCenter, sRePart, sImPart };
	for (uint8_t i = 0; i < 4; i++)
	{
		lcd->DrawString(lcd->PointX(0), lcd->PointY(12 + i), infos[i].c_str());
	}
}
