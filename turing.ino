/*
 * MAQUINA DE TURING
 *
 * Pinos do arduino
 * fita: 15, 14, 13, 12, 11, 10, 9, 8, 0, 1, 2, 3, 4, 5, 6, 7
 *
 *
 * TO-DO LIST:
 * - Verificar os pinos do Arduino
 * 
 *
 *
 * Outubro/2014
 */


short pins[38] = {//pinos do arduino
  -1,
  22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, //pinos da fita
  34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, //pinos dos estados
  45, 46, 47, 48, 49, 50, 51, 52, 53, 7, 8, 9 //pinos da cabeça
};

short fita[12] = {// define a fita e atribui o pino
  pins[1], //0
  pins[2],
  pins[3],
  pins[4],
  pins[5],
  pins[6],
  pins[7],
  pins[8],
  pins[9],
  pins[10],  
  pins[11],
  pins[12] //11
};

short estados[13] = {
  pins[13], //0
  pins[12],
  pins[13],
  pins[14],
  pins[15],
  pins[16],
  pins[17],
  pins[18],
  pins[19],
  pins[20],  
  pins[21],
  pins[22],
  pins[23] //12
};

short cabeca[12] = {
  pins[24], //0
  pins[25],
  pins[26],
  pins[27],
  pins[28],
  pins[29],
  pins[30],
  pins[31],
  pins[32],
  pins[33],  
  pins[34],
  pins[35] //11
};

short auxFita[14]; //variavel aux de fita
short auxEstados[13]; //variavel aux de estados
short auxCabeca[13]; //variavel aux de Cabeça
short posicao=1;
short estado=15;

//define as portas dos botões
short comeca=11;
short reinicia=A1;
short direita=A2;
short esquerda=A3;
short muda=A4;

void setup() {
  iniciaPinos(); //Coloca os pinos como OUTPUT
  testeInicial(); // Ascende todos os leds da placa
  inicializaLeds(); //Inicializa variavel Leds
  apagaLeds(); //Apaga todos os leds

    // Inicializa os pinos dos botões
  pinMode(comeca, INPUT);
  pinMode(reinicia, INPUT);
  pinMode(direita, INPUT);
  pinMode(esquerda, INPUT);
  pinMode(muda, INPUT);
}

void loop() {
  verificaBotao(); // Verifica qual botão foi apertado
  apagaLeds(); // Apaga todos os Leds
  estadoInicial(); // Carrega um valor padrão
  organizaFita(); // inicia as transições
  ligaLeds(); // Joga os valores de Leds para os pinos
}

void testeInicial(){ // ascende todos os leds da placa
  for (byte x=1; x<12; x++) {
    auxFita[x] = 1;
  }
  for (byte x=0; x<=13; x++) {
    auxEstados[x] = 1;
  }
  for (byte x=0; x<=12; x++) {
    auxCabeca[x] = 1;
  }
  ligaLeds();
}

void iniciaPinos() { //Coloca todos os pinos como OUTPUT
  for (short i = 0; i <= 38; i++) {
    pinMode(pins[i], OUTPUT);
  }
}

void inicializaLeds() { //inicializa a variavel com zero
  auxFita[0]=5;
  auxFita[13]=8;
  for (byte x=1; x<12; x++) {
    auxFita[x] = 0;
  }
  for (byte x=0; x<=13; x++) {
    auxEstados[x] = 0;
  }
  for (byte x=0; x<=12; x++) {
    auxCabeca[x] = 0;
  }
}


void apagaLeds() { //apaga todos os leds
  for (short i = 1; i <= 38; i++) {
    digitalWrite(pins[i], LOW);
  }
}

void verificaBotao() {
  if (digitalRead(comeca) == HIGH){
    estado = 0;
  }
  if (digitalRead(reinicia) == HIGH){
    executaReinicia();
  }
  if (digitalRead(direita) == HIGH){
    if (posicao>12){
      posicao++;
      estado = 16;
    }
  }	
  if (digitalRead(esquerda) == HIGH){
    if (posicao<0){
      posicao--;
      estado = 16;
    }
  }	
  if (digitalRead(muda) == HIGH){
    estado = 16;
    if (auxFita[posicao]=1)
      auxFita[posicao]=0;
    else
      auxFita[posicao]=1;		
  }	
}

void executaReinicia(){
  inicializaLeds(); // Coloca todos as posições da variavel Leds para zero
  estado = 15;
}

void estadoInicial(){
  if (estado == 15) {
    auxFita[0] = 5;
    auxFita[1] = 0; //1º led
    auxFita[2] = 1;
    auxFita[3] = 1;
    auxFita[4] = 0;
    auxFita[5] = 0;
    auxFita[6] = 1;
    auxFita[7] = 1;
    auxFita[8] = 0;
    auxFita[9] = 0;
    auxFita[10] = 1;
    auxFita[11] = 1;
    auxFita[12] = 1; //último led
    auxFita[13] = 8;
  }
}

