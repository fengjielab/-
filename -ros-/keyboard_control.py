import serial
import time
import threading

from pynput import keyboard


# ==========================
# 修改成你自己的串口。
# ==========================

ser = serial.Serial(
    '/dev/ttyUSB0',
    115200,
    timeout=0.1
)


# 当前正在执行的命令
current_cmd = 'X'

lock = threading.Lock()

running = True


# ==========================
# 键盘按下
# ==========================

def on_press(key):

    global current_cmd

    try:
        k = key.char.lower()
    except AttributeError:
        return

    key_map = {
        'w': 'W',
        's': 'S',
        'a': 'A',
        'd': 'D',
        'q': 'Q',
        'e': 'E',
    }

    if k in key_map:

        with lock:
            current_cmd = key_map[k]


# ==========================
# 键盘松开
# ==========================

def on_release(key):

    global current_cmd
    global running

    try:
        k = key.char.lower()
    except AttributeError:

        if key == keyboard.Key.esc:
            running = False
            return False

        return

    if k in ['w', 's', 'a', 'd', 'q', 'e']:

        with lock:
            current_cmd = 'X'


# ==========================
# 周期发送命令
# ==========================

def send_task():

    while running:

        with lock:
            cmd = current_cmd

        ser.write(cmd.encode())

        time.sleep(0.05)


# ==========================
# 启动发送线程
# ==========================

sender = threading.Thread(
    target=send_task,
    daemon=True
)

sender.start()


print("机器人键盘控制启动")
print("W/S = 前后")
print("A/D = 左右")
print("Q/E = 左右旋转")
print("松开按键 = 停止")
print("ESC = 退出")


with keyboard.Listener(
        on_press=on_press,
        on_release=on_release) as listener:

    listener.join()


# 退出程序前强制停止
ser.write(b'X')

time.sleep(0.1)

ser.close()