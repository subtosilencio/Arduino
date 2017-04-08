/***********************************************************************
  NOME DO ARQUIVO :        horteligente.ino

  DESCRIÇÃO : Sistema controle para hidroponia pessoal, controla bomba d'água, 
              grow ligth para manutenção interna, monitora pH, temperatura e umidade
              do ambiente, temperatura e condutividade da água e informa o período para
              realimentação de nutrientes.  

  TO-DO : 

  AUTOR : Paulo Dias       DATA INICIAL :    26 Fevereiro 2016

                              DATA FINAL :    05 Junho 2016
  
  NOTAS : Compatível com Arduino Mega 2560 e LCD TFT 2.4' touch Screen (ILI9328 - 0x9328)
          Sensores utilizados:
          DHT11 - Umidade e temperatura
          DS18B20 - Temperatura prova d'água
          EC - condutividade, fabricação própria
          PH0-14 e BNC PH - sensor de PH
          
************************************************************************/

#include <Adafruit_GFX.h>// Biblioteca da Adafruit
#include <SWTFT.h> // Biblioteca do Hardware
#include "TouchScreen.h" //Biblioteca do TouchScreen
#include "RTClib.h" //Bioblioteca para o relógio
#include "DHT.h" //Biblioteca do termometro e sensor de umidade
#include <Wire.h> //Biblioteca do sensor de temperatura d'água 
#include <OneWire.h> //Biblioteca do sensor de temperatura d'água
#include <EEPROM.h>  //Biblioteca para gravar informações na EEPROM
#include <Servo.h> //Biblioteca para os motores servo dos reguladores de ph
#include <string.h> //Biblioteca para trabalhar com váriaveis

//criando as cores mais usadas
#define BLACK  0x0000
#define WHITE  0xFFFF
#define RED    0xF800

//definição dos pinos para a função de Touch
#define YP A1
#define XM A2
#define YM 7
#define XP 6

//Sensor de Condutividade
#define CONDUTIVIDADE A10
int condVal;
float EC;

//Sensor de umidade
#define DHTPIN 51     // pino que está conectado o sensor
#define DHTTYPE DHT11

//sensor de temperatura
#define pinSensorTemp 53 // pino que está conectado o sensor
#define ph_pin  A15 //pino para leitura do do pH ligado ao Po

//define o máximo e minimo do touch para evitar erro
#define MINPRESSURE 10
#define MAXPRESSURE 1500

//Buffer para carregar a imagem BMP
#define BUFFPIXEL 60

//versão da informação na memória
#define CONFIG_VERSION 1

//pino do regulador de pH
#define reguladorDePHmais 34 // pino que está conectado o regulador mais
#define reguladorDePHmenos 36 // pino que está conectado o regulador menos

//intervalo de tempo em que a bomba é ligada
#define intervaloLeSensores 5

//variaveis do Rele
#define releBomba 40
#define releLed 42

//Variaveis de configuração recebidas da EEPROM
struct configStruct {
  int version;
  int horaLigaLuz;
  int horaDesligaLuz;
  int phMin;
  int phMax;
  int bombaDuracao;
  byte dataNutriente;
  unsigned long proximoAjuste;
} configuracao;


int tela = 1;   // define o que será exibido na tela
int carregaFundo = 1; // ajuda a carregar o fundo

//variaveis para leitura de temperatura
byte present = 0;
byte type_s;
byte data[12];
byte addr[8];
int celsius;

//variavel do sensor de nível
const byte pinoSensorNivel = 38;

//variaveis do sensor de umidade
float h;
float t;
float hic;

char OnOff[] = "OFF";
char ledOnOff[] = "Desligado";

//variaveis para leitura de pH
int measure;
double voltage;
float Po;

//Inicializa a variavel do reguladore de pH
int posicaoReguladorMais = 90; //posição 90 fechado e 30 aberto
int posicaoReguladorMenos = 90; //posição 90 fechado e 30 aberto

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300); //inicializa a tela
OneWire  ds(pinSensorTemp); // pino do sensor de temperatura da agua
SWTFT tft; //inicializa o tela de lcd
RTC_DS1307 RTC;// variavel do relogio
DateTime now; //inicializa sensor de relogio
DHT dht(DHTPIN, DHTTYPE); //inicializa sensor de umidade
Servo servoMainMais; //inicializa o servo do regulador de pH
Servo servoMainMenos; //inicializa o servo do regulador de pH


