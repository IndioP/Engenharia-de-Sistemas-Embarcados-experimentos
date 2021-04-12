#include <REG51F.H>

//precisamos usar esse tipo 'sbit' para acessar umbit de uma porta paralela 
//0xf0 == 0b11110000
//0x0f == 0b00001111

sbit bit0P2 = P2^0; 
sbit bit1P2 = P2^1;

void main(){
	while(1){
		if(bit0P2){ 
			P1 = (P1 & 0xf0) | (P0 & 0x0f); 
		}else{
			P1 = P1 & 0xf0;
		}	
		if(bit1P2){
			P1 = (P0 & 0xf0) | (P1 & 0x0f); 
		}else{
			P1 = P0 & 0x0f;
		}
	}
}
