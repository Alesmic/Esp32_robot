#include "Arduino.h"
#include "emoji.h"
#include "head.h"
#include "connect.h"
#include "act.h"
#include "animation.h"


// ---------------------------------------------------------
// 新增功能：轮播演示模式
// ---------------------------------------------------------
int demo_mode_index = 0;          // 当前播放的表情索引
unsigned long demo_last_time = 0; // 上次切换的时间点

void cycle_all_expressions() {
  // 检查是否到达 5 秒 (5000ms) 切换间隔
  if (millis() - demo_last_time > 5000) {
    demo_last_time = millis(); // 重置计时器
    
    Serial.print("Demo Mode Switching to: ");
    Serial.println(demo_mode_index);

    // 1. 程序化几何表情 (来自 emoji.cpp)
    switch (demo_mode_index) {
      case 0: eye_happy();    break; // 开心
      case 1: eye_sad();      break; // 难过
      case 2: eye_anger();    break; // 生气
      case 3: eye_surprise(); break; // 惊讶
      case 4: eye_blink(10);  break; // 快速眨眼
      case 5: eye_sleep();    break; // 睡觉
      case 6: eye_wakeup();   break; // 醒来
      case 7: 
        eye_right(); 
        delay(500); 
        eye_center();         break; // 向右看
      case 8: 
        eye_left(); 
        delay(500); 
        eye_center();         break; // 向左看
      
      // 2. 位图动画 (来自 animation.h, ID 0~40)
      default:
        // 计算动画 ID (减去前面 9 个几何表情的偏移量)
        int anim_id = demo_mode_index - 9;
        
        // 确保 ID 在有效范围内 (animation.h 中定义 FRAMES_COUNT 为 41)
        if (anim_id < 41) { 
          play_animation(anim_id);
        } else {
          // 如果播完了所有动画，重置索引，从头开始
          demo_mode_index = -1; // 设置为 -1，下面 ++ 后变成 0
        }
        break;
    }

    // 切换到下一个表情
    demo_mode_index++;
    
    // 每次动作结束后，确保眼睛回到中心并在 OLED 上刷新出来，防止残影
    if(demo_mode_index > 8) { // 仅针对动画模式复位，几何表情内部通常有复位逻辑
        eye_center(true); 
    }
  }
}
// ---------------------------------------------------------
// 初始化函数 (Setup)
// ---------------------------------------------------------
void setup() {
  // 1. 启动串口调试，波特率 115200
  Serial.begin(115200);
  Serial.flush();
  Serial.println("Initializing...");

  // 2. 初始化底层与外设
  // 导师提示：初始化顺序很有讲究。
  // 先初始化文件系统(FFat)，因为舵机校准需要读取里面的文件。
  setup_FFat();   // 挂载文件系统
  setup_BLE();    // 启动蓝牙广播
  setup_emoji();  // 启动 OLED 屏幕并显示开机动画
  setup_head();   // 启动舵机并回中 (会读取 FFat 中的校准值)

  // 3. 记录启动时间，用于后续的随机动作计时
  last_time = millis();
  Serial.println("Robot initialization Done.");
}

// ---------------------------------------------------------
// 主循环 (Loop)
// ---------------------------------------------------------
void loop() {
// -------------------------------------------------------
  // [导师注释] 原有逻辑已注释 (交互模式)
  // -------------------------------------------------------
  // handle_cmd(); // 处理蓝牙指令
  
  // if (enable_act && millis() - last_time > random(6, 10) * 1000) {
  //   random_act(); // 随机发呆动作
  //   last_time = millis();
  // }

  // -------------------------------------------------------
  // [导师注释] 新增逻辑 (轮播模式)
  // -------------------------------------------------------
  cycle_all_expressions(); // 每隔 5 秒切换下一个表情
  delay(10);
}