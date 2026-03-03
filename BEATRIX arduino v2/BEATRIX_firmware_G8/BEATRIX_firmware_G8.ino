// BEATRIX firmware 1.0
// Author: Multimodal inte-R-action lab
// date: 12-06-2023

// Multimodal Interaction and Robot Active Perception (inte-R-action) Lab
// University of Bath

// BEATRIX firmware - beta version 1.0
// Bath opEn humAnoid for Teaching and Research in robotICS(X)

// Modified by Aya Woolf (ltw31) and Will Crook (WPFC20)

#include <AccelStepper.h>
#include <MultiStepper.h>

// Define pins for stepper motor of robot neck
// motor X
#define X_STEP_PIN 2          
#define X_DIR_PIN 5
// motor Y
#define Y_STEP_PIN 3
#define Y_DIR_PIN 6
// motor Z
#define Z_STEP_PIN 4
#define Z_DIR_PIN 7
// pin to enable/disable all motors
#define ENABLE_PIN 8

// maximum number of characters for commands
#define MAX_SIZE_COMMAND 20
// maximum number of paramater for each command
#define MAX_NUM_PARAMETERS 20
// maximum motor speed
#define MAX_SPEED 200   
// maximum motor acceleration
#define MAX_ACCEL 200

//WPFC20 custom mic output
#define MIC1_PIN A0
#define MIC2_PIN A1

// creates object for motor X
AccelStepper stepperX(1, X_STEP_PIN, X_DIR_PIN); // (num. of motor, dir, step)
// creates object for motor Y
AccelStepper stepperY(1, Y_STEP_PIN, Y_DIR_PIN); // (num. of motor, dir, step)
// creates object for motor Z
AccelStepper stepperZ(1, Z_STEP_PIN, Z_DIR_PIN); // (num. of motor, dir, step)
// creates object to handle multiple motors
MultiStepper steppers;

// define the number of motors to handle simultaneously
long multiStepperPositions[3];

// array to store commands received
char commands_char[MAX_NUM_PARAMETERS][MAX_SIZE_COMMAND];
// counter for commands
int ncommand = 0;
// counter for parameters
int count = 0;
// gets characters
char current_char;
// stores the current status of a command
bool commandStatus = false;
// stores calibration status of the robot
int calibrationStatus = 0;

// temporary used for @MOVALL command
boolean inPositionX = false;
boolean inPositionY = false;   
boolean inPositionZ = false;

bool commandList(char *cmdReceived);
void replyAcknowledge(bool cmdStatus);
bool executeCommand(char cmdReceived[][MAX_SIZE_COMMAND]);
void sendACK();
void sendNACK();

void setup()
{
    // initialise motors to move zero steps
    stepperX.move(0);
    stepperY.move(0);
    stepperZ.move(0);

    // initialise speed and acceleration for motor X
    stepperX.setMaxSpeed(MAX_SPEED);
    stepperX.setAcceleration(MAX_ACCEL);

    // initialise speed and acceleration for motor Y
    stepperY.setMaxSpeed(MAX_SPEED);
    stepperY.setAcceleration(MAX_ACCEL);

    // initialise speed and acceleration for motor Z
    stepperZ.setMaxSpeed(MAX_SPEED);
    stepperZ.setAcceleration(MAX_ACCEL);
  
    // Allocates all motors to MultiStepper to manage in corresponding commands
    steppers.addStepper(stepperX);
    steppers.addStepper(stepperY);
    steppers.addStepper(stepperZ);

    // set baudrate for communication with Arduino board
    Serial.begin(230400);
    // configures the enable_pin as output
    pinMode(ENABLE_PIN, OUTPUT);
    // initialise enable_pin as HIGH to disable the motors
    // HIGH: disable robot motors
    // LOW: enable robot motors
    digitalWrite(ENABLE_PIN, HIGH);

    //WPFC20 mic output command
    pinMode(MIC1_PIN, INPUT);
    pinMode(MIC2_PIN, INPUT);
}


