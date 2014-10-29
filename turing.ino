/*
 * MAQUINA DE TURING
 *
 * Pinos do arduino
 * fita: 15, 14, 13, 12, 11, 10, 9, 8, 0, 1, 2, 3, 4, 5, 6, 7
 *
 *
 * TO-DO LIST:
 * - Integrar organizaFita() com verificaBotao()
 * - Corrigir a função terminaFita()
 * - Verificar os pinos do Arduino
 * - Corrigir o vaiteia no estado parado
 * - Acender e apagar os leds durante as transições da maquina
 * - Acender todos os leds para testar
 *
 *
 *
 *
 *
 *
 * Outubro/2014
 */
short pins[12] = {
	-1,
	15,
	14,
	13,
	12,
	11,
	10,
	9,
	8,
	0,
	1,
	2,
	3,
	4,
	5,
	6,
	7
}; //corrigir

short fita[12] = {
	pins[8],
	pins[7],
	pins[6],
	pins[5],
	pins[4],
	pins[3],
	pins[2],
	pins[1]
};

short estados[13] = {
	pins[8],
	pins[7],
	pins[6],
	pins[5],
	pins[4],
	pins[3],
	pins[2],
	pins[1]
};

short leds[13];
short transicao[13];

// Pinos dos botoes
short start = 16, reset = 17, direita = 18, esquerda = 19, muda = 20;

short estado = 15;

short x = 1;

void setup() {
	iniciaPinos(); // Coloca os pinos como OUTPUT
	inicializaLeds(); // Inicializa variavel Leds
	apagaLeds(); // Apaga todos os leds

	// Inicializa os pinos dos botões
	pinMode(start, INPUT);
	pinMode(reset, INPUT);
	pinMode(direita, INPUT);
	pinMode(esquerda, INPUT);
	pinMode(muda, INPUT);
}

void loop() {
	verificaBotao(); // Verifica qual botão foi apertado
	apagaLeds(); // Apaga todos os Leds
	organizaFita(); // inicia as transições
	ligaLeds(); // Joga os valores de Leds para os pinos
}

void iniciaPinos() { //Coloca todos os pinos como OUTPUT
	for (short i = 1; i <= 16; i++) {
		pinMode(pins[i], OUTPUT);
	}
}

void inicializaLeds() { //inicializa a variavel
	led[0]=5;
	led[13]=8;
	for (byte x=1; x<=12; x++) {
		leds[x] = 0;
	}
	for (byte x=0; x<=13; x++) {
		transicao[x] = 0;
	}
}

void apagaLeds() { //apaga todos os leds
	for (short i = 1; i <= 12; i++) {
		digitalWrite(pins[i], LOW);
	}
}

void verificaBotao() {
	if (digitalRead(START) == HIGH){
		estado = 0;
	}
	if (digitalRead(RESET) == HIGH){
		reset();
	}
	if (digitalRead(DIREITA) == HIGH){
	
	}	
	if (digitalRead(ESQUERDA) == HIGH){
	
	}	
	if (digitalRead(MUDA) == HIGH){
	
	}	
}

void reset(){
	inicializaLeds(); // Coloca todos as posições da variavel Leds para zero
	estado = 15;
}

//Percorre todos os leds ligando e desligando
void ligaLeds() {
	for (byte x=1; x<=12; x++) {
		if (leds[x] == 1) {
			pulseLed(x);
		}
	}
	for (byte x=0; x<=13; x++) {
		if (transicao[x] == 1) {
			pulseLed(x);
		}
	}
}

// Liga e desliga um led por um período curto
void pulseLed(byte x) {
	digitalWrite(fita[x], HIGH);
	digitalWrite(transicao[x], HIGH);
	delayMicroseconds(100);
	digitalWrite(fita[x], LOW);
	digitalWrite(transicao[x], LOW);
}

//inicio da fita i=5;
//fim da fita B=8;
//AQUI COMEÇA A LOGICA DA MAQUINA DE TURING

void organizaFita() {
// inserir delay
	if (estado == 0) {
		if (leds[x] == 1) {
			x++;
			estado = 0;
		}
		if (leds[x] == 0) {
			x++;
			estado = 1;
		}
	}

	if (estado == 1) {
		if (leds[x] == 1) {
			leds[x] = 0;
			x--;
			estado = 2;
		}
		if (leds[x] == 0) {
			x++;
			estado = 1;
		}
		if (leds[x] == 8) {
			x--;
			estado = 4;
		}
	}

	if (estado == 2) {
		if (leds[x] == 1) {
			x++;
			estado = 3;
		}
		if (leds[x] == 0) {
			x--;
			estado = 2;
		}
		if (leds[x] == 5) {
			x++;
			estado = 5;
		}
	}

	if (estado == 3) {
		if (leds[x] == 1) {
			terminaFita();
		}
		if (leds[x] == 0) {
			x++;
			estado = 2;
		}
	}

	if (estado == 4) {
		if (leds[x] == 1) {
			x--;
			estado = 4;
		}
		if (leds[x] == 0) {
			x--;
			estado = 4;
		}
		if (leds[x] == 5) {
			x++;
			estado = 6;
		}
	}

	if (estado == 5) {
		if (leds[x] == 1) {
			terminaFita();
		}
		if (leds[x] == 0) {
			leds[x] = 1;
			x++;
			estado = 0;
		}
	}

	if (estado == 6) {
		if (leds[x] == 1) {
			x++;
			estado = 7;
		}
		if (leds[x] == 0) {
			terminaFita();
		}		
	}

	if (estado == 7) {
		if (leds[x] == 1) {
			x++;
			estado = 8;
		}
		if (leds[x] == 0) {
			x++;
			estado = 12;
		}
	}

	if (estado == 8) {
		if (leds[x] == 1) {
			x++;
			estado = 9;
		}
		if (leds[x] == 0) {
			x++;
			estado = 12;
		}
	}

	if (estado == 9) {
		if (leds[x] == 1) {
			leds[x] = 0;
			x++;
			estado = 10;
		}
		if (leds[x] == 0) {
			x++;
			estado = 12;
		}
	}

	if (estado == 10) {
		if (leds[x] == 1) {
			x++;
			estado = 10;
		}
		if (leds[x] == 0) {
			leds[x] = 1;
			x--;
			estado = 11;
		}
	}

	if (estado == 11) {
		if (leds[x] == 1) {
			x--;
			estado = 11;
		}
		if (leds[x] == 0) {
			x++;
			estado = 6;
		}
	}

	if (estado == 12) {
		ligaLeds();
	}

}
void terminaFita() {
	while (1) {
		ligaLeds();
	}
}
