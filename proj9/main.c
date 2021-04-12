#include <REG51F.H>

//para o timer 1
#define FR_CLOCK 12000000
#define BOUDRATE 1200
#define AUX (FR_CLOCK/(12*BOUDRATE))
#define TH1_INICIAL (-(AUX*2/32)+256) //equivalente a 204

//para o timer 0
#define FR_TIMER 100
#define CORRECAO 7
#define TL0_INICIAL (65536 - (FR_CLOCK/(12*FR_TIMER))+CORRECAO) & 0xFF;
#define TH0_INICIAL (65536 - (FR_CLOCK/(12*FR_TIMER))+CORRECAO) >> 8;

bit is_TI_busy = 0;
bit one_second_flag = 0; //essa flag vai ser setada sempre que se passar 1 segundo.


void inicializa_timer1(){
	TR1 = 0; //desligando a contagem 
	// GATE | C/T | M1 | M0 | GATE | C/T | M1 | M0
	// GATE define se havera intervenção externa GATE = 0 define que não existirá intervenção
	// C/T define se vamos contar os pulsos de clock ou pulsos da entrada T0, C/T = 0 contamos os pulsos de clock
	// TR0 = 0 suspende a contagem, TR0 = 1 retoma a contagem.
	// M1 e M0 controlam o modo de operação
	TMOD = (TMOD & 0x0F) | 0x20; // Não modifico os 4 ultimos bits que equivalem ao timer0 e modifico os 4 primeiros que configuram o timer 1 
	TH1 = (int) TH1_INICIAL; // inicializando o TH1, ele vai inicializar o TL0 toda vez que der overflow
	//Não vai ser necessario colocar o ET1 para 1 pois, não precisaremos da interrupção do timer1
	TR1 = 1; //religando a contagem
}

void serial_int() interrupt 4
{
	if(TI){
		TI=0; //preciso zerar o TI pois isso não seria feito automaticamente, isso é necessario para evitar que a interrupção seja chamada indevidamente
		is_TI_busy = 0;
	}
	if(RI){
		RI = 0; //preciso zerar o RI pois isso não seria feito automaticamente, isso é necessario para evitar que a interrupção seja chamada indevidamente
	}
}

void inicializa_serial(){
	SCON = (SCON & 0x0F)| 0x50; //setando o SM0|SM1|SM2|REN para 0101, para a serial operar no modo 1.
	PCON |= 0x80; //setando o smod para 1 para conseguir um boudrate mais proximo de 1200bps
}

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
	static int count1 = 0;
	TR0 = 0; // Desliga Timer0
	TL0 += TL0_INICIAL; // Faz recarga da contagem do Timer0
	TH0 += TH0_INICIAL + (unsigned char) CY;
	TR0 = 1; // Habilita contagem do timer 0
	
	//a cada 10ms executo o trecho a cima, ele defasa o relogio em 0.000007s
		
	count1++;
	if(count1 >= 100){ //10ms * 100 = 1segundo, sempre que se passar 1 segundo vamos setar essa flag.
		count1 = 0;
		one_second_flag = 1;
	}


}

//função countSetBits é necessaria para checar se eu estou na condição de 5s ou na de 3s
char countSetBits(char n){ //essa função conta quantos 1s existem em um byte
	char count = 0;
	while(n!=0){
		count += n&1;
		n = n>>1;
	}
	
	return count;
}

char decrementCounter(int counter){
	if(counter>0){
		counter--;
	}
	return counter;
}	

void stateMachine(){
	static char counterCamera1 = 0;
	static char counterCamera2 = 0;
	static char counterCamera3 = 0;
	static char counterCamera4 = 0;
	static char camerasIndex = 0;
	char numeroDeCamerasAtivadas = 0;
	
	if(one_second_flag){
		//decrementando todos os contadores até zero
		counterCamera1 = decrementCounter(counterCamera1);
		counterCamera2 = decrementCounter(counterCamera2);
		counterCamera3 = decrementCounter(counterCamera3);
		counterCamera4 = decrementCounter(counterCamera4);
		one_second_flag = 0;
	}
	numeroDeCamerasAtivadas = countSetBits(P0 & 0x0f);
	if(numeroDeCamerasAtivadas == 1){ //só existe uma camera excitada então posso inicializar o contador equivalente para 5 segundos
		if(P0 & 0x01){
			counterCamera1 = 5;
		}else if(P0 & 0x02){
			counterCamera2 = 5;
		}else if(P0 & 0x04){
			counterCamera3 = 5;
		}else if(P0 & 0x08){
			counterCamera4 = 5;
		}
	}else if(numeroDeCamerasAtivadas > 0){ //se existe mais de uma camera, eu vou ter que alternar entre as cameras de 3 em 3 segundos.
		if(!(counterCamera1 | counterCamera2 | counterCamera3 | counterCamera4)){ //só troco de camera apos o tempo de tolerancia(3 ou 5 segundos)
			camerasIndex++; //variavel que será incrementada para rotacionar entre as cameras ativadas
			if(camerasIndex > 3){
				camerasIndex = 0;
			}
			
			switch(camerasIndex){
				case 0:
					if(P0 & 0x01){
						counterCamera1 = 3;
					}
				break;
				case 1:
					if(P0 & 0x02){
						counterCamera2 = 3;
					}
				break;
				case 2:
					if(P0 & 0x04){
						counterCamera3 = 3;
					} 
				break;
				case 3:
					if(P0 & 0x08){
						counterCamera4 = 3;
					}
				break;
			}
		}
	}
			
	if(!is_TI_busy){ //checando se a serial não ta ocupada antes de escrever
		is_TI_busy = 1;
		if(counterCamera1){ //valores ilustrativos para os valores de controle do MUX
			SBUF = '1'; 
		}else if(counterCamera2){
			SBUF = '2';
		}else if(counterCamera3){
			SBUF = '3';
		}else if(counterCamera4){
			SBUF = '4';
		}else{
			SBUF = 'p';
		}
	}
}

void main(void){
	inicializa_timer1(); //configurando o timer1 para o modo 2
	inicializa_timer0(); //configurando o timer0 para o modo 1
	inicializa_serial(); //configurando o serial para o modo 1

	EA = 1; // Habilita o tratamento de interrupções
	ES = 1; // Habilita o tratamento da interrupção do serial quando TI e/ou RI é 1 
	
	while(1){
		stateMachine();
	}
}