void loop()
{
    // if data is received, then it is stored in commands_char
    if( Serial.available() > 0 )
    {      
        for( int i = 0; i < MAX_NUM_PARAMETERS; i++ )
        {
            for( int j = 0; j < MAX_SIZE_COMMAND; j++ )
                commands_char[i][j] = '\0';
        }

        count = 0;
        ncommand = 0;

        // stores commmands and parameters in commands_char
        do
        {
            current_char = Serial.read();
            
            delay(3);

            if( current_char != ' ' )
            {
                commands_char[ncommand][count] = current_char;
                count++;
            }
            else
            {
                commands_char[ncommand][count] = '\0';
                count = 0;
                ncommand++;                
            }            
        }while( current_char != '\r' );

        // check if the command received is correct
        commandStatus = commandList(commands_char[0]);
        replyAcknowledge(commandStatus);

        // if the command is correct, then it is executed by the corresponding function
        if( commandStatus == true )
            replyAcknowledge(executeCommand(commands_char));

        // cleans the serial pipe
        Serial.flush();
    }

}    

/* Function for execution of commands */
bool executeCommand(char cmdReceived[][MAX_SIZE_COMMAND])
{
    int step_size_int[20];
    int step_size_x[20];
    int step_size_y[20];
    int abs_step_size_int[20];
    int speed_int[20];
    int xMotorPos = 0;
    int yMotorPos = 0;
    int zMotorPos = 0;

    /* Enable/disable motors */
    if( !strcmp(cmdReceived[0],"@ENMOTORS") )
    {
      if( !strcmp(cmdReceived[1],"ON\r") )
        digitalWrite(ENABLE_PIN, LOW);
      else if( !strcmp(cmdReceived[1],"OFF\r") )
        digitalWrite(ENABLE_PIN, HIGH);
      else
        return false;

      calibrationStatus = 0;

      return true;
    }
    /* Calibration of X axis */
    if( !strcmp(cmdReceived[0],"@CALX") )
    {
      return true;
      // TBD
    }
    /* Calibration of Y axis */
    else if( !strcmp(cmdReceived[0],"@CALY") )
    {
      return true;
      // TBD
    }
    /* Calibration of Z axis */
    else if( !strcmp(cmdReceived[0],"@CALZ") )
    {
      return true;
      // TBD
    }
    /* Calibration - OK*/ 
    else if( !strcmp(cmdReceived[0],"@CALNOW\r") )
    {
        stepperX.setCurrentPosition(0);        
        stepperY.setCurrentPosition(0);        
        stepperZ.setCurrentPosition(0);
            
        calibrationStatus = 1;

        return true;
    }
    /* Move all axes to home position - OK*/
    else if( !strcmp(cmdReceived[0], "@MOVHOME\r") )
    {
        if( calibrationStatus == 1 )
        {
            multiStepperPositions[0] = 0;
            multiStepperPositions[1] = 0;
            multiStepperPositions[2] = 0;

            steppers.moveTo(multiStepperPositions);
            steppers.runSpeedToPosition();
            delay(1000);

            stepperX.setCurrentPosition(stepperX.currentPosition());
            stepperY.setCurrentPosition(stepperY.currentPosition());
            stepperZ.setCurrentPosition(stepperZ.currentPosition());

            return true;
        }
        else
            return false;    
    }
    /* Stop all motors - OK*/
    else if( !strcmp(cmdReceived[0],"@STOPALL\r") )
    {
        stepperX.stop();
        stepperY.stop();
        stepperZ.stop();
    }
    /* Get position from X axis - OK*/
    else if( !strcmp(cmdReceived[0],"@GETXPOS\r") )
    {
        Serial.println(stepperX.currentPosition());
    }
    /* Get position from Y axis - OK*/
    else if( !strcmp(cmdReceived[0],"@GETYPOS\r") )
    {
        Serial.println(stepperY.currentPosition());
    }
    /* Get position from Z axis - OK*/
    else if( !strcmp(cmdReceived[0],"@GETZPOS\r") )
    {
        Serial.println(stepperZ.currentPosition());
    }
    /* Get position from all axis - OK*/
    else if( !strcmp(cmdReceived[0],"@GETALLPOS\r") )
    {
        xMotorPos = stepperX.currentPosition();
        yMotorPos = stepperY.currentPosition();
        zMotorPos = stepperZ.currentPosition();
        Serial.print("\n");
        Serial.print(xMotorPos);
        Serial.print(" ");
        Serial.print(yMotorPos);
        Serial.print(" ");
        Serial.print(zMotorPos);
        Serial.print("\n");
    }
    /* Relative movement of X axis - OK*/
    else if( !strcmp(cmdReceived[0],"@MOVRX") )
    {
        if( calibrationStatus == 1 )
        {
          if( strcmp(cmdReceived[1]," ") && strcmp(cmdReceived[2]," ") )
          {
              step_size_int[0] = atoi(cmdReceived[1]);           
              speed_int[0] = atoi(cmdReceived[2]);
                              
              if( speed_int[0] > MAX_SPEED )
                  speed_int[0] = MAX_SPEED;
                  
              stepperX.move(step_size_int[0]);
        
              while( stepperX.distanceToGo() != 0 )
                  stepperX.run();
                  
              stepperX.stop();
              stepperX.runToPosition();

              return true;
          }
          else
              return false;       
        }
        else       
            return false;
    }
    /* Relative movement of Y axis - OK*/
    else if( !strcmp(cmdReceived[0],"@MOVRY") )
    {
        if( calibrationStatus == 1 )
        {
          if( strcmp(cmdReceived[1]," ") && strcmp(cmdReceived[2]," ") )
          {
              step_size_int[0] = atoi(cmdReceived[1]);
              speed_int[0] = atoi(cmdReceived[2]);
               
              if( speed_int[0] > MAX_SPEED )
                  speed_int[0] = MAX_SPEED;

              stepperY.move(step_size_int[0]);
        
              while( stepperY.distanceToGo() != 0 )
                  stepperY.run();
                  
              stepperY.stop();
              stepperY.runToPosition();
    
              return true;
          }
          else
              return false;       
        }
        else       
            return false;
    }
    /* Relative movement of Z axis - OK*/
    else if( !strcmp(cmdReceived[0],"@MOVRZ") )
    {
        if( calibrationStatus == 1 )
        {
          if( strcmp(cmdReceived[1]," ") && strcmp(cmdReceived[2]," ") )
          {
              step_size_int[0] = atoi(cmdReceived[1]);
              speed_int[0] = atoi(cmdReceived[2]);
               
              if( speed_int[0] > MAX_SPEED )
                  speed_int[0] = MAX_SPEED;

              stepperZ.move(step_size_int[0]);
        
              while( stepperZ.distanceToGo() != 0 )
                  stepperZ.run();
                  
              stepperZ.stop();
              stepperZ.runToPosition();
    
              return true;
          }
          else
              return false;       
        }
        else       
            return false;
    }
    /* Absolute movement of X axis - OK */
    else if( !strcmp(cmdReceived[0],"@MOVAX") )
    {
        if( calibrationStatus == 1 )
        {
          if( strcmp(cmdReceived[1]," ") && strcmp(cmdReceived[2]," ") )
          {
              step_size_int[0] = atoi(cmdReceived[1]);
              speed_int[0] = atoi(cmdReceived[2]);
              
              if( speed_int[0] > MAX_SPEED )
                  speed_int[0] = MAX_SPEED;

              stepperX.moveTo(step_size_int[0]);

              while( stepperX.currentPosition() != step_size_int[0] )
                  stepperX.run();
                  
              stepperX.stop();
              stepperX.runToPosition();
    
              return true;
          }
          else
              return false;       
        }
        else       
            return false;
    }
    /* Absolute movement of Y axis - OK */
    else if( !strcmp(cmdReceived[0],"@MOVAY") )
    {
        if( calibrationStatus == 1 )
        {
          if( strcmp(cmdReceived[1]," ") && strcmp(cmdReceived[2]," ") )
          {
              step_size_int[0] = atoi(cmdReceived[1]);
              speed_int[0] = atoi(cmdReceived[2]);
              
              if( speed_int[0] > MAX_SPEED )
                  speed_int[0] = MAX_SPEED;

              stepperY.moveTo(step_size_int[0]);

              while( stepperY.currentPosition() != step_size_int[0] )
                  stepperY.run();
                  
              stepperY.stop();
              stepperY.runToPosition();
    
              return true;
          }
          else
              return false;       
        }
        else       
            return false;
    }
    /* Absolute movement of Z axis - OK */
    else if( !strcmp(cmdReceived[0],"@MOVAZ") )
    {
        if( calibrationStatus == 1 )
        {
          if( strcmp(cmdReceived[1]," ") && strcmp(cmdReceived[2]," ") )
          {
              step_size_int[0] = atoi(cmdReceived[1]);
              speed_int[0] = atoi(cmdReceived[2]);
              
              if( speed_int[0] > MAX_SPEED )
                  speed_int[0] = MAX_SPEED;

              stepperZ.moveTo(step_size_int[0]);

              while( stepperZ.currentPosition() != step_size_int[0] )
                  stepperZ.run();
                  
              stepperZ.stop();
              stepperZ.runToPosition();
    
              return true;
          }
          else
              return false;       
        }
        else       
            return false;
    }
    /* Relative movement of all axes - OK*/
    else if( !strcmp(cmdReceived[0],"@MOVRALL") )
    {
        if( calibrationStatus == 1 )
        {
          if( strcmp(cmdReceived[1]," ") && strcmp(cmdReceived[2]," ") && strcmp(cmdReceived[3]," ") && strcmp(cmdReceived[4]," ") && 
              strcmp(cmdReceived[5]," ") && strcmp(cmdReceived[6]," ") && strcmp(cmdReceived[7]," ") && strcmp(cmdReceived[8]," "))
          {

              inPositionX = false;
              inPositionY = false;   
              inPositionZ = false;

              step_size_int[0] = atoi(cmdReceived[1]);
              step_size_int[1] = atoi(cmdReceived[2]);
              step_size_int[2] = atoi(cmdReceived[3]);
              speed_int[0] = atoi(cmdReceived[5]);
              speed_int[1] = atoi(cmdReceived[6]);
              speed_int[2] = atoi(cmdReceived[7]);
               
              if( speed_int[0] > MAX_SPEED )
                  speed_int[0] = MAX_SPEED;

              if( speed_int[1] > MAX_SPEED )
                  speed_int[1] = MAX_SPEED;

              if( speed_int[2] > MAX_SPEED )
                  speed_int[2] = MAX_SPEED;

              stepperX.move(step_size_int[0]);
              stepperY.move(step_size_int[1]);
              stepperZ.move(step_size_int[2]);
        
              do
              {          
                  if( stepperX.distanceToGo() != 0 )
                    stepperX.run();
                  else
                  {
                    stepperX.stop();
                    stepperX.runToPosition();
                    inPositionX = true;
                  }
        
                  if( stepperY.distanceToGo() != 0 )
                    stepperY.run();
                  else
                  {
                    stepperY.stop();
                    stepperY.runToPosition();
                    inPositionY = true;
                  }
    
                  if( stepperZ.distanceToGo() != 0 )
                    stepperZ.run();
                  else
                  {
                    stepperZ.stop();
                    stepperZ.runToPosition();
                    inPositionZ = true;
                  }

              }while( ( inPositionX != true ) || ( inPositionY != true ) || ( inPositionZ != true ) );

              delay(1000);

              stepperX.setCurrentPosition(stepperX.currentPosition());
              stepperY.setCurrentPosition(stepperY.currentPosition());
              stepperZ.setCurrentPosition(stepperZ.currentPosition());

              return true;
          }
          else
              return false;       
        }
        else       
            return false;
    }
    /* Absolute movement of all axes - OK*/
    else if( !strcmp(cmdReceived[0],"@MOVAALL") )
    {
        if( calibrationStatus == 1 )
        {
          if( strcmp(cmdReceived[1]," ") && strcmp(cmdReceived[2]," ") && strcmp(cmdReceived[3]," ") && strcmp(cmdReceived[4]," ") && 
              strcmp(cmdReceived[5]," ") && strcmp(cmdReceived[6]," ") && strcmp(cmdReceived[7]," ") && strcmp(cmdReceived[8]," "))
          {
              step_size_int[0] = atoi(cmdReceived[1]);
              step_size_int[1] = atoi(cmdReceived[2]);
              step_size_int[2] = atoi(cmdReceived[3]);
              speed_int[0] = atoi(cmdReceived[5]);
              speed_int[1] = atoi(cmdReceived[6]);
              speed_int[2] = atoi(cmdReceived[7]);
               
              if( speed_int[0] > MAX_SPEED )
                  speed_int[0] = MAX_SPEED;

              if( speed_int[1] > MAX_SPEED )
                  speed_int[1] = MAX_SPEED;

              if( speed_int[2] > MAX_SPEED )
                  speed_int[2] = MAX_SPEED;

              if( speed_int[3] > MAX_SPEED )
                  speed_int[3] = MAX_SPEED;

              multiStepperPositions[0] = step_size_int[0];
              multiStepperPositions[1] = step_size_int[1];
              multiStepperPositions[2] = step_size_int[2];

              steppers.moveTo(multiStepperPositions);
              steppers.runSpeedToPosition();
              delay(1000);

              stepperX.setCurrentPosition(stepperX.currentPosition());
              stepperY.setCurrentPosition(stepperY.currentPosition());
              stepperZ.setCurrentPosition(stepperZ.currentPosition());
    
              return true;
          }
          else
              return false;       
        }
        else       
            return false;
    }
    /* Move to relative position, inputs are posX, posY, speed*/
    else if( !strcmp(cmdReceived[0], "@MOVTORPOS"))
    {
        if( calibrationStatus == 1 )
        {
            if( strcmp(cmdReceived[1]," ") && strcmp(cmdReceived[2]," ") && strcmp(cmdReceived[3]," ") && strcmp(cmdReceived[4]," "))
            {

            inPositionX = false;
            inPositionY = false;   
            inPositionZ = false;

            step_size_x[0] = atoi(cmdReceived[1]); // Steps for the x direction for all motors
            step_size_y[0] = atoi(cmdReceived[2]); // Steps for the y direction for the 1st motor
            step_size_y[1] = int(atoi(cmdReceived[2]) * (2.0/3.0)); // Steps for the y direction for the 2nd motor
            step_size_y[2] = int(atoi(cmdReceived[2]) * (1.0/3.0)); // Steps for the y direction for the 3rd motor

            step_size_int[0] = step_size_x[0] + step_size_y[0];
            step_size_int[1] = step_size_x[0] - step_size_y[1];
            step_size_int[2] = step_size_x[0] - step_size_y[2];

            int step_size_total = step_size_int[0] + step_size_int[1] + step_size_int[2];

            speed_int[0] = atoi(cmdReceived[3]) * (step_size_int[0]/step_size_total);
            speed_int[1] = atoi(cmdReceived[3]) * (step_size_int[1]/step_size_total);
            speed_int[2] = atoi(cmdReceived[3]) * (step_size_int[2]/step_size_total);
               
            if( speed_int[0] > MAX_SPEED )
                speed_int[0] = MAX_SPEED;

            if( speed_int[1] > MAX_SPEED )
                speed_int[1] = MAX_SPEED;

            if( speed_int[2] > MAX_SPEED )
                speed_int[2] = MAX_SPEED;

            stepperX.move(step_size_int[0]);
            stepperY.move(step_size_int[1]);
            stepperZ.move(step_size_int[2]);
        
            do
            {          
                if( stepperX.distanceToGo() != 0 )
                    stepperX.run();
                else
                {
                    stepperX.stop();
                    stepperX.runToPosition();
                    inPositionX = true;
                }
        
                if( stepperY.distanceToGo() != 0 )
                    stepperY.run();
                else
                {
                    stepperY.stop();
                    stepperY.runToPosition();
                    inPositionY = true;
                }
    
                if( stepperZ.distanceToGo() != 0 )
                    stepperZ.run();
                else
                {
                    stepperZ.stop();
                    stepperZ.runToPosition();
                    inPositionZ = true;
                }

            }while( ( inPositionX != true ) || ( inPositionY != true ) || ( inPositionZ != true ) );

            delay(1000);

            stepperX.setCurrentPosition(stepperX.currentPosition());
            stepperY.setCurrentPosition(stepperY.currentPosition());
            stepperZ.setCurrentPosition(stepperZ.currentPosition());

            return true;
        }
        else
            return false;       
        }
        else       
            return false;
    }
    /* Move to absolute position, inputs are posX, posY, speed*/
    else if( !strcmp(cmdReceived[0], "@MOVTOAPOS"))
    {
        if( calibrationStatus == 1 )
        {
            if( strcmp(cmdReceived[1]," ") && strcmp(cmdReceived[2]," ") && strcmp(cmdReceived[3]," ") && strcmp(cmdReceived[4]," "))
            {

            inPositionX = false;
            inPositionY = false;   
            inPositionZ = false;

            step_size_x[0] = atoi(cmdReceived[1]); // Steps for the x direction for all motors
            step_size_y[0] = atoi(cmdReceived[2]); // Steps for the y direction for the 1st motor
            step_size_y[1] = int(atoi(cmdReceived[2]) * (2.0/3.0)); // Steps for the y direction for the 2nd motor
            step_size_y[2] = int(atoi(cmdReceived[2]) * (1.0/3.0)); // Steps for the y direction for the 3rd motor

            step_size_int[0] = step_size_x[0] + step_size_y[0];
            step_size_int[1] = step_size_x[0] - step_size_y[1];
            step_size_int[2] = step_size_x[0] - step_size_y[2];

            int step_size_total = step_size_int[0] + step_size_int[1] + step_size_int[2];

            speed_int[0] = atoi(cmdReceived[3]) * (step_size_int[0]/step_size_total);
            speed_int[1] = atoi(cmdReceived[3]) * (step_size_int[1]/step_size_total);
            speed_int[2] = atoi(cmdReceived[3]) * (step_size_int[2]/step_size_total);
               
            if( speed_int[0] > MAX_SPEED )
                speed_int[0] = MAX_SPEED;

            if( speed_int[1] > MAX_SPEED )
                speed_int[1] = MAX_SPEED;

            if( speed_int[2] > MAX_SPEED )
                speed_int[2] = MAX_SPEED;

              multiStepperPositions[0] = step_size_int[0];
              multiStepperPositions[1] = step_size_int[1];
              multiStepperPositions[2] = step_size_int[2];

              steppers.moveTo(multiStepperPositions);
              steppers.runSpeedToPosition();
              delay(1000);

              stepperX.setCurrentPosition(stepperX.currentPosition());
              stepperY.setCurrentPosition(stepperY.currentPosition());
              stepperZ.setCurrentPosition(stepperZ.currentPosition());
    
              return true;
          }
          else
              return false;       
        }
        else       
            return false;
    }
    /* Check the status of the current command*/
    else if( !strcmp(cmdReceived[0],"@COMSTATUS\r") )
    {
        return true;
      // TBD
    }
    /* Check the status of the robot calibration*/
    else if( !strcmp(cmdReceived[0],"@CALSTATUS\r") )
    {
        if( calibrationStatus == 1 )
            return true;
        else
            return false;
    }

//WPFC20 custom mic code
    else if ( !strcmp(cmdReceived[0], "@GETMICS\r") )
    {
        sendMicrophoneDataRAW();
        return true;
    }

    else
        return false;
}

