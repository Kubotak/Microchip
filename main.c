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

__EEPROM_DATA(0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00);		//内蔵EEPROMの初期化データ（８バイト単位）必要に応じ同じ形式で行数を増やして使用

//予約グローバル変数-------------------------------------------------
	unsigned char rxdata =0x00;		//USART受信データ用変数
	unsigned char txdata =0x00;		//USART送信データ用変数
	unsigned char pdataa=0x00;		//ポートAデータ処理用変数
	unsigned char pdatab=0x00;		//ポートBデータ処理用変数
	unsigned char pdatac=0x00;		//ポートCデータ処理用変数

//グローバル変数定義エリア-------------------------------------------




//追加関数定義エリア-------------------------------------------------
void delay(unsigned int);
void adinit(unsigned char anselh,unsigned char ansel);
void timerinit(void);
void pwmportinit(void);
void pwmsetfreq(unsigned char prescale,unsigned char pr2data);
void pwmsetduty(unsigned char setduty);

//メイン関数-------------------------------------------------
void main (void) {
	TRISB = 0xFC;		// RB0,1 output
	PORTB = 0;

	//ローカル変数定義エリア------------------------------------



	//クロック設定----------------------------------------------
	OSCCON=0x70;	//OSC ８MHZ


	//ポートの設定と初期化--------------------------------------
	TRISA=0x00;	//PORTA設定　固定（０：出力　１：入力）
	TRISB=0x00;	//PORTB設定（０：出力　１：入力）
	TRISC=0x00;	//PORTC設定（０：出力　１：入力）

	PORTA=0x00;	//Lに初期化　
	PORTB=0x00;	//Lに初期化
	PORTC=0x00;	//Lに初期化
	
	//AD機能の初期化------------------------------------------
	adinit(0x00,0x04);	//左バイトANSELH、右バイトANSEL
						//（注）実行でADに設定した対応するポートは入力に自動設定される
						//ポートのDIOの設定も自動で行うため必ず実行すること

	//タイマー１・タイマー２の初期化（固定）------------------
	timerinit();	

	//USARTの設定---------------------------------------------
	//usartinit(207); 	//使用する場合は注釈をとること
						//9600bpc（ボーレート 207:9600bps 103:19.1kbps 34:57.6kbps 16:115.2kbps 他も可）
						//スタート１ビット、データ８ビット、ストップ１ビット、ノンパリティ
						//（注）実行でRB5が入力　RB7が出力に自動設定される
						//（注）ポートがプルダウンされているため電源オン時に受信側がスタートビット検出し、
						//　　　１バイト0x00を受信。RXのR12をオープンにすると改善。

	//PWMの設定-----------------------------------------------
	pwmportinit();				//（注）実行でRC5が出力＆PWMに自動設定される
	pwmsetfreq(2,0xfe);			//PWM初期周波数488Hz
	pwmsetduty(0x00);			//PWM初期デューティ0%に設定
	
	//全割り込みの許可（固定）---------------------------------
	PEIE=1;		//INTCONレジスタのPIC内蔵モジュール割り込み許可フラグをセット	
	GIE=1;		//INTCONレジスタのグローバル割り込み制御ビット有効

	//その他最終処理-------------------------------------------
	TMR1ON=1;	//タイマー１（1mS基準タイマー）開始（０：停止　１：開始）
	//pwmstart();	//PWM開始　使用する場合は注釈をとること
	
	while (1) {
	//メインルーチン記載エリア---------------------------------
//		PORTBbits.RB0 = 1;
		RA0 = 0;
		delay(60000);
//		PORTBbits.RB0 = 0;
		RA0 = 1;
		delay(60000);
	}
}

//ディレイ値の設定
void delay(unsigned int ms) {
	unsigned int i;
	for	(i=0 ; i<ms ; i++) {}
}