void setup() {

  //define a velociadade de conexão 
  Serial.begin(9600);

  //inicia o sistema com os relês desligados
  digitalWrite(releBomba, LOW);
  digitalWrite(releLed, LOW);

  //define os pinos como saida para injetar o sina no transitor
  pinMode(releBomba, OUTPUT);
  pinMode(releLed, OUTPUT);

  //define o pino como entrada para fazer a leitura do sinal
  pinMode (pinoSensorNivel, INPUT);

 //carrega para memória os dados da EEPROM
  leiaEEPROM(0, configuracao); 

  tft.reset();  //tela LCD
  Wire.begin(); //sensor temperatura agua
  RTC.begin();  //relogio
  dht.begin();  //umidade

  //inicializa as variaveis
  servoMainMais.attach(reguladorDePHmais); // regulador de pH Mais conectado no pino 34
  servoMainMais.write(posicaoReguladorMais);//coloca na posição fechado
  
  //inicializa as variaveis
  servoMainMenos.attach(reguladorDePHmenos); // regulador de pH Menos conectado no pino 35
  servoMainMenos.write(posicaoReguladorMenos); //coloca na posição fechado

  //verifica se shield do tempo esta funcionando e atualiza a hora
  if (! RTC.isrunning()) {
    Serial.println("RTC nao esta rodando...");
    //ajusta o relógio com a hora do computador 
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }

 //atualiza a variavei que guarda o tempo na memória
  now = RTC.now();
 
  //inicializa a variavel pegando o ID da hardware lcd
  uint16_t identifier = tft.readID();

  Serial.print(F("Encontrado o LCD driver: "));
  Serial.println(identifier, HEX);

  tft.begin(identifier);

  //coloca a tela na posição lateral
  tft.setRotation(1);

  //Faz a primeira leitura dos sensores 
  lerSensores();
  
  // desenha no LCD a tela inicial
  fundoInicial();

  

} //fim SETUP


void loop() {

 now = RTC.now();
 
 // faz a leitura dos sensores a cada 5 minutos
 for (int x = 0; x <= (60/intervaloLeSensores); x++){
  if (now.minute()==(intervaloLeSensores*x) )
    lerSensores();
 }

  //verifica se está na hora de ligar o LED
  if ( (now.hour() >= configuracao.horaLigaLuz) && (now.hour() <= configuracao.horaDesligaLuz)) {
    strcpy(ledOnOff, "Ligado"); //escreve na tela
    controlaLed(1);
  } else {
    strcpy(ledOnOff, "Desligado"); //escreve na tela
    controlaLed(0);
  }

 //chama a função que desliga a bomba pelo tempo configurado na variável
 for (int x = 0; x <= (60/configuracao.bombaDuracao); x++){
  if (now.minute()==(configuracao.bombaDuracao*x) )
    desligaBomba();
 }

 //chama a função que liga a bomba a cada 30 minutos
 if (now.minute()==30 || now.minute()==0){
  ligaBomba();
 }

  //verifica se algum botão foi apertado
 lerTouch();

}//fim loop

//função que faz a leitura de todos os sensores
void lerSensores() {
  lerTemperatura();
  lerUmidade();
  lerPH();
  lerEC();
}

void verificaRegulaPh(){
 if (now.day() >= configuracao.proximoAjuste){
  if(Po < configuracao.phMin){ //posição 90 fechado e 30 aberto
    regulaPHmais(30);
    delay(500);
    regulaPHmais(90);
    configuracao.proximoAjuste = now.day() + 1;
  }
  if(Po > configuracao.phMax){
    regulaPHmenos(30);
    delay(500);
    regulaPHmenos(90);
    configuracao.proximoAjuste = now.day() + 1;
   }
  }
  
}//fecha verificaRegulaPh

