#include <Arduino.h>
#include <Memory.h>
#include <Motor.h>
#include <Ultrasonic.h>
#include <Encoder.h>
#include <Gyro.h>

// Define maze size
#define ROWS 9
#define COLS 9

// char mappedMaze[ROWS][COLS];  // Global array to store mapped maze

char mappedMaze[][9] = {
  {'#', '#', '#', '#','#','#','#','E','#'},
  {'#', '.', '#', '.','.','.','.','.','#'},
  {'#', '.', '#', '.','#','.','#','.','#'},
  {'#', '.', '#', '.','#','.','#','.','#'},
  {'#', '.', '#', '#','#','#','#','.','#'},
  {'#', '.', '.', '.','.','.','#','.','#'},
  {'#', '.', '#', '#','#','.','#','.','#'},
  {'#', '.', '#', '.','.','.','.','.','#'},
  {'#', 'S', '#', '#','#','#','#','#','#'}
};

// Current (x, y) position of car, top left is (0, 0)
int currentX = 0;
int currentY = 0;

int previousX = 0;
int previousY = 0;

// Assuming orientation at starting point always face north
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
    Serial.println();
    Serial.println("Path Summary:");
    for (int i = 0; i < pathIndex; i++) {
        // cout << "Step " << i + 1 << ": " << pathTravelled[i] << endl;
        Serial.print("Step ");
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.println(pathTravelled[i]);
    }
}

int gyroReading = 180;
//  180: facing front
//   90: facing right
//    0: facing backward
//  270: facing left

// No more using this function (cuz encoder doesn't work very well)
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