//AD機能とポートの設定
void adinit(unsigned char anselh,unsigned char ansel)
{

	/**************************************************
	使用するADポートの設定を行う

	anselh:	bit0:AN8(RC6)	1=ADとして使う	0=使わない
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

	ADON=1;			//ADコンバータ機能の許可
	ADIE=0;			//AD割り込み不許可

}

//タイマー初期化
void timerinit(void)
{

	/*************************************************
	１ｍS周期割り込み用タイマー１＆PWM用タイマー２の設定
	
	タイマー１の初期化：１ｍS周期割り込み
	タイマー２の初期化：RC5からのPWM出力

	（使用上の注意）
	SSPでタイマー２を使用するため、PWMとSSPモジュールの同時使用は不可
	**************************************************/

	#define TMR1H_init 0xf8		//タイマー１　１ｍS用カウント値初期化定数
	#define TMR1L_init 0x30		//タイマー１　１ｍS用カウント値初期化定数
	#define TMR2_init 0x00		//タイマー２　カウント値初期化定数

	//タイマー１ 1mSの設定（変更しないこと）---------------------------
	T1CON=0x00;				//
	TMR1H=TMR1H_init;		//タイマーカウントH値を初期化
	TMR1L=TMR1L_init;		//タイマーカウントL値を初期化
	TMR1IF=0;				//PIR1レジスタの割り込みフラグをクリア
	TMR1IE=1; 				//PIE1レジスタのオーバーフロー割り込み許可フラグをセット

/* タイマー２は、16F677には無いのでコメントアウトする
	//タイマー２ PWM用の設定（変更しないこと）-------------------------
	TMR2=TMR2_init;			//タイマーカウント値を初期化
	//TMR2IF=0;				//割り込みフラグをクリア（割り込みで使う時のみ設定）
	TMR2IE=0;				//割り込み許可フラグをクリア（不許可）
*/

}

//PWMポート初期化
void pwmportinit(void)
{
	/*************************************************
	RC5をPWM出力に設定
	*************************************************/

	TRISC5=0;	//RC5出力に設定　
	RC5=0;		//Lに初期化
	//CCP1CONは、16F677には無いのでコメントアウト
	//CCP1CON=0x0c ;	//00xx1100 P1A(RC5)はPWMモード出力優先に設定

}

//PWM周波数設定
void pwmsetfreq(unsigned char prescale,unsigned char pr2data)
{

	/*************************************************
	RC5から出力するPWMの周波数を設定する

	（PWM周波数の設定式）
		PWM周波数=OSC/4/(pr2data+1)/(4^prescale)

		設定範囲：0<=prescale<=2
				：1<=pr2data<=0xfe
				：OSC=8MHz固定

		（例）7843Hz:OSC=8MHz/PR2=0xfe/prescale=0
			  1960Hz/OSC=8MHz/PR2=0xfe/prescale=1
			   490Hz/OSC=8MHz/PR2=0xfe/prescale=2

    ＰＷＭ出力はこの周期をさらにデューティ分割する

　　**************************************************/
     //PR2, T2CONは、16F677には無いのでコメントアウト
	//PR2=pr2data;	//PWMの１周期を決定する値
	//T2CON=(T2CON & 0xfc) | (prescale & 0x03);	//PWM周波数の設定

}

//PWMデューティ設定
void pwmsetduty(unsigned char setduty)
{
	/*************************************************
	RC5から出力するPWMのオン期間を設定する	

	（PWMのオンデューティを設定）
		setduty: オンデューティを８ビット値最大で設定

		設定範囲：0<=setduty<=(PR2+1)

	　　（例）PR2=0xfeの場合　
			setduty=0x00:オン  0%
			setduty=0xff:オン100%

		＊0xffまで設定可能だが、PR2+1を超えた設定
		　は全て100%になる
		＊PR2はPWM周波数設定関数pr2dataを参照
	**************************************************/

	//オンデューティの設定
	//CCPR1Lは、16F677には無いのでコメントアウト
	//CCPR1L=setduty;	//オンの期間
}