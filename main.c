#include <pic.h>

//#include <p16F677.h>
/*
#pragma config OSC = HS
#pragma config PWRT = OFF
#pragma config BOR = OFF
#pragma config WDT = OFF
#pragma config LVP = OFF
*/

//Config bit---------------------------------------------------
__CONFIG(INTIO & WDTDIS & PWRTEN & MCLRDIS & UNPROTECT & BORDIS & IESODIS & FCMDIS);

__EEPROM_DATA(0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00);		//����EEPROM�̏������f�[�^�i�W�o�C�g�P�ʁj�K�v�ɉ��������`���ōs���𑝂₵�Ďg�p

//�\��O���[�o���ϐ�-------------------------------------------------
	unsigned char rxdata =0x00;		//USART��M�f�[�^�p�ϐ�
	unsigned char txdata =0x00;		//USART���M�f�[�^�p�ϐ�
	unsigned char pdataa=0x00;		//�|�[�gA�f�[�^�����p�ϐ�
	unsigned char pdatab=0x00;		//�|�[�gB�f�[�^�����p�ϐ�
	unsigned char pdatac=0x00;		//�|�[�gC�f�[�^�����p�ϐ�

//�O���[�o���ϐ���`�G���A-------------------------------------------




//�ǉ��֐���`�G���A-------------------------------------------------
void delay(unsigned int);
void adinit(unsigned char anselh,unsigned char ansel);
void timerinit(void);
void pwmportinit(void);
void pwmsetfreq(unsigned char prescale,unsigned char pr2data);
void pwmsetduty(unsigned char setduty);

//���C���֐�-------------------------------------------------
void main (void) {
	TRISB = 0xFC;		// RB0,1 output
	PORTB = 0;

	//���[�J���ϐ���`�G���A------------------------------------



	//�N���b�N�ݒ�----------------------------------------------
	OSCCON=0x70;	//OSC �WMHZ


	//�|�[�g�̐ݒ�Ə�����--------------------------------------
	TRISA=0x00;	//PORTA�ݒ�@�Œ�i�O�F�o�́@�P�F���́j
	TRISB=0x00;	//PORTB�ݒ�i�O�F�o�́@�P�F���́j
	TRISC=0x00;	//PORTC�ݒ�i�O�F�o�́@�P�F���́j

	PORTA=0x00;	//L�ɏ������@
	PORTB=0x00;	//L�ɏ�����
	PORTC=0x00;	//L�ɏ�����
	
	//AD�@�\�̏�����------------------------------------------
	adinit(0x00,0x04);	//���o�C�gANSELH�A�E�o�C�gANSEL
						//�i���j���s��AD�ɐݒ肵���Ή�����|�[�g�͓��͂Ɏ����ݒ肳���
						//�|�[�g��DIO�̐ݒ�������ōs�����ߕK�����s���邱��

	//�^�C�}�[�P�E�^�C�}�[�Q�̏������i�Œ�j------------------
	timerinit();	

	//USART�̐ݒ�---------------------------------------------
	//usartinit(207); 	//�g�p����ꍇ�͒��߂��Ƃ邱��
						//9600bpc�i�{�[���[�g 207:9600bps 103:19.1kbps 34:57.6kbps 16:115.2kbps �����j
						//�X�^�[�g�P�r�b�g�A�f�[�^�W�r�b�g�A�X�g�b�v�P�r�b�g�A�m���p���e�B
						//�i���j���s��RB5�����́@RB7���o�͂Ɏ����ݒ肳���
						//�i���j�|�[�g���v���_�E������Ă��邽�ߓd���I�����Ɏ�M�����X�^�[�g�r�b�g���o���A
						//�@�@�@�P�o�C�g0x00����M�BRX��R12���I�[�v���ɂ���Ɖ��P�B

	//PWM�̐ݒ�-----------------------------------------------
	pwmportinit();				//�i���j���s��RC5���o�́�PWM�Ɏ����ݒ肳���
	pwmsetfreq(2,0xfe);			//PWM�������g��488Hz
	pwmsetduty(0x00);			//PWM�����f���[�e�B0%�ɐݒ�
	
	//�S���荞�݂̋��i�Œ�j---------------------------------
	PEIE=1;		//INTCON���W�X�^��PIC�������W���[�����荞�݋��t���O���Z�b�g	
	GIE=1;		//INTCON���W�X�^�̃O���[�o�����荞�ݐ���r�b�g�L��

	//���̑��ŏI����-------------------------------------------
	TMR1ON=1;	//�^�C�}�[�P�i1mS��^�C�}�[�j�J�n�i�O�F��~�@�P�F�J�n�j
	//pwmstart();	//PWM�J�n�@�g�p����ꍇ�͒��߂��Ƃ邱��
	
	while (1) {
	//���C�����[�`���L�ڃG���A---------------------------------
//		PORTBbits.RB0 = 1;
		RA0 = 0;
		delay(60000);
//		PORTBbits.RB0 = 0;
		RA0 = 1;
		delay(60000);
	}
}

//�f�B���C�l�̐ݒ�
void delay(unsigned int ms) {
	unsigned int i;
	for	(i=0 ; i<ms ; i++) {}
}

