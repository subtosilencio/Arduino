/*
* MAQUINA DE TURING
*
* Pinos do arduino
* fita: 15, 14, 13, 12, 11, 10, 9, 8, 0, 1, 2, 3, 4, 5, 6, 7 
*
*
* TO-DO LIST:
*	- Integrar organizaFita() com verificaBotao()
*	- Corrigir a função terminaFita()
*	- Verificar os pinos do Arduino
*	- Corrigir o vaiteia no estado parado
*	- Acender e apagar os leds durante as transições da maquina
*
*	
*
*	
*
*	
*
* Outubro/2014
*/



short pins[17]= {
	-1, 15, 14, 13, 12, 11, 10, 9, 8, 0, 1, 2, 3, 4, 5, 6, 7}; //corrigir

short fita[16] = {
	pins[8], pins[7], pins[6], pins[5], pins[4], pins[3], pins[2], pins[1]}; 

short leds[16];

// Pinos dos botoes
short cima=16, baixo=17, direita=18, esquerda=19;

const short START = 0;
const short RESET = 1;
const short DIREITA = 2;
const short ESQUERDA = 3;
const short ESTADO = 4;
const short PARADO = 4;

short VAITEIA = PARADO;

void setup() {

	iniciaPinos(); // Coloca os pinos como OUTPUT
	inicializaLeds(); // Inicializa variavel Leds
	apagaLeds(); // Apaga todos os leds

	// Inicializa os pinos dos botões
	pinMode(START, INPUT);
	pinMode(RESET, INPUT);
	pinMode(DIREITA, INPUT);
	pinMode(ESQUERDA, INPUT);
	pinMode(ESTADO, INPUT);
}

void loop() {
	verificaBotao(); // Verifica qual botão foi apertado
	apagaLeds(); // Apaga todos os Leds
	inicializaLeds(); // Coloca todos as posições da variavel Leds para zero
	ligaLeds(); // Joga os valores de Leds para os pinos
}


void iniciaPinos(){//Coloca todos os pinos como OUTPUT
	for (short i=1; i<=16; i++) {
		pinMode(pins[i], OUTPUT);
	}
}


void inicializaLeds(){ 	//inicializa a variavel
	for (byte x=0; x<16; x++) {
		leds[x] = 0;
	}
}

void apagaLeds() { // apaga todos os leds
	for (short i = 1; i <= 16; i++) {
		digitalWrite(pins[i], LOW);
	}
}

void verificaBotao(){
	if(digitalRead(START)==HIGH)
		VAITEIA = START;
	if(digitalRead(RESET)==HIGH)
		VAITEIA = RESET;
	if(digitalRead(DIREITA)==HIGH)
		VAITEIA = DIREITA;
	if(digitalRead(ESQUERDA)==HIGH)
		VAITEIA = ESQUERDA;
	if(digitalRead(ESTADO)==HIGH)
		VAITEIA = ESTADO;
	if(VAITEIA==PARADO)
		VAITEIA = PARADO;
}


//Percorre todos os leds ligando e desligando
void ligaLeds() {
	for (byte x=0; x<=16; x++) {
		if (leds[x]==1) {
			pulseLed(x);
	
		}
	}
}

// Liga e desliga um led por um período curto
void pulseLed(byte x) {
	digitalWrite(fita[x], HIGH);
	delayMicroseconds(100);
	digitalWrite(fita[x], LOW);
}

void organizaFita(){
	short x = 1;
	q0(x);
}

void terminaFita(){
	while(1) {
		ligaLeds();
	}
}

//inicio da fita i=5;
//fim da fita B=8;




// AQUI COMEÇA A LOGICA DA MAQUINA DE TURING

void q0(short x){
	if (leds[x]==1){
		x++;
		q0(x);
	}	
	if (leds[x]==0){
		x++;
		q1(x);			
	}
}

void q1(short x){
	if (leds[x]==1){
		leds[x]=0;
		x--;
		q2(x);
	}
	if (leds[x]==0){
		x++;
		q1(x);			
	}
	if (leds[x]==8){
		x--;
		q4(x);
	}
}

void q2(short x){
	if (leds[x]==1){
		x++;
		q3(x);
	}	
	if (leds[x]==0){
		x--;
		q2(x);			
	}
	if (leds[x]==5){
		x++;
		q5(x);
	}
}

void q3(short x){
	if (leds[x]==1){
		terminaFita();
	}	
	if (leds[x]==0){
		x++;
		q2(x);			
	}
}

void q4(short x){
	if (leds[x]==1){
		x--;
		q4(x);
	}	
	if (leds[x]==0){
		x--;
		q4(x);			
	}
	if (leds[x]==5){
		x++;
		q6(x);		
	}
}

void q5(short x){
	if (leds[x]==1){
		
		terminaFita();
	}	
	if (leds[x]==0){
		leds[x]=1;
		x++;
		q0(x);		
	}
}

void q6(short x){
	if (leds[x]==1){
		x++;
		q7(x);
	}	
	if (leds[x]==0){
		x++;
		q11(x);
	}
}

void q7(short x){
	if (leds[x]==1){
		x++;
		q8(x);
	}	
	if (leds[x]==0){
		x++;
		q12(x);
	}
}

void q8(short x){
	if (leds[x]==1){
		x++;
		q9(x);
	}	
	if (leds[x]==0){
		x++;
		q12(x);
	}
}

void q9(short x){
	if (leds[x]==1){
		leds[x]=0;
		x++;
		q10(x);
	}	
	if (leds[x]==0){
		x++;
		q12(x);
	}
}

void q10(short x){
	if (leds[x]==1){
		x++;
		q10(x);
	}	
	if (leds[x]==0){
		leds[x]=1;
		x--;
		q11(x);
	}
}

void q11(short x){
	if (leds[x]==1){
		x--;
		q11(x);
	}	
	if (leds[x]==0){
		x++;
		q6(x);
	}
}

void q12(short x){
	ligaLeds();
}
