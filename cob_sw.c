/*****************************************************
Project : Korail EMU 138R
Version : 0
Date    : 2008-09-29
Author  : TTIB
Company : Goguryo Engineering Co.,Ltd.
Comments: 


Chip type           : ATmega16
Program type        : Application
Clock frequency     : 16.000000 MHz
Memory model        : Small
External SRAM size  : 0
Data Stack size     : 256
*****************************************************/

#include <mega16.h>
#include "cob_sw.h"

// Timer 0 overflow interrupt service routine
interrupt [TIM0_OVF] void timer0_ovf_isr(void)
{
// Reinitialize Timer 0 value
TCNT0+=0xE7;
// Place your code here
	count10msec++;
	if(count10msec >= 100){
		count10msec = 0;
		flag10msec = 1;
		
		if(flagCtEm)
		{
			CT_EM = 0;
			countEmReOut++;
			if(countEmReOut >= EM_RE_OUT_TIME)
			{
				countEmReOut = 0;
				flagCtEm = 0;
			}
		}
		else
		{
			CT_EM = 1;
		}
		
		if(countWaitCab < WAIT_CAB_TIME)
		{
			countWaitCab++;
		}
	}

}

// Declare your global variables here

void main(void)
{
// Declare your local variables here

// Input/Output Ports initialization
// Port A initialization
// Func7=Out Func6=Out Func5=Out Func4=Out Func3=Out Func2=Out Func1=Out Func0=Out 
// State7=1 State6=1 State5=1 State4=1 State3=1 State2=1 State1=1 State0=1 
PORTA=0xF8;
DDRA=0xFF;

// Port B initialization
// Func7=In Func6=In Func5=In Func4=In Func3=In Func2=Out Func1=Out Func0=Out 
// State7=P State6=P State5=P State4=P State3=P State2=1 State1=1 State0=1 
PORTB=0xFc;
DDRB=0x07;

// Port C initialization
// Func7=Out Func6=Out Func5=In Func4=In Func3=In Func2=In Func1=In Func0=In 
// State7=1 State6=1 State5=P State4=P State3=P State2=P State1=P State0=P 
PORTC=0xFF;
DDRC=0xC0;

// Port D initialization
// Func7=In Func6=In Func5=In Func4=In Func3=In Func2=In Func1=In Func0=In 
// State7=P State6=P State5=P State4=P State3=P State2=P State1=P State0=P 
PORTD=0xFF;
DDRD=0x00;

// Timer/Counter 0 initialization
// Clock source: System Clock
// Clock value: 250.000 kHz
// Mode: Normal top=FFh
// OC0 output: Disconnected
TCCR0=0x03;
TCNT0=0xE7;
OCR0=0x00;

// Timer/Counter 1 initialization
// Clock source: System Clock
// Clock value: Timer 1 Stopped
// Mode: Normal top=FFFFh
// OC1A output: Discon.
// OC1B output: Discon.
// Noise Canceler: Off
// Input Capture on Falling Edge
// Timer 1 Overflow Interrupt: Off
// Input Capture Interrupt: Off
// Compare A Match Interrupt: Off
// Compare B Match Interrupt: Off
TCCR1A=0x00;
TCCR1B=0x00;
TCNT1H=0x00;
TCNT1L=0x00;
ICR1H=0x00;
ICR1L=0x00;
OCR1AH=0x00;
OCR1AL=0x00;
OCR1BH=0x00;
OCR1BL=0x00;

// Timer/Counter 2 initialization
// Clock source: System Clock
// Clock value: Timer 2 Stopped
// Mode: Normal top=FFh
// OC2 output: Disconnected
ASSR=0x00;
TCCR2=0x00;
TCNT2=0x00;
OCR2=0x00;

// External Interrupt(s) initialization
// INT0: Off
// INT1: Off
// INT2: Off
MCUCR=0x00;
MCUCSR=0x00;

// Timer(s)/Counter(s) Interrupt(s) initialization
TIMSK=0x01;

// Analog Comparator initialization
// Analog Comparator: Off
// Analog Comparator Input Capture by Timer/Counter 1: Off
ACSR=0x80;
SFIOR=0x00;

flagEm = 0x00;
flagIp = 0x00;

SGCNT01 =1;
SGCNT02 =1;


CarType = ID_CREW;

// Global enable interrupts
#asm("sei")

tempint = 0;
while(tempint < 800){
	if(flag10msec){
		flag10msec = 0;
		tempint++;
		
		temp = 0x00;
		if(!CAR_PORT1){
			temp |= CAR1;
			//LED_LEFT = 0;
		}
		if(!CAR_PORT2){
			temp |= CAR2;
			//LED_RIGHT = 0;
		}

		CarBuff[2] = CarBuff[1];
		CarBuff[1] = CarBuff[0];
		CarBuff[0] = temp;
	}
}

while (1)
      {
      // Place your code here
	Proc10msec();
      };
}