//função do regulador de pH mais
void regulaPHmais(int posicaoReguladorMais) {
  servoMainMais.write(posicaoReguladorMais);
  Serial.print("Posicao do regulador Mais: ");
  Serial.println(posicaoReguladorMais);
}

//função do regulador de pH menos
void regulaPHmenos(int posicaoReguladorMenos) {
  servoMainMenos.write(posicaoReguladorMenos);
  Serial.print("Posicao do regulador Menos: ");
  Serial.println(posicaoReguladorMenos);
}


void lerUmidade() {
  h = dht.readHumidity();
  t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("não foi possível ler o sensor de umidade");
  }

  // calcula a temperatura em graus Celsius
  hic = dht.computeHeatIndex(t, h, false);
}

void lerPH() {
  // resumo do calculo
  // PH_step = (voltage@PH7 - voltage@PH4) / (PH7 - PH4)
  // PH_probe = PH7 - ((voltage@PH7 - voltage@probe) / PH_step)
  
  // faz a leitura do sensor
  measure = analogRead(ph_pin);

  //faz o calculo
  voltage = 5 / 1024.0 * measure;
  Po = 7 + ((2.5 - voltage) / 0.18);
}

//função que liga e desliga a bomba
void ligaBomba() { 
  digitalWrite(releBomba, HIGH);
  strcpy(OnOff, "ON");//escreve na tela ON
  Serial.println("Bomba Ligada");
}

void desligaBomba() { 
  digitalWrite(releBomba, LOW);
  strcpy(OnOff, "OFF");//escreve na tela OFF
  Serial.println("Bomba desligado");
}

//função que liga e desliga o LED
void controlaLed(int ligado) { 
  digitalWrite(releLed, ligado);//1 ligado, 0 desligado
  //Serial.print("LED: ");
  //Serial.println(ligado);
}

//função que faz a leitura do botão pressionado na tela
void lerTouch() {
  // a variavel p guarda a informação de onde foi pressionado
  TSPoint p = ts.getPoint();

  //inicialização dos pinos para o touch
  digitalWrite(13, LOW);
  pinMode(XP, OUTPUT);
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  pinMode(YM, OUTPUT);


  // se tiver não tiver toque a pressão é 0
  if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
    Serial.print("X = "); Serial.print(p.x);
    Serial.print(" Y = "); Serial.print(p.y);
    Serial.print(" Pressao = "); Serial.println(p.z);
  }

  //verifica se foi selecionado o botão para ir tela de configuração
  if ((p.z > MINPRESSURE && p.z < MAXPRESSURE) && (tela == 1))
  {
    if ((p.x - 130 <= 300) && (p.x - 130 >= 100) && (p.y >= 100) && (p.y <= 300))
    {
      fundoConfig();
      tela = 2;
      carregaFundo = 2;
    }
  }



