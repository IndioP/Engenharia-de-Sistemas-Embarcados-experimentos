#include <REG517A.H>

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


void serial_int() interrupt 4
{
	if(TI0){
		is_TI_busy = 0;
		TI0=0; //preciso zerar o TI pois isso não seria feito automaticamente, isso é necessario para evitar que a interrupção seja chamada indevidamente
	}
	if(RI0){
		new_byte_arrived = 1; 
		RI0 = 0; //preciso zerar o RI pois isso não seria feito automaticamente, isso é necessario para evitar que a interrupção seja chamada indevidamente
	}
}

void inicializa_serial(){
	S0CON = (S0CON & 0x0F)| 0x50; //setando o SM0|SM1|SM2|REN para 0101, para a serial operar no modo 1.
	PCON |= 0x80; //setando o smod para 1 para conseguir um boudrate mais proximo de 1200bps
	ADCON0 = 0x80 | ADCON0;

}

void inicializa_Rx_Tx(){
	RxBuffer.in_index = 0;
  RxBuffer.out_index = 0;
	TxBuffer.in_index = 0;
  TxBuffer.out_index = 0;
}

void main(void){
	

	inicializa_Rx_Tx();
	inicializa_serial(); //configurando o serial para o modo 1
	EAL = 1; // Habilita o tratamento de interrupções
	ES0 = 1; // Habilita o tratamento da interrupção do serial quando TI e/ou RI é 1 
	while(1){
		if(new_byte_arrived){
			PushChar2RxBuffer(S0BUF);
			if(S0BUF == '$'){
				RecebeuString = 1;
			}
			new_byte_arrived = 0;
		}
		if(RecebeuString){
			char aux[BUFFER_SIZE];
			ReceiveString(aux);
			SendString(aux);
			TI0 = 1;
			RecebeuString = 0;
		}
		if(!is_TI_busy){
			if(!TxBufferVazio()){
				S0BUF = PopCharFromTxBuffer();
			}
			is_TI_busy = 1;
		}
		
		
	}
}

