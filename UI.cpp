

#include <stm32f1xx_hal.h>
#include <../CMSIS_RTOS/cmsis_os.h>
#include "__debug__.h"
#include "max485.h"
#include "UI.h"
#include "timer.h"



//UI output commands
#define LED_STATE		'L'

//UI input command
#define KEY_VAL			'K'



osThreadId uiThreadID;
osThreadDef_t uiTask = 
{ 
	.name = "UI",
	.pthread = ui_MainTask,
	.tpriority = osPriorityNormal,
	.instances = 0,
	.stacksize = configMINIMAL_STACK_SIZE
};

osThreadId eventThreadID;
osThreadDef_t eventTask = 
{ 
	.name = "ET",
	.pthread = event_handler,
	.tpriority = osPriorityNormal,
	.instances = 0,
	.stacksize = configMINIMAL_STACK_SIZE*2
};

uiOutputdData temp = 
{ 
	.LED_data = { }
};

uiOutputdData *outpuTData = &temp;  //Read by ui_MainTask() write by application
uiInputdHandel *InputdHandel; //Read by event_handler() write by application

unsigned char TX_data[10];
unsigned char RX_data[10];
volatile unsigned char resivedKey; //Read by event_handler() write by ui_MainTask()

void uiOutputdData::writeBargraph(unsigned char val)
{
	unsigned short mask = 0xFFFF; //10 bits high
	
	if(val > 10) val = 10;
	

	mask <<= val;
	
	mask = (~mask) & 0x3FF;
	
	LED_data[0] = (mask >> 5) & 0b1111;
	LED_data[1] = ((mask >> 9) & 0b1) | ((mask << 1) & 0b1110);
	LED_data[2] = (LED_data[2] & ~(0b11)) | ((mask >> 3) & 0b11);
}

void uiOutputdData::writeLed(unsigned char LED, unsigned char state)
{
	
	
	switch (LED)
	{
	case 0 ... 3:
		LED_data[4] = (LED_data[4] & ~(1 << LED)) | (state == LED_ON) ? (1 << LED) : (0);
		break;
		
	case 4:
		if (state == LED_RED)
			LED_data[2] = (LED_data[2] & ~(0b1100)) | (1 << 3);
		else if (state == LED_GREEN)
			LED_data[2] = (LED_data[2] & ~(0b1100)) | (1 << 2);
		else if (state == LED_YELLOW)
			LED_data[2] |= (0b1100);
		else
			LED_data[2] &= ~(0b1100);
		break;
		
	case 5:
		if (state == LED_RED)
			LED_data[3] = (LED_data[3] & ~(0b11)) | (1 << 1);
		else if (state == LED_GREEN)
			LED_data[3] = (LED_data[3] & ~(0b11)) | (1 << 0);
		else if (state == LED_YELLOW)
			LED_data[3] |= (0b11);
		else
			LED_data[3] &= ~(0b11);
		break;
		
	case 6:
		if (state == LED_RED)
			LED_data[3] = (LED_data[3] & ~(0b1100)) | (1 << 3);
		else if (state == LED_GREEN)
			LED_data[3] = (LED_data[3] & ~(0b1100)) | (1 << 2);
		else if (state == LED_YELLOW)
			LED_data[3] |= (0b1100);
		else
			LED_data[3] &= ~(0b1100);
		break;
		
	case 7 ... 17:
		
		
		
		break;
	}
}


void uiBegin()
{
	max485Init();
	setRxBuff(RX_data);
	uiThreadID = osThreadCreate(&uiTask, NULL);
	eventThreadID = osThreadCreate(&eventTask, NULL);
}


void addEvent(uiInputdHandel *Handel)
{
	uiInputdHandel *ptr;
	
	if (Handel == 0)
		return;
	
	removeEvent(Handel);
	Handel->next_link = InputdHandel;
	InputdHandel = Handel;
}



void addEvent(unsigned char key_no, uiInputdHandel *Handel)
{
	if (Handel == 0)
		return;
	
	removeEvent(key_no);
	Handel->key_no = key_no;
	addEvent(Handel);
}

void removeEvent(uiInputdHandel *Handel)
{
	if (Handel == 0)
		return;
	
	uiInputdHandel *ptr = InputdHandel, *ptrn_1 = 0;
	
	if (InputdHandel == Handel)
		InputdHandel = InputdHandel->next_link;
	else
	for (ptr = InputdHandel; ptr != 0; ptr = ptr->next_link)
	{
		if (ptr->next_link == Handel)
			ptr->next_link = Handel->next_link;
	}
}

void removeEvent(unsigned char key_no)
{
	uiInputdHandel *ptr = InputdHandel, *ptrn_1 = 0;
	
	if (InputdHandel->key_no == key_no)
		InputdHandel = InputdHandel->next_link;
	else
		for (ptr = InputdHandel; ptr != 0; ptr = ptr->next_link)
		{
			if (ptr->next_link->key_no == key_no)
				ptr->next_link = ptr->next_link->next_link;
		}
}