/*
* exibe o botão informando que precisa abastecer o nutriente
* O botão é exibido se tiver passado 15 dias da última data em que o nutriente foi abastecido
*/
 if ((p.z > MINPRESSURE && p.z < MAXPRESSURE) && (tela == 1) && ((configuracao.dataNutriente - now.day()) >= 15)){
  if ((p.x >= 190) && (p.x <= 500) && (p.y >= 600) && (p.y <= 720)) {
    configuracao.dataNutriente = now.day();//escreve a data de hoje 
    escreveEEPROM(0, configuracao); //guarda a informação na EEPROM    
  }
 }
 
  //botão de volta
  if ((p.z > MINPRESSURE && p.z < MAXPRESSURE) && (tela == 2))
  {
    if ((p.x >= 120) && (p.x <= 320) && (p.y >= 780) && (p.y <= 840))
    {
      tela = 1;
      carregaFundo = 1;
      fundoInicial();
      escreveEEPROM(0, configuracao);
    }
  }

  //botão +H da hora
  if ((p.z > MINPRESSURE && p.z < MAXPRESSURE) && (tela == 2))
    if ((p.x >= 450) && (p.x <= 505) && (p.y >= 315) && (p.y <= 370)) {
      RTC.adjust(DateTime(now.year(), now.month(), now.day(), now.hour() + 1, now.minute(), now.second()));
    }

  //botão +M da hora
  if ((p.z > MINPRESSURE && p.z < MAXPRESSURE) && (tela == 2))
    if ((p.x >= 345) && (p.x <= 405) && (p.y >= 315) && (p.y <= 370)) {
      if (now.minute() == 60) {
        RTC.adjust(DateTime(now.year(), now.month(), now.day(), now.hour(), 0, now.second()));
      } else
        RTC.adjust(DateTime(now.year(), now.month(), now.day(), now.hour(), now.minute() + 1, now.second()));
    }

  //botão +H da Luz ON
  if ((p.z > MINPRESSURE && p.z < MAXPRESSURE) && (tela == 2))
    if ((p.x >= 450) && (p.x <= 505) && (p.y >= 400) && (p.y <= 470))
    { if (configuracao.horaLigaLuz == 24)
        configuracao.horaLigaLuz = 1;
      else
        configuracao.horaLigaLuz = configuracao.horaLigaLuz + 1;
    }

  //botão -H da Luz ON
  if ((p.z > MINPRESSURE && p.z < MAXPRESSURE) && (tela == 2))
    if ((p.x >= 345) && (p.x <= 405) && (p.y >= 400) && (p.y <= 470))
    { if (configuracao.horaLigaLuz == 1)
        configuracao.horaLigaLuz = 24;
      else
        configuracao.horaLigaLuz = configuracao.horaLigaLuz - 1;
    }

  //botão +H da Luz off
  if ((p.z > MINPRESSURE && p.z < MAXPRESSURE) && (tela == 2))
    if ((p.x >= 450) && (p.x <= 505) && (p.y >= 500) && (p.y <= 570))
    { if (configuracao.horaDesligaLuz == 24)
        configuracao.horaDesligaLuz = 1;
      else
        configuracao.horaDesligaLuz = configuracao.horaDesligaLuz + 1;
    }

  //botão -M da Luz
  if ((p.z > MINPRESSURE && p.z < MAXPRESSURE) && (tela == 2))
    if ((p.x >= 345) && (p.x <= 405) && (p.y >= 500) && (p.y <= 570))
    { if (configuracao.horaDesligaLuz == 1)
        configuracao.horaDesligaLuz = 24;
      else
        configuracao.horaDesligaLuz = configuracao.horaDesligaLuz - 1;
    }

  //botão +P do pH max
  if ((p.z > MINPRESSURE && p.z < MAXPRESSURE) && (tela == 2))
    if ((p.x >= 450) && (p.x <= 505) && (p.y >= 600) && (p.y <= 670))
    { if (configuracao.phMax == 14)
        configuracao.phMax = 1;
      else
        configuracao.phMax = configuracao.phMax + 1;
    }


  //botão -P do pH max
  if ((p.z > MINPRESSURE && p.z < MAXPRESSURE) && (tela == 2))
    if ((p.x >= 345) && (p.x <= 405) && (p.y >= 600) && (p.y <= 670))
    { if (configuracao.phMax == 1)
        configuracao.phMax = 14;
      else
        configuracao.phMax = configuracao.phMax - 1;
    }

  //botão +P do pH min
  if ((p.z > MINPRESSURE && p.z < MAXPRESSURE) && (tela == 2))
    if ((p.x >= 450) && (p.x <= 505) && (p.y >= 700) && (p.y <= 770))
    { if (configuracao.phMin == 14)
        configuracao.phMin = 1;
      else
        configuracao.phMin = configuracao.phMin + 1;
    }


  //botão -P do pH min
  if ((p.z > MINPRESSURE && p.z < MAXPRESSURE) && (tela == 2))
    if ((p.x >= 345) && (p.x <= 405) && (p.y >= 700) && (p.y <= 770))
    { if (configuracao.phMin == 1)
        configuracao.phMin = 14;
      else
        configuracao.phMin = configuracao.phMin - 1;
    }

  //botão +M da Bomba
  if ((p.z > MINPRESSURE && p.z < MAXPRESSURE) && (tela == 2))
    if ((p.x >= 450) && (p.x <= 505) && (p.y >= 800) && (p.y <= 870))
    { if (configuracao.bombaDuracao == 60)
        configuracao.bombaDuracao = 1;
      else
        configuracao.bombaDuracao = configuracao.bombaDuracao + 1;
    }


  //botão -M da Bomba
  if ((p.z > MINPRESSURE && p.z < MAXPRESSURE) && (tela == 2))
    if ((p.x >= 345) && (p.x <= 405) && (p.y >= 800) && (p.y <= 870))
    { if (configuracao.bombaDuracao == 1)
        configuracao.bombaDuracao = 60;
      else
        configuracao.bombaDuracao = configuracao.bombaDuracao - 1;
    }

  if (tela == 1)
    if (carregaFundo == 1)
      telaInicial();
    else
    {
      fundoInicial();
      telaInicial();
      carregaFundo = 1;
    }
  else
    telaConfig();


}

