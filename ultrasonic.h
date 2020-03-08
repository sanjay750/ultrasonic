


#ifndef ULTRASONIC_H
#define ULTRASONIC_H

void ultrasonicInit();
void ultrasonic_MainTask(void const *argument);
bool ultrasonicRunning();
void startUltrasonic(unsigned char mode = 0);
void stopUltrasonic();

void ultrasonic_workPause(void *args);
void ultrasonic_selectUltrasonic(void *args);	
	
#endif // !ULTRASONIC_H
