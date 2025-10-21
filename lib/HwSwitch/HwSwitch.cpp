//	ハードウェアスイッチ
//	『昼夜逆転』工作室	@jsdiy	https://github.com/jsdiy
//	2025/01 - 2025/10
/*
2025/09	初回公開版
2025/10	Clicked()等を追加
*/

#include <Arduino.h>
#include "HwSwitch.hpp"

//初期化
void	HwSwitch::Initialize(gpio_num_t swPin, ulong longHoldThresholdMillis)
{
	this->swPin = swPin;
	pinMode(swPin, INPUT_PULLUP);
	longHoldThresholdTime = longHoldThresholdMillis;
	prevMillis = 0;
	prevPinState = SwOff;
	currentSwState = ESwState::Off;
}

//ボタンの押下状態を判定する
ESwState	HwSwitch::State(void)
{
	int8_t nowPinState;
	auto nowMillis = millis();
	if (DebounceTimeMillis < nowMillis - prevMillis)
	{
		nowPinState = digitalRead(swPin);
		prevMillis = nowMillis;
	}
	else
	{
		nowPinState = prevPinState;
	}

	if (prevPinState == SwOff && nowPinState == SwOn)
	{
		currentSwState = ESwState::On;
		holdStartTime = nowMillis;
	}
	else if (prevPinState == SwOn && nowPinState == SwOn)
	{
		currentSwState = (nowMillis - holdStartTime < longHoldThresholdTime)
			? ESwState::ShortHold : ESwState::LongHold;
	}
	else if (prevPinState == SwOn && nowPinState == SwOff)
	{
		currentSwState = ESwState::Release;
		holdStartTime = 0;
	}
	else	//(prevPinState == SwOff && nowPinState == SwOff)
	{
		currentSwState = ESwState::Off;
	}

	prevPinState = nowPinState;
	return currentSwState;
}
