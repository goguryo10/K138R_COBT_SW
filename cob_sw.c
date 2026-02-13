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


unsigned char flagTx;
unsigned char TxSize;
unsigned char TxPoint;
unsigned char TxBuff[10];

unsigned char flagRx;
unsigned char RxSize;
unsigned char RxPoint;
unsigned char RxBuff[10];

unsigned char flagEmRemote;
unsigned char flagIpRemote;

unsigned char ttl_out0;	// 운전실통화
unsigned char ttl_out1;	// 비상호출


#define RXB8 1
#define TXB8 0
#define UPE 2
#define OVR 3
#define FE 4
#define UDRE 5
#define RXC 7

#define FRAMING_ERROR (1<<FE)
#define PARITY_ERROR (1<<UPE)
#define DATA_OVERRUN (1<<OVR)
#define DATA_REGISTER_EMPTY (1<<UDRE)
#define RX_COMPLETE (1<<RXC)


// USART Receiver interrupt service routine
interrupt [USART_RXC] void usart_rx_isr(void)
{
	unsigned char status,data;
	status = UCSRA;
	data=UDR;
	
	if ((status & (FRAMING_ERROR | PARITY_ERROR | DATA_OVERRUN))==0)
	{
		switch(data)
		{
			case 0xFFU:
				flagRx = 0;
				RxSize = 0;
				RxPoint = 0;
				break;
				
			case 0xFEU:
				flagRx = 1;
				RxSize = RxPoint;
				RxPoint = 0;
				break;

			default:
				if(RxPoint < (10-1)) {
					RxBuff[RxPoint++] = data;
				}
				break;
		}
	}
}


// USART Transmitter interrupt service routine
interrupt [USART_TXC] void usart_tx_isr(void)
{
	if ((flagTx == 0x01) && (TxSize>0 ))
	{	
		if(TxPoint >= TxSize)
		{
			TxPoint = 0;
			TxSize = 0;
			flagTx = 0x00;
		}else{
			UDR = TxBuff[TxPoint];
			TxPoint++;
		}
	}
}


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


void SendPda(unsigned char d0, unsigned char d1, unsigned char d2, unsigned char d3, unsigned char d4, unsigned char d5, unsigned char d6, unsigned char d7)
{
	int i = 0;
	
	if(flagTx == 0x00)
	{	
 		TxSize = 0;
		TxPoint = 0;
		TxBuff[i++] = 0xFFU;
		TxBuff[i++] = d0;
		TxBuff[i++] = d1;
		TxBuff[i++] = d2;
		TxBuff[i++] = d3;
		TxBuff[i++] = d4;
		TxBuff[i++] = d5;
		TxBuff[i++] = d6;
		TxBuff[i++] = d7;
		TxBuff[i++] = 0xFEU;
		TxSize = i;
		UDR = TxBuff[0];
		TxPoint = 1;
		flagTx = 0x01;
	}
}


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
// Func7=In Func6=In Func5=In Func4=In Func3=In Func2=Out Func1=Txd Func0=Rxd 
// State7=P State6=P State5=P State4=P State3=P State2=P State1=P State0=P 
PORTD=0xFF;
DDRD=0x06;

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

// 2020-11-02 ADD USART instead TTL0, TTL1 by xsnake
// ##########################################################
// 시리얼 통신 설정 
// USART initialization
// Communication Parameters: 8 Data, 1 Stop, No Parity
// USART Receiver: On
// USART Transmitter: On
// USART Mode: Asynchronous
// USART Baud rate: 19200
UCSRA=0x00;
UCSRB=0xD8;
UCSRC=0x86;
UBRRH=0x00;
UBRRL=0x33;
// ##########################################################


// Analog Comparator initialization
// Analog Comparator: Off
// Analog Comparator Input Capture by Timer/Counter 1: Off
ACSR=0x80;
SFIOR=0x00;