void updateData()
{
	unsigned char i, LRC, L;
	for (i = 0, LRC = 0; i < 5; i++)
	{
		TX_data[i] = ((unsigned char*)outpuTData)[i];
		LRC += ((unsigned char*)outpuTData)[i];
	}
	
	LRC = -LRC;
	
	TX_data[i++] = LRC;
	
	sendData(TX_data, i);
	while (!dataSent()) ;
	osDelay(50);
	L = dataAvailable();
	if (L > 0)
	{
		for (i = 0, LRC = 0; i < L - 1; i++)
			LRC += RX_data[i];
		
		LRC = -LRC;
		
		if (RX_data[i] == LRC)
		{
			resivedKey = RX_data[1];
		} 
	}	
}

void ui_MainTask(void const *argument)
{
	while (1)
	{
		updateData();
	}
}

unsigned int ref_hold;

uiInputdHandel keyA, keyB, keyC, keyD, keyE;

void pushed(void *arg)
{
	printf("pushed from key");
		
	if ((uiInputdHandel*)arg == &keyA)
	{
		printf("A\n");
		removeEvent(keyB.key_no);
	}
	else if ((uiInputdHandel*)arg == &keyB)
		printf("B\n");
	else if ((uiInputdHandel*)arg == &keyC)
	{
		printf("C\n");
		addEvent(&keyB);
	}
	else if((uiInputdHandel*)arg == &keyD)
	printf("D\n");
	else if((uiInputdHandel*)arg == &keyE)
	printf("E\n");
}

void hold(void *arg)
{
	printf("hold from key");
		
	if ((uiInputdHandel*)arg == &keyA)
		printf("A\n");
	else if ((uiInputdHandel*)arg == &keyB)
		printf("B\n");
	else if ((uiInputdHandel*)arg == &keyC)
		printf("C\n");
	else if ((uiInputdHandel*)arg == &keyD)
		printf("D\n");
	else if ((uiInputdHandel*)arg == &keyE)
		printf("E\n");
}

void released(void *arg)
{
	printf("released from key");
		
	if ((uiInputdHandel*)arg == &keyA)
		printf("A\n");
	else if ((uiInputdHandel*)arg == &keyB)
		printf("B\n");
	else if ((uiInputdHandel*)arg == &keyC)
		printf("C\n");
	else if ((uiInputdHandel*)arg == &keyD)
		printf("D\n");
	else if ((uiInputdHandel*)arg == &keyE)
		printf("E\n");
}

unsigned char bar = 4;

void debug_loop(void const *args)
{
	keyA.hold = hold;
	keyB.hold = hold;
	keyC.hold = hold;
	keyD.hold = hold;
	keyE.hold = hold;
	
	keyA.pushed = pushed;
	keyB.pushed = pushed;
	keyC.pushed = pushed;
	keyD.pushed = pushed;
	keyE.pushed = pushed;
	
	keyA.released = released;
	keyB.released = released;
	keyC.released = released;
	keyD.released = released;
	keyE.released = released;
	
	keyA.next_link = &keyB;
	keyB.next_link = &keyC;
	keyC.next_link = &keyD;
	keyD.next_link = &keyE;
	keyE.next_link = 0;
	
	keyA.key_no = MOTOR_DIRECTION;
	keyB.key_no = HANDS_FREE;
	keyC.key_no = SELECT_ULTRASONIC;
	keyD.key_no = HANDS_FREE;
	keyE.key_no = SELECT_MOTOR;
	
	InputdHandel = &keyA;
	
	while (1)
	{
		printf("bar = %d\n", bar);
		
		temp.writeLed(bar, LED_RED);
		printf("color = RED\n");
		osDelay(500);
		temp.writeLed(bar, LED_GREEN);
		printf("color = GREEN\n");
		osDelay(500);
		temp.writeLed(bar, LED_YELLOW);
		printf("color = YELLOW\n");
		osDelay(500);
		
		bar++;
		if (bar > 6) bar = 4;
	}
}


void event_handler(void const *argument)
{
	uiInputdHandel *ptr;
	unsigned char key;
	
	while (1)
	{
		osDelay(100);
start:	
		while (resivedKey == 0xFF) ;
		
		key = resivedKey;
		ptr = InputdHandel;
	
		while (1)
		{
			if (ptr == 0)
			{
				printf("unknown key -> %d\n", resivedKey);
				while (resivedKey != 0xFF) ;
				goto start;
			}
			
			if (ptr->key_no == resivedKey)
			{
				if (ptr->pushed != 0)
				{
					ptr->pushed(ptr);
					break;
				}
			}
			
			ptr = ptr->next_link;
		}
		
		ref_hold = millis();
		
		while (resivedKey == key)
		{
			if (millis() - ref_hold > 500)
			{
				if (ptr->hold != 0)
					ptr->hold(ptr);
				ref_hold = millis();
			}
		}
		
		if (ptr->released != 0)
			ptr->released(ptr);
	}
}