void Proc10msec(void)
{
	if(flag10msec){
		flag10msec = 0;
		CarTypeProc();
		//if(CarType == KORAIL){
		if(CarType == ID_CREW){
			CrewProc();
		}
		else{		// else if(CarType == SEOULMETRO){
			//MetroProc();
			CabProc();
		}
	}
}

void CarTypeProc(void)
{
	temp = 0x00;
	if(!CAR_PORT1){
		temp |= CAR1;
		//LED_LEFT = 0;
	}
	if(!CAR_PORT2){
		temp |= CAR2;
		//LED_RIGHT = 0;
	}

	CarBuff[2] = CarBuff[1];
	CarBuff[1] = CarBuff[0];
	CarBuff[0] = temp;
	if((CarBuff[2] == CarBuff[1]) && (CarBuff[1] == CarBuff[0])){
		if(CarBuff[3] != CarBuff[0]){
			CarBuff[3] = CarBuff[0];
			if((CarBuff[3] & CAR1) || (CarBuff[3] & CAR2)){
				//CarType = KORAIL;
				CarType = ID_CREW;
			}
			else{
				//CarType = SEOULMETRO;
				CarType = ID_CAB;
			}
		}
	}
}

void CrewProc(void)
{
	if((PdaBuff[3] == 0x03) && !flagIp && !flagEm && CALL_EM){
		LED_EM = 0;
		LED_CAB = 0;
		
		//CT_EM = 1;
	}
	else{
		EmLedProc();
		CabLedProc();
	}
	//MoniOnProc();
	EmSwProc();
	CabSwProc();
	BrSwProc();
	MicProc();
	LedProc();
	/*if((flagEm == 0x01) && (countEmDelay<150)){
		countEmDelay++;
		if(PdaBuff[3] == 0x03){
			PdaBuff[0] = 0x00;
			PdaBuff[1] = 0x00;
			PdaBuff[2] = 0x00;
			PdaBuff[3] = 0x00;
		}
	}*/
	//else{
		PdaProc();
	//}
	if(!Brflag && (NoSwIncount < NO_IN_TIMEOUT)){
		NoSwIncount++;
		if(NoSwIncount >= NO_IN_TIMEOUT){
			NoSwIncount = NO_IN_TIMEOUT;
			BrWaitBuff = 0x00;
		}
	}

	if(countPdaEmDelay < 200){
		countPdaEmDelay++;
	}
}

void EmLedProc(void)
{
	if(!CALL_EM){
		CallEmcount++;
		if(CallEmcount >= LED_DEBOUNCE_CREW){
			CallEmcount = LED_DEBOUNCE_CREW;
			
			BrWaitBuff = 0x00;
			if(!(flagEm & 0x01) && EmRqOff){
				flagEm = 0x01;
				
				// 운전실 통화중 대비
				CT_EM = 1;
				
				// 바로 점등 가능하도록 카운터 보정
				countEmBlink = 49;
				
				countPdaEmDelay = 0;
				
				//countEmDelay = 0;
			}
			
			EmRqOff = 0;

		}
	}
	else{
		if(CallEmcount != 0){
			CallEmcount--;
		}
		else
		{
			EmRqOff = 1;
		}
	}
	
	// 비상호출
	if(flagEm == 0x01){
		countEmBlink++;
		if(countEmBlink == 50){
			LED_EM = 1;
		}
		else if(countEmBlink >= 100){
			countEmBlink = 0;
			LED_EM = 0;
		}
	}
	// 비상통화
	else if(flagEm){
		LED_EM = 1;
	}
	// normal
	else{
		LED_EM = 0;
	}
}

void CabLedProc(void)
{
	if(flagIp & 0x10){
		LED_CAB = 1;
		/*if(!flagEm){
			CT_EM = 0;
		}*/
	}
	else{
		LED_CAB = 0;
		/*if(!flagEm){
			CT_EM = 1;
		}*/
	}
}