//Percorre todos os leds ligando e desligando
void ligaLeds() {
  for (byte x=1; x<=12; x++) {
    if (auxFita[x] == 1) {
      digitalWrite(fita[x-1], HIGH);
      delayMicroseconds(100);
      digitalWrite(fita[x-1], LOW);
    }
  }
  for (byte x=0; x<=13; x++) {
    if (auxEstados[x] == 1) {
      digitalWrite(estados[x], HIGH);
      delayMicroseconds(100);
      digitalWrite(estados[x], LOW);
    }
  }
  for (byte x=0; x<=13; x++) {
    if (auxCabeca[x] == 1) {
      digitalWrite(cabeca[x], HIGH);
      delayMicroseconds(100);
      digitalWrite(cabeca[x], LOW);      
    }
  }

}

//inicio da fita i=5;
//fim da fita B=8;
//AQUI COMEÇA A LOGICA DA MAQUINA DE TURING

void organizaFita() {
  switch (estado) {
  case 0:
    auxEstados[0] = 1; //liga o led do estado atual
    if (auxFita[posicao] == 1) {
      posicao++;
      estado = 0;
      break;
    }
    if (auxFita[posicao] == 0) {
      posicao++;
      auxEstados[0] = 0; //desliga o led do estado atual quando muda de estado
      estado = 1;
      break;
    }
    break;
  case 1:
    auxEstados[1] = 1;//liga o led do estado atual
    if (auxFita[posicao] == 1) {
      auxFita[posicao] = 0;
      posicao--;
      auxEstados[1] = 0;
      estado = 2;
      break;
    }
    if (auxFita[posicao] == 0) {
      posicao++;
      estado = 1;
      break;
    }
    if (auxFita[posicao] == 8) {
      posicao--;
      auxEstados[1] = 0;
      estado = 4;
    }
    break;
  case 2:
    auxEstados[2] = 1;//liga o led do estado atual
    if (auxFita[posicao] == 1) {
      posicao++;
      auxEstados[2] = 0;
      estado = 3;
      break;
    }
    if (auxFita[posicao] == 0) {
      posicao--;
      estado = 2;
      break;
    }
    if (auxFita[posicao] == 5) {
      posicao++;
      auxEstados[2] = 0;
      estado = 5;
    }
    break;
  case 3:
    auxEstados[3] = 1;//liga o led do estado atual
    if (auxFita[posicao] == 1) {
      terminaFita();
      break;
    }
    if (auxFita[posicao] == 0) {
      posicao++;
      auxEstados[3] = 0;
      estado = 2;
    }
    break;
  case 4:
    auxEstados[4] = 1;//liga o led do estado atual
    if (auxFita[posicao] == 1) {
      posicao--;
      estado = 4;
      break;
    }
    if (auxFita[posicao] == 0) {
      posicao--;
      estado = 4;
      break;
    }
    if (auxFita[posicao] == 5) {
      posicao++;
      auxEstados[4] = 0;
      estado = 6;
    }
    break;
  case 5:
    auxEstados[5] = 1;    //liga o led do estado atual
    if (auxFita[posicao] == 1) {
      terminaFita();
      break;
    }
    if (auxFita[posicao] == 0) {
      auxFita[posicao] = 1;
      posicao++;
      auxEstados[5] = 0;      
      estado = 0;
      break;
    }
    break;
  case 6:
    auxEstados[6] = 1;    //liga o led do estado atual
    if (auxFita[posicao] == 1) {
      posicao++;
      auxEstados[6] = 0;      
      estado = 7;
      break;
    }
    if (auxFita[posicao] == 0) {
      terminaFita();
      break;
    }		
    break;
  case 7:
    auxEstados[7] = 1;//liga o led do estado atual
    if (auxFita[posicao] == 1) {
      posicao++;
      auxEstados[7] = 0;    
      estado = 8;
      break;
    }
    if (auxFita[posicao] == 0) {
      posicao++;
      auxEstados[7] = 0;
      estado = 12;
    }
    break;
  case 8:
    auxEstados[8] = 1;//liga o led do estado atual
    if (auxFita[posicao] == 1) {
      posicao++;
      auxEstados[8] = 0;
      estado = 9;
      break;
    }
    if (auxFita[posicao] == 0) {
      posicao++;
      auxEstados[8] = 0;
      estado = 12;
    }
    break;
  case 9:
    auxEstados[9] = 1;//liga o led do estado atual
    if (auxFita[posicao] == 1) {
      auxFita[posicao] = 0;
      posicao++;
      auxEstados[9] = 0;
      estado = 10;
      break;
    }
    if (auxFita[posicao] == 0) {
      posicao++;
      auxEstados[9] = 0;
      estado = 12;
    }
    break;
  case 10:
    auxEstados[10] = 1;//liga o led do estado atual
    if (auxFita[posicao] == 1) {
      posicao++;
      auxEstados[10] = 0;
      estado = 10;
      break;
    }
    if (auxFita[posicao] == 0) {
      auxFita[posicao] = 1;
      posicao--;
      auxEstados[11] = 0;
      estado = 11;
    }
    break;
  case 11:
    auxEstados[11] = 1;//liga o led do estado atual
    if (auxFita[posicao] == 1) {
      posicao--;
      estado = 11;
      break;
    }
    if (auxFita[posicao] == 0) {
      posicao++;
      auxEstados[11] = 0;
      estado = 6;
    }
    break;
  case 12:
    auxEstados[12] = 1;
    terminaFita();
    break;
  default: 
    break;
  }
}


void terminaFita() {
  while (1) {
    ligaLeds();
  }
}
