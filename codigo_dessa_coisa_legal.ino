#include "Adafruit_VL53L0X.h"
Adafruit_VL53L0X lox = Adafruit_VL53L0X();


const int PWM_PIN = 18;
const int pwmFreq = 25000;
const int pwmResolution = 10;


const float hTubo = 45.0;


const int pwmBase = 420;
float soma = 0.0;


const int alturaDes = 23;


unsigned long tempoAnt = millis();
int erroAnt = 0;


const float Kp = 3;
const float Ki = 0.5;
const float Kd = 0.5;


float integral = 0;


void setup() {
  Serial.begin(115200);


  // wait until serial port opens for native USB devices
  while (! Serial) {
    delay(1);
  }
 
  Serial.println("Adafruit VL53L0X test");
  if (!lox.begin()) {
    Serial.println(F("Failed to boot VL53L0X"));
    while(1);
  }
  // power
  //Serial.println(F("VL53L0X API Simple Ranging example\n\n"));


  ledcAttach (
    PWM_PIN ,
    pwmFreq ,
    pwmResolution
  ) ;
  // PWM fixo
  ledcWrite (PWM_PIN , 0) ;

  delay(2500);
}




void loop() {


  unsigned long tempoAtual = millis();
  VL53L0X_RangingMeasurementData_t measure;
   
  if(measure.RangeStatus == 4){
      return;
  }


  for(int i = 0; i < 5; i++){
    lox.rangingTest(&measure, false);
    soma += measure.RangeMilliMeter;
  }


  float media = (soma / 5) / 10;


  /*Serial.print("Média: ");
  Serial.println(media);*/


  soma = 0;


  float alturaBol = hTubo - media;


  //Serial.print("Bolinha: ");
  Serial.print(alturaBol);


  float erro = alturaDes - alturaBol;


  Serial.print(",");
  //Serial.print("Erro: ");
  Serial.println(erro);

  


  float dt = (tempoAtual - tempoAnt) / 1000.0;


  if(dt <= 0) return;


  integral += erro * dt;


  float derivada = (erro - erroAnt)/dt;

  float Kp_local = Kp;

  if (alturaBol < 10 || alturaBol > 33) {
    Kp_local = Kp * 1.5; 
  }


  float saida = pwmBase + Kp_local*erro + Ki*integral + Kd*derivada;


  saida = constrain(saida, 0, 1023);


  //Serial.print("Saída: ");
  //Serial.println(saida);


  ledcWrite (
    PWM_PIN,
    saida
  );


  tempoAnt = tempoAtual;
  erroAnt = erro;


}