void EmSwProc(void)
{
	if(!SW_EM){
		SwEmcount++;
		if(SwEmcount >= SW_DEBOUNCE){
			SwEmcount = SW_DEBOUNCE;
			if(flagEm == 0x01){
				flagEm |= 0x10;
				countEmReOut = 0;
				flagCtEm = 1;
				//CT_EM = 0;
				
				countWaitEm = 0;
			}
			else if(flagEm && (countWaitEm >= WAIT_EM_TIME)){
				flagEm = 0x00;
				//CT_EM = 1;
				countEmReOut = 0;
				flagCtEm = 1;
				
				countWaitCab = 0;
			}
		}
	}
	else{
		if(SwEmcount != 0){
			SwEmcount--;
		}
		if(SwEmcount == 0){
			//CT_EM = 1;
		}
	}

	if(countWaitEm < WAIT_EM_TIME){
		countWaitEm++;
	}
}
void CabSwProc(void)
{
	if((!SW_CAB) && (countWaitCab >= WAIT_CAB_TIME)){
		SwCabcount++;
		if(SwCabcount >= SW_DEBOUNCE){
			if(SwCabcount == SW_DEBOUNCE){
				if(!flagEm){
					if(flagIp & 0x10){
						flagIp &= 0xef;
						if(flagIp & 0x01){
							flagIp |= 0x20;
						}
					}
					else{
						flagIp |= 0x10;
						BrWaitBuff = 0x00;
					}
					countEmReOut = 0;
					flagCtEm = 1;
				}
				else{
					flagIp &= 0xef;
				}
			}
				
			SwCabcount = SW_DEBOUNCE;
			//CT_CAB = 0;
		}
	}
	else{
		if(SwCabcount != 0){
			//SwCabcount--;
			SwCabcount = 0;
		}
		/*if(SwCabcount == 0){
			//CT_CAB = 1;
		}*/
	}
}
void BrSwProc(void)
{
	temp = 0x00;
	if(!SW_ROOM)
		temp |= ROOM;
	if(!SW_LEFT)
		temp |= LEFT;
	if(!SW_RIGHT)
		temp |= RIGHT;
	
	BrSwBuff[2] = BrSwBuff[1];
	BrSwBuff[1] = BrSwBuff[0];
	BrSwBuff[0] = temp;
	
	if((BrSwBuff[2] == BrSwBuff[1]) && (BrSwBuff[1] == BrSwBuff[0])){
		if(BrSwBuff[3] != BrSwBuff[0]){
			temp = BrSwBuff[0] ^ BrSwBuff[3];
			// 차내 스위치
			if((temp & ROOM) && (BrSwBuff[0] & ROOM)){
				if(BrWaitBuff & ROOM){
					BrWaitBuff &= ~ROOM;
				}
				else{
					BrWaitBuff |= ROOM;
					if(flagIp & 0x10){
						flagIp &= 0xef;
						flagIp |= 0x20;
					}
					//BrWaitBuff &= ~(LEFT | RIGHT);
				}
			}
			if((temp & LEFT) && (BrSwBuff[0] & LEFT)){
				if(BrWaitBuff & LEFT){
					BrWaitBuff &= ~LEFT;
				}
				else{
					BrWaitBuff |= LEFT;
					if(flagIp & 0x10){
						flagIp &= 0xef;
						flagIp |= 0x20;
					}
					//BrWaitBuff &= ~ROOM;
				}
			}
			if((temp & RIGHT) && (BrSwBuff[0] & RIGHT)){
				if(BrWaitBuff & RIGHT){
					BrWaitBuff &= ~RIGHT;
				}
				else{
					BrWaitBuff |= RIGHT;
					if(flagIp & 0x10){
						flagIp &= 0xef;
						flagIp |= 0x20;
					}
					//BrWaitBuff &= ~ROOM;
				}
			}
			BrSwBuff[3] = BrSwBuff[0];
			NoSwIncount = 0;
			Blinkcount = 0;
		}
	}
}
void MicProc(void)
{
	if(!SW_MIC){
		SwMiccount++;
		if(SwMiccount >= SW_DEBOUNCE){
			SwMiccount = SW_DEBOUNCE;
			NoSwIncount = 0;
			if(BrWaitBuff & ROOM){
				CT_ROOM = 0;
				Brflag = 1;
			}
			else{
				CT_ROOM = 1;
			}
			if(BrWaitBuff & LEFT){
				CT_LEFT = 0;
				Brflag = 1;
			}
			else{
				CT_LEFT = 1;
			}
			if(BrWaitBuff & RIGHT){
				CT_RIGHT = 0;
				Brflag = 1;
			}
			else{
				CT_RIGHT = 1;
			}
		}
	}
	else{
		if(SwMiccount != 0){
			SwMiccount--;
		}
		if(SwMiccount == 0){
			CT_ROOM = 1;
			CT_LEFT = 1;
			CT_RIGHT = 1;
			Brflag = 0;
		}
	}
}

