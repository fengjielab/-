/*
 * Nano -> ESP32 -> STM32F103ZET6 按键命令转发
 *
 * 数据链路：
 *   电脑 SSH 终端按键
 *       -> Nano 终端脚本
 *       -> Nano USB 串口
 *       -> ESP32 Serial(UART0)
 *       -> ESP32 Serial1(UART1)
 *       -> STM32 USART1
 *
 * STM32 当前有效命令：
 *   W/w : 前进       S/s : 后退
 *   A/a : 左移       D/d : 右移
 *   Q/q : 原地左转   E/e : 原地右转
 *   X/x : 停止
 */

#include <Arduino.h>

// ESP32 USB 串口，接收 Nano 发来的命令。
// 不要在这里输出调试文字，否则会混入给 Nano 的串口数据。
#define NANO_SERIAL Serial

// ESP32-C3 只有 UART0 和 UART1；UART1 连接 STM32 USART1。
HardwareSerial Stm32Serial(1);

// ESP32-C3 UART1 引脚，可按开发板实际接线修改。
static constexpr int STM32_RX_PIN = 16;  // ESP32-C3 RX1 <- STM32 PA9(TX)，可选
static constexpr int STM32_TX_PIN = 17;  // ESP32-C3 TX1 -> STM32 PA10(RX)

static constexpr uint32_t UART_BAUD = 115200;
static constexpr uint32_t COMMAND_TIMEOUT_MS = 250;
static constexpr bool DEBUG_LINK = true;

char lastCommand = 'X';
uint32_t lastCommandMs = 0;

bool isCommand(uint8_t value) {
  switch (value) {
    case 'W': case 'w':
    case 'S': case 's':
    case 'A': case 'a':
    case 'D': case 'd':
    case 'Q': case 'q':
    case 'E': case 'e':
    case 'X': case 'x':
      return true;
    default:
      return false;
  }
}

void forwardCommand(char command) {
  // 统一转成大写，STM32 两种大小写都支持。
  if (command >= 'a' && command <= 'z') {
    command = static_cast<char>(command - 'a' + 'A');
  }

  const bool commandChanged = command != lastCommand;

  Stm32Serial.write(static_cast<uint8_t>(command));
  Stm32Serial.flush();

  lastCommand = command;
  lastCommandMs = millis();

  // 只在命令改变时回报，避免 20 Hz 的保活命令刷屏。
  if (DEBUG_LINK && commandChanged) {
    NANO_SERIAL.print("C3 -> STM32: ");
    NANO_SERIAL.println(command);
  }
}

void setup() {
  // Nano 通过 USB 串口发送，波特率要和 Nano 脚本一致。
  NANO_SERIAL.begin(UART_BAUD);

  Stm32Serial.begin(
      UART_BAUD,
      SERIAL_8N1,
      STM32_RX_PIN,
      STM32_TX_PIN);

  // ESP32 上电先让 STM32 停车。
  forwardCommand('X');
}

void loop() {
  while (NANO_SERIAL.available() > 0) {
    const uint8_t value = static_cast<uint8_t>(NANO_SERIAL.read());

    if (isCommand(value)) {
      forwardCommand(static_cast<char>(value));
    }
    // 忽略 SSH 终端脚本可能发送的回车、换行等字符。
  }

  // Nano/SSH/脚本异常退出时，最多 250 ms 后自动停止 STM32。
  const uint32_t now = millis();
  if (lastCommand != 'X' &&
      (now - lastCommandMs) > COMMAND_TIMEOUT_MS) {
    forwardCommand('X');
  }

  // 将 STM32 的 ACK 回传 Nano，供调试链路使用。
  while (Stm32Serial.available() > 0) {
    const int value = Stm32Serial.read();
    if (DEBUG_LINK && value >= 0) {
      NANO_SERIAL.write(static_cast<uint8_t>(value));
    }
  }

  delay(1);
}