//guarda na EEPROM a informação de configuração
template <class T> int escreveEEPROM(int ee, const T& value)
{
  const byte* p = (const byte*)(const void*)&value;
  unsigned int i;
  for (i = 0; i < sizeof(value); i++)
    EEPROM.write(ee++, *p++);
  return i;
}

//guarda na EEPROM a informação de configuração
template <class T> int leiaEEPROM(int ee, T& value)
{
  byte* p = (byte*)(void*)&value;
  unsigned int i;
  for (i = 0; i < sizeof(value); i++)
    *p++ = EEPROM.read(ee++);
  return i;
}

void lerTemperatura() {

  byte i;

  ds.search(addr);

  // the first ROM byte indicates which chip
  switch (addr[0]) {
    case 0x10:
      type_s = 1;
      break;
    case 0x28:
      type_s = 0;
      break;
    case 0x22:
      type_s = 0;
      break;
    default:
      return;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end

  present = ds.reset();
  ds.select(addr);
  ds.write(0xBE);         // Read Scratchpad

  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
  }

  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (int)raw / 16;
  //Serial.print(F("Temperatura: "));
  //Serial.println(celsius);

}//fim ler temperatura



void fundoInicial() {
  //preenche de azul a parte de cima
  tft.fillRect(0, 0, 320, 55, tft.color565(51, 102, 153));
  //preenche de azul mais escuro a parte de baixo
  tft.fillRect(0, 55, 320, 200, tft.color565(0, 51, 102));

  //desenha linha
  tft.drawLine(160, 55, 160, 205, BLACK);
  tft.drawLine(160, 155, 450, 155, BLACK);
  tft.drawLine(0, 205, 480, 205, BLACK);

}


void fundoConfig() {
  //preenche de azul a parte de cima
  tft.fillRect(0, 0, 320, 55, tft.color565(51, 102, 153));
  //preenche de azul mais escuro a parte de baixo
  tft.fillRect(0, 55, 320, 200, tft.color565(0, 51, 102));
}


