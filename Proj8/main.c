#include <REG517A.H>
//valores que servirão de gatilho
//sempre que o compare timer tiver um valor maior que o valor atual dos CMx
//P4 receberá 1
//do contrario P4 receberá 0

int LUT[] = {65535,58981,52428,45875,39321,32768,26214,19660,13107,6553}; //valores calculados com uma simples regra de 3... 

bit is_TI_busy = 0;

bit new_byte_arrived = 0; //variavel auxiliar para comunicação da interrupção com a main()

char index = 0;

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

void set_compare_register(int index){//inicializando um valor inicial para os registradores CMx
	CM0 = CM1 = CM2 = CM3 = CM4 = CM5 = CM6 = CM7 = LUT[index];
}

void inicializa_compare_timer(){
	CTREL = 0; //define o valor de reset do compare timer
	//colocando o compare_timer pra ser comparado com os registradores CMx
	CMSEL = 0xff;  
	CMEN = 0Xff; 
	set_compare_register(0); 
}

void main(void){
	inicializa_serial(); //configurando o serial para o modo 1
	EAL = 1; // Habilita o tratamento de interrupções
	ES0 = 1; // Habilita o tratamento da interrupção do serial quando TI e/ou RI é 1 
	inicializa_compare_timer();

	while(1){
		if(new_byte_arrived){
			if(!is_TI_busy){
				index = S0BUF;
				S0BUF = index;
				index -= 48;
				set_compare_register(index);
				new_byte_arrived = 0;
				is_TI_busy = 1;
			}
			
		}
	}
}

