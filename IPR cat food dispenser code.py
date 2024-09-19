
redOnTime = 2.3
greeOnTime = 2
repeatTimes = 1


import time
from AADFramework.ArduinoComponents import DigitalInput, DigitalOutput, ControlBoard

controller = ControlBoard('/dev/cu.usbmodem1101')

xMotor = controller.buildServoMotor(10, 'xMotor')
xMotor.homePos =0
xMotor.minAngle =0
xMotor.maxAngle = 180

irSensor = controller.buildDigitalInput(6, 'irSensor')
redLED = controller.buildDigitalOutput(4, 'redLED')
greenLED = controller.buildDigitalOutput(3, 'greenLED')

monitor = controller.buildInputMonitor()

controller.start()
monitor.start()


print("LED light up now")
for x in range(0, repeatTimes):
    redLED.turnOn()  # RED is on
    time.sleep(redOnTime)
    redLED.turnOff()

print("You can block your irSensor to start")
while True:
    if irSensor.getCountValue() >= 1:
        xMotor.threadTurnTo(180, 10)
        xMotor.threadTurnTo(0, 10)
        greenLED.turnOn()
        time.sleep(2)  # buzzer is buzzing for  5 secs
        greenLED.turnOff()