/***********************************************************************
* Função que exibe as informações na tela de configuração 
* 
***********************************************************************/
void telaConfig() {
  tft.setCursor(40, 17);
  tft.setTextColor(BLACK);
  tft.setTextSize(3);
  tft.print("CONFIGURACAO");

  tft.fillRect(60, 59, 80, 25, tft.color565(0, 51, 102));
  tft.setCursor(10, 60);
  tft.setTextColor(BLACK);
  tft.setTextSize(2);
  tft.print("Hora: ");
  tft.print(now.hour(), DEC);
  tft.print(":");
  if (now.minute() < 10)
  {
    tft.print("0");
    tft.print(now.minute(), DEC);
  }
  else
    tft.print(now.minute(), DEC);
  tft.fillRoundRect(170, 58, 25, 20, 3, tft.color565(50, 50, 50));
  tft.setCursor(170, 60);
  tft.print("+H");
  tft.fillRoundRect(210, 58, 25, 20, 3, tft.color565(50, 50, 50));
  tft.setCursor(210, 60);
  tft.print("+M");

  tft.fillRect(90, 89, 35, 25, tft.color565(0, 51, 102));
  tft.setCursor(10, 90);
  tft.setTextColor(BLACK);
  tft.setTextSize(2);
  tft.print("Luz ON: ");
  tft.print(configuracao.horaLigaLuz);
  tft.fillRoundRect(170, 88, 25, 20, 3, tft.color565(50, 50, 50));
  tft.setCursor(170, 90);
  tft.print("+H");
  tft.fillRoundRect(210, 88, 25, 20, 3, tft.color565(50, 50, 50));
  tft.setCursor(210, 90);
  tft.print("-H");

  tft.fillRect(100, 119, 40, 25, tft.color565(0, 51, 102));
  tft.setCursor(10, 120);
  tft.setTextColor(BLACK);
  tft.setTextSize(2);
  tft.print("Luz OFF: ");
  tft.print(configuracao.horaDesligaLuz);
  tft.fillRoundRect(170, 118, 25, 20, 3, tft.color565(50, 50, 50));
  tft.setCursor(170, 120);
  tft.print("+H");
  tft.fillRoundRect(210, 118, 25, 20, 3, tft.color565(50, 50, 50));
  tft.setCursor(210, 120);
  tft.print("-H");

  tft.fillRect(90, 149, 35, 25, tft.color565(0, 51, 102));
  tft.setCursor(10, 150);
  tft.setTextColor(BLACK);
  tft.setTextSize(2);
  tft.print("pH MAX: ");
  tft.print(configuracao.phMax);
  tft.fillRoundRect(170, 148, 25, 20, 3, tft.color565(50, 50, 50));
  tft.setCursor(170, 150);
  tft.print("+P");
  tft.fillRoundRect(210, 148, 25, 20, 3, tft.color565(50, 50, 50));
  tft.setCursor(210, 150);
  tft.print("-P");

  tft.fillRect(90, 179, 35, 25, tft.color565(0, 51, 102));
  tft.setCursor(10, 180);
  tft.setTextColor(BLACK);
  tft.setTextSize(2);
  tft.print("pH MIN: ");
  tft.print(configuracao.phMin);
  tft.fillRoundRect(170, 178, 25, 20, 3, tft.color565(50, 50, 50));
  tft.setCursor(170, 180);
  tft.print("+P");
  tft.fillRoundRect(210, 178, 25, 20, 3, tft.color565(50, 50, 50));
  tft.setCursor(210, 180);
  tft.print("-P");

  tft.fillRect(85, 209, 35, 25, tft.color565(0, 51, 102));
  tft.setCursor(10, 210);
  tft.setTextColor(BLACK);
  tft.setTextSize(2);
  tft.print("Bomba: ");
  tft.print(configuracao.bombaDuracao);
  tft.fillRoundRect(170, 208, 25, 20, 3, tft.color565(50, 50, 50));
  tft.setCursor(170, 210);
  tft.print("+M");
  tft.fillRoundRect(210, 208, 25, 20, 3, tft.color565(50, 50, 50));
  tft.setCursor(210, 210);
  tft.print("-M");

  tft.setCursor(245, 210);
  tft.setTextColor(BLACK);
  tft.setTextSize(2);
  tft.print("Volta");
}



