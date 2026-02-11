#define	CT_ROOM		PORTA.7
#define CT_LEFT		PORTA.6
#define	CT_RIGHT	PORTA.5
#define	CT_CAB		PORTA.4
#define CT_EM		PORTA.3
#define CT_FAULT	PORTB.2

#define	LED_ROOM	PORTA.2
#define	LED_LEFT	PORTA.1
#define	LED_RIGHT	PORTA.0
#define	LED_CAB		PORTB.0
#define	LED_EM		PORTB.1

#define SW_ROOM		PIND.4
#define	SW_LEFT		PIND.5
#define	SW_RIGHT	PIND.6
#define	SW_CAB		PIND.7
#define	SW_EM		PINC.0
#define	SW_MIC		PINC.1

#define	CALL_CAB	PIND.2
#define	CALL_EM		PIND.3

#define	TTL_OUT0	PIND.0
#define	TTL_OUT1	PIND.1

#define	SGCNT01		PORTC.6
#define	SGCNT02		PORTC.7

#define CAR_PORT1	PINC.2
#define	CAR_PORT2	PINC.5

#define	ROOM		0x01
#define	LEFT		0x02
#define	RIGHT		0x04
#define	CAB		0x08
#define	EM		0x10
#define	EM_IN	0x20
#define	CAB_IN	0x40

#define CAR1		0x01
#define CAR2		0x02

//#define	KORAIL		0x01
//#define	SEOULMETRO	0x02
#define	ID_CAB		0x01
#define	ID_CREW		0x02

#define	LED_DEBOUNCE_CAB	3		// CALL_EM, CALL_CAB 인식시간 * 10msec
#define	LED_DEBOUNCE_CREW	30		// CALL_EM, CALL_CAB 인식시간 * 10msec
#define	SW_DEBOUNCE	5		// 비상인터폰 응답, 운전실 통화 스위치 인식 * 10msec
#define	NO_IN_TIMEOUT	3000		// 무입력 선택 취소 시간 * 10msec

#define	BRSW_DEBOUNCE	7		// 스위치 인식 지연 *10msec + 30msec

#define	WAIT_EM_TIME	400

#define	EM_RE_OUT_TIME	300		// EM RE 출력 시간

#define	WAIT_CAB_TIME	350

unsigned char temp;
unsigned int tempint;

unsigned int count10msec;
bit flag10msec;

unsigned char CarType;
unsigned char CarBuff[4];

unsigned int CallEmcount;
unsigned int CallCabcount;
//unsigned int MoniOncount;
unsigned int SwEmcount;
unsigned int SwCabcount;
unsigned int SwMiccount;

unsigned char BrSwBuff[4];
unsigned char BrWaitBuff;

unsigned int NoSwIncount;

bit Brflag;

unsigned int Blinkcount;

bit Cabflag;
//bit MoniOnflag;

unsigned char countBrSw;

unsigned char flagEm;
unsigned char flagIp;

unsigned char countEmBlink;
unsigned int countWaitEm;

unsigned char PdaBuff[4];
unsigned char countPdaDebounce;

//unsigned char countEmDelay;

unsigned char countPdaEmDelay;

bit EmRqOff;

bit flagCtEm;
unsigned int countEmReOut;

unsigned int countWaitCab;

void Proc10msec(void);

void CarTypeProc(void);
void CrewProc(void);
void EmLedProc(void);
void CabLedProc(void);
//void MoniOnProc(void);
void EmSwProc(void);
void CabSwProc(void);
void BrSwProc(void);
void MicProc(void);
void LedProc(void);
void PdaProc(void);
//void MetroProc(void);
void CabProc(void);