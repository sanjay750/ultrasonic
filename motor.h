

#ifndef MOTOR_H
#define MOTOR_H

void motorBegin();
void motorTask(void const *args);

void startMotor();
void stopMotor();
void setSpeed(unsigned short speed);
void setDirection(unsigned char direction);
void selectMotor(unsigned char motor);

void motor_workPause(void *args);
void motor_motorSelect(void *args);

#endif // !MOTOR_H
