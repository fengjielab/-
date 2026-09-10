#!/usr/bin/env python

import rospy
import serial

from geometry_msgs.msg import Twist


class FourdriverSerialNode(object):

    def __init__(self):
        port = rospy.get_param('~port', '/dev/ttyUSB0')
        baud = rospy.get_param('~baud', 115200)

        self.ser = serial.Serial(
            port=port,
            baudrate=baud,
            timeout=0.1
        )

        rospy.on_shutdown(self.shutdown)

        self.ser.write(b'X')

        self.sub = rospy.Subscriber(
            '/cmd_vel',
            Twist,
            self.cmd_vel_callback,
            queue_size=1
        )

        rospy.loginfo('Serial connected: %s at %d baud', port, baud)
        rospy.loginfo('Subscribed to /cmd_vel')

    def send_command(self, command):
        self.ser.write(command.encode('ascii'))
        rospy.loginfo_throttle(
            1.0,
            'Sent command: %s' % command
        )

    def cmd_vel_callback(self, msg):
        vx = msg.linear.x
        vy = msg.linear.y
        wz = msg.angular.z

        threshold = 0.01

        if abs(vx) < threshold and abs(vy) < threshold and abs(wz) < threshold:
            command = 'X'

        elif abs(vx) >= abs(vy) and abs(vx) >= abs(wz):
            command = 'W' if vx > 0 else 'S'

        elif abs(vy) >= abs(wz):
            command = 'A' if vy > 0 else 'D'

        else:
            command = 'Q' if wz > 0 else 'E'

        self.send_command(command)

    def shutdown(self):
        try:
            self.ser.write(b'X')
            self.ser.close()
            rospy.loginfo('Sent stop command and closed serial port')
        except Exception:
            pass


def main():
    rospy.init_node('fourdriver_serial_node')
    FourdriverSerialNode()
    rospy.spin()


if __name__ == '__main__':
    main()
