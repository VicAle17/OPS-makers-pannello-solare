#include <Servo.h>
#include <LiquidCrystal.h>

int pinLed=3;
int pioggia=0;
int misurazione=1;
int pinWind = 4; 
int media=0;
int n=0;
int val = analogRead(A7);
//LiquidCrystal lcd(0x3f, 16, 2);
float valPrec;

// coordinate punto luce
struct punto{
  float x;
  float y;
};
// vettore spostamento
struct spostamento{
  float dx;
  float dy;
};
// array che contiene le letture degli LDR (valori interi da 0 a 1023)
// A0 -> ldr_a[0]; A1 -> ldr_a[1]; A2 -> ldr_a[2]; A3 -> ldr_a[3]; 
int ldr_a[4]; 

// servo motori
// i servo motori vengono pilotati con la funzione writeMicrosenconds() per un posizionamento più preciso
const int SERVO_MIN_MS=544;     // angolo 0°
const int SERVO_MAX_MS=2400;    // angolo 180°
// dichiarazione oggetti corrispondenti ai due motori
Servo motorPan;   // motore rotazione base
Servo motorTilt;  // motore elevazione pannello

bool notMoving=true;

// posizione pannello (aPan, aTilt) intervalli da -90° a +90°
float aPan=0;
float aTilt=0;
// costante utilizzata per attendere il posizionamento dei motori
int delayServo=80;
// costante utilizzata per determinare l'ampiezza degli angoli di spostamento dei motori
int k=5;


/*
  Funzione per calcolare il punto luce
  a partire dai valori letti dalle 4  LDR
  mediante media pesata sulle x e sulle y
*/ 
punto calcolaCentro(){
  // somma delle letture dei valori LDR
  float s=0;
  for(int i=0;i<4;i++){
    ldr_a[i]=analogRead(i);
    s=s+ldr_a[i];
  }
  punto cm;

  // calcola il punto cm solo se la somma è diversa da 0
  // il calcolo viene fatto in base alla posizione spaziale relativa delle LDR sul supporto
  if(s>0){
    cm.x=(ldr_a[2]-ldr_a[0]+ldr_a[3]-ldr_a[1])/s;
    cm.y=(ldr_a[1]-ldr_a[0]+ldr_a[3]-ldr_a[2])/s;    
  }
  return cm;
}

/*
  Funzione per spostare il pannello verso
  la fonte di luce in modo che il centro
  luminoso coincida con il centro geometrico
  del supporto delle LDR
*/
void moveTo(float pan,float tilt){
  // determina un valore di attesa necessario per il posizionamento del pannello alla posizione richiesta
  // calcolato come il valore massimo tra il tempo per lo spostamento pan e il tempo per lo spostamento tilt
  int dt=max(abs(pan-aPan)*delayServo, abs(tilt-aTilt)*delayServo);
  // imposta la nuova posizione da raggiungere
  aPan=pan;
  aTilt=tilt;  
  // verifica che gli angoli non superino i limiti consentiti: intervallo da -90° a +90°
  if (aPan>=90){
    aPan=90;
  }
  if (aPan<=-90){
    aPan=-90;
  }
  if (aTilt>=90){
    aTilt=90;
  }
  if (aTilt<=-90){
    aTilt=-90;
  }
  // calcola i microsecondi corrispondenti agli angoli da raggiungere 
  // gli angoli vengono riportati nell'intervallo 0° - 180°
  int msp=map(pan+90,0,180,SERVO_MIN_MS,SERVO_MAX_MS);
  int mst=map(tilt+90,0,180,SERVO_MIN_MS,SERVO_MAX_MS);
  // attivazione motori
  motorPan.attach(9);
  motorTilt.attach(10);
  // richiesta spostamento motori
  if (msp!=0.0){
    motorPan.writeMicroseconds(msp); 
  }
  if (mst!=0.0){
    motorTilt.writeMicroseconds(mst);  
  }
  // attesa posizionamento
  delay(dt);

  
  // disattivazione motori per evitare ronzii
  motorPan.detach();
  motorTilt.detach();  
}

void reset(){
  int msp=map(90,0,180,SERVO_MIN_MS,SERVO_MAX_MS);
  int mst=map(90,0,180,SERVO_MIN_MS,SERVO_MAX_MS);
  // attivazione motori
  motorPan.attach(9);
  motorTilt.attach(10);
  // richiesta spostamento motori
  if (msp!=0){
    motorPan.writeMicroseconds(msp);  
  }
  if (mst!=0){
    motorTilt.writeMicroseconds(mst);  
  }
  // attesa posizionamento
  delay(2000);  
  motorPan.detach();
  motorTilt.detach();
}

void setup() {
  // posizione pannello angolo di riposo
  Serial.begin(9600);
  aPan=0;
  aTilt=0;
  reset();
}

void loop() {
int sensorValue = analogRead(8);  
  int outputValue = map(sensorValue, 0, 1023, 255, 0);
  if (sensorValue<350 || analogRead(7)>250 ){
    Serial.println(sensorValue);
    moveTo(0,-77);
  } else { 
  punto cm=calcolaCentro();         // determina punto luce
  spostamento ds={k*cm.x,k*cm.y};   // determina spostamento
  moveTo(aPan-ds.dx,aTilt+ds.dy);   // sposta pannello di una quantità ds rispetto alla posizione corrente 
  }
  
  int solPan = analogRead(A5);
  float v= (float)solPan*6/613;
  Serial.print("Pannello Solare (Volt): ");
  Serial.println(v);
//  lcd.print("Pannello Solare (Volt): ");
  //lcd.println(solPan);
 delay(100);

}
