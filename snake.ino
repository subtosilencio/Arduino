/*
 * JOGO DA COBRINHA
 * 
 * Também conhecido como Snake
 * 
 * Configuração de 8x8
 *
 * Pinos do arduino
 * Colunas: 5,4,3,2,1,O,14,15
 * Linhas:  13,12,11,10,9,8,7,6
 *
 *
 * Paulo D.
 * eu@paulo.eng.br
 * Abril/2014
 */

// Fazendo desta forma o processamento de inicialização é menor, o primeiro valor não conta
short pins[17]= {
  -1, 15, 14, 13, 12, 11, 10, 9, 8, 0, 1, 2, 3, 4, 5, 6, 7};

//pinos em x - colunas
short colunas[8] = {
  pins[16], pins[15], pins[14], pins[13], pins[12], pins[11], pins[10], pins[9]};

//pinos em y - linhas
short linhas[8] = {
  pins[8], pins[7], pins[6], pins[5], pins[4], pins[3], pins[2], pins[1]};  


// Pinos dos botoes
short cima=16, baixo=17, direita=18, esquerda=19;

unsigned long prevTime = 0;  
unsigned long delayTime = 500;
unsigned long fruitPrevTime = 0;
unsigned long fruitBlinkTime = 900/250;
int fruitLed = 1;


//        x  y
byte leds[8][8];                //variavel que guarda posição da cobrinha e comida
const short tamanho_maximo = 10;
short tamanho_atual = 1;   
short cobrinhaX[tamanho_maximo];                      
short cobrinhaY[tamanho_maximo];
short frutaX, frutaY;

const short CIMA     = 0;
const short BAIXO    = 1;
const short DIREITA  = 2;
const short ESQUERDA = 3;

short direcao = CIMA;

void setup() {
  int seed = get_seed(19);  
  randomSeed(seed);
  UCSR0B = 0;
  iniciaPinos();            // Coloca os pinos como OUTPUT
  inicializaLeds();         // Inicializa variavel Leds
  apagaLeds();              // Apaga todos os leds
  inicializaCobrinha();

  // Inicializa os pinos dos botões  
  pinMode(cima,     INPUT);
  pinMode(baixo,    INPUT);
  pinMode(direita,  INPUT);
  pinMode(esquerda, INPUT);

  frutaX  = random(1, 7);
  frutaY  = random(1, 7);

  // começa o joga com a cobrinha no meio
  cobrinhaX[0] = 3; 
  cobrinhaY[0] = 3;
  criaFruta();
}

void loop() {
  verificaBotao();         // Verifica qual botão foi apertado      
  unsigned long currentTime = millis();
  if(currentTime - prevTime >= delayTime){
    proximoPasso(); 
    prevTime = currentTime;
  }
  apagaLeds();             // Apaga todos os Leds
  inicializaLeds();        // Coloca todos as posições da variavel Leds para zero
  desenhaCobrinha();       // joga os valores da variavel cobrinha para a variavel Leds
  desenhaFruta();
  ligaLeds();              // Joga os valores de Leds para os pinos
}

void apagaLeds() {  // apaga todos os leds
  for (short i = 1; i <= 16; i++) {
    digitalWrite(pins[i], LOW);
  }
}

void inicializaLeds(){ //inicializa variavel que guarda posição da cobrinha
  //inicializa a variavel
  for (byte x=0; x<8; x++) {
    for (byte y=0; y<8; y++) {
      leds[x][y] = 0;
    }
  }
}

void verificaBotao(){
  if(digitalRead(cima)==HIGH&&direcao!=BAIXO)
    direcao = CIMA;
  if(digitalRead(baixo)==HIGH&&direcao!=CIMA)
    direcao = BAIXO;
  if(digitalRead(direita)==HIGH&&direcao!=ESQUERDA)
    direcao = DIREITA;
  if(digitalRead(esquerda)==HIGH&&direcao!=DIREITA)
    direcao = ESQUERDA;
}

