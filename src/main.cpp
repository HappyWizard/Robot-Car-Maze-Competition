#include <Arduino.h>
#include <Memory.h>
#include <Motor.h>
#include <Ultrasonic.h>
#include <Encoder.h>
#include <Gyro.h>

char maze[][4] = {
  {'.', '.', '.', 'E'},
  {'.', '#', '#', '*'},
  {'*', '*', '*', '*'},
  {'S', '#', '*', '*'}
};

int currentX = 0;
int currentY = 0;

int previousX = 0;
int previousY = 0;

int desiredOrientation = 0; 
int currentOrientation = 0;

// 0: north
// 1: east
// 2: south
// 3: west

String pathTravelled[100] = {""};
int pathIndex = 0;

void printPaths() {
    // cout << endl;
    // cout << "Path Summary: " << endl;
    for (int i = 0; i < pathIndex; i++) {
        // cout << "Step " << i + 1 << ": " << pathTravelled[i] << endl;
    }
}

int gyroReading = 180;
//  180: facing front
//   90: facing right
//    0: facing backward
//  270: facing left

void goStraight(float targetDistance){
  float initialDistance = getMovingDistance();
  float initialLeftDistance = getMovingDistanceLeft();
  float initialRightDistance = getMovingDistanceRight();
  while ((getMovingDistance()-initialDistance)<targetDistance) {
    float movedLeft = getMovingDistanceLeft() - initialLeftDistance;
    float movedRight = getMovingDistanceRight() - initialRightDistance;
    if ((movedLeft-movedRight)>0.5) {
      slightLeft();
    } else if ((movedRight-movedLeft)>0.5) {
      slightRight();
    } else {
        goForwardMotor(120);
    }
  }
  restMotor();
}

void goStraightGyro(float targetDistance, float desiredAngle){
  
  float initialDistance = getMovingDistance();
  while ((getMovingDistance()-initialDistance)<targetDistance) {
    Serial.println(update());
    if (update()<desiredAngle) {
      slightLeft();
      // Serial.println("Slight Left");
    } else if (update()>desiredAngle) {
      slightRight();
      // Serial.println("Slight Right");
    } else {
      goForwardMotor(120);
      // Serial.println("go forward");
    }
  }
  restMotor();
}



void turnRightUntilCorrect(){
  float targetAngle = update() - 90;
  if (targetAngle<0){
      targetAngle += 360;
  }
  while (abs(update()-targetAngle)>25){
      goHardRightMotor(120);
  }
  restMotor();
}

void turnLeftUntilCorrect(){
  float targetAngle = update() + 90;
  if (targetAngle>360){
      targetAngle -= 360;
  }
  while (abs(update()-targetAngle)>25){
      goHardLeftMotor(120);
  }
  restMotor();
}

void setGyroReading(int angle){
  gyroReading = angle;
}

void setPathTravelled(String path){
  pathTravelled[pathIndex] = path;
  pathIndex++;
}

void goForward(float currentAngle){
  setPathTravelled("forward");
  Serial.println("Went Forward");
  // cout << " => Went Forward" << endl;
  // moves forward for 25 cm
  goStraightGyro(100, currentAngle);
}
void turnRight(){
  setPathTravelled("Right");
  Serial.println("turned Right");
  // cout << " => turned Right" << endl;
  turnRightUntilCorrect();
  // delay(3000);
  // turn right 90 degree
}
void turnLeft(){
  setPathTravelled("Left");
  Serial.println("turned Left");
  // cout << " => turned Left" << endl;
  turnLeftUntilCorrect();
  // delay(3000);
  // turn left 90 degree
}

void checkAndUpdateOrientation(int requiredOrientation) {
  if (currentOrientation == requiredOrientation) {
      return; // Already in the correct orientation, no changes needed
  }
  // Calculate the difference in orientation (mod 4 to handle wrap-around cases)
  int diff = (requiredOrientation - currentOrientation + 4) % 4;

  if (diff == 2) {  
      // Opposite directions (e.g., North <-> South, East <-> West)
      turnRight();
      turnRight();
      gyroReading = (gyroReading + 180) % 360;
  } 
  else if (diff == 1) {  
      // Turning right is shorter
      turnRight();
      gyroReading = (gyroReading - 90 + 360) % 360;
  } 
  else if (diff == 3) {  
      // Turning left is shorter
      turnLeft();
      gyroReading = (gyroReading + 90) % 360;
  }

  // Update the current orientation to the required orientation
  currentOrientation = requiredOrientation;
}


