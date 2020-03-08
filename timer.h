


#ifndef TIMER_H
#define TIMER_H

void timerBegin();
unsigned int millis();
unsigned int micros();
bool linkTask(uint8_t place, void (*task)(), uint16_t timing);
void enableTask(uint8_t place);
void disableTask(uint8_t place);
void delay(unsigned int del);

#endif // !TIMER_H
