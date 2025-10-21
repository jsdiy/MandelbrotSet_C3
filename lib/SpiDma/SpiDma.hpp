//SPI-DMA制御
//『昼夜逆転』工作室	@jsdiy	https://github.com/jsdiy
//	2024/06 - 2025/10
/*	履歴
2025/9	初版
2025/10	「namespace SpiPinConfig」を追加。それに伴いInitialize()を更新。
*/

#pragma	once

#include <Arduino.h>
#include "driver/spi_master.h"

namespace	SpiPinConfig
{
#if	CONFIG_IDF_TARGET_ESP32
	constexpr	spi_host_device_t	HostID = VSPI_HOST;
	constexpr	gpio_num_t	PinMOSI = GPIO_NUM_23, PinMISO = GPIO_NUM_19, PinSCK = GPIO_NUM_18, PinSS = GPIO_NUM_5;
#endif
#if	CONFIG_IDF_TARGET_ESP32C3
	constexpr	spi_host_device_t	HostID = SPI2_HOST;
	constexpr	gpio_num_t	PinMOSI = GPIO_NUM_7, PinMISO = GPIO_NUM_2, PinSCK = GPIO_NUM_6, PinSS = GPIO_NUM_10;
#endif
}

class	SpiDma
{
public:
	static	constexpr	size_t	MaxTxBufferSize = 4092UL;	//spi_transaction_t::tx_bufferで一度に送信できるデータの長さ(byte)
	static	constexpr	size_t	GetBitLength(size_t byteLength) { return byteLength * 8; }

private:
	spi_host_device_t	spiHostId;
	spi_transaction_t	trans;
	SemaphoreHandle_t	spiMutex;	//SPIバスの排他制御用
	bool	isInitialized;
	void	InitializeSpiBusConfig(spi_bus_config_t* spiBusConfig, gpio_num_t mosi, gpio_num_t miso, gpio_num_t sck);

public:
	SpiDma(void) { spiMutex = xSemaphoreCreateMutex();	isInitialized = false; }
	void	Initialize(spi_host_device_t hostId = SpiPinConfig::HostID,
		gpio_num_t mosi = SpiPinConfig::PinMOSI, gpio_num_t miso = SpiPinConfig::PinMISO, gpio_num_t sck = SpiPinConfig::PinSCK);
	void	AddDeviceToBus(const spi_device_interface_config_t* devConfig, spi_device_handle_t* devHandle);
	void	RemoveDeviceFromBus(spi_device_handle_t devHandle);
	void	TakeBusControll(void) { xSemaphoreTake(spiMutex, pdMS_TO_TICKS(1000)); }	//有限時間（デッドロック低減）
	void	GiveBusControll(void) { xSemaphoreGive(spiMutex); }
	void	Transmit(spi_device_handle_t devHandle, uint8_t data);
	void	Transmit(spi_device_handle_t devHandle, uint8_t data1, uint8_t data2);
	void	Transmit(spi_device_handle_t devHandle, uint8_t data1, uint8_t data2, uint8_t data3);
	void	Transmit(spi_device_handle_t devHandle, uint8_t data1, uint8_t data2, uint8_t data3, uint8_t data4);
	void	Transmit(spi_device_handle_t devHandle, const uint8_t* datas, size_t length);
	void	TransmitOverSize(spi_device_handle_t devHandle, const uint8_t* datas, size_t length);
};

extern	SpiDma	spiDma;
