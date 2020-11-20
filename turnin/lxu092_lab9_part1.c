/*	Author: lab
 *  Partner(s) Name: Luofeng Xu
 *	Lab Section:022
 *	Assignment: Lab 9  Exercise 1
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 * 	Demo Link: Youtube URL>https://youtu.be/cf-DfHqyMwA
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif


typedef struct task{
        int state;
        unsigned long period;
        unsigned long elapsedTime;
        int(*TickFct)(int);
}task;

task tasks[3];
const unsigned short tasksNum=3;
const unsigned long tasksPeriod=1000;





volatile unsigned char TimerFlag = 0; 
unsigned long _avr_timer_M = 1; 
unsigned long _avr_timer_cntcurr = 0; 
void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}
void TimerOn() {
	TCCR1B 	= 0x0B;	
	OCR1A 	= 125;	
	TIMSK1 	= 0x02; 
	TCNT1 = 0;
	_avr_timer_cntcurr = _avr_timer_M;	
	SREG |= 0x80;
}

void TimerOff() {
	TCCR1B 	= 0x00; 
}

void TimerISR() {
	unsigned char i;
	for(i=0;i<tasksNum;++i){
		if(tasks[i].elapsedTime>=tasks[i].period){
			tasks[i].state=tasks[i].TickFct(tasks[i].state);
			tasks[i].elapsedTime=0;
		}
		tasks[i].elapsedTime+=tasksPeriod;
	}
}
ISR(TIMER1_COMPA_vect)
{
	_avr_timer_cntcurr--;
	if (_avr_timer_cntcurr == 0) { 	
		TimerISR(); 				
		_avr_timer_cntcurr = _avr_timer_M;
	}
}



unsigned char t;
unsigned char threeLEDs;
unsigned char blinkingLED;
enum ThreeLEDsSM{TL_start,TL};
int Tick_TL(int TL_state){
	switch(TL_state){
		case TL_start:
			t=0;
			TL_state=TL;
			break;
		case TL:
			TL_state=TL;
			t=t+1;
			break;
		default:
			break;
	}
	switch(TL_state){
		case TL_start:
			break;
		case TL:
			threeLEDs=(0x01<<(t%3));
			break;
		default:
			break;
	}
	return TL_state;
}
enum BlinkingLEDSM{BL_start,BL};
int Tick_BL(int BL_state){
	switch(BL_state){
		case BL_start:
			BL_state=BL;
			blinkingLED=1;
			break;
		case BL:
			BL_state=BL;
			break;
		default:
			break;
	}
	switch(BL_state){
		case BL_start:
			break;
		case BL:
			blinkingLED=!blinkingLED;
			break;
		default:
			break;
	}
	return BL_state;
}
enum CombineLEDsSM{C_start,C};
int Tick_C(int C_state){
	switch(C_state){
		case C_start:
			C_state=C;
			break;
		case C:
			C_state=C;
			break;
		default:
			break;
	}
	switch(C_state){
		case C_start:
			break;
		case C:
			PORTB=threeLEDs|(blinkingLED<<3);
			break;
		default:
			break;
	}
	return C_state;
}


int main() {
	DDRB=0xFF;PORTB=0x00;
	unsigned char i=0;
	tasks[i].state=TL_start;
	tasks[i].period=1000;
	tasks[i].elapsedTime=0;
	tasks[i].TickFct=&Tick_TL;
	i++;
	tasks[i].state=BL_start;
        tasks[i].period=1000;
        tasks[i].elapsedTime=0;
        tasks[i].TickFct=&Tick_BL;
        i++;
	tasks[i].state=C_start;
        tasks[i].period=1000;
        tasks[i].elapsedTime=0;
        tasks[i].TickFct=&Tick_C; 
	TimerSet(1000);
	TimerOn();
	while (1) {
	}
	return 0;
}
