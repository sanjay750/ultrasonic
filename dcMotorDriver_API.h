

#ifndef MOTOR_DRIVER_API_H
#define MOTOR_DRIVER_API_H


void motorDriverBegin();

float readVoltage();
float readSpeed();

void enableSpeedControl();
void disableSpeedControl();

void setSpeed(unsigned short speed);

void setHbridge(unsigned char mode);

void set_voltage(unsigned short volt);
void enableVoltageControl();
void disableVoltageControl();

void enableSpeedControl();
void disableSpeedControl();


#endif // !MOTOR_DRIVER_API_H