void LedProc(void)
{
	if(Brflag){
		SGCNT01 =0;
		SGCNT02 =0;
		if(BrWaitBuff & ROOM){
			LED_ROOM = 1;
		}
		else{
			LED_ROOM = 0;
		}
		if(BrWaitBuff & LEFT){
			LED_LEFT = 1;
		}
		else{
			LED_LEFT = 0;
		}
		if(BrWaitBuff & RIGHT){
			LED_RIGHT = 1;
		}
		else{
			LED_RIGHT = 0;
		}
	}
	else{
		SGCNT01 = 1;
		if(flagIp || (flagEm & 0x10) || (PdaBuff[0] == 0x03)){
			SGCNT02 =0;
		}
		else{
			SGCNT02 =1;
		}
		/*if(flagIp & 0x10){
			SGCNT01 =0;
		}*/
		
		Blinkcount++;
		if(Blinkcount < 50){
			if(BrWaitBuff & ROOM){
				LED_ROOM = 1;
			}
			else{
				LED_ROOM = 0;
			}
			if(BrWaitBuff & LEFT){
				LED_LEFT = 1;
			}
			else{
				LED_LEFT = 0;
			}
			if(BrWaitBuff & RIGHT){
				LED_RIGHT = 1;
			}
			else{
				LED_RIGHT = 0;
			}
		}
		else if(Blinkcount < 100){
			LED_ROOM = 0;
			LED_LEFT = 0;
			LED_RIGHT = 0;
		}
		else{
			Blinkcount = 0;
		}
	}
}
void PdaProc(void)
{
unsigned char val=0;
	if(TTL_OUT0)
		val |= 0x01;
	if(TTL_OUT1)
		val |= 0x02;
	
	PdaBuff[0] = PdaBuff[1];
	PdaBuff[1] = PdaBuff[2];
	PdaBuff[2] = val;
	
	if((PdaBuff[0] == PdaBuff[1]) && (PdaBuff[1] == PdaBuff[2])){
		if(PdaBuff[0] != PdaBuff[3]){
			countPdaDebounce++;
			if(countPdaDebounce >= 7){
				countPdaDebounce = 7;
				PdaBuff[3] = PdaBuff[0];
			}
		}
		if(PdaBuff[0] == PdaBuff[3]){
			if(PdaBuff[0] != 0x03){
				if(PdaBuff[0] & 0x01){
					flagIp |= 0x01;
					if(!(flagIp & 0x20)){
						flagIp |= 0x10;
						
						if(countEmReOut)
						{
							countEmReOut = EM_RE_OUT_TIME;
						}
					}
				}
				else{
					if(flagIp & 0x01){
						flagIp = 0x00;
						
						if(countEmReOut)
						{
							countEmReOut = EM_RE_OUT_TIME;
						}
					}
				}
			
				if(PdaBuff[0] & 0x02){
					if(flagEm & 0x01){
						if(!(flagEm & 0x10)){
							countWaitEm = 0;
							flagEm |= 0x10;
							//CT_EM = 0;
						}
						flagEm |= 0x02;
						
						if(countEmReOut)
						{
							countEmReOut = EM_RE_OUT_TIME;
						}
					}
				}
				else{
					if((flagEm & 0x10) && (flagEm & 0x02)){
						flagEm = 0x00;
						//CT_EM = 1;
						
						if(countEmReOut)
						{
							countEmReOut = EM_RE_OUT_TIME;
						}
						
						countWaitCab = 0;
					}
				}
			}
			else if(flagEm != 0x01){
				if(flagIp & 0x01){
					flagIp = 0x00;
				}
				if(countPdaEmDelay >= 200){
					if((flagEm & 0x10) && (flagEm & 0x02)){
						flagEm = 0x00;
						//CT_EM = 1;
					}
				}
			}
		}
	}
	else{
		countPdaDebounce = 0;
	}
		
	//if(TTL_OUT0 || Brflag || MoniOnflag || (CallEmcount == LED_DEBOUNCE) || (CallCabcount == LED_DEBOUNCE)){
	/*if(TTL_OUT0 || Brflag || (CallEmcount == LED_DEBOUNCE) || (CallCabcount == LED_DEBOUNCE)){
		SGCNT01 =1;
		if(TTL_OUT0){
			SGCNT02 = 1;
		}
		else{
			SGCNT02 = 0;
		}
	}
	else{
		//SGCNT01 =0;
		SGCNT01 =1;
		SGCNT02 =0;
	}*/
	/*if(TTL_OUT1){
		SGCNT02 =1;
	}
	else{
		SGCNT02 =0;
	}*/
}

