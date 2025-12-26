#include "head.h"

// ---------------------------------------------------------
// 舵机参数定义
// ---------------------------------------------------------
// 这些中心位不是写死的，而是可以通过文件系统动态调整的
int X_CENTER = 90;
int Y_CENTER = 90;
// 运动范围限制，防止舵机撞击外壳或拉断线缆
int X_MIN = X_CENTER - X_OFFSET;
int X_MAX = X_CENTER + X_OFFSET;
int Y_MIN = Y_CENTER - Y_OFFSET;
int Y_MAX = Y_CENTER + Y_OFFSET;

Servo servo_x; // 水平舵机 (Pan)
Servo servo_y; // 垂直舵机 (Tilt)

// 初始化舵机对象并立即归位
void servo_init() {
  servo_x.attach(X_PIN);
  servo_y.attach(Y_PIN);
  servo_x.write(X_CENTER);
  servo_y.write(Y_CENTER);
}

// 头部初始化流程
void setup_head() {
  update_x_center(); // 从文件读取 X 轴校准值
  update_y_center(); // 从文件读取 Y 轴校准值
  servo_init();      // 挂载舵机
  head_center();     // 移动到中心
  delay(500);        // 等待动作完成
}

// ---------------------------------------------------------
// 校准数据持久化 (工程亮点)
// ---------------------------------------------------------
// 将校准后的中心位写入 Flash 文件系统，断电不丢失
void adjust_x_center(int offset) {
  File file = FFat.open(X_CENTER_FILE, FILE_WRITE);
  if (file) {
    X_CENTER = X_CENTER + offset; // 更新内存中的值
    file.print(String(X_CENTER)); // 写入文件
    file.close();
    Serial.print("[Factory] Adjust X_CENTER to ");
    Serial.println(X_CENTER);
  } else {
    Serial.print("[Error] Could not write file ");
    Serial.println(X_CENTER_FILE);
  }
}
// Y 轴同理...
void adjust_y_center(int offset) {
  File file = FFat.open(Y_CENTER_FILE, FILE_WRITE);
  if (file) {
    Y_CENTER = Y_CENTER + offset;
    file.print(String(Y_CENTER));
    file.close();
    Serial.print("[Factory] Adjust Y_CENTER to ");
    Serial.println(Y_CENTER);
  } else {
    Serial.print("[Error] Could not write file ");
    Serial.println(Y_CENTER_FILE);
  }
}

// 从文件系统读取校准值，如果文件不存在则使用默认值并创建文件
void update_x_center() {
  File file = FFat.open(X_CENTER_FILE, FILE_READ);
  if (file) {
    String valueStr = file.readString();
    file.close();
    X_CENTER = valueStr.toInt();
    // 读取新中心位后，必须重新计算运动范围边界
    X_MIN = X_CENTER - X_OFFSET;
    X_MAX = X_CENTER + X_OFFSET;
  } else {
    adjust_x_center(0); // 文件不存在，创建默认值文件
  }
  Serial.print("X center angle: ");
  Serial.println(X_CENTER);
}
// Y 轴同理...
void update_y_center() {
  File file = FFat.open(Y_CENTER_FILE, FILE_READ);
  if (file) {
    String valueStr = file.readString();
    file.close();
    Y_CENTER = valueStr.toInt();
    Y_MIN = Y_CENTER - Y_OFFSET;
    Y_MAX = Y_CENTER + Y_OFFSET;
  } else {
    adjust_y_center(0);
  }
  Serial.print("Y center angle: ");
  Serial.println(Y_CENTER);
}

// ---------------------------------------------------------
// 核心运动函数 (带缓动)
// ---------------------------------------------------------
// 这是一个阻塞函数 (Blocking)，它会卡住主程序直到移动完成
void head_move(int x_offset, int y_offset, int servo_delay) {
  int x_angle = servo_x.read();
  int y_angle = servo_y.read();
  // 计算目标角度并限制在安全范围内 (Constrain)
  int to_x_angle = constrain(x_angle + x_offset, X_MIN, X_MAX);
  int to_y_angle = constrain(y_angle + y_offset, Y_MIN, Y_MAX);

  // 逐步逼近目标，产生平滑的运动效果
  while (x_angle != to_x_angle || y_angle != to_y_angle) {
    if (x_angle != to_x_angle) {
      x_angle += (to_x_angle > x_angle ? STEP : -STEP);
      servo_x.write(x_angle);
    }
    if (y_angle != to_y_angle) {
      y_angle += (to_y_angle > y_angle ? STEP : -STEP);
      servo_y.write(y_angle);
    }
    delay(servo_delay); // 控制速度，延迟越小转得越快
  }
}

void head_center(int servo_delay) {
  int x_angle = servo_x.read();
  int y_angle = servo_y.read();
  head_move(X_CENTER - x_angle, Y_CENTER - y_angle, servo_delay);
}

void head_right(int offset) {
  head_move(offset, 0);
}

void head_left(int offset) {
  head_move(-offset, 0);
}

void head_down(int offset) {
  head_move(0, offset);
}

void head_up(int offset) {
  head_move(0, -offset);
}

void head_nod(int servo_delay) {
  for (int i = 0; i < 2; i++) {
    head_move(0, 20, servo_delay);
    delay(50);
    head_move(0, -20, servo_delay);
    delay(50);
  }
}

void head_shake(int servo_delay) {
  head_move(-10, 0, servo_delay);
  delay(50);
  head_move(20, 0, servo_delay);
  delay(50);
  head_move(-20, 0, servo_delay);
  delay(50);
  head_move(20, 0, servo_delay);
  delay(50);
  head_move(-10, 0, servo_delay);
  delay(50);
}

void head_roll_left(int servo_delay) {
  head_center();
  head_down(Y_OFFSET / 2 + 5);
  head_move(-X_OFFSET, -Y_OFFSET / 2, servo_delay);
  head_move(X_OFFSET, -Y_OFFSET / 2, servo_delay);
  head_move(X_OFFSET, Y_OFFSET / 2, servo_delay);
  head_move(-X_OFFSET, Y_OFFSET / 2, servo_delay);
  head_center();
}

void head_roll_right(int servo_delay) {
  head_center();
  head_down(Y_OFFSET / 2 + 5);
  head_move(X_OFFSET, -Y_OFFSET / 2, servo_delay);
  head_move(-X_OFFSET, -Y_OFFSET / 2, servo_delay);
  head_move(-X_OFFSET, Y_OFFSET / 2, servo_delay);
  head_move(X_OFFSET, Y_OFFSET / 2, servo_delay);
  head_center();
}
