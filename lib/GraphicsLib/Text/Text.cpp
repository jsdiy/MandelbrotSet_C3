//テキスト描画
//『昼夜逆転』工作室	@jsdiy	https://github.com/jsdiy
//	2025/10

#include <Arduino.h>
#include "Text.hpp"

//初期化
void	Text::Initialize(int16_t screenWidth, int16_t screenHeight)
{
	font.Initialize();
	SetTextScreen(screenWidth, screenHeight);
	SetScale(1, 1);
}

//画面サイズをセットする
void	Text::SetTextScreen(int16_t screenWidth, int16_t screenHeight)
{
	this->screenWidth = screenWidth;
	this->screenHeight = screenHeight;
}

//文字の色（前景色）を設定する
void	Text::SetTextColor(const Color* color)
{
	foreColor = *color;
}

//文字画面の色（背景色）を設定する
void	Text::SetTextBgColor(const Color* color)
{
	bgColor = *color;
}

//描画の拡大率を設定する
//引数	realloc: スケーリング後に現在より小さいバッファで済む場合、メモリを再確保するか否か
//true: メモリを再確保する
//false: メモリを再確保せず、現在のバッファを使用する（デフォルト）
//※スケーリング後に現在より大きなバッファが必要な場合、この引数に関係なく再確保される
bool	Text::SetScale(uint8_t xW, uint8_t xH, bool realloc)
{
	widthScale = (0 < xW) ? xW : 1;
	heightScale = (0 < xH) ? xH : 1;

	scaledCharW = font.CharW() * widthScale;
	scaledCharH = font.CharH() * heightScale;

	//1文字分の描画データのバッファを確保する
	size_t newBufLength = scaledCharW * scaledCharH * Color::Length;
	if (imageBufferLength < newBufLength) { realloc = true; }
	if (imageBufferLength == newBufLength) { realloc = false; }
	// (imageBufferLength > newBufLength) { 再確保するかはreallocに従う }
	if (realloc)
	{
		if (imageBuffer != nullptr) { heap_caps_free(imageBuffer); }
		imageBuffer = (uint8_t*)heap_caps_malloc(newBufLength, MALLOC_CAP_DMA | MALLOC_CAP_32BIT);
		//Serial.printf("Text imageBuffer size: %d byte\n", newBufLength);
	}
	imageBufferLength = (imageBuffer == nullptr) ? 0 : newBufLength;

	return (imageBuffer != nullptr);
}

//スケーリングと文字間隔を考慮した1文字の大きさ
void	Text::GetCharSize(int16_t* charWidth, int16_t* charHeight)
{
	*charWidth = scaledCharW;
	*charHeight = scaledCharH;
}

//桁位置（0以上）のx座標
int16_t	Text::PointX(uint8_t column)
{
	return scaledCharW * column;
}

//行位置（0以上）のy座標
int16_t	Text::PointY(uint8_t row)
{
	return scaledCharH * row;
}

//文字を描く
//引数:	開始位置
//戻り値：	次に描く文字のx座標
int16_t	Text::DrawChar(int16_t x, int16_t y, char cc)
{
	//スケーリングを考慮した1文字が画面に収まるか
	//・縦横とも余白を含めた大きさで判定する。文字画像バッファへは余白込みで描画しているため。
	//・文字が画面の端に掛かる（文字が欠ける）場合は描かない。
	if (	(0 <= x) && (x + scaledCharW <= screenWidth)	&&
			(0 <= y) && (y + scaledCharH <= screenHeight)	)
	{
		auto fontDatas = font.GetFontData(cc);
		DrawCharToImageBuffer(fontDatas);
		DrawImage(x, y, scaledCharW, scaledCharH, imageBuffer, imageBufferLength);
	}
	return x + scaledCharW;
}

//色データを画素数分、画像バッファへ出力する
size_t	Text::WriteColorToImageBuffer(const Color* color, int16_t repeat, size_t bufIndex)
{
	while (repeat--)
	{
		imageBuffer[bufIndex++] = color->Bytes[0];
		imageBuffer[bufIndex++] = color->Bytes[1];
	}
	return bufIndex;
}

//スケーリングされた文字を画像バッファに描く（内部処理）
void	Text::DrawCharToImageBuffer(const uint8_t* fontDatas)
{
	//文字データの高さの分、繰り返す
	size_t bufIdx = 0;
	for (uint8_t idxY = 0; idxY < font.CharH(); idxY++)
	{
		uint8_t lineData = fontDatas[idxY];
		size_t startIndex = bufIdx;

		//文字データの幅の分、繰り返す
		for (uint8_t idxX = 0; idxX < font.CharW(); idxX++)
		{
			//画素の色を決める（前景色か背景色か）
			uint8_t pixel = lineData & (0x80 >> idxX);
			Color& color = (pixel != 0) ? foreColor : bgColor;

			//拡大倍数の分、1画素の色データを出力する
			bufIdx = WriteColorToImageBuffer(&color, widthScale, bufIdx);
		}

		//いま出力した1行を拡大倍数の分、画像バッファ上で2行目、3行目…へとコピーする
		size_t length = bufIdx - startIndex;	//1行分の描いたデータの長さ
		for (uint8_t lineRepeat = 0; lineRepeat < heightScale - 1; lineRepeat++)
		{
			memcpy(&imageBuffer[bufIdx], &imageBuffer[startIndex], length);
			bufIdx += length;
		}
	}
}

//文字列を描く
//引数	x,y:	開始位置
//		s:	文字列
//戻り値：	次に描く文字のx座標
int16_t Text::DrawString(int16_t x, int16_t y, const char* s)
{
	while (x + scaledCharW < screenWidth)
	{
		char cc = *(s++);
		if (cc == '\0') { break; }
		x = DrawChar(x, y, cc);
	}
	return x;
}

//WORD値を表示する
//戻り値：	次に描く文字のx座標
int16_t	Text::DrawWord(int16_t x, int16_t y, uint16_t n)
{
	x = DrawByte(x, y, (n >> 8) & 0xFF);
	x = DrawByte(x, y, n & 0xFF);
	return x;
}

//BYTE値を表示する
//戻り値：	次に描く文字のx座標
int16_t	Text::DrawByte(int16_t x, int16_t y, uint8_t n)
{
	char cc;
	cc = ToHexChar((n >> 4) & 0x0F);
	x = DrawChar(x, y, cc);
	cc = ToHexChar(n & 0x0F);
	x = DrawChar(x, y, cc);
	return x;
}

//4bit値を16進数表現の文字に変換する
char	Text::ToHexChar(uint8_t n)
{
	char cc = (n < 10) ? ('0' + n) : ('A' + (n - 10));
	return cc;
}

//整数を表示する(int32_t: -2,147,483,648～2,147,483,647)
//戻り値：	次に描く文字のx座標
int16_t	Text::DrawInt(int16_t x, int16_t y, int32_t n)
{
	uint32_t m;

	if (n < 0)
	{
		x = DrawChar(x, y, '-');
		m = -n;
	}
	else
	{
		m = n;
	}

	x = DrawUInt(x, y, m);
	return x;
}

//32bit正数を表示する(uint32_t: 0～4,294,967,295)
//戻り値：	次に描く文字のx座標
int16_t	Text::DrawUInt(int16_t x, int16_t y, uint32_t n)
{
	uint8_t	kbuf[10];	//32bit整数は10進数で最大10桁
	int8_t	i = 0;

	do
	{
		kbuf[i++] = n % 10;
		n /= 10;
	}
	while (0 < n);
	
	while (i != 0) { x = DrawChar(x, y, '0' + kbuf[--i]); }
	return x;
}
