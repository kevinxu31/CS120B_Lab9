/*	Author: lab
 *  Partner(s) Name: Luofeng Xu
 *	Lab Section:022
 *	Assignment: Lab 9  Exercise 4
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 * 	Demo Link: Youtube URL>https://youtu.be/KBSvZf_mqrQ
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

task tasks[5];
const unsigned short tasksNum=5;
const unsigned long tasksPeriod=1;





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


unsigned char x=2;
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
unsigned char sound;
unsigned char s;
enum Speaker{V_start,Vw,Vh,Vl};
int Tick_V(int V_state){
	switch(V_state){
		case V_start:
			V_state=Vw;
			break;
		case Vw:
			if((~PINA & 0x04) == 0x04){
				s=0;
				V_state=Vh;
				break;
			}
			else if((~PINA&0x04)==0x00){
				V_state=Vw;
				break;
			}
		case Vh:
			if(((~PINA&0x04)==0x04)&&s<x){
				V_state=Vh;
				break;
			}
			else if(((~PINA&0x04)==0x04)&&s>=x){
                                V_state=Vl;
                                break;
                        }

			else if((~PINA&0x04)==0x00){
				V_state=Vw;
				sound=0;
				break;
			}
		case Vl:
                        if(((~PINA&0x04)==0x04)&&s<(x+x)){
                                V_state=Vl;
                                break;
                        }
			else if(((~PINA&0x04)==0x04)&&s>=(x+x)){
				s=0;
                                V_state=Vh;
                                break;
                        }

                        else if((~PINA&0x04)==0x00){
                                V_state=Vw;
                                sound=0;
                                break;
                        }

		default:
			break;
	}
	switch(V_state){
		case V_start:
			break;
		case Vw:
			sound=0;
			s=0;
			break;
		case Vh:
			sound=0x10;
			s=s+1;
			break;
		case Vl:
			sound=0x00;
			s=s+1;
			break;
		default:
			break;
	}
	return V_state;
}

enum Turning{T_start,RA,PA_0,PA_1};
int Tick_T(int T_state){
	unsigned char A0=~PINA&0x01;
	unsigned char A1=(~PINA>>1)&0x01;
	switch(T_state){
		case T_start:
			T_state=RA;
			break;
		case RA:
			if(A0&(!A1)){
				T_state=PA_0;
				if(x<9){x=x+1;}
			}
			else if((!A0)&A1){
				T_state=PA_1;
				if(x>1){x=x-1;}
			}
			else{T_state=RA;}
			break;
		case PA_0:
			if(A0&(!A1)){T_state=PA_0;}
			else if((!A0)&(!A1)){T_state=RA;}
			break;
                case PA_1:
                        if(A1&(!A0)){T_state=PA_1;}
                        else if((!A0)&(!A1)){T_state=RA;}
                        break;
		default:
			break;
	}
	return T_state;
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
			PORTB=(threeLEDs|(blinkingLED<<3)|sound);
			break;
		default:
			break;
	}
	return C_state;
}


int main() {
	DDRA=0x00;PORTA=0xFF;
	DDRB=0xFF;PORTB=0x00;
	unsigned char i=0;
	tasks[i].state=TL_start;
	tasks[i].period=300;
	tasks[i].elapsedTime=0;
	tasks[i].TickFct=&Tick_TL;
	i++;
	tasks[i].state=BL_start;
        tasks[i].period=1000;
        tasks[i].elapsedTime=0;
        tasks[i].TickFct=&Tick_BL;
        i++;
	tasks[i].state=T_start;
        tasks[i].period=100;
        tasks[i].elapsedTime=0;
        tasks[i].TickFct=&Tick_T;
	i++;
	tasks[i].state=V_start;
        tasks[i].period=1;
        tasks[i].elapsedTime=0;
        tasks[i].TickFct=&Tick_V;
        i++;
	tasks[i].state=C_start;
        tasks[i].period=1;
        tasks[i].elapsedTime=0;
        tasks[i].TickFct=&Tick_C; 
	TimerSet(1);
	TimerOn();
	while (1) {
	}
	return 0;
}
