#include <REG52.H>

#define FR_CLOCK 12000000
#define BOUDRATE 1200
#define AUX (12000000/(12*1200))
#define TH1_INICIAL (-(AUX*2/32)+256) //equivalente a 204

#define BUFFER_SIZE 16

typedef struct{
	char buffer[BUFFER_SIZE];
	unsigned char in_index;
	unsigned char out_index;
}circularBuffer;

circularBuffer RxBuffer;
circularBuffer TxBuffer;

bit RecebeuString = 0;
bit is_TI_busy = 0;

bit new_byte_arrived = 0; //variavel auxiliar para comunicação da interrupção com a main()


char RxBufferVazio(){
	return RxBuffer.in_index == RxBuffer.out_index;
}

char TxBufferVazio(){
	return TxBuffer.in_index == TxBuffer.out_index;
}

char ReceiveChar(){
	char atual = RxBuffer.buffer[RxBuffer.out_index];
	RxBuffer.out_index = (RxBuffer.out_index + 1) % BUFFER_SIZE;
	return atual;
}

void PushChar2RxBuffer(char c){
	RxBuffer.buffer[RxBuffer.in_index] = c;
	RxBuffer.in_index = (RxBuffer.in_index + 1) % BUFFER_SIZE;
}

void ReceiveString(char *s){
	int i = 0;
	while(!RxBufferVazio()){
			s[i] = ReceiveChar();
			i++;
	}
}

void SendChar(char c){
	TxBuffer.buffer[TxBuffer.in_index] = c;
	TxBuffer.in_index = (TxBuffer.in_index + 1) % BUFFER_SIZE;
}

char PopCharFromTxBuffer(){
	char aux = TxBuffer.buffer[TxBuffer.out_index];
	TxBuffer.out_index = (TxBuffer.out_index + 1) % BUFFER_SIZE;
	return aux;
}

void SendString(char *s){
	int i = -1;
	do{
			SendChar(s[++i]);
	}while(s[i] != '$');
}



void inicializa_timer1(){

	TR1 = 0; //desligando a contagem 

	TMOD = (TMOD & 0x0F) | 0x20; // Não modifico os 4 ultimos bits que equivalem ao timer0 e modifico os 4 primeiros que configuram o timer 1 

	TH1 = (int) TH1_INICIAL; // inicializando o TH1, ele vai inicializar o TL0 toda vez que der overflow

	TR1 = 1; //religando a contagem

}

void serial_int() interrupt 4
{
	if(TI){
		is_TI_busy = 0;
		TI=0; //preciso zerar o TI pois isso não seria feito automaticamente, isso é necessario para evitar que a interrupção seja chamada indevidamente
	}
	if(RI){
		new_byte_arrived = 1; 
		RI = 0; //preciso zerar o RI pois isso não seria feito automaticamente, isso é necessario para evitar que a interrupção seja chamada indevidamente
	}
}

void inicializa_serial(){
	SCON = (SCON & 0x0F)| 0x50; //setando o SM0|SM1|SM2|REN para 0101, para a serial operar no modo 1.
	PCON |= 0x80; //setando o smod para 1 para conseguir um boudrate mais proximo de 1200bps
}

void inicializa_Rx_Tx(){
	RxBuffer.in_index = 0;
  RxBuffer.out_index = 0;
	TxBuffer.in_index = 0;
  TxBuffer.out_index = 0;
}

void main(void){
	

	inicializa_Rx_Tx();
	inicializa_timer1(); //configurando o timer1 para o modo 2
	inicializa_serial(); //configurando o serial para o modo 1

	EA = 1; // Habilita o tratamento de interrupções
	ES = 1; // Habilita o tratamento da interrupção do serial quando TI e/ou RI é 1 
	while(1){
		if(new_byte_arrived){
			PushChar2RxBuffer(SBUF);
			if(SBUF == '$'){
				RecebeuString = 1;
			}
			new_byte_arrived = 0;
		}
		if(RecebeuString){
			char aux[BUFFER_SIZE];
			ReceiveString(aux);
			SendString(aux);
			TI = 1;
			RecebeuString = 0;
		}
		if(!is_TI_busy){
			if(!TxBufferVazio()){
				SBUF = PopCharFromTxBuffer();
			}
			is_TI_busy = 1;
		}
		
		
	}
}

