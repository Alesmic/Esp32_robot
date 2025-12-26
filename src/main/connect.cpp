#include "connect.h"

// 实例化指令队列
std::queue<String> commandQueue;

// ---------------------------------------------------------
// BLE 回调类 (Listener)
// ---------------------------------------------------------
// 当手机 App 向 ESP32 写入数据时，会触发此类中的方法
class BltCallbacks: public BLECharacteristicCallbacks {
    // 覆写 onWrite 方法
    void onWrite(BLECharacteristic* pCharacteristic) {  
        // 获取接收到的原始数据
        std::string value = pCharacteristic->getValue();  
        
        if (!value.empty()) {  
            // 转换为 Arduino String 方便处理
            String cmd = String(value.c_str()); 
            cmd.trim(); // 去除首尾空格/换行符
            
            // *关键设计*：不要在这里执行耗时动作！
            // 蓝牙回调是在中断或高优先级任务中运行的。
            // 应该只把指令推入队列，让主循环(loop)去处理。
            commandQueue.push(cmd); 
            
            Serial.print("[BLE] Received: ");
            Serial.println(cmd);
        }  
    }  
};

// ---------------------------------------------------------
// 初始化蓝牙
// ---------------------------------------------------------
void setup_BLE() {
  Serial.println("[Init] Starting BLE...");

  // 1. 初始化设备名称
  BLEDevice::init("ESP32-Robot"); 
  
  // 2. 创建服务器
  BLEServer *pServer = BLEDevice::createServer();
  
  // 3. 创建服务 (Service)
  BLEService *pService = pServer->createService(SERVICE_UUID);
  
  // 4. 创建特征值 (Characteristic)
  // 设置属性为：可读(READ) + 可写(WRITE)
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  // 5. 挂载回调函数
  pCharacteristic->setCallbacks(new BltCallbacks());
  
  // 6. 启动服务
  pService->start();
  
  // 7. 开始广播 (让手机能搜到)
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // 设置广播间隔优化 iPhone 连接
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  
  Serial.println("[Init] BLE Ready! Waiting for connection...");
}

// ---------------------------------------------------------
// 指令处理主逻辑 (在 loop 中调用)
// ---------------------------------------------------------
void handle_cmd() {
  String cmd = "";

  // 1. 优先检查蓝牙队列
  if (!commandQueue.empty()) {
    cmd = commandQueue.front(); // 取出队首
    commandQueue.pop();         // 移除队首
  } 
  // 2. 其次检查串口调试输入 (方便电脑调试)
  else if (Serial.available() > 0) {
    cmd = Serial.readStringUntil('\n');
    cmd.trim();
  }

  // 如果没有指令，直接返回
  if (cmd.isEmpty()) return;

  // --- JSON 解析阶段 ---
  // 使用 ArduinoJson 库解析指令
  // 示例指令: {"actions": ["eye_blink", "head_nod"]}
  JsonDocument doc;  
  DeserializationError error = deserializeJson(doc, cmd);

  if (error) {
    // 如果不是 JSON，尝试直接作为简单指令处理
    Serial.print("[Parser] Not JSON, trying raw command: ");
    Serial.println(cmd);
    executeCommand(cmd);
    return;
  }

  // 处理 "actions" 数组：批量执行动作
  if (doc["actions"].is<JsonArray>()) {
    JsonArray actions = doc["actions"].as<JsonArray>();
    for (JsonVariant action : actions) {
      executeCommand(action.as<String>());
    }
  }

  // 处理 "factory" 指令：用于校准或系统设置
  if (doc["factory"].is<String>()) {
    executeFactoryCommand(doc["factory"].as<String>());
  }
}

// ---------------------------------------------------------
// 动作执行路由表
// ---------------------------------------------------------
void executeCommand(String cmd) {
  Serial.print("[Exec] Action: ");
  Serial.println(cmd);

  // 表情类
  if (cmd == "eye_blink")      eye_blink();
  else if (cmd == "eye_happy") eye_happy();
  else if (cmd == "eye_sad")   eye_sad();
  else if (cmd == "eye_left")  eye_left();
  else if (cmd == "eye_right") eye_right();
  
  // 动作类
  else if (cmd == "head_nod")   head_nod();
  else if (cmd == "head_shake") head_shake();
  
  // 动画类 (播放位图)
  else if (cmd == "love")       play_animation(41); // 假设41是爱心ID
  
  else {
    Serial.println("[Exec] Unknown command");
  }
}

// ---------------------------------------------------------
// 工厂指令 (调试用)
// ---------------------------------------------------------
void executeFactoryCommand(String cmd) {
  Serial.print("[Factory] Cmd: ");
  Serial.println(cmd);

  if (cmd == "reboot") {
    ESP.restart();
  }
  // 示例：动态校准 X 轴中心位 "adjust_x_10"
  else if (cmd.startsWith("adjust_x")) {
    // 实际代码需要解析后面的数字，这里简化处理
    Serial.println("Adjusting X Center...");
    // adjust_x_center(10); 
  }
}