/* Send reply ACK/NACK to client */
void replyAcknowledge(bool cmdStatus)
{
    if( cmdStatus == true )
        sendACK();
    else
        sendNACK();

    Serial.flush();
}

/* Print ACK message */
void sendACK()
{
    Serial.print("ACK\n");
}

/* Print NACK message */
void sendNACK()
{
    Serial.print("NACK\n");
}

//WPFC20 custom microphone data
void sendMicrophoneDataRAW() //raw audio signals
{
    const int numSamples = 200;          // burst size
    const unsigned int sampleDelay = 250;  // 4kHz (250 µs) and 8 kHz (125 µs)

    for(int i = 0; i < numSamples; i++)
    {
        uint16_t mic1 = analogRead(MIC1_PIN);
        uint16_t mic2 = analogRead(MIC2_PIN);

        Serial.write((uint8_t*)&mic1, 2); //print binary to save data
        Serial.write((uint8_t*)&mic2, 2);

        delayMicroseconds(sampleDelay);
    }
}

//WPFC20 custom microphone data
void sendMicrophoneDataMAV() //compute MAV to save serial bandwidth
{
     const int numSamples = 200;

    long sum1 = 0;
    long sum2 = 0;

    int buffer1[numSamples];
    int buffer2[numSamples];

    // First pass: collect samples and compute mean
    for(int i = 0; i < numSamples; i++)
    {
        buffer1[i] = analogRead(MIC1_PIN);
        buffer2[i] = analogRead(MIC2_PIN);

        sum1 += buffer1[i];
        sum2 += buffer2[i];

        delayMicroseconds(250);  // ~8 kHz
    }

    int offset1 = sum1 / numSamples;
    int offset2 = sum2 / numSamples;

    // Second pass: compute MAV
    long mavSum1 = 0;
    long mavSum2 = 0;

    for(int i = 0; i < numSamples; i++)
    {
        mavSum1 += abs(buffer1[i] - offset1);
        mavSum2 += abs(buffer2[i] - offset2);
    }

    int mav1 = mavSum1 / numSamples;
    int mav2 = mavSum2 / numSamples;

    Serial.print(mav1);
    Serial.print(" ");
    Serial.println(mav2);
}
/* Check the command received */
bool commandList(char *cmdReceived)
{
    char *commandArray[] = {"@CALSTART\r",    // TBD: Start calibration automatically
                            "@CALX\r",        // TBD: Calibrate motor X only
                            "@CALY\r",        // TBD: Calibrate motor Y only
                            "@CALZ\r",        // TBD: Calibrate motor Z only
                            "@CALSTATUS\r",   // Check the current calibration status of all motors
                            "@CALNOW\r",      // Calibrate the robot initial/home position (x=0, y=0, z=0)
                            "@CALEND\r",      // TBD: End robot calibration
                            "@MOVHOME\r",     // Move all motors to initial/home position
                            "@MOVRX",         // Move motor X relative to current X motor position
                            "@MOVRY",         // Move motor Y relative to current Y motor position
                            "@MOVRZ",         // Move motor Z relative to current Z motor position
                            "@MOVAX",         // Move motor X from home position
                            "@MOVAY",         // Move motor Y from home position
                            "@MOVAZ",         // Move motor Z from home position
                            "@MOVRALL",       // Move all motors relative to their current motor positions
                            "@MOVAALL",       // Move all motors from their home positions
                            "@MOVTORPOS",   // Move all motors to a certain position relative to their current positions. Parameters are posx, posy and speed
                            "@MOVTOAPOS",   // Move all motors to a certain position relative to their calibrated centre positions. Parameters are posx, posy and speed
                            "@STOPALL\r",     // Stop all motor movements
                            "@GETALLPOS\r",   // Get the current positions of all motors
                            "@GETXPOS\r",     // Get the current position of motor X
                            "@GETYPOS\r",     // Get the current position of motor Y
                            "@GETZPOS\r",     // Get the current position of motor Z
                            "@COMSTATUS\r",   // TBD: Check the status of a command
                            "@ENMOTORS",       // Enable/disable motors to be actuated
                            "@GETMICS\r"         //Gets the analogue mic signal from the pin
                            };   
    int ncommands = 26;
    
    // Search whether the command receive exist in the commandArray list
    for( int i = 0; i < ncommands; i++ )
    {
        if( !strcmp(commandArray[i], cmdReceived) )
            return true;
    }
    
    return false;
}