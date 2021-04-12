#include <REG51F.H>

#define FR_CLOCK 12000000
#define BOUDRATE 1200
#define AUX (12000000/(12*1200))
#define TH1_INICIAL (-(AUX*2/32)+256) //equivalente a 204

bit new_byte_arrived = 0; //variavel auxiliar para comunica��o da interrup��o com a main()

void inicializa_timer1(){

	TR1 = 0; //desligando a contagem 

	// GATE | C/T | M1 | M0 | GATE | C/T | M1 | M0

	// GATE define se havera interven��o externa GATE = 0 define que n�o existir� interven��o

	// C/T define se vamos contar os pulsos de clock ou pulsos da entrada T0, C/T = 0 contamos os pulsos de clock

	// TR0 = 0 suspende a contagem, TR0 = 1 retoma a contagem.

	// M1 e M0 controlam o modo de opera��o

	TMOD = (TMOD & 0x0F) | 0x20; // N�o modifico os 4 ultimos bits que equivalem ao timer0 e modifico os 4 primeiros que configuram o timer 1 

	TH1 = (int) TH1_INICIAL; // inicializando o TH1, ele vai inicializar o TL0 toda vez que der overflow

	//N�o vai ser necessario colocar o ET1 para 1 pois, n�o precisaremos da interrup��o do timer1

	TR1 = 1; //religando a contagem

}

void serial_int() interrupt 4
{
	if(TI){
		TI=0; //preciso zerar o TI pois isso n�o seria feito automaticamente, isso � necessario para evitar que a interrup��o seja chamada indevidamente
	}
	if(RI){
		new_byte_arrived = 1; 
		RI = 0; //preciso zerar o RI pois isso n�o seria feito automaticamente, isso � necessario para evitar que a interrup��o seja chamada indevidamente
	}
}

void inicializa_serial(){
	SCON = (SCON & 0x0F)| 0x50; //setando o SM0|SM1|SM2|REN para 0101, para a serial operar no modo 1.
	PCON |= 0x80; //setando o smod para 1 para conseguir um boudrate mais proximo de 1200bps
}

void main(void){

	inicializa_timer1(); //configurando o timer1 para o modo 2
	inicializa_serial(); //configurando o serial para o modo 1

	EA = 1; // Habilita o tratamento de interrup��es
	ES = 1; // Habilita o tratamento da interrup��o do serial quando TI e/ou RI � 1 
	while(1){
		if(new_byte_arrived){
			char aux = SBUF; //pego o byte recebido
			aux++; //somo para transformar no proximo caractere
			SBUF = aux; //envio ela pelo barramento serial
			new_byte_arrived = 0;
		}
		
	}
}

