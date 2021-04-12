#include <reg51.h>

#define FrClk 12000000
#define FreqTimer0_emHz 100
#define VALOR_TH0 ((65536 - (FrClk / (12 * FreqTimer0_emHz))) >>8)
#define VALOR_TL0 ((65536 - (FrClk / (12 * FreqTimer0_emHz))) & 0xFF)



unsigned int contador,contador2;

void timer0_inicializa() 
  {
  TR0 = 0;              // Desliga Timer0
  TMOD = (TMOD & 0xF0) | 0x01;  // Timer 0 programado como timer de 16 bits
  TH0 = VALOR_TH0;  // Programa contagem do Timer0
  TL0 = VALOR_TL0;
  ET0 = 1;      // Habilita interrupcao do timer 0
  TR0 = 1;      // Habilita contagem do timer 0
  }

void timer0_int (void) interrupt 1 using 2
  {
  TR0 = 0;              // Desliga Timer0
  TL0 += VALOR_TL0;
  TH0 += VALOR_TH0 + (unsigned char) CY;   // Programa contagem do Timer0
  TR0 = 1;      // Habilita contagem do timer 0

  if (contador != 0)
     contador--;
  }

void main()
	{
  timer0_inicializa();
	EA=1; //Habilita o tratamento de interrupções

	while (1)
      {
      contador = 300;
			do{
				contador2 = contador;
			}while(contador2!=0);
      
			//while(contador != 0); // Fica em loop enquanto "contador != 0"
			//o problema era uma condição de corrida, que tava acontecendo devido a uma concorrencia 
			//entre a interrupção e o loop da main.
			//a linha 'while(contador != 0)' é traduzido para dois comandos em assembly, isso por que o 8052 é um processador de 8 bits.
			//o primeiro comando é pra mover a parte baixa do contador para o registrador A.
			//o segundo comando é para fazer um ou logico entre A e a parte alta de contador.
			//o bug ocorria quando a interrupção ocorria entre a execução do primeiro comando e o segundo. e o valor de contador era 0x0100
			//executando o primeiro comando A receberia 00(parte baixa de contador)
			//dentro da interrupção, contador seria decrementado para 0x00FF
			//no retorno da função o ou logico seria feito entre A e a parte alta de contador ou seja (0x00 ou 0x00) o que daria 0.
			//apesar de que o valor real para contador seja 0x00FF.
			
			//uma forma de resolver isso é copiando o valor de contador para outra variavel e fazer a checagem com essa variavel auxiliar, garantindo assim que o valor não vai mudar no meio.
			//importante notar que a copia da variavel é feita primeiro a parte alta depois a parte baixa diferente do ou logico e do decremento.
			
      if (contador == 0) // Testa se contador é igual a zero.
         P0 = 0;
      else
         P0 = 0xFF;	// ESTA INSTRUÇÃO NUNCA DEVERIA SER EXECUTADA!
      }
	}
	

