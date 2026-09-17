#!/usr/bin/env python

import rospy
import serial

from geometry_msgs.msg import Twist


class FourdriverSerialNode(object):

    def __init__(self):
        port = rospy.get_param('~port', '/dev/ttyUSB0')
        baud = rospy.get_param('~baud', 115200)

        self.wheel_center_x = rospy.get_param(
            '~wheel_center_x',
            0.10
        )
        self.wheel_center_y = rospy.get_param(
            '~wheel_center_y',
            0.1225
        )
        self.command_rate = rospy.get_param(
            '~command_rate',
            20.0
        )
        self.command_timeout = rospy.get_param(
            '~command_timeout',
            0.5
        )

        self.vx = 0.0
        self.vy = 0.0
        self.wz = 0.0
        self.last_cmd_time = rospy.Time.now()

        self.ser = serial.Serial(
            port=port,
            baudrate=baud,
            timeout=0.1
        )

        rospy.on_shutdown(self.shutdown)

        self.send_wheel_speeds(0.0, 0.0, 0.0, 0.0)

        self.sub = rospy.Subscriber(
            '/cmd_vel',
            Twist,
            self.cmd_vel_callback,
            queue_size=1
        )

        self.timer = rospy.Timer(
            rospy.Duration(1.0 / self.command_rate),
            self.timer_callback
        )

        rospy.loginfo('Serial connected: %s at %d baud', port, baud)
        rospy.loginfo('Subscribed to /cmd_vel')
        rospy.loginfo(
            'Wheel geometry: Lx=%.4f m, Ly=%.4f m, command rate=%.1f Hz',
            self.wheel_center_x,
            self.wheel_center_y,
            self.command_rate
        )

    def send_wheel_speeds(self, m1, m2, m3, m4):
        frame = 'V,%.5f,%.5f,%.5f,%.5f\n' % (
            m1,
            m2,
            m3,
            m4
        )

        self.ser.write(frame.encode('ascii'))
        rospy.loginfo_throttle(
            1.0,
            'Sent wheel speeds: %s' % frame.rstrip()
        )

    def cmd_vel_callback(self, msg):
        self.vx = msg.linear.x
        self.vy = msg.linear.y
        self.wz = msg.angular.z
        self.last_cmd_time = rospy.Time.now()

    def timer_callback(self, _event):
        now = rospy.Time.now()
        command_age = (now - self.last_cmd_time).to_sec()

        if command_age > self.command_timeout:
            vx = 0.0
            vy = 0.0
            wz = 0.0
        else:
            vx = self.vx
            vy = self.vy
            wz = self.wz

        rotation_term = (
            self.wheel_center_x
            + self.wheel_center_y
        ) * wz

        m1 = vx + vy - rotation_term
        m2 = vx - vy + rotation_term
        m3 = vx - vy - rotation_term
        m4 = vx + vy + rotation_term

        self.send_wheel_speeds(m1, m2, m3, m4)

    def shutdown(self):
        try:
            self.send_wheel_speeds(0.0, 0.0, 0.0, 0.0)
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