//void MetroProc(void)
void CabProc(void)
{
	/*if(!CALL_EM){
		LED_EM = 1;
	}
	else{
		LED_EM = 0;
	}
	
	if(!CALL_CAB){
		LED_CAB = 1;
	}
	else{
		LED_CAB = 0;
	}*/
	
	temp = 0x00;
	if(!SW_ROOM)
		temp |= ROOM;
	if(!SW_LEFT)
		temp |= LEFT;
	if(!SW_RIGHT)
		temp |= RIGHT;
	if(!SW_CAB)
		temp |= CAB;
	if(!SW_EM)
		temp |= EM;
		
	if(!CALL_EM)
		temp |= EM_IN;
	if(!CALL_CAB)
		temp |= CAB_IN;
	
	BrSwBuff[2] = BrSwBuff[1];
	BrSwBuff[1] = BrSwBuff[0];
	BrSwBuff[0] = temp;
	
	if((BrSwBuff[2] == BrSwBuff[1]) && (BrSwBuff[1] == BrSwBuff[0])){
		countBrSw++;
		if((BrSwBuff[3] != BrSwBuff[0]) && (countBrSw >= BRSW_DEBOUNCE)){
			BrSwBuff[3] = BrSwBuff[0];
			countBrSw = BRSW_DEBOUNCE;
			if(BrSwBuff[3] & ROOM){
				CT_ROOM = 0;
				LED_ROOM = 1;
			}
			else{
				CT_ROOM = 1;
				LED_ROOM = 0;
			}
			
			if(BrSwBuff[3] & LEFT){
				CT_LEFT = 0;
				LED_LEFT = 1;
			}
			else{
				CT_LEFT = 1;
				LED_LEFT = 0;
			}
			
			if(BrSwBuff[3] & RIGHT){
				CT_RIGHT = 0;
				LED_RIGHT = 1;
			}
			else{
				CT_RIGHT = 1;
				LED_RIGHT = 0;
			}
			
			if(BrSwBuff[3] & CAB){
				CT_CAB = 0;
			}
			else{
				CT_CAB = 1;
			}
			
			if(BrSwBuff[3] & EM){
				CT_EM = 0;
			}
			else{
				CT_EM = 1;
			}
			
			if((BrSwBuff[3] & EM_IN) && !(BrSwBuff[3] & CAB_IN)){
				LED_EM = 1;
			}
			else{
				LED_EM = 0;
			}
			
			if((BrSwBuff[3] & CAB_IN) && !(BrSwBuff[3] & EM_IN)){
				LED_CAB = 1;
			}
			else{
				LED_CAB = 0;
			}
			
			if(BrSwBuff[3] & (ROOM|LEFT|RIGHT)){
				SGCNT01 = 0;
				SGCNT02 = 0;
			}
			else{
				/*SGCNT01 =0;
				if(BrSwBuff[3] & CAB){
					SGCNT02 =1;
				}
				else{
					SGCNT02 = 0;
				}*/
				//if((!CALL_EM)||(!CALL_CAB)){
				if(BrSwBuff[3] & (CAB_IN | EM_IN)){
					SGCNT01 = 0;
					SGCNT02 = 0;
				}
				else{
					SGCNT01 = 1;
					SGCNT02 = 1;
				}
			}
		}
	}
	else{
		countBrSw = 0;
	}

	/*SGCNT01 =1;
	if(TTL_OUT0){
		SGCNT02 = 1;
	}
	else{
		SGCNT02 = 0;
	}*/
}