/***********************************************************************
* Função que exibe as informações na tela inicial  
* 
***********************************************************************/
void telaInicial() {

  tft.setCursor(5, 1);
  tft.setTextColor(BLACK);
  tft.setTextSize(1);
  tft.print("Temp");

  tft.setCursor(75, 1);
  tft.setTextColor(BLACK);
  tft.setTextSize(1);
  tft.print("Umidade");

  tft.fillRoundRect(235, 15, 80, 27, 3, tft.color565(50, 50, 50));
  tft.setCursor(240, 20);
  tft.setTextColor(BLACK);
  tft.setTextSize(2);
  tft.print("Opcoes");

  tft.drawRoundRect(133, 15, 70, 27, 3, BLACK);
  tft.fillRect(138, 20, 60, 18, tft.color565(51, 102, 153));
  tft.setCursor(138, 20);
  tft.setTextColor(BLACK);
  tft.setTextSize(2);
  tft.print(now.hour(), DEC);
  tft.print(":");
  if (now.minute() < 10)
  {
    tft.print("0");
    tft.print(now.minute(), DEC);
  }
  else
    tft.print(now.minute(), DEC);

  tft.fillRect(5, 17, 36, 25, tft.color565(51, 102, 153));
  tft.setCursor(5, 17);
  tft.setTextColor(BLACK);
  tft.setTextSize(3);
  tft.print(t, 0); //temperatura do ambiente
  tft.print("C");

  tft.fillRect(70, 17, 36, 25, tft.color565(51, 102, 153));
  tft.setCursor(70, 17);
  tft.setTextColor(BLACK);
  tft.setTextSize(3);
  tft.print(h, 0); //umidade do ambiente
  tft.print("%");

  tft.setCursor(10, 63);
  tft.setTextColor(BLACK);
  tft.setTextSize(3);
  tft.print("AGUA");

  tft.fillRect(70, 94, 50, 25, tft.color565(0, 51, 102));
  tft.setCursor(10, 95);
  tft.setTextColor(BLACK);
  tft.setTextSize(2);
  tft.print("Temp: ");
  tft.print(celsius); //temperatura da agua
  tft.print("'");
  tft.print("C");

  tft.fillRect(90, 124, 60, 25, tft.color565(0, 51, 102));
  tft.setCursor(10, 125);
  tft.setTextColor(BLACK);
  tft.setTextSize(2);
  tft.print("Condu: ");
  tft.print(EC); //temperatura da agua

  tft.fillRect(60, 154, 60, 25, tft.color565(0, 51, 102));
  tft.setCursor(10, 155);
  tft.setTextColor(BLACK);
  tft.setTextSize(2);
  tft.print("pH: ");
  tft.print(Po * -1, 1);

  tft.fillRect(90, 184, 40, 20, tft.color565(0, 51, 102));
  tft.setCursor(10, 185);
  tft.setTextColor(BLACK);
  tft.setTextSize(2);
  tft.print("Bomba: ");
  tft.print(OnOff);

  tft.setCursor(10, 210);
  tft.setTextColor(BLACK);
  tft.setTextSize(1);
  tft.print("AVISOS");

  tft.setCursor(10, 220);
  tft.setTextColor(BLACK);
  tft.setTextSize(1);
  //verifica o nível da d'água e exibe aviso
  if (digitalRead (pinoSensorNivel) == HIGH) 
    tft.print("Nivel dagua normal!");
    else
    tft.print("Nivel dagua esta baixo!");
    
  

  tft.setCursor(170, 63);
  tft.setTextColor(BLACK);
  tft.setTextSize(3);
  tft.print("LUZ");

  tft.setCursor(175, 100);
  tft.setTextColor(BLACK);
  tft.setTextSize(2);
  tft.print(configuracao.horaLigaLuz);
  tft.print("h - ");
  tft.print(configuracao.horaDesligaLuz);
  tft.print("h");

  tft.fillRect(199, 124, 100, 25, tft.color565(0, 51, 102));
  tft.setCursor(200, 125);
  tft.setTextColor(BLACK);
  tft.setTextSize(2);
  tft.print(ledOnOff);

  tft.setCursor(170, 160);
  tft.setTextColor(BLACK);
  tft.setTextSize(2);
  tft.print("NUTRIENTES");

  tft.fillRect(169, 184, 120, 20, tft.color565(0, 51, 102));
  tft.setCursor(170, 185);
  tft.setTextColor(BLACK);
  tft.setTextSize(2);
  tft.print(now.day() - configuracao.dataNutriente);

  tft.setCursor(220, 185);
  tft.setTextColor(BLACK);
  tft.setTextSize(2);
  tft.print("dias");
  
  if ( (now.day() - configuracao.dataNutriente) >= 15 ){
    tft.fillRoundRect(169, 184, 120, 20, 3, tft.color565(50, 50, 50));
    tft.setCursor(170, 185);
    tft.print("Abastecer");
  }


}

void lerEC() {
  condVal = analogRead(CONDUTIVIDADE);
  EC = condVal * (5.0 / 1023.0); //calcula a conductividade
}
