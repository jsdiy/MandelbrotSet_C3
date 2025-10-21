//	ハードウェアスイッチ
//	『昼夜逆転』工作室	@jsdiy	https://github.com/jsdiy
/*
2025/01	AVR版(2019/01)を移植、ReleaseとOffの意味を入れ替え
2025/09	ESP32版としてmills()利用へ改造
2025/10	Clicked()等を追加
*/
/*
	| gpio|--<SW>--+	SW: button switch, slide switch, etc.
	|     |        |	turn on:LOW, off:HIGH
	|ESP32|       GND
*/

#pragma	once

#include <Arduino.h>

//スイッチの押下状態
enum	class	ESwState	: uint8_t
{
	On			= 0x11,	//オンになった瞬間
	ShortHold	= 0x12,	//オンであり続けている状態（指定時間未満）
	LongHold	= 0x13,	//オンであり続けている状態（指定時間以上経過）
	Release		= 0x01,	//オフになった瞬間
	Off			= 0x02	//オフであり続けている状態
};

//ハードウェアスイッチ
class	HwSwitch
{
private:
	static	const	int8_t	SwOn = LOW,	SwOff = HIGH;
	static	const	ulong	DebounceTimeMillis = 50;
	static	const	uint8_t	SwOnMask = 0x10;
	gpio_num_t	swPin;
	int8_t	prevPinState;	//値はSwOn/SwOffを取る
	ulong	prevMillis;
	ulong	longHoldThresholdTime, holdStartTime;
	ESwState	currentSwState;

public:
	HwSwitch(void) {}	//この場合は後でInitialize()を呼ぶ必要がある
	HwSwitch(gpio_num_t swPin, ulong longHoldThresholdMillis = 0) { Initialize(swPin, longHoldThresholdMillis); }
	void	Initialize(gpio_num_t swPin, ulong longHoldThresholdMillis = 0);
	ESwState	State(void);
	bool	Clicked(void) { return (currentSwState == ESwState::Release); }
	bool	Holding(void) { return (currentSwState == ESwState::LongHold); }
	bool	IsOn(void) { return (static_cast<uint8_t>(currentSwState) & SwOnMask); }
	bool	IsOff(void) { return !IsOn(); }
};