void executePath(char maze[][4], int rows, int cols) {
  int currentX = 0, currentY = 0; // Starting position

  // Find the starting position 'S'
  for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {
          if (maze[i][j] == 'S') {
              currentX = i;
              currentY = j;
              Serial.print("At starting point: ");
              Serial.print(currentX);
              Serial.print(" , ");
              Serial.println(currentY); 
              
              Serial.print("Orientation: ");
              Serial.println(gyroReading);
              
              // cout << "At starting point: " << currentX << " , " << currentY << endl; 
              // cout << "Orientation: " << gyroReading << endl; 
              break;
          }
      }
  }

    // Follow the path marked by '*'
    while (maze[currentX][currentY] != 'E') {
      // cout << "Previous Points: " << previousX << " , " << previousY << endl; 

      if (currentX > 0 && maze[currentX-1][currentY] == 'E' && (currentX-1) != previousX){ // x-1 in the array means go up one row
          desiredOrientation = 0;
          checkAndUpdateOrientation(desiredOrientation);
          if (gyroReading == 180){

              Serial.print("Currently at point: ");
              Serial.print(currentX);
              Serial.print(" , ");
              Serial.println(currentY); 

              Serial.print("Orientation: ");
              Serial.println(gyroReading);

              // cout << "\nCurrently at point: " << currentX << " , " << currentY << endl; 
              // cout << "Orientation: " << gyroReading << endl; 
              goForward(180);
              
              previousX = currentX;
              previousY = currentY;

              currentX = currentX-1;
              currentY = currentY;
          }else{
              // cout << "Incorrect Orientation" << endl;
              break;
          }
          break;
      }else if (currentY < cols-1 && maze[currentX][currentY+1] == 'E' && (currentY+1) != previousY){ // y+1 in the array means go one column to the right
          desiredOrientation = 1;
          checkAndUpdateOrientation(desiredOrientation);
          if (gyroReading == 90){
              // cout << "\nCurrently at point: " << currentX << " , " << currentY << endl; 
              // cout << "Orientation: " << gyroReading << endl; 
              Serial.print("Currently at point: ");
              Serial.print(currentX);
              Serial.print(" , ");
              Serial.println(currentY); 

              Serial.print("Orientation: ");
              Serial.println(gyroReading);
              goForward(90);

              previousX = currentX;
              previousY = currentY;

              currentX = currentX;
              currentY = currentY+1;
          }else{
              // cout << "Incorrect Orientation" << endl;
              break;
          }
          break;
      }else if (currentY > 0 && maze[currentX][currentY-1] == 'E' && (currentY-1) != previousY){ // y-1 in the array means go one column to the left
          desiredOrientation = 3;
          checkAndUpdateOrientation(desiredOrientation);
          if (gyroReading == 270){
              Serial.print("Currently at point: ");
              Serial.print(currentX);
              Serial.print(" , ");
              Serial.println(currentY); 

              Serial.print("Orientation: ");
              Serial.println(gyroReading);
              // cout << "\nCurrently at point: " << currentX << " , " << currentY << endl; 
              // cout << "Orientation: " << gyroReading << endl; 
              goForward(270);

              previousX = currentX;
              previousY = currentY;

              currentX = currentX;
              currentY = currentY-1;
          }else{
              // cout << "Incorrect Orientation" << endl;
              break;
          }
          break;
      }else if (currentX < rows-1 && maze[currentX+1][currentY] == 'E' && (currentX+1) != previousX){ // x+1 in the array means go down one row
          desiredOrientation = 2;
          checkAndUpdateOrientation(desiredOrientation);
          if (gyroReading == 0){
              Serial.print("Currently at point: ");
              Serial.print(currentX);
              Serial.print(" , ");
              Serial.println(currentY); 

              Serial.print("Orientation: ");
              Serial.println(gyroReading);
              // cout << "\nCurrently at point: " << currentX << " , " << currentY << endl; 
              // cout << "Orientation: " << gyroReading << endl; 
              goForward(0);

              previousX = currentX;
              previousY = currentY;

              currentX = currentX+1;
              currentY = currentY;
          }else{
              // cout << "Incorrect Orientation" << endl;
              break;
          }
          break;
      }

      if (currentX > 0 && maze[currentX-1][currentY] == '*' && (currentX-1) != previousX){ // x-1 in the array means go up one row
          desiredOrientation = 0;
          checkAndUpdateOrientation(desiredOrientation);
          if (gyroReading == 180){
              Serial.print("Currently at point: ");
              Serial.print(currentX);
              Serial.print(" , ");
              Serial.println(currentY); 

              Serial.print("Orientation: ");
              Serial.println(gyroReading);
              // cout << "\nCurrently at point: " << currentX << " , " << currentY << endl; 
              // cout << "Orientation: " << gyroReading << endl; 
              goForward(180);
              
              previousX = currentX;
              previousY = currentY;

              currentX = currentX-1;
              currentY = currentY;
          }else{
              // cout << "Incorrect Orientation" << endl;
              break;
          }
      }
      else if (currentY < cols-1 && maze[currentX][currentY+1] == '*' && (currentY+1) != previousY){ // y+1 in the array means go one column to the right
          desiredOrientation = 1;
          checkAndUpdateOrientation(desiredOrientation);
          if (gyroReading == 90){
              Serial.print("Currently at point: ");
              Serial.print(currentX);
              Serial.print(" , ");
              Serial.println(currentY); 

              Serial.print("Orientation: ");
              Serial.println(gyroReading);
              // cout << "\nCurrently at point: " << currentX << " , " << currentY << endl; 
              // cout << "Orientation: " << gyroReading << endl; 
              goForward(90);

              previousX = currentX;
              previousY = currentY;

              currentX = currentX;
              currentY = currentY+1;
          }else{
              // cout << "Incorrect Orientation" << endl;
              break;
          }
      }
      else if (currentY > 0 && maze[currentX][currentY-1] == '*' && (currentY-1) != previousY){ // y-1 in the array means go one column to the left
          desiredOrientation = 3;
          checkAndUpdateOrientation(desiredOrientation);
          if (gyroReading == 270){
              Serial.print("Currently at point: ");
              Serial.print(currentX);
              Serial.print(" , ");
              Serial.println(currentY); 

              Serial.print("Orientation: ");
              Serial.println(gyroReading);
              // cout << "\nCurrently at point: " << currentX << " , " << currentY << endl; 
              // cout << "Orientation: " << gyroReading << endl; 
              goForward(270);

              previousX = currentX;
              previousY = currentY;

              currentX = currentX;
              currentY = currentY-1;
          }else{
              // cout << "Incorrect Orientation" << endl;
              break;
          }
      }
      else if (currentX < rows-1 && maze[currentX+1][currentY] == '*' && (currentX+1) != previousX){ // x+1 in the array means go down one row
          desiredOrientation = 2;
          checkAndUpdateOrientation(desiredOrientation);
          if (gyroReading == 0){
              Serial.print("Currently at point: ");
              Serial.print(currentX);
              Serial.print(" , ");
              Serial.println(currentY); 

              Serial.print("Orientation: ");
              Serial.println(gyroReading);
              // cout << "\nCurrently at point: " << currentX << " , " << currentY << endl; 
              // cout << "Orientation: " << gyroReading << endl; 
              goForward(0);

              previousX = currentX;
              previousY = currentY;

              currentX = currentX+1;
              currentY = currentY;
          }else{
              // cout << "Incorrect Orientation" << endl;
              break;
          }
      }

  }
  // cout << "Reached the finish line!"<< endl;
  // cout << "Finished at point: " << currentX << " , " << currentY << endl; 
  
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  motorSetup();
  ultrasonicSetup();
  encoderSetup();
  mpuSetup();
  // executePath(maze, 4, 4);
}

int ori = 180;

void loop() {
  // put your main code here, to run repeatedly:

  float fx = getDistanceFront();
  float rx = getDistanceRight();
  float lx = getDistanceLeft();

  if (fx<5) {
    turnRight(); ori-=90;
  } else if (lx>25) {
    turnLeft(); ori+=90;
  } else {
    goForward(ori);
  }

  if (ori>360) {ori-=360;}
  if (ori<0) {ori +=360;}
  // goStraightGyro(400);

  // restMotor();
  // delay(1000);

  // turnLeft();
  // restMotor();
  // delay(1000);

  
  // turnRight();
  // restMotor();
  // delay(1000);

}
