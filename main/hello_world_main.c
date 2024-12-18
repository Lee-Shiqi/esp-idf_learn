// /*
//  * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
//  *
//  * SPDX-License-Identifier: CC0-1.0
//  */

// #include <stdio.h>
// #include <inttypes.h>
// #include "sdkconfig.h"
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "esp_chip_info.h"
// #include "esp_flash.h"
// #include "esp_system.h"
// #include "driver/gpio.h"
// #include <led_strip.h>

// #define BLINK_GPIO 2
// #define LED_STRIP_LED_COUNT 24
// void app_main(void)
// {
// 	// 打印 "Hello world!" 到控制台
// 	printf("Hello world!\n");

// 	// 打印芯片信息
// 	esp_chip_info_t chip_info;
// 	uint32_t flash_size;
// 	esp_chip_info(&chip_info);
// 	printf("这是一个 %s 芯片，具有 %d 个 CPU 核心，%s%s%s%s，",
// 		   CONFIG_IDF_TARGET,
// 		   chip_info.cores,
// 		   (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi/" : "",
// 		   (chip_info.features & CHIP_FEATURE_BT) ? "蓝牙" : "",
// 		   (chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "",
// 		   (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");

// 	// 计算并打印硅片修订版本
// 	unsigned major_rev = chip_info.revision / 100;
// 	unsigned minor_rev = chip_info.revision % 100;
// 	printf("硅片修订版本 v%d.%d，", major_rev, minor_rev);

// 	// 获取并打印闪存大小
// 	if (esp_flash_get_size(NULL, &flash_size) != ESP_OK)
// 	{
// 		printf("获取闪存大小失败");
// 		return;
// 	}
// 	printf("%" PRIu32 "MB %s 闪存\n", flash_size / (uint32_t)(1024 * 1024),
// 		   (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "内置" : "外置");

// 	// 打印最小空闲堆大小
// 	printf("最小空闲堆大小：%" PRIu32 " 字节\n", esp_get_minimum_free_heap_size());

// 	// 配置 LED 灯带
// 	led_strip_config_t strip_config = {
// 		.strip_gpio_num = BLINK_GPIO,								 // 连接到 LED 灯带数据线的 GPIO
// 		.max_leds = 1,												 // 灯带中的 LED 数量
// 		.led_model = LED_MODEL_WS2812,								 // LED 灯带型号，决定位时序
// 		.color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB, // 颜色组件格式为 G-R-B
// 		.flags = {
// 			.invert_out = false, // 不反转输出信号
// 		}};

// 	// RMT 后端特定配置
// 	led_strip_rmt_config_t rmt_config = {
// 		.clk_src = RMT_CLK_SRC_DEFAULT,	   // 不同的时钟源会导致不同的功耗
// 		.resolution_hz = 10 * 1000 * 1000, // RMT 计数器时钟频率：10MHz
// 		.mem_block_symbols = 64,		   // 每个 RMT 通道的内存大小，以字（4 字节）为单位
// 		.flags = {
// 			.with_dma = false, // DMA 功能在 ESP32-S3 等芯片上可用/P4
// 		}};

// 	/// 创建 LED 灯带对象
// 	led_strip_handle_t led_strip;

// 	ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));

// 	for (int i = 0; i < LED_STRIP_LED_COUNT; i++)
// 	{
// 		ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, i, 5, 5, 5));
// 	}
// 	/* Refresh the strip to send data */
// 	ESP_ERROR_CHECK(led_strip_refresh(led_strip));

// }

/*
 * SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led_strip.h"
#include "esp_log.h"
#include "esp_err.h"
#include <math.h>

// GPIO 引脚定义
#define LED_STRIP_GPIO_PIN 2
// LED 灯带中的 LED 数量
#define LED_STRIP_LED_COUNT 15
// 10MHz 分辨率，1 个 tick = 0.1 微秒（LED 灯带需要高分辨率）
#define LED_STRIP_RMT_RES_HZ (10 * 1000 * 1000)

static const char *TAG = "example";

// 配置 LED 灯带
led_strip_handle_t configure_led(void)
{
	// 根据 LED 灯带设计进行初始化
	led_strip_config_t strip_config = {
		.strip_gpio_num = LED_STRIP_GPIO_PIN,						 // 连接到 LED 灯带数据线的 GPIO
		.max_leds = LED_STRIP_LED_COUNT,							 // 灯带中的 LED 数量
		.led_model = LED_MODEL_WS2812,								 // LED 灯带型号
		.color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB, // 颜色顺序：GRB
		.flags = {
			.invert_out = false, // 不反转输出信号
		}};

	// LED 灯带后端配置：RMT
	led_strip_rmt_config_t rmt_config = {
		.clk_src = RMT_CLK_SRC_DEFAULT,		   // 不同的时钟源会导致不同的功耗
		.resolution_hz = LED_STRIP_RMT_RES_HZ, // RMT 计数器时钟频率
		.mem_block_symbols = 64,			   // 每个 RMT 通道的内存大小，以字（4 字节）为单位
		.flags = {
			.with_dma = false, // DMA 功能在 ESP32-S3 等芯片上可用
		}};

	// LED 灯带对象句柄
	led_strip_handle_t led_strip;
	ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
	ESP_LOGI(TAG, "创建了带有 RMT 后端的 LED 灯带对象");
	return led_strip;
}

// 主应用程序
void app_main(void)
{
	int j = 0;
	led_strip_handle_t led_strip = configure_led();
	bool led_on_off = false;

	ESP_LOGI(TAG, "开始闪烁 LED 灯带");
	while (1)
	{
		for (int j = 0; j < 256; j++)
		{
			for (int i = 0; i < LED_STRIP_LED_COUNT; i++)
			{
				uint8_t r = (sin((i + j) * M_PI / 128) + 1) * 127.5;
				uint8_t g = (sin((i + j + 85) * M_PI / 128) + 1) * 127.5;
				uint8_t b = (sin((i + j + 170) * M_PI / 128) + 1) * 127.5;
				ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, i, r, g, b));
			}
			/* 刷新灯带以发送数据 */
			ESP_ERROR_CHECK(led_strip_refresh(led_strip));
			vTaskDelay(pdMS_TO_TICKS(5));
		}
	}
}
