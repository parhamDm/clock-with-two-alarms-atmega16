#include <avr/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define SegDataPort    PORTC
#define SegDataPin    PINC
#define SegDataDDR    DDRC

#define SegCntrlPort  PORTA
#define SegCntrlPin    PINA
#define SegCntrlDDR    DDRA


/*Global Variables Declarations*/
unsigned char hours = 0;
unsigned char minutes = 0;
unsigned char seconds = 0;
unsigned char day = 1;
unsigned char month = 1;
unsigned char year = 97;
unsigned char minutesAlarmStart0 = 10;
unsigned char hoursAlarmStart0 =11;
unsigned char minutesAlarmFinish0 = 12;
unsigned char hoursAlarmFinish0 = 13;
unsigned char minutesAlarmStart1 = 14;
unsigned char hoursAlarmStart1 = 15;
unsigned char minutesAlarmFinish1 = 16;
unsigned char hoursAlarmFinish1 = 17;
int isTime = 1;
int setAlarm = 0;
int stateTable[6][3]={
	{1,2,4},
	{0,2,4},
	{-1,3,-1},
	{-1,0,-1},
	{-1,-1,5},
	{-1,-1,0}
};
int cs=0;
unsigned char DigitTo7SegEncoder(unsigned char digit);
void changeHour(unsigned char&);
void changeMinute(unsigned char&);
void alarm(void);
void state0();
void state1();
void state2();
void state3();
void state4();
void state5();
void showNumber(unsigned char h,unsigned char m,unsigned char s);
ISR(TIMER1_COMPA_vect);

/*Main Program*/
/*****************************************************************************/
int main(void)
{
	SegDataDDR = 0xFF;
	SegCntrlDDR = 0xFF;
	SegCntrlPort = 0xFF;

	
	TCCR1B = (1<<CS12|1<<WGM12);
	OCR1A = 15625-1;
	TIMSK = 1<<OCIE1A;
	
	DDRB = 0x00;
	PORTB =0xFF;
	DDRD = 0xFF;
	//PINB = 0xFF;

	sei();
	
	
	while(1)
	{
		alarm();
		if((PINB & 0x01)==0 & stateTable[cs][0]!=-1){
			cs=stateTable[cs][0];
			_delay_ms(1000);
		}
		else if((PINB & 0x02)==0 & stateTable[cs][1]!=-1){
			cs=stateTable[cs][1];
			_delay_ms(1000);
		}
		else if((PINB & 0x04)==0 & stateTable[cs][2]!=-1){
			cs=stateTable[cs][2];
			_delay_ms(1000);
		}

		switch(cs){
			case 0: state0();
			break;
			case 1: state1();
			break;
			case 2: state2();
			break;
			case 3: state3();
			break;
			case 4: state4();
			break;
			case 5: state5();
			break;
		}
	}
	return 0;
}
void state0(){
	changeMinute(minutes);
	changeHour(hours);
	showNumber(hours,minutes,seconds);
}

void state1(){
	showNumber(year,month,day);
}

void state2(){
	changeMinute(minutesAlarmStart0);
	changeHour(hoursAlarmStart0);
	showNumber(hoursAlarmStart0,minutesAlarmStart0,0);
}

void state3(){
	changeMinute(minutesAlarmFinish0);
	changeHour(hoursAlarmFinish0);
	showNumber(hoursAlarmFinish0,minutesAlarmFinish0,0);
}
void state4(){
	changeMinute(minutesAlarmStart1);
	changeHour(hoursAlarmStart1);
	showNumber(hoursAlarmStart1,minutesAlarmStart1,0);
}
void state5(){
	changeMinute(minutesAlarmFinish1);
	changeHour(hoursAlarmFinish1);
	showNumber(hoursAlarmFinish1,minutesAlarmFinish1,0);
}
void changeMinute(unsigned char &m){
	if((PINB & 0x08)==0){
		m++;
		if(m == 60){
			m = 0;
		}
		_delay_ms(1000);
	}
}

void changeHour(unsigned char &h){
	if((PINB & 0x10)==0){
		h++;
		if(h == 24){
			h = 0;
		}
		_delay_ms(1000);
	}
}

void showNumber(unsigned char h,unsigned char m,unsigned char s){
	SegDataPort = DigitTo7SegEncoder(s%10);
	SegCntrlPort = ~0x01;
	_delay_ms(1000);
	SegDataPort = DigitTo7SegEncoder(s/10);
	SegCntrlPort = ~0x02;
	_delay_ms(1000);
	SegDataPort = DigitTo7SegEncoder(m%10);
	SegCntrlPort = ~0x04;
	_delay_ms(1000);
	SegDataPort = DigitTo7SegEncoder(m/10);
	SegCntrlPort = ~0x08;
	_delay_ms(1000);
	SegDataPort = DigitTo7SegEncoder(h%10);
	SegCntrlPort = ~0x10;
	_delay_ms(1000);
	SegDataPort = DigitTo7SegEncoder(h/10);
	SegCntrlPort = ~0x20;
	_delay_ms(1000);

}


void alarm(){
	if (hours==hoursAlarmStart0 && minutes == minutesAlarmStart0)
	{
		PORTD =0xFF;
		}else{
		PORTD =0x00;
	}

}
unsigned char DigitTo7SegEncoder(unsigned char digit)
{
	unsigned char SegVal;
	
	switch(digit)
	{
		case 0:  SegVal = 0b00111111;
		break;
		case 1: SegVal = 0b00000110;
		break;
		case 2:  SegVal = 0b01011011;
		break;
		case 3:  SegVal = 0b01001111;
		break;
		case 4:  SegVal = 0b01100110;
		break;
		case 5:  SegVal = 0b01101101;
		break;
		case 6:  SegVal = 0b01111101;
		break;
		case 7:  SegVal = 0b00000111;
		break;
		case 8:  SegVal = 0b01111111;
		break;
		case 9:  SegVal = 0b01101111;
	}
	return SegVal;
}

ISR(TIMER1_COMPA_vect)
{
	seconds++;
	if(seconds == 60){
		seconds = 0;
		minutes++;
	}
	if(minutes == 60){
		minutes = 0;
		hours++;
	}
	if(hours > 23){
		hours = 0;
		day++;
	}
	if(day == 31 && month > 7){
		day = 1;
		month++;
	}
	if(day == 32 && month < 7){
		day = 1;
		month++;
	}
	if(month == 13){
		month = 1;
		year++;
	}
	if(year > 99){
		year = 97;
	}
	
}