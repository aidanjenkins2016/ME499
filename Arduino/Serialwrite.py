import serial

ser = serial.Serial('/dev/ttyACM0', 9600')

val=1000;

while(True):
    val=val+1
        if(val>2000):
        val=1000
        break
        ser.write(val)
        