//AD�@�\�ƃ|�[�g�̐ݒ�
void adinit(unsigned char anselh,unsigned char ansel)
{

	/**************************************************
	�g�p����AD�|�[�g�̐ݒ���s��

	anselh:	bit0:AN8(RC6)	1=AD�Ƃ��Ďg��	0=�g��Ȃ�
			bit1:AN9(RC7)
			bit2:AN10(RB4)
			bit3:AN11(RB5)
	ansel:	bit0:AN0(RA0)
			bit1:AN1(RA1)
			bit2:AN2(RA2)
			bit3:AN3(RA4)
			bit4:AN4(RC0)
			bit5:AN5(RC1)
			bit6:AN6(RC2)
			bit7:AN7(RC3)
	**************************************************/


	TRISA=TRISA | ((ansel & 0x07)+((ansel & 0x08)<<1));
	TRISB=TRISB | ((anselh & 0x0c)<<2);
	TRISC=TRISC | (((ansel & 0xf0)>>4)+(anselh<<6));

	ANSEL=ansel;	//AN0-7
	ANSELH=anselh;	//AN8-AN11

	ADON=1;			//AD�R���o�[�^�@�\�̋���
	ADIE=0;			//AD���荞�ݕs����

}

//�^�C�}�[������
void timerinit(void)
{

	/*************************************************
	�P��S�������荞�ݗp�^�C�}�[�P��PWM�p�^�C�}�[�Q�̐ݒ�
	
	�^�C�}�[�P�̏������F�P��S�������荞��
	�^�C�}�[�Q�̏������FRC5�����PWM�o��

	�i�g�p��̒��Ӂj
	SSP�Ń^�C�}�[�Q���g�p���邽�߁APWM��SSP���W���[���̓����g�p�͕s��
	**************************************************/

	#define TMR1H_init 0xf8		//�^�C�}�[�P�@�P��S�p�J�E���g�l�������萔
	#define TMR1L_init 0x30		//�^�C�}�[�P�@�P��S�p�J�E���g�l�������萔
	#define TMR2_init 0x00		//�^�C�}�[�Q�@�J�E���g�l�������萔

	//�^�C�}�[�P 1mS�̐ݒ�i�ύX���Ȃ����Ɓj---------------------------
	T1CON=0x00;				//
	TMR1H=TMR1H_init;		//�^�C�}�[�J�E���gH�l��������
	TMR1L=TMR1L_init;		//�^�C�}�[�J�E���gL�l��������
	TMR1IF=0;				//PIR1���W�X�^�̊��荞�݃t���O���N���A
	TMR1IE=1; 				//PIE1���W�X�^�̃I�[�o�[�t���[���荞�݋��t���O���Z�b�g

/* �^�C�}�[�Q�́A16F677�ɂ͖����̂ŃR�����g�A�E�g����
	//�^�C�}�[�Q PWM�p�̐ݒ�i�ύX���Ȃ����Ɓj-------------------------
	TMR2=TMR2_init;			//�^�C�}�[�J�E���g�l��������
	//TMR2IF=0;				//���荞�݃t���O���N���A�i���荞�݂Ŏg�����̂ݐݒ�j
	TMR2IE=0;				//���荞�݋��t���O���N���A�i�s���j
*/

}

//PWM�|�[�g������
void pwmportinit(void)
{
	/*************************************************
	RC5��PWM�o�͂ɐݒ�
	*************************************************/

	TRISC5=0;	//RC5�o�͂ɐݒ�@
	RC5=0;		//L�ɏ�����
	//CCP1CON�́A16F677�ɂ͖����̂ŃR�����g�A�E�g
	//CCP1CON=0x0c ;	//00xx1100 P1A(RC5)��PWM���[�h�o�͗D��ɐݒ�

}

//PWM���g���ݒ�
void pwmsetfreq(unsigned char prescale,unsigned char pr2data)
{

	/*************************************************
	RC5����o�͂���PWM�̎��g����ݒ肷��

	�iPWM���g���̐ݒ莮�j
		PWM���g��=OSC/4/(pr2data+1)/(4^prescale)

		�ݒ�͈́F0<=prescale<=2
				�F1<=pr2data<=0xfe
				�FOSC=8MHz�Œ�

		�i��j7843Hz:OSC=8MHz/PR2=0xfe/prescale=0
			  1960Hz/OSC=8MHz/PR2=0xfe/prescale=1
			   490Hz/OSC=8MHz/PR2=0xfe/prescale=2

    �o�v�l�o�͂͂��̎���������Ƀf���[�e�B��������

�@�@**************************************************/
     //PR2, T2CON�́A16F677�ɂ͖����̂ŃR�����g�A�E�g
	//PR2=pr2data;	//PWM�̂P���������肷��l
	//T2CON=(T2CON & 0xfc) | (prescale & 0x03);	//PWM���g���̐ݒ�

}

//PWM�f���[�e�B�ݒ�
void pwmsetduty(unsigned char setduty)
{
	/*************************************************
	RC5����o�͂���PWM�̃I�����Ԃ�ݒ肷��	

	�iPWM�̃I���f���[�e�B��ݒ�j
		setduty: �I���f���[�e�B���W�r�b�g�l�ő�Őݒ�

		�ݒ�͈́F0<=setduty<=(PR2+1)

	�@�@�i��jPR2=0xfe�̏ꍇ�@
			setduty=0x00:�I��  0%
			setduty=0xff:�I��100%

		��0xff�܂Őݒ�\�����APR2+1�𒴂����ݒ�
		�@�͑S��100%�ɂȂ�
		��PR2��PWM���g���ݒ�֐�pr2data���Q��
	**************************************************/

	//�I���f���[�e�B�̐ݒ�
	//CCPR1L�́A16F677�ɂ͖����̂ŃR�����g�A�E�g
	//CCPR1L=setduty;	//�I���̊���
}