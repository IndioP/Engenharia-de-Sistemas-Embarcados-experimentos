#include <REG51F.H>

#define FR_CLOCK 12000000
#define FR_TIMER 100
#define CORRECAO 7
#define TL0_INICIAL (65536 - (FR_CLOCK/(12*FR_TIMER))+CORRECAO) & 0xFF;
#define TH0_INICIAL (65536 - (FR_CLOCK/(12*FR_TIMER))+CORRECAO) >> 8;

bit flag_counting_state_machine1 = 0;
bit flag_counting_state_machine2 = 0;

sbit bit0deP2 = P2^0;
sbit bit1deP2 = P2^1;

void inicializa_timer0(){
	TR0 = 0; //desligando a contagem 
	// GATE | C/T | M1 | M0 | GATE | C/T | M1 | M0
	// GATE define se havera intervenção externa GATE = 0 define que não existirá intervenção
	// C/T define se vamos contar os pulsos de clock ou pulsos da entrada T0, C/T = 0 contamos os pulsos de clock
	// TR0 = 0 suspende a contagem, TR0 = 1 retoma a contagem.
	// M1 e M0 controlam o modo de operação
	
	TMOD = (TMOD & 0xF0) | 0x01; // Não modifico os 4 primeiros bits pois eles condizem ao timer 1, para colocar 
	TH0 = TH0_INICIAL; // inicializando o THO
	TL0 = TL0_INICIAL; // inicializando o TL0
	ET0 = 1; //necessario para que o timer0 gere interrupções.
	TR0 = 1; //religando a contagem
}

void timer0_int (void) interrupt 1 using 2
{
	static unsigned char count1 = 0;
	static unsigned char count2 = 0;
	TR0 = 0; // Desliga Timer0
	TL0 += TL0_INICIAL; // Faz recarga da contagem do Timer0
	TH0 += TH0_INICIAL + (unsigned char) CY;
	TR0 = 1; // Habilita contagem do timer 0
	
	//a cada 10ms executo o trecho a cima, ele defasa o relogio em 0.000007s
	
	
	if(flag_counting_state_machine1){
		count1++;
		if(count1 >= 100){ //10ms * 100 = 1segundo
			count1 = 0;
			flag_counting_state_machine1 = 0; //liberando a transição na maquina de estados
		}
	}
	
	if(flag_counting_state_machine2){
		count2++;
		if(count2 >= 100){ //10ms * 100 = 1segundo
			count2 = 0;
			flag_counting_state_machine2 = 0; //liberando a transição na maquina de estados
		}
	}	

}

void SMLowerBits(){
	static unsigned char state = 2;
	switch(state){
		case 0:
			if(bit0deP2){
				P1 = (P1 & 0xf0) | (P0 & 0x0f);
				state = 1;
				flag_counting_state_machine1 = 1; //ESSA FLAG VAI LIBERAR A CONTAGEM
			}
		break;
		case 1: //A MAQUINA DE ESTADOS VAI FICAR PRESA NESSE ESTADO POR 1 SEGUNDO ATÉ QUE A INTERRUPÇÃO A LIBERE
			if(!flag_counting_state_machine1){
				P1 = (P1 & 0xf0);
				state = 2;
			}
		break;
		case 2: //ESTADO NECESSARIO, PARA ESPERAR QUE O BIT VÁ PARA 0 PARA PERMITIR TRANSIÇÃO DE 0 PARA 1
			if(!bit0deP2){
				state = 0;
			}
		break;	
	}
}


void SMHigherBits(){
	static unsigned char state = 2;
	switch(state){
		case 0:
			if(bit1deP2){
				P1 = (P0 & 0xf0) | (P1 & 0x0f);
				state = 1;
				flag_counting_state_machine2 = 1; //ESSA FLAG VAI LIBERAR A CONTAGEM DE 1 SEGUNDO
			}
		break;
		case 1://A MAQUINA DE ESTADOS VAI FICAR PRESA NESSE ESTADO POR 1 SEGUNDO ATÉ QUE A INTERRUPÇÃO A LIBERE
			if(!flag_counting_state_machine2){
				P1 = (P1 & 0x0f);
				state = 2;
			}
		break;
		case 2: //ESTADO NECESSARIO, PARA ESPERAR QUE O BIT VÁ PARA 0 PARA PERMITIR TRANSIÇÃO DE 0 PARA 1
			if(!bit1deP2){
				state = 0;
			}
		break;
	}
}

int main(void){
	inicializa_timer0();
	EA = 1; // Habilita o tratamento de interrupções
	
	while(1){
		SMLowerBits();
		SMHigherBits();
	}

}

