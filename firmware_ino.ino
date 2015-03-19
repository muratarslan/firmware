#define DEBUG
#include <Servo.h>

int robotId = 0;

//                  SEL DIR BRE
int motors[4][3] = {{8,  2, A4},  //RF
                    {7,  4, A3},  //LF
                    {10, 5, A1},  //RR
                    {12, 9, A2}};

int chargeTone = 3;
int kickerRelease = 5;
int dacMosi = 11;
Servo spinnerMotor;


bool breaks[4] = {true,true,true,true};
int  cmdBuffer[5];


void processMotorCommand(int* dirs,byte* pwms){
#ifdef DEBUG
  Serial.print("PWMs: ");
#endif

  for(int i=0; i<4; i++){
    int pwm = pwms[i];
    int dir = dirs[i];

#ifdef DEBUG
    Serial.print("[");
    Serial.print(dir);
    Serial.print(" ");
    Serial.print(pwm);
    Serial.print(" ");
    Serial.print(motors[i][0]);
    Serial.print("] ");
#endif

    if (pwm == 0){
      digitalWrite(motors[i][2],LOW);
      breaks[i] = true;
    }else{
      if (breaks[i] == true){
        digitalWrite(motors[i][2],HIGH);
        breaks[i] = false;
      }
      digitalWrite(motors[i][1],dir);
      analogWrite(motors[i][0],pwm);
    }
  }
#ifdef DEBUG
  Serial.print("\n");
#endif
}

void motorCommand(int* buff){
  byte cmd = ((byte)buff[0]);
  int dirs[4] = {bitRead(cmd, 3),
                 bitRead(cmd, 2),
                 bitRead(cmd, 1),
                 bitRead(cmd, 0)};

  byte pwms[4] = {(byte)buff[1],
                  (byte)buff[2],
                  (byte)buff[3],
                  (byte)buff[4]};

  processMotorCommand(dirs,pwms);
}


void kickCommand(int force){
#ifdef DEBUG
  Serial.println("Kick");
#endif

  analogWrite(kickerRelease, 100);
  delay(force);
  analogWrite(kickerRelease, LOW);
}


void escArm() {
  spinnerMotor.write(10); 
  delay(2000);
  Serial.println("Armed!");
}


void spinnerCommand(int state){
#ifdef DEBUG
  Serial.println("Spinner");
#endif

  if (state == 1){
    spinnerMotor.write(255);
  }
  else
    spinnerMotor.write(10);
}

void charge(){
  #ifdef DEBUG
 // Serial.println("Charging!");
#endif

// spinner
//  spinnerMotor.write(255);
// spinner


  int volt = analogRead(A0);
  if(volt > 360)
    noTone(chargeTone);
  if(volt < 350)
    tone(chargeTone, 50000);
}


void setup(){
  Serial.begin(57600);

  pinMode(kickerRelease, OUTPUT);
  pinMode(chargeTone, OUTPUT);
  spinnerMotor.attach(6);

  for(int i=0; i<4; i++){
    pinMode(motors[i][0], OUTPUT);
    pinMode(motors[i][1], OUTPUT);
    pinMode(motors[i][2], OUTPUT);
    digitalWrite(motors[i][2],LOW);
  }

  escArm();

#ifdef DEBUG
  Serial.println("Data Link Up");
#endif
}


int blockingRead(){
  while (Serial.available() < 1)
  charge();
  return Serial.read();
}


void nextCommand(){
  int flag = 0;
  while(flag != 0xFF)
  charge();
    flag = blockingRead();
}


void loop(){

  for(;;){
    nextCommand();
    charge();
    
    while(blockingRead() != robotId){
      nextCommand();
#ifdef DEBUG
      Serial.println("Skip Cmd");
#endif
    }

    cmdBuffer[0] = blockingRead();
    int cmd = cmdBuffer[0] >> 4;

    if (cmd == 1){
      cmdBuffer[1] = blockingRead();
      cmdBuffer[2] = blockingRead();
      cmdBuffer[3] = blockingRead();
      cmdBuffer[4] = blockingRead();
      motorCommand(cmdBuffer);
    }else if (cmd == 2){
      kickCommand(blockingRead());
    }else if (cmd == 3){
      spinnerCommand(blockingRead());
    }else{
#ifdef DEBUG
      Serial.println("Unknown Command.");
#endif
    }
  }
}
