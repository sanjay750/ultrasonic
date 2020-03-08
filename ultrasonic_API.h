

#ifndef ULTRASONIC_API_H
#define ULTRASONIC_API_H

void ultrasonic_HardwareInit();

void ultrasonic_DriverEnable();
void ultrasonic_DriverDisable();
void ultrasonic_CollingFanEnable();
void ultrasonic_collingFanDisable();

void ultrasonic_OffsetEnable();
void ultrasonic_OffsetDisable();

unsigned short ultrasonic_ReadOffset_CH();
unsigned short ultrasonic_ReadPhase_CH();

bool ultrasonic_OfsetLocked();

void ultrasonic_Setrequency(unsigned short val);

//input range 0 to 100 sets wiper of digital pot
void ultrasonic_SetCurrent(unsigned char val);

#endif // !ULTRASONIC_API_H
