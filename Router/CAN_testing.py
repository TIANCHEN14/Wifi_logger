import os
import can
import time

#check system name, in linux will print 'posix' and in windows will print 'nt'
print(os.name)

os.system('sudo ifconfig can0 down')
os.system('sudo ip link set can0 type can bitrate 1000000')
os.system("sudo ifconfig can0 txqueuelen 10000")
os.system('sudo ifconfig can0 up')

can1 = can.interface.Bus(channel = 'can0', bustype = 'socketcan')
send_count = 0

while True:
    msg = can.Message(arbitration_id=0x123, data=[0,1,2,3,4,5,6,7])
    can1.send(msg)
    send_count = send_count + 1
    print("Currecnt send frame count:", send_count)
    time.sleep(0.001)