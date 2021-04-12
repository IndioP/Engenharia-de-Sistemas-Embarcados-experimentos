#include <REG51F.H>

sbit bit0deP2 = P2^0;
sbit bit1deP2 = P2^1;

unsigned int TEMP_EXEC = 120000;

void SMLowerBits();
void SMHigherBits();

void main(){
	while(1){
		SMLowerBits();
		SMHigherBits();
	}
}

void SMLowerBits(){
	static unsigned char state = 2;
	static unsigned int count = 0;
	switch(state){
		case 0:
			if(bit0deP2){
				P1 = (P1 & 0xf0) | (P0 & 0x0f);
				state = 1;
			}
		break;
		case 1:
			count++;
			if(count > TEMP_EXEC){
				count = 0;
				P1 = (P1 & 0xf0);
				state = 2;
			}
		break;
		case 2:
			if(!bit0deP2){
				state = 0;
			}
		break;
		
	}
}

void SMHigherBits(){
	static unsigned char state = 2;
	static unsigned int count = 0;
	switch(state){
		case 0:
			if(bit1deP2){
				P1 = (P0 & 0xf0) | (P1 & 0x0f);
				state = 1;
			}
		break;
		case 1:
			count++;
			if(count > TEMP_EXEC){
				count = 0;
				P1 = (P1 & 0x0f);
				state = 2;
			}
		break;
		case 2:
			if(!bit1deP2){
				state = 0;
			}
		break;
		
	}
}