// Use gyro to ensure car go in straight line for a certain distance
// target distance for some reason has to multiply by 4, if want move 25cm then put 100 as argument
// Biggest problem with this is gyro go haywire where the angle keep decreasing infinitely
void goStraightGyro(float targetDistance, float desiredAngle){
  targetDistance = 3.7 * targetDistance;
  float initialDistance = getMovingDistance();
  while ((getMovingDistance()-initialDistance)<targetDistance) {
    Serial.println(update());
    if (desiredAngle==0) {
      if (update()<180) {
        slightRight();
      } else {
        slightLeft();
      }
    } else if (update()<desiredAngle) {
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

// cannot put delay after calling this function, or else gyro will not update and shit happens
void turnRightUntilCorrect(){
  float targetAngle = update() - 90;
  if (targetAngle<0){
      targetAngle += 360;
  }
  while (abs(update()-targetAngle)>25){
      goHardRightMotor(120);
  }
  restMotor(); // for gyro to get the next reading
}
// cannot put delay after calling this function, or else gyro will not update and shit happens
void turnLeftUntilCorrect(){
  float targetAngle = update() + 90;
  if (targetAngle>360){
      targetAngle -= 360;
  }
  while (abs(update()-targetAngle)>25){
      goHardLeftMotor(120);
  }
  restMotor(); // for gyro to get the next reading
}
// this function not in use currently cuz i just set reading manually
void setGyroReading(int angle){
  gyroReading = angle;
}

// Record the movement of robot car throughout the entire journey (e.g forward, left, right)
void setPathTravelled(String path){
  pathTravelled[pathIndex] = path;
  pathIndex++;
}

void goForward(float currentAngle){
  setPathTravelled("Forward");
  Serial.println("going Forward");
  // cout << " => Went Forward" << endl;
  // moves forward for 25 cm
  goStraightGyro(12.5, currentAngle);
}

void turnRight(){
  setPathTravelled("Right");
  Serial.println("turning Right");
  // cout << " => turned Right" << endl;
  turnRightUntilCorrect();
  // delay(3000);
  // turn right 90 degree
}

void turnLeft(){
  setPathTravelled("Left");
  Serial.println("turning Left");
  // cout << " => turned Left" << endl;
  turnLeftUntilCorrect();
  // delay(3000);
  // turn left 90 degree
}
// check current orientation of car and turn until correct orientation
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

// main function to execute path, size of maze must change manually in the function parameter
void executePath(char maze[][COLS], int rows, int cols) {
  int currentX = 0, currentY = 0; // Starting position

  // Find the starting position 'S'
  for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {
          if (maze[i][j] == 'S') {
              currentX = i;
              currentY = j;
              Serial.print("At Starting Point: ");
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
          desiredOrientation = 0; // we want to go north
          checkAndUpdateOrientation(desiredOrientation);
          if (gyroReading == 180){ // car must face north then only can go up

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
          desiredOrientation = 1; // we want to go east
          checkAndUpdateOrientation(desiredOrientation);
          if (gyroReading == 90){ // car must face east in order to go right
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
          desiredOrientation = 3; // we want to go west
          checkAndUpdateOrientation(desiredOrientation);
          if (gyroReading == 270){ // car must face west to go left
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
          desiredOrientation = 2; // we want to go south
          checkAndUpdateOrientation(desiredOrientation);
          if (gyroReading == 0){ // car must face south to go down
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
          desiredOrientation = 0; // we want to go north
          checkAndUpdateOrientation(desiredOrientation);
          if (gyroReading == 180){ // car must face north to go up
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
          desiredOrientation = 1; // we want to go east
          checkAndUpdateOrientation(desiredOrientation);
          if (gyroReading == 90){ // car must face east to go right
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
          desiredOrientation = 3; // we want to go west
          checkAndUpdateOrientation(desiredOrientation);
          if (gyroReading == 270){ // car must face west to go left
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
          desiredOrientation = 2; // we want to go south
          checkAndUpdateOrientation(desiredOrientation);
          if (gyroReading == 0){ // car must face south to go down
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
  Serial.println("You've reached the finish line!");
  // cout << "Reached the finish line!"<< endl;
  // cout << "Finished at point: " << currentX << " , " << currentY << endl; 
}


// BFS algorithm to find shortest path
// Structure to represent a cell in the maze
struct Cell {
    int row;
    int col;
};

// Fixed-size queue for BFS (instead of dynamic memory allocation)
struct Queue {
    Cell queue[ROWS * COLS]; // Maximum size of queue
    int front;
    int rear;

    Queue() {
        front = 0;
        rear = 0;
    }

    bool isEmpty() {
        return front == rear;
    }

    void enqueue(Cell c) {
        if (rear < ROWS * COLS) {
            queue[rear++] = c;
        }
    }

    Cell dequeue() {
        if (!isEmpty()) {
            return queue[front++];
        }
        return {-1, -1}; // Invalid cell
    }
};

// Function to check if a cell is valid (within bounds and not a wall)
bool isValid(char maze[ROWS][COLS], int r, int c, bool visited[ROWS][COLS]) {
    return (r >= 0 && r < ROWS && c >= 0 && c < COLS && maze[r][c] != '#' && !visited[r][c]);
}

// Function to reconstruct the path
void reconstructPath(Cell came_from[ROWS][COLS], Cell start, Cell end, char maze[ROWS][COLS]) {
    Cell current = end;

    while (current.row != start.row || current.col != start.col) {
        if (maze[current.row][current.col] != 'S' && maze[current.row][current.col] != 'E') {
            maze[current.row][current.col] = '*'; // Mark shortest path
        }
        current = came_from[current.row][current.col];
    }
}

// BFS function to find the shortest path
bool solveMaze(char maze[ROWS][COLS], Cell start, Cell end) {
    Queue q;
    bool visited[ROWS][COLS] = {false};
    Cell came_from[ROWS][COLS];

    q.enqueue(start);
    visited[start.row][start.col] = true;
    came_from[start.row][start.col] = {-1, -1}; // Mark as start

    int dr[] = {-1, 1, 0, 0}; // Row offsets
    int dc[] = {0, 0, -1, 1}; // Column offsets

    while (!q.isEmpty()) {
        Cell current = q.dequeue();

        if (current.row == end.row && current.col == end.col) {
            reconstructPath(came_from, start, end, maze);
            return true; // Path found
        }

        for (int i = 0; i < 4; i++) {
            int nr = current.row + dr[i];
            int nc = current.col + dc[i];

            if (isValid(maze, nr, nc, visited)) {
                Cell next = {nr, nc};
                q.enqueue(next);
                visited[nr][nc] = true;
                came_from[nr][nc] = current;
            }
        }
    }

    return false; // No path found
}

// Function to get the shortest path modified maze
void getShortestPath(char maze[ROWS][COLS]) {
    Cell start = {8, 1}; // Example start position
    Cell end = {0, 7};   // Example end position

    if (!solveMaze(maze, start, end)) {
        Serial.println("No path found.");
    }
}

// Example function to print the maze (for debugging)
void printMaze(char maze[ROWS][COLS]) {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            Serial.print(maze[i][j]);
            Serial.print(" ");
        }
        Serial.println();
    }
}

// Store the maze mapping in EEPROM
void storeMazeInEEPROM(char maze[ROWS][COLS]) {
  int address = 0;
  for (int i = 0; i < ROWS; i++) {
    for (int j = 0; j < COLS; j++) {
      EEPROM.write(address++, maze[i][j]);
    }
  }
}

// Read the maze from EEPROM
void readMazeFromEEPROM(char maze[ROWS][COLS]) {
  int address = 0;
  for (int i = 0; i < ROWS; i++) {
    for (int j = 0; j < COLS; j++) {
      maze[i][j] = EEPROM.read(address++);
    }
  }
}

// Simulated wall detection function (replace with actual sensor logic)
bool detectWall(int row, int col) {
  // Implement actual wall detection using ultrasonic sensors
  return false;  // Placeholder (assume no walls for now)
}
// Function to explore the maze and map it
void getMapping() {
  // Robot movement logic to explore the maze
  for (int i = 0; i < ROWS; i++) {
    for (int j = 0; j < COLS; j++) {
      if (detectWall(i, j)) {
        mappedMaze[i][j] = '#';  // Wall detected
      } else {
        mappedMaze[i][j] = '.';  // Open path
      }
    }
  }

  mappedMaze[0][0] = 'S';  // Start position
  mappedMaze[ROWS - 1][COLS - 1] = 'E';  // End position
}


void setup() {
  Serial.begin(115200);
  motorSetup();
  ultrasonicSetup();
  encoderSetup();
  mpuSetup();
  
  // Step 1: Map the maze using the robot
  // getMapping();
  
  // check the mapping
  // Serial.println("Original Maze:");
  // printMaze(mappedMaze);
  
  // // Step 2: find the shortest path using BFS
  // getShortestPath(mappedMaze);
  
  // // check the resulting shortest path mapping
  // Serial.println("Maze with Shortest Path:");
  // printMaze(mappedMaze);
  
  // // // Step 3: Store the maze in EEPROM
  // memoryReset();
  // storeMazeInEEPROM(mappedMaze);

  // Uncomment this section after finish doing mapping, or u want to run all together also can
  // Step 4: Read the maze from EEPROM into a local array
  char retrievedMaze[ROWS][COLS];
  readMazeFromEEPROM(retrievedMaze); 

  // check the resulting shortest path mapping
  Serial.println("Read Maze in EEPROM:");
  printMaze(retrievedMaze);

  // Step 5: Execute pathfinding using the retrieved maze
  executePath(retrievedMaze, ROWS, COLS);

}

int ori = 180;

void loop() {
  // put your main code here, to run repeatedly:

  // Jimm's Left Wall Following Code
  // float fx = getDistanceFront();
  // // float rx = getDistanceRight();
  // float lx = getDistanceLeft();

  // if (fx<5) {
  //   turnRight(); ori-=90;
  // } else if (lx>25) {
  //   turnLeft(); ori+=90; 
  // } else {
  //   goForward(ori);
  // }

  // if (ori>360) {ori-=360;}
  // if (ori<0) {ori +=360;}


  // Testing Individual Functions
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
