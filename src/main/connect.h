#ifndef CONNECT_H
#define CONNECT_H

#include <Arduino.h>
// 引入蓝牙低功耗库 (ESP32标准库)
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
// 引入 JSON 库，用于解析复杂指令
#include <ArduinoJson.h>
#include <queue> // 引入标准模板库队列

#include "act.h"
#include "emoji.h"
#include "head.h"
#include "animation.h"

// ---------------------------------------------------------
// BLE UUID 配置
// ---------------------------------------------------------
// 使用在线生成器生成的 UUID，确保唯一性
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// ---------------------------------------------------------
// 全局变量声明
// ---------------------------------------------------------
// 指令队列：用于在中断(BLE回调)和主循环之间安全传递消息
extern std::queue<String> commandQueue;

// ---------------------------------------------------------
// 函数声明
// ---------------------------------------------------------
void setup_BLE();   // 初始化蓝牙
void handle_cmd();  // 处理指令主循环
void executeCommand(String cmd); // 执行具体动作
void executeFactoryCommand(String cmd); // 执行调试指令

#endif