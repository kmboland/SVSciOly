//The following code demonstrates basic control of the DC motors. Motor1 is controlled through digital pins 3 and 10. 
//Motor 2 is controlled through digital pins 6 and 9. In addition, motor driver could be completely turned off using 
//analog pin A2.

//In the setup() function, it enables the motor driver by calling setupMotor().

//In the main loop, it calls the functions to move the robot forward, left , backward and right in sequence. In each step, 
//it waits for 200 milliseconds.

// pins controller motors
#define motor1_pos 3
#define motor1_neg 10
#define motor2_pos 6
#define motor2_neg 9
#define motor_en   A2


void setup()
{
   Serial.begin(57600);
   setupMotor();
}


void loop() 
{
    robotForward(200);
    robotLeft(200);
    robotBackward(200);
    robotRight(200);
    robotStop(500);
}
   
void setupMotor() {
  pinMode(motor1_pos,OUTPUT);
  pinMode(motor1_neg,OUTPUT);
  pinMode(motor2_pos,OUTPUT);
  pinMode(motor2_neg,OUTPUT);
  pinMode(motor_en,OUTPUT);
  enableMotor();
  robotStop(50);
}


//-----------------------------------------------------------------------------------------------------
// motor
//-----------------------------------------------------------------------------------------------------


void enableMotor() {
  //Turn on the motor driver chip : L293D
  digitalWrite(motor_en, HIGH); 
}


void disableMotor() {
  //Turn off the motor driver chip : L293D
  digitalWrite(motor_en, LOW); 
}


void robotStop(int ms){
  digitalWrite(motor1_pos, LOW); 
  digitalWrite(motor1_neg, LOW); 
  digitalWrite(motor2_pos, LOW); 
  digitalWrite(motor2_neg, LOW); 
  delay(ms);
}


void robotForward(int ms){
  digitalWrite(motor1_pos, HIGH); 
  digitalWrite(motor1_neg, LOW); 
  digitalWrite(motor2_pos, HIGH); 
  digitalWrite(motor2_neg, LOW); 
  delay(ms);
}


void robotBackward(int ms){
  digitalWrite(motor1_pos, LOW); 
  digitalWrite(motor1_neg, HIGH); 
  digitalWrite(motor2_pos, LOW); 
  digitalWrite(motor2_neg, HIGH); 
  delay(ms);
}
void robotRight(int ms){
  digitalWrite(motor1_pos, LOW); 
  digitalWrite(motor1_neg, HIGH); 
  digitalWrite(motor2_pos, HIGH); 
  digitalWrite(motor2_neg, LOW); 
  delay(ms);
}


void robotLeft(int ms){
  digitalWrite(motor1_pos, HIGH); 
  digitalWrite(motor1_neg, LOW); 
  digitalWrite(motor2_pos, LOW); 
  digitalWrite(motor2_neg, HIGH); 
  delay(ms);
}


<br>
