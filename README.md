# CSE321_part3
-----------------------------------------------------------------------------------------
BABY BREATHING MONITORING SYSTEM
-----------------------------------------------------------------------------------------
According to kidshealth.org, the leading cause of death in infants between 1 month and 1 
year is SIDS, Sudden Infant Death Syndrome. “Most SIDS deaths are associated with sleep, 
which is why it’s sometimes called ‘crib death’” (kidshealth.org). This system works to
give parents peace of mind when their baby is sleeping by detecting the baby's breathing
movements and alerting the parent when an anomoly has occurred so that life-saving 
measures can be taken in a timely manner. 

What the system does:
=========================================================================================
Once the system is turned on, the accelerometer's z-axis is constantly checked to ensure
the baby's chest is rising and falling as he/she breathes (if the system is ON, it is
assumed that the MPU65050 has been placed on the baby's chest). Each rise and fall is 
counted as one complete breath. Breaths are counted over 10 second increments, which is 
used to find the approximate breaths per minute, which is displayed on the SSD1306 OLED 
display. If the bpm is irregular, the red LED will light to alert the parent of an issue.
If the bpm is 0, the red LED will light and the buzzer will sound to alert the parent 
that apnea has occurred and life-saving measures must be taken.

How to setup the system:
=========================================================================================
Our system consists of one Arduino Uno, two LED's and 220ohm resistors, one buzzer, two 
push buttons, one MPU6050, one SSD1306, and one breadboard with connecting wires. Assuming 
the system is not assembled, you need to ensure the following connections on the Arduino:
Red (alert) LED anode -> pin 7
Red (alert) LED cathode -> GND
Green (normal) LED anode -> pin 6
Green (normal) LED cathode -> GND
Buzzer positive pin -> pin 8
Buzzer negative pin -> GND
Green (start) push button 1.r -> pin 5
Green (start) push button 2.r -> GND
Red (stop) push bitton 1.r -> pin 3
Red (stop) push button 2.r -> GND
MPU6050 SDA -> A4 pin
MPU6050 SCL -> A5 pin
MPU6050 GND -> GND
MPU6050 VCC -> 3.3V
SSD1306 SDA -> A4 pin
SSD1306 SCL -> A5 pin
SSD1306 GND -> GND
SSD1306 VCC -> 3.3V

How to use and interact with the system:
=========================================================================================
Securely place the MPU6050 on the baby's chest. Before the system is turned on, you will 
see "_Baby Breathing Monitor_" title. To turn the system on, press the green (start) push 
button. You will see "Monitoring..." to show the system is on and currently monitoring 
the baby's breathing. The system will run continuously until you turn the system off by 
pushing the red (stop) push button. You will see "bpm: x" where x is the current 
apporximately calcualted breaths per minute. The breaths per minute of a baby should be 
30-40 when sleeping and 40-60 when awake. If the bpm is out of this range, you will see 
"irregular bpm!!" and the red LED will light. If the bpm is 0, no breathing movement is 
detected and the buzzer will trigger. You should take immediate life-saving measures or 
seek immediate medical attention for the baby if breathing has stopped. To reset the 
system or turn the system off, press the red (stop) push button. The system will stay off 
until you turn it on again.
