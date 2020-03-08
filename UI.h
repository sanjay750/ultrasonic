

#ifndef UI_H
#define UI_H


#define MOTOR_DIRECTION		((unsigned char)2)
#define HANDS_FREE			((unsigned char)1)
#define SELECT_ULTRASONIC	((unsigned char)10)
#define WORK_PAUSE			((unsigned char)9)
#define SELECT_MOTOR		((unsigned char)8)
#define DECRENEMT			((unsigned char)7)
#define INCREMENT			((unsigned char)6)

#define LED_OFF				0
#define LED_ON				1
#define LED_RED				2
#define LED_GREEN			3
#define LED_YELLOW			4

struct uiOutputdData
{
	unsigned char LED_data[6];
	void writeBargraph(unsigned char val);
	void writeLed(unsigned char LED, unsigned char state);
};

struct uiInputdHandel
{
	unsigned char key_no;
	void(*pushed)(void *arg);
	void(*hold)(void *arg);
	void(*released)(void *arg);
	uiInputdHandel *next_link;
};

void uiBegin();
void ui_MainTask(void const *argument);
void event_handler(void const *argument);
void setOutputBuffer(uiOutputdData* Buffer);
void addEvent(uiInputdHandel *Handel);
void addEvent(unsigned char key_no, uiInputdHandel *Handel);
void removeEvent(uiInputdHandel *Handel);
void removeEvent(unsigned char key_no);
#endif // !UI_H
