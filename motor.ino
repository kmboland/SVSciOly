// Ported to SdFat from the native Arduino SD library example by Bill Greiman
// On the Ethernet Shield, CS is pin 4. SdFat handles setting SS
const int chipSelect = 10;
// Reference the Newping library for the Sonar device
#include <NewPing.h>
// Reference the Servo Library
#include <Servo.h> 
// Reference the Wire library for I2C
#include <Wire.h>
// Reference the FAT lib to access the SD card
#include <SdFat.h>
// Reference the HMC5883L Compass Library
#include <HMC5883L.h>
SdFat sd;
SdFile myFile;
 
int DirM1 = 2; // forward motor 1
int DirM2 = 8; // direction motor 1
int DirM3 = 9; // forward motor 2
int DirM4 = 3; // direction motor 2
int speedpin = 5;  // pwm motor speed
int gospeed = 0;
int automode = 0;
int pingrng = 0;
int trycount = 0;
int savespeed;
long timer;
float headingDegrees;
float saveHeading;
float headingDiff;
float testHeading;

const byte MY_ADDRESS = 42;
const int ECHO_PIN = 7; 	// Arduino pin tied to echo pin on ping sensor.
const int MAX_DISTANCE = 200; 	// Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.
NewPing sonar(ECHO_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.
int leftDistance, rightDistance, fwdDistance, lastFwdDistance;
// Store our compass as a variable.
HMC5883L compass;
// Record any errors that may occur in the compass.
int error = 0;
 
Servo MyServo;        // servo objects
 
void setup() {
  //Serial.begin(9600);
  Wire.begin ();
  // Initialize SdFat or print a detailed error message and halt
  // Use half speed like the native library.
  // change to SPI_FULL_SPEED for more performance.
  if (!sd.begin(chipSelect, SPI_FULL_SPEED)) sd.initErrorHalt();
  // open the file for write at end like the Native SD library
  if (!myFile.open("test.txt", O_RDWR | O_CREAT | O_AT_END)) { sd.errorHalt("opening test.txt for write failed"); }
  myFile.println("Initializing hardware");  
  //Serial.println("Initializing hardware"); 
  myFile.println("Constructing new HMC5883L");
  //Serial.println("Constructing new HMC5883L");
  compass = HMC5883L(); // Construct a new HMC5883 compass.
      
  myFile.println("Setting compass scale to +/- 1.3 Ga");
  //Serial.println("Setting compass scale to +/- 1.3 Ga");
  error = compass.SetScale(1.3); // Set the scale of the compass.
  //if(error != 0) // If there is an error, print it out.
    //Serial.println(compass.GetErrorText(error));
  
  myFile.println("Setting compass measurement mode to continous.");
  //Serial.println("Setting compass measurement mode to continous.");
  error = compass.SetMeasurementMode(Measurement_Continuous); // Set the measurement mode to Continuous
  //if(error != 0) // If there is an error, print it out.
    //Serial.println(compass.GetErrorText(error));

  //init output pins
  pinMode(DirM1, OUTPUT);  // motors 1&2 
  pinMode(DirM2, OUTPUT);  // motors 1&2 
  pinMode(DirM3, OUTPUT);  // motors 3&4 
  pinMode(DirM4, OUTPUT);  // motors 3&4 
  pinMode(speedpin, OUTPUT);  // speed control motors 3&4
  timer = millis();
  
  //init Servo
  MyServo.attach(4);      
  MyServo.write(90) ; // center the sonar servo
  delay(150);
  
  Wire.begin (MY_ADDRESS);
  // set up receive handler
  Wire.onReceive (receiveEvent);
  Wire.onRequest (dataRequest);
  }  // end of setup

void loop() 
  {
      if (automode==1) {
     //Ping out to see what's ahead.  ping median takes 5 pings, so we may still miss things
     pingrng =  sonar.ping_median();
     fwdDistance = sonar.convert_in(pingrng);
     ///if our distance is more than 15 in, keep going
     if (fwdDistance > 15)  {
        forward();
        }
     else    {    //otherwise, lets take a look around
        stopall(); // stop the drive motors
        lookaround(); // have a look around
        compareDistance();  //now compare those distances and do something
             }     
      }
  }
  
// called by interrupt service routine when incoming data arrives
void receiveEvent (int howMany)
  {
  char c = Wire.read();
      switch(c) {
        case 'F':   //go forward
           forward();
           myFile.println("event received - forward");  
           break;
         case 'B':  //go backward
           backward(); 
           myFile.println("event received - backward"); 
           break;
         case 'L':  //turn left
           turnleft();
           myFile.println("event received - turn left"); 
           break;
         case 'R':  //turn right
           turnright();
           myFile.println("event received - turn right"); 
           break;
         case 'S':  //Stop
           stopall();
           myFile.println("event received - stop"); 
           break;
         case '+':  // Increase speed
           gospeed = gospeed + 10;
           gospeed = constrain(gospeed, 0, 250);
           myFile.println("event received - speed up"); 
           break;
         case '-':  // Decrease speed
           gospeed = gospeed - 10;
           gospeed = constrain(gospeed, 0, 250);
           myFile.println("event received - slow down"); 
           break;
         case '0':  // reset speed
           gospeed = 0;
           myFile.println("event received - reset speed and close log"); 
           myFile.close();
           break;  
         case 'A':  // Auto Mode
           automode=1;
           lastFwdDistance=0;
           myFile.println(" "); 
           myFile.println("event received - ** Auto mode**");
           myFile.println(" ");  
           break;  
         case 'M':  // Manual Mode
           automode=0;
           myFile.println(" ");            
           myFile.println("event received - ** Manual mode **");            
           myFile.println(" ");            
           break;         
         default: //Stop if nothing read
           stopall();
      }
   }    
  
void turnleft()
{ digitalWrite(DirM1, HIGH);
  digitalWrite(DirM2, LOW);
  digitalWrite(DirM3, LOW);
  digitalWrite(DirM4, HIGH);
  analogWrite(speedpin, gospeed);
}
 
void turnright()
{ digitalWrite(DirM1, LOW);
  digitalWrite(DirM2, HIGH);
  digitalWrite(DirM3, HIGH);
  digitalWrite(DirM4, LOW);
  analogWrite(speedpin, gospeed);
}
 
void forward()
{ digitalWrite(DirM1, HIGH);
  digitalWrite(DirM2, LOW);
  digitalWrite(DirM3, HIGH);
  digitalWrite(DirM4, LOW);
  analogWrite(speedpin, gospeed);
}

void backward()
{ digitalWrite(DirM1, LOW);
  digitalWrite(DirM2, HIGH);
  digitalWrite(DirM3, LOW);
  digitalWrite(DirM4, HIGH);
  analogWrite(speedpin, gospeed);
}
 
void stopall()
{ digitalWrite(DirM1, LOW);
  digitalWrite(DirM2, LOW);
  digitalWrite(DirM3, LOW);
  digitalWrite(DirM4, LOW);
}

void compareDistance() {
      myFile.print("comparing distance ");  
      myFile.print("fwd: ");
      myFile.print(fwdDistance);
      myFile.print("  left: ");
      myFile.print(leftDistance);
      myFile.print("  right: ");
      myFile.println(rightDistance);  
          if (fwdDistance < 15 && rightDistance < 15 && leftDistance < 15) //we are boxed in - back up and then turn 
            {
           backward();
           myFile.println("We are boxed in - back up!");            
           delay(1000);
            };    
        if (fwdDistance == lastFwdDistance && fwdDistance < 15) //if we are stuck, than back up and then turn 
            {
           backward();
           myFile.println("We are stuck - back up!");            
           delay(1000);
            };
            
        if (leftDistance>rightDistance) //if left is less obstructed
            {
           myFile.println("right is clearest - turn right"); 
           myFile.print("Saved heading before I turn is:  "); 
           myFile.println(headingDegrees);              
           myFile.println("left is clearest - turn left");
           savespeed=gospeed; 
           saveHeading=headingDegrees;
           do {  turnleft();
                  delay(100); 
                 getHeading();  
                 if (saveHeading > headingDegrees) 
                    { testHeading = saveHeading - headingDegrees; }
                 else {testHeading = headingDegrees - saveHeading; }
                 if (testHeading > 180) { headingDiff = 360 - testHeading; }
                 else {headingDiff = testHeading;}
             myFile.print("Left in-turn heading is:  "); 
             myFile.println(headingDegrees);  
             trycount = trycount + 1;  // If our trycount >10, then we are stuck.  Increase the power
             if (trycount > 10) 
                {gospeed = gospeed + 5;
                 myFile.print("Left in-turn power up to:  "); 
                 myFile.println(gospeed); 
                };               
             }
           while ( headingDiff < 90);
           gospeed = savespeed;
           trycount = 0;

           myFile.print("New heading after turn is:  "); 
           myFile.println(headingDegrees);            
            }
            
        else if (rightDistance>leftDistance) //if right is less obstructed
            {
           myFile.println("right is clearest - turn right"); 
           myFile.print("Saved heading before I turn is:  "); 
           myFile.println(headingDegrees);
           savespeed=gospeed;            
           saveHeading=headingDegrees;
           do {  turnright();
                  delay(100); 
                 getHeading();  
                 if (saveHeading > headingDegrees) 
                    { testHeading = saveHeading - headingDegrees; }
                 else {testHeading = headingDegrees - saveHeading; }
                 if (testHeading > 180) { headingDiff = 360 - testHeading; }
                 else {headingDiff = testHeading;}
             myFile.print("Right in-turn heading is:  "); 
             myFile.println(headingDegrees);
             trycount = trycount + 1;  // If our trycount >10, then we are stuck.  Increase the power
             if (trycount > 10) 
                {gospeed = gospeed + 5;
                 myFile.print("Right in-turn power up to:  "); 
                 myFile.println(gospeed);               
                 };              
             }
           while ( headingDiff < 90);              
           gospeed = savespeed;
           trycount = 0;
           
           myFile.print("New heading after turn is:  "); 
           myFile.println(headingDegrees);            
            }
            
        lastFwdDistance = fwdDistance;
}

void lookaround()
{
  // spin the servo around to see what is close to us
      myFile.println("Looking around");  
        MyServo.write(20);  // move servo left
        delay(500);
        pingrng =  sonar.ping_median();
        leftDistance = sonar.convert_in(pingrng);
        delay(50);
        MyServo.write(180);    // move servo right
        delay(500);
        pingrng =  sonar.ping_median();
        rightDistance = sonar.convert_in(pingrng);
        MyServo.write(90);    // move servo to center
        delay(50);
}

void dataRequest()  // The master has requested to know what the speed setting is 
{ Wire.write(gospeed); }

void getHeading()
{
  // Retrive the raw values from the compass (not scaled).
  MagnetometerRaw raw = compass.ReadRawAxis();
  // Retrived the scaled values from the compass (scaled to the configured scale).
  MagnetometerScaled scaled = compass.ReadScaledAxis();
  
  // Values are accessed like so:
  int MilliGauss_OnThe_XAxis = scaled.XAxis;// (or YAxis, or ZAxis)

  // Calculate heading when the magnetometer is level, then correct for signs of axis.
  float heading = atan2(scaled.YAxis, scaled.XAxis);
  
  // Once you have your heading, you must then add your 'Declination Angle', which is the 'Error' of the magnetic field in your location.
  // Find yours here: http://www.magnetic-declination.com/
  // Mine is: 2� 37' W, which is 2.617 Degrees, or (which we need) 0.0456752665 radians, I will use 0.0457
  // If you cannot find your Declination, comment out these two lines, your compass will be slightly off.
  float declinationAngle = -0.1230;
  heading += declinationAngle;
  
  // Correct for when signs are reversed.
  if(heading < 0)
    heading += 2*PI;
    
  // Check for wrap due to addition of declination.
  if(heading > 2*PI)
    heading -= 2*PI;
   
  // Convert radians to degrees for readability.
  headingDegrees = heading * 180/M_PI; 

  // Log our heading
      myFile.print("Heading Degrees:  ");
      myFile.println(headingDegrees);
}
