#include <MultiStepper.h>
#include <AccelStepper.h>

#define motorXStepPin 2
#define motorXDirPin 11
#define motorYStepPin 3
#define motorYDirPin 12

#define limitSwitchX 4
#define limitSwitchY 5

// Ultrasonic sensor pins for hole and tube detection
#define trigPin 6
#define echoPin 7

// Create instances for X and Y motors
AccelStepper motorX(AccelStepper::DRIVER, motorXStepPin, motorXDirPin);
AccelStepper motorY(AccelStepper::DRIVER, motorYStepPin, motorYDirPin);

// Variables for ultrasonic sensor
long duration;
int distance;
int holeDistanceThreshold = 10; // Distance threshold for detecting a hole
int xMax = 0, yMax = 0; // Store detected size (heat exchanger boundaries)
int startEdge = 0, endEdge = 0; // Store tube edge positions

void setup() 
{
  // Set motor speed and acceleration
  motorX.setMaxSpeed(1000); 
  motorX.setAcceleration(500);
  
  motorY.setMaxSpeed(1000); 
  motorY.setAcceleration(500);
  
  // Initialize ultrasonic sensor pins and cleaning motor
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Initialize limit switch pins
  pinMode(limitSwitchX, INPUT_PULLUP);
  pinMode(limitSwitchY, INPUT_PULLUP);

  // Initialize serial communication for debugging
  Serial.begin(9600);

  // Detect the boundaries of the heat exchanger
  detectSize();
}

void loop() 
{
  // Dynamic grid scanning to clean each hole
  for (int yPos = 0; yPos < yMax; yPos += 100) 
  { // Adjust '100' for grid step size
    for (int xPos = 0; xPos < xMax; xPos += 100)
      {
      motorX.moveTo(xPos);
      motorY.moveTo(yPos);
      
      // Move motors while detecting holes and cleaning
      while (motorX.distanceToGo() != 0 || motorY.distanceToGo() != 0) {
        motorX.run();
        motorY.run();
        
        // If a hole is detected, initiate cleaning
        if (detectHole()) {
          int tubeSize = detectTubeSize(); // Measure tube size dynamically
          Serial.print("Detected tube size: ");
          Serial.println(tubeSize);
          
          delay(2000);      // Adjust cleaning duration

        }
      }
    }
  }
}

// Function to detect the boundaries of the heat exchanger
void detectSize() 
{
  // Move X axis until limit switch is triggered
  motorX.move(10000); // Arbitrary large move to find the limit
  while (digitalRead(limitSwitchX) == HIGH) 
  {
    motorX.run();
  }
  xMax = motorX.currentPosition(); // Store max X position
  motorX.setCurrentPosition(0); // Reset to 0
  
  // Move Y axis until limit switch is triggered
  motorY.move(10000); 
  while (digitalRead(limitSwitchY) == HIGH) 
  {
    motorY.run();
  }
  yMax = motorY.currentPosition(); // Store max Y position
  motorY.setCurrentPosition(0); // Reset to 0
  
  // Log detected size
  Serial.print("Detected X size: ");
  Serial.println(xMax);
  Serial.print("Detected Y size: ");
  Serial.println(yMax);
}

// Function to detect a hole using the ultrasonic sensor
bool detectHole() 
{
  // Send ultrasonic pulse
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Measure distance
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2; // Convert to cm
  
  // Return true if a hole is detected
  return (distance > holeDistanceThreshold);
}

// Function to detect tube size using ultrasonic sensor
int detectTubeSize() 
{
  int tubeDiameter = 0;
  bool edgeDetected = false;

  // Start scanning for tube edges
  motorX.moveTo(0); // Start at the center of a hole
  for (int i = 0; i < 200; i++) 
  {  // Adjust '200' based on expected tube size range
    motorX.moveTo(i);
    motorX.run();
    
    int currentDistance = measureDistance();
    
    if (!edgeDetected && currentDistance < holeDistanceThreshold) 
    {
      // Tube edge detected (closer distance)
      startEdge = i;
      edgeDetected = true;
      Serial.println("Start of tube detected.");
    }
    
    if (edgeDetected && currentDistance >= holeDistanceThreshold) 
    {
      // Tube hole detected (farther distance), marking the end of the tube
      endEdge = i;
      Serial.println("End of tube detected.");
      break;
    }
  }
  
  // Calculate tube diameter based on start and end positions
  if (startEdge > 0 && endEdge > 0) 
  {
    tubeDiameter = endEdge - startEdge; // Diameter in step units
  }
  
  return tubeDiameter;
}

// Function to measure distance using ultrasonic sensor
int measureDistance() 
{
  // Send ultrasonic pulse
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Calculate distance based on echo time
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2; // Convert to cm
  
  return distance;
}
