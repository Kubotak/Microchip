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
int port_val;
int port_cnt;
int port_chg;


//追加関数定義エリア-------------------------------------------------
void delay(unsigned int);
void spi_mode_init(void);
void adinit(unsigned char anselh,unsigned char ansel);
void timerinit(void);
void pwmportinit(void);
void pwmsetfreq(unsigned char prescale,unsigned char pr2data);
void pwmsetduty(unsigned char setduty);
void spi_mode_init(void);

//メイン関数-------------------------------------------------
void main (void) {

	//ローカル変数定義エリア------------------------------------
	int i ;
	
	port_val = 0;
	port_cnt = 0;
	port_chg = 0;
	
	//クロック設定----------------------------------------------
	OSCCON=0x70;	//OSC ８MHZ

	//SPIモードに関わる初期化-----------------------------------
	spi_mode_init();
	
	//AD機能の初期化------------------------------------------
	//adinit(0x00,0x04);	//左バイトANSELH、右バイトANSEL
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
		
		if( RA0 == 0 )
		{
			
		}
		
		// ポートに変化があった場合
		if( port_chg == 1 )
		{
			port_chg = 0;
			RA1 = 1; // テスト出力
			
			// SPIでデータ送信
			for (i = 0x30 ; i < 0x39 ; i++) 
			{
				SSPBUF = (char)i ;
				delay(1000); // 1秒毎にデータ送信する
			}
			RA1 = 0; // テスト出力
		}
	}
}

//ディレイ値の設定
void delay(unsigned int ms) {
	unsigned int i;
	for	(i=0 ; i<ms ; i++) {}
}

void interrupt interrupt_func(void)
{
	if( TMR1IF == 1 ) // Timer1割り込みだった場合
	{
		TMR1IF = 0; // Timer1割り込みフラグをクリア
		
		if( RA0 == 0 )
		{
			if( port_val == 0 )
			{
				port_cnt++;
			}
			else
			{
				port_cnt = 0;
			}
			port_val = 0;
		}
		else
		{
			if( port_val == 1 )
			{
				port_cnt++;
			}
			else
			{
				port_cnt = 0;
			}
			port_val = 1;
		}
		
		if( port_cnt == 10 )
		{
			port_chg = 1;
		}
		else if( port_cnt > 10 )
		{
			port_cnt = 11;
		}
	}

	if( SSPIF == 1 ) // SPIの受信完了割り込みだった場合
	{
		SSPIF = 0; // SPIの受信完了割り込みフラグをクリア
	}
}

//SPIモードに関わる初期化
void spi_mode_init(void)
{
	 ADCON1 = 0b00000110 ;     // アナログは使用しない、RA0-RA4をデジタルI/Oに割当
	 TRISA  = 0b00000001 ;     // 1で入力 0で出力 RA0-RA7全て出力に設定(RA5は入力専用)
	 //                ↑ RA0は、暫定で入力モードに設定
	 TRISB  = 0b00010010 ;     // 1:in 0:out SDI(RB1:in) SDO(RB2:out) SCK(RB4:in) SS(RB5:未)
	 TRISC  = 0b00000000 ;     // 1で入力 0で出力 

	 PORTA  = 0b00000000 ;     // 出力ピンの初期化(全てLOWにする)
	 PORTB  = 0b00000000 ;     // 出力ピンの初期化(全てLOWにする)
	 PORTC  = 0b00000000 ;     // 出力ピンの初期化(全てLOWにする)

	 ANSEL  = 0b00000000 ;     // Digital I/Oに設定
	 ANSELH = 0b00000000 ;     // Digital I/Oに設定
	
	// SPIモードの設定と初期化
	SSPCON = 0b00100001 ;     // クロック極性はLOW　スレーブモードでＳＳ使わない
	SSPSTAT= 0b00000000 ;     // クロック位相は立上がりでデータを送る
	SSPIF= 0 ;                // ＳＰＩの割込みフラグを初期化する
	SSPIE= 1 ;                // ＳＰＩの割込みを許可する
	PEIE = 1 ;                // 周辺装置割込み有効
	GIE  = 1 ;                // 全割込み処理を許可する
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

/* AD 機能は未使用

	TRISA=TRISA | ((ansel & 0x07)+((ansel & 0x08)<<1));
	TRISB=TRISB | ((anselh & 0x0c)<<2);
	TRISC=TRISC | (((ansel & 0xf0)>>4)+(anselh<<6));

	ANSEL=ansel;	//AN0-7
	ANSELH=anselh;	//AN8-AN11

	ADON=1;			//ADコンバータ機能の許可
	ADIE=0;			//AD割り込み不許可
*/
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