flagTx = 0x00;
TxSize = 0x00;
TxPoint = 0x00;
TxBuff[0] = 0x00;
TxBuff[1] = 0x00;
TxBuff[2] = 0x00;
TxBuff[3] = 0x00;
TxBuff[4] = 0x00;
TxBuff[5] = 0x00;
TxBuff[6] = 0x00;
TxBuff[7] = 0x00;
TxBuff[8] = 0x00;
TxBuff[9] = 0x00;

flagRx = 0x00;
RxSize = 0x00;
RxPoint = 0x00;


flagEm = 0x00;
flagIp = 0x00;
flagEmRemote = 0x00;
flagIpRemote = 0x00;

SGCNT01 =1;
SGCNT02 =1;
SGCNT03 =1;


CarType = ID_CREW;

// Global enable interrupts
#asm("sei")

	tempint = 0;
	while(tempint < 800)
	{
		if(flag10msec)
		{
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
	unsigned char EMRQ;
	static unsigned char EMRQ_BK = 0;
	static unsigned char flagEm_BK = 0;
	static unsigned char flagIp_BK = 0;
	static unsigned char Brflag_BK = 0;
	static unsigned char resend = 0;

	if(flagRx) {
		flagRx = 0;

		if(RxBuff[1] & 0x01U) {
			ttl_out0 = 1;
		}
		else {
			ttl_out0 = 0;
		}
		if(RxBuff[1] & 0x02U) {
			ttl_out1 = 1;
		}
		else {
			ttl_out1 = 0;
		}
	}

	
	if(flag10msec)
	{
		flag10msec = 0;
		CarTypeProc();

		// 2026-02-12 ramarama
		//	All ways COB(T) for Crew
		CarType = ID_CREW;
		
		//if(CarType == KORAIL){
		if(CarType == ID_CREW){
			CrewProc();
		}
		else{		// else if(CarType == SEOULMETRO){
			//MetroProc();
			CabProc();
		}

		// 2026-02-12 ramarama
		//	운통/비상통화 
		{
			EMRQ = CALL_EM;

			//if((EMRQ_BK!=EMRQ) || (flagEm_BK!=flagEm) || (flagIp_BK!=flagIp) || (Brflag_BK!=Brflag))
			if((resend>0) || (flagEm_BK!=flagEm) || (flagIp_BK!=flagIp) || (Brflag_BK!=Brflag))
			{
				if(resend == 0) {
					resend = 3;
				}
				if(resend > 0) {
					resend --;
				}
				SendPda(flagEm, flagIp, Brflag, 0xF0, SGCNT01, SGCNT02, SGCNT03, 0xF0);
			}
			EMRQ_BK = EMRQ;
			flagEm_BK = flagEm;
			flagIp_BK = flagIp;
			Brflag_BK = Brflag;
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

	PdaProc();

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
			// 2026-02-10 ramarama
			//	수동방송시에 PCC 제어하여 자동방송 중단하기 위해			
			if((BrWaitBuff & (ROOM|LEFT|RIGHT)) != 0x00U) {
				CT_PCC = 0;
			}
			else {
				CT_PCC = 1;
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
			// 2026-02-10 ramarama
			//	수동방송시에 PCC 제어하여 자동방송 중단하기 위해			
			CT_PCC = 1;
		}
	}
}

void LedProc(void)
{
	if(Brflag){
		SGCNT01 =0;
		//SGCNT02 =0;
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
			SGCNT03 =0;
		}
		else{
			SGCNT02 =1;
			SGCNT03 =1;
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

#if 1
	if(ttl_out0)
		val |= 0x01;
	if(ttl_out1)
		val |= 0x02;
#else
	if(TTL_OUT0)
		val |= 0x01;
	if(TTL_OUT1)
		val |= 0x02;
#endif

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
// 2026-01-10 ramarama
//	COB(T) 에서 미사용
#if 0
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
#endif
}

