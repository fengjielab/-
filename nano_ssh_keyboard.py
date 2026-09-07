#!/usr/bin/env python3
"""在 Nano 的 SSH 终端中读取电脑按键，并通过串口发送给 ESP32。"""

import os
import select
import sys
import termios
import tty
import time

import serial


# ESP32 接在 Nano 上的串口设备；先用 ls /dev/ttyUSB* /dev/ttyACM* 确认。
SERIAL_PORT = "/dev/ttyUSB0"
BAUDRATE = 115200
SEND_PERIOD = 0.05
DEBUG_LINK = True

KEY_MAP = {
    "w": "W",
    "s": "S",
    "a": "A",
    "d": "D",
    "q": "Q",
    "e": "E",
}


def send(ser, command):
    ser.write(command.encode("ascii"))


def show_debug_messages(ser):
    """显示 ESP32-C3/STM32 从串口回传的调试信息。"""
    if not DEBUG_LINK or ser.in_waiting == 0:
        return

    message = ser.read(ser.in_waiting).decode("utf-8", "replace")
    print(message, end="", flush=True)


def main():
    ser = serial.Serial(SERIAL_PORT, BAUDRATE, timeout=0)
    old_terminal = termios.tcgetattr(sys.stdin)
    current_command = "X"
    running = True

    try:
        # SSH 终端进入单字符模式，不需要按回车。
        tty.setcbreak(sys.stdin.fileno())
        print("SSH keyboard control started")
        print("W/S forward/back, A/D left/right, Q/E rotate")
        print("Press X to stop, Ctrl-C or ESC to exit")

        next_send = 0.0
        while running:
            readable, _, _ = select.select([sys.stdin], [], [], 0.01)

            if readable:
                value = os.read(sys.stdin.fileno(), 1).decode("utf-8", "ignore")

                if value == "\x1b" or value == "\x03":
                    running = False
                    break

                command = KEY_MAP.get(value.lower())
                if command is not None:
                    if command != current_command:
                        current_command = command
                        if DEBUG_LINK:
                            print(f"\nNano -> ESP32-C3: {current_command}")
                elif value.lower() == "x":
                    if current_command != "X":
                        current_command = "X"
                        if DEBUG_LINK:
                            print("\nNano -> ESP32-C3: X")

            now = time.monotonic()
            if now >= next_send:
                send(ser, current_command)
                next_send = now + SEND_PERIOD

            show_debug_messages(ser)

    except KeyboardInterrupt:
        pass
    finally:
        # 退出脚本前必须先让 STM32 停车。
        send(ser, "X")
        time.sleep(0.1)
        ser.close()
        termios.tcsetattr(sys.stdin, termios.TCSADRAIN, old_terminal)
        print("\nStopped")


if __name__ == "__main__":
    main()
