

#ifndef ULTRASONIC_APP_H
#define ULTRASONIC_APP_H

void ultrasonicInit();
void ultrasonicHandler();
unsigned short findResonencePwm();
void setFrequency(unsigned short pwm);
void runUltrasonic();
void stopUltrasonic();
unsigned char ultrasonicEvents();

#endif // !ULTRASONIC_APP_H