void proximoPasso(){
  for(short i=tamanho_atual-1; i>0; i--){
    cobrinhaX[i] = cobrinhaX[i-1];
    cobrinhaY[i] = cobrinhaY[i-1];
  } 

  if(direcao == CIMA){
    if (cobrinhaY[0] == 7){
      cobrinhaY[0] = 0;
      delay(190);
    }
    else{
      cobrinhaY[0] = cobrinhaY[0]+1;
      delay(190);
    }
  }

  if(direcao == BAIXO){
    if (cobrinhaY[0] == 0){
      cobrinhaY[0] = 7;
      delay(190);
    }
    else{
      cobrinhaY[0] = cobrinhaY[0]-1;
      delay(190);
    }
  }

  if(direcao == DIREITA){
    if (cobrinhaX[0] == 7){
      cobrinhaX[0] = 0;
      delay(190);
    }
    else{
      cobrinhaX[0] = cobrinhaX[0]+1;
      delay(190);
    }
  }

  if(direcao == ESQUERDA){
    if (cobrinhaX[0] == 0){
      cobrinhaX[0] = 7;
      delay(190);
    }
    else{
      cobrinhaX[0] = cobrinhaX[0]-1;
      delay(190);    
    }
  }

  if((cobrinhaX[0] == frutaX) && (cobrinhaY[0] == frutaY)){
    tamanho_atual++;
    if(tamanho_atual < tamanho_maximo)
      criaFruta();
    else
      terminaJogo();
  } 
}

void terminaJogo(){
  for (byte x=0; x<8; x++) {
    for (byte y=0; y<8; y++) {
      leds[x][y] = 1;
    }
  }
  delay(800);
  while(1) {
    ligaLeds();
  }
}

//Percorre todos os leds ligando e desligando
void ligaLeds() {
  for (byte linha=0; linha<8; linha++) {
    for (byte coluna=0; coluna<8; coluna++) {
      if (leds[linha][coluna]==1) {
        pulseLed(linha, coluna);
      }
    }
  }
}

// Liga e desliga um led por um período curto
void pulseLed(byte linha, byte coluna) {
  digitalWrite(linhas[linha], HIGH);
  digitalWrite(colunas[coluna], HIGH); 
  delayMicroseconds(100);
  digitalWrite(linhas[linha], LOW);
  digitalWrite(colunas[coluna], LOW); 
}

void iniciaPinos(){//Coloca todos os pinos como OUTPUT
  for (short i=1; i<=16; i++) {
    pinMode(pins[i], OUTPUT);
  }
}

// joga os valores da variavel Led para os pinos
void desenhaCobrinha(){
  for(short i=0; i<tamanho_atual; i++){
    leds[cobrinhaX[i]][cobrinhaY[i]] = 1;
  }
}

// Inicializa a variável colocando zero em todas ela
void inicializaCobrinha(){
  for(short i=0; i<tamanho_maximo; i++){
    cobrinhaX[i] = 0;
    cobrinhaY[i] = 0;
  }
}

boolean inPlayField(byte x, byte y){
  return (x>=0) && (x<8) && (y>=0) && (y<8);
}

void desenhaFruta(){
  if(inPlayField(frutaX, frutaY)){
    unsigned long currenttime = millis();
    if(currenttime - fruitPrevTime >= fruitBlinkTime){
      fruitLed = (fruitLed == 1) ? 0 : 1;
      fruitPrevTime = currenttime;
    }
    leds[frutaX][frutaY] = fruitLed;
  }
}

void criaFruta(){
  int x, y;
  x = random(0, 8);
  y = random(0, 8);
  while(isPartOfSnake(x, y)){
    x = random(0, 8);
    y = random(0, 8);
  }
  frutaX = x;
  frutaY = y;
}

boolean isPartOfSnake(int x, int y){
  for(int i=0; i<tamanho_atual-1; i++){
    if((x == cobrinhaX[i]) && (y == cobrinhaY[i])){
      return true;
    }
  }
  return false;
}


/*
* função para gerar o SEED Aleatório
 * Fonte: http://forum.arduino.cc/index.php/topic,69877
 * Usuário: Gardner 
 * Postado em: September 09, 2011
 */
uint32_t get_seed(int pin){
  uint16_t aread =0;
  union {
    uint32_t as_uint32_t;
    uint8_t  as_uint8_t[4];
  } 
  seed;
  uint8_t i, t;
  for (i = 0; i < 240; i++) {
    aread <<= 3;
    aread |= analogRead(pin) & 0x7;
    t = seed.as_uint8_t[0];
    seed.as_uint8_t[0] = seed.as_uint8_t[3];
    seed.as_uint8_t[3] = seed.as_uint8_t[1];
    seed.as_uint8_t[1] = seed.as_uint8_t[2];
    seed.as_uint8_t[2] = t;
    seed.as_uint32_t ^= aread;
  }
  return(seed.as_uint32_t);
}
