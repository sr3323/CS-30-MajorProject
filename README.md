# CS-30-MajorProject
This project is a USB Foot pedal. It can control either MIDI CC values or press keyboard buttons, and can be configured using a serial connection.
# Installation
You need to install both Arduino and TeensyDuino in order to upload this program onto the teensy. You also need to copy usb_desc.h into\
/Applications/Arduino.app/Contents/Java/hardware/teensy/avr/cores/teensy3 for Mac OS\
[placeholder] for windows
# Uploading the code
Upload the code as you would do normally using the Arduino IDE. Select Teensy LC as the board, and select Keyboard as the device. If it gives you an error that means you forgot to copy usb_desc.h to the right folder or you copied it to the wrong folder. 
  
