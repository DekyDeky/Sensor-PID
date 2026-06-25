#include "Adafruit_VL53L0X.h"
Adafruit_VL53L0X lox = Adafruit_VL53L0X();

const int PWM_PIN = 18; //Pino da ventoinha
const int pwmFreq = 25000; // Frequência para o motor da ventoinha (25 kHz)
const int pwmResolution = 10; //o pwmResolution é 10 para aguentar valores de 10 bits (0 a 1023), podendo controlar a ventoinha com mais precisão.


const float hTubo = 45.0; //Altura do Tubo


const int pwmBase = 420; //pwm base, flutua a bolinha devagar
// >= 420 pwm para subir
// = 410 pwm para flutuar
// <= 400 pwm para descer

//soma para tirar média do sensor
float soma = 0.0; 

//configura a altura que a bolinha deve estar
const int alturaDes = 23;

// pega o tempo 0
unsigned long tempoAnt = millis();
int erroAnt = 0; //pega o último erro

//valor da proporcional
const float Kp = 3;
const float Ki = 0.5; //valor da integral
const float Kd = 0.5; //valor da derivativa


float integral = 0; //valor acumulativo da integral


void setup() {
  Serial.begin(115200);


  // Detecta falha na inicialização do sensor
  while (! Serial) {
    delay(1);
  }

  //mostra inicialização (parte do código de exemplo do sensor da Adafruit)
  Serial.println("Adafruit VL53L0X test");
  if (!lox.begin()) {
    Serial.println(F("Failed to boot VL53L0X"));
    while(1);
  }
  // power
  //Serial.println(F("VL53L0X API Simple Ranging example\n\n"));


  //inicializa a ventoinha
  ledcAttach (
    PWM_PIN ,
    pwmFreq ,
    pwmResolution
  ) ;
  // desliga a ventoinha
  ledcWrite (PWM_PIN , 0) ;

  //da um tempo para a ventoinha ficar desligada
  delay(2500);
}




void loop() {

  //pega o tempo atual
  unsigned long tempoAtual = millis();

  //lê o sensor
  VL53L0X_RangingMeasurementData_t measure;

  //detecta se o sensor não está registrando nada e pula esse loop
  if(measure.RangeStatus == 4){
      return;
  }

  //coleta 5 amostras de distâncias
  for(int i = 0; i < 5; i++){
    lox.rangingTest(&measure, false);
    soma += measure.RangeMilliMeter;
  }

  //faz a média e transforma mm em cm
  float media = (soma / 5) / 10;


  /*Serial.print("Média: ");
  Serial.println(media);*/

  //reseta o valor de soma
  soma = 0;

  //calcula a altura atual da bolinha
  float alturaBol = hTubo - media;

  //Serial.print("Bolinha: ");
  Serial.print(alturaBol);

  //calcula o erro para cálculo do PDI  (quanto falta pra chegar na altura desejada)
  float erro = alturaDes - alturaBol;

  //formatação para o python
  Serial.print(",");
  //Serial.print("Erro: ");
  Serial.println(erro);

  

  //calcula a diferença de tempo entre o último loop e esse.
  float dt = (tempoAtual - tempoAnt) / 1000.0;

  //se der 0 pula esse loop
  if(dt <= 0) return;

  //calculo da integral
  //tenta ajustar o erro baseado na frequência dele (Ex.: se o valor desejado é 25 e a bolinha permanece no 15 por muito tempo)
  //quanto menor a integral, menos ele irá alterar o valor caso a bolinha esteja estacionada em um local
  //quanto maior a integral, mais ela tentará alterar o valor, mesmo estando no local por pouco tempo.
  integral += erro * dt;

  //calculo da derivada
  //tenta ajustar o erro de acordo com sua variação
  //valores muito baixos fazem com que o sistema não consiga detectar oscilações, não tentando ajustar
  //valores muito altos fazem com que o sistema tente ajustar o pwm com qualquer minima oscilação
  float derivada = (erro - erroAnt)/dt;

  //kp local serve para tentar concertar o problema do sensor de não ler corretamente nas extremidades ao
  //alterar o valor da proporcional nas extremidades
  //calculo da proporcional
  //quanto maior da proporcional, mais esforço a ventoinha faz para corrigir o erro.
  //valores muito baixos fazer com que a bolinha não corrija o erro por conta da falta de força da ventoinha
  //valors muito altos podem fazer a bolinha oscilar muito por conta da força excessiva.
  float Kp_local = Kp;

  if (alturaBol < 10 || alturaBol > 33) {
    Kp_local = Kp * 1.5; 
  }

  //calcula o novo PWM de acordo com o cálculo do PID.
  float saida = pwmBase + Kp_local*erro + Ki*integral + Kd*derivada;

  //limita essa saída pra ser entre 0 e 1023
  saida = constrain(saida, 0, 1023);


  //Serial.print("Saída: ");
  //Serial.println(saida);

  //faz a ventoinha rodar nessa velocidade.
  ledcWrite (
    PWM_PIN,
    saida
  );

  //transfere o tempoatual para o anterior.
  tempoAnt = tempoAtual;
  erroAnt = erro; //transfere o erro atual para o erro anterior.


}







