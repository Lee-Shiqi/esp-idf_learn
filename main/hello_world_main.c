/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "driver/gpio.h"

void app_main(void)
{
    // 打印 "Hello world!" 到控制台
    printf("Hello world!\n");

    // 打印芯片信息
    esp_chip_info_t chip_info;
    uint32_t flash_size;
    esp_chip_info(&chip_info);
    printf("这是一个 %s 芯片，具有 %d 个 CPU 核心，%s%s%s%s，",
           CONFIG_IDF_TARGET,
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi/" : "",
           (chip_info.features & CHIP_FEATURE_BT) ? "蓝牙" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "",
           (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");

    // 计算并打印硅片修订版本
    unsigned major_rev = chip_info.revision / 100;
    unsigned minor_rev = chip_info.revision % 100;
    printf("硅片修订版本 v%d.%d，", major_rev, minor_rev);

    // 获取并打印闪存大小
    if(esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
        printf("获取闪存大小失败");
        return;
    }
    printf("%" PRIu32 "MB %s 闪存\n", flash_size / (uint32_t)(1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "内置" : "外置");

    // 打印最小空闲堆大小
    printf("最小空闲堆大小：%" PRIu32 " 字节\n", esp_get_minimum_free_heap_size());


    gpio_config_t gpio_init_struct = {0};

    gpio_init_struct.intr_type = GPIO_INTR_DISABLE;
    gpio_init_struct.mode = GPIO_MODE_INPUT_OUTPUT;
    gpio_init_struct.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_init_struct.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_init_struct.pin_bit_mask = (1ULL << GPIO_NUM_48);
    gpio_config(&gpio_init_struct);

    while (true)
    {
        gpio_set_level(GPIO_NUM_48, 1);
        vTaskDelay(20 / portTICK_PERIOD_MS);
        gpio_set_level(GPIO_NUM_48, 0);
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}