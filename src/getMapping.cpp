#include <Arduino.h>
#include <Memory.h>
#include <Motor.h>
#include <Ultrasonic.h>
#include <Encoder.h>
#include <Gyro.h>
#include <ArduinoSTL.h>
#include <stack>
#include <queue>
#include <ArxTypeTraits.h>

char unmappedMaze[][17] = {
    {'-', '-', '-', '-','-','-','-','-','-','-','-','-','-','-','-','E','-'},
    {'-', '-', '-', '-','-','-','-','-','-','-','-','-','-','-','-','-','-'},
    {'-', '-', '-', '-','-','-','-','-','-','-','-','-','-','-','-','-','-'},
    {'-', '-', '-', '-','-','-','-','-','-','-','-','-','-','-','-','-','-'},
    {'-', '-', '-', '-','-','-','-','-','-','-','-','-','-','-','-','-','-'},
    {'-', '-', '-', '-','-','-','-','-','-','-','-','-','-','-','-','-','-'},
    {'-', '-', '-', '-','-','-','-','-','-','-','-','-','-','-','-','-','-'},
    {'-', '-', '-', '-','-','-','-','-','-','-','-','-','-','-','-','-','-'},
    {'-', '-', '-', '-','-','-','-','-','-','-','-','-','-','-','-','-','-'},
    {'-', '-', '-', '-','-','-','-','-','-','-','-','-','-','-','-','-','-'},
    {'-', '-', '-', '-','-','-','-','-','-','-','-','-','-','-','-','-','-'},
    {'-', '-', '-', '-','-','-','-','-','-','-','-','-','-','-','-','-','-'},
    {'-', '-', '-', '-','-','-','-','-','-','-','-','-','-','-','-','-','-'},
    {'-', '-', '-', '-','-','-','-','-','-','-','-','-','-','-','-','-','-'},
    {'-', '-', '-', '-','-','-','-','-','-','-','-','-','-','-','-','-','-'},
    {'-', 'S', '-', '-','-','-','-','-','-','-','-','-','-','-','-','-','-'},
    {'#', '#', '#', '-','-','-','-','-','-','-','-','-','-','-','-','-','-'}
};

const int ROWS = 17;
const int COLS = 17;

int currentX = 15;
int currentY = 1;

int startingX = 15;
int startingY = 1;

int endingX = 0;
int endingY = 15;

// Track the robot's current orientation (0: UP, 1: DOWN, 2: LEFT, 3: RIGHT)
int currentOrientation = 0;
int gyroReading = 180;

// 0 = UP, 1 = RIGHT, 2 = DOWN, 3 = LEFT
int dr[] = {-1, 0, 0};
int dc[] = {0, 1, -1};

int nodePathNum = 0;
int completedNodePathNum = 0;

// Structure to represent a position
struct Position {
    int row, col;
};

void setHash(char maze[][COLS]){
    for (int i = 0; i < ROWS; i=i+2) {
        for (int j = 0; j < COLS; j=j+2) {
            // cout << maze[i][j] << " ";
            maze[i][j] = '#';
        }
    }
}
void setBorderHash(char maze[][COLS]){

    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (i==0 || i==16 || j==0 || j==16){
                maze[i][j] = '#';
            }
        }
    }
}
void setSymbol(char maze[][COLS], int current_x, int current_y, int currentOrientation) {
    // cout << "Setting symbol for coordinate: " << current_x << ", " << current_y << endl;
    Serial.print("Setting symbol for coordinate: ");
    Serial.print(current_x);
    Serial.print(", ");
    Serial.println(current_y);

    if (currentOrientation == 0){
        if (current_x>0){
            if (getDistanceFront()<8){
                if (maze[current_x-1][current_y] == '-'|| maze[current_x-1][current_y] != '#'){
                    maze[current_x-1][current_y] = '#';
                }
            }else{
                if (maze[current_x-1][current_y] == '-'|| maze[current_x-1][current_y] != '#'){
                    maze[current_x-1][current_y] = '.';
                }
            }
        }
        if (current_y>0){
            if (getDistanceLeft()<15){
                if (maze[current_x][current_y-1] == '-'|| maze[current_x][current_y-1] != '#'){
                    maze[current_x][current_y-1] = '#'; 
                }
            }else{
                if (maze[current_x][current_y-1] == '-'|| maze[current_x][current_y-1] != '#'){
                    maze[current_x][current_y-1] = '.'; 
                }
            }
        }
        if (current_y<(COLS-1)){
            if (getDistanceRight()<15){
                if (maze[current_x][current_y+1] == '-'|| maze[current_x][current_y+1] != '#'){
                    maze[current_x][current_y+1] = '#';
                }
            }else{
                if (maze[current_x][current_y+1] == '-'|| maze[current_x][current_y+1] != '#'){
                    maze[current_x][current_y+1] = '.';
                }
            }
        }

    }else if (currentOrientation == 1){
        if (current_y<COLS-1){
            if (getDistanceFront()<8){
                if (maze[current_x][current_y+1] == '-'|| maze[current_x][current_y+1] != '#'){
                    maze[current_x][current_y+1] = '#';
                }
            }else{
                if (maze[current_x][current_y+1] == '-'|| maze[current_x][current_y+1] != '#'){
                    maze[current_x][current_y+1] = '.';
                }
            }
        }
        if (current_x>0){
            if (getDistanceLeft()<15){
                if (maze[current_x-1][current_y] == '-'|| maze[current_x-1][current_y] != '#'){
                    maze[current_x-1][current_y] = '#'; 
                }
            }else{
                if (maze[current_x-1][current_y] == '-'|| maze[current_x-1][current_y] != '#'){
                    maze[current_x-1][current_y] = '.'; 
                }
            }
        }
        if (current_x<ROWS-1){
            if (getDistanceRight()<15){
                if (maze[current_x+1][current_y] == '-'|| maze[current_x+1][current_y] != '#'){
                    maze[current_x+1][current_y] = '#';
                }
            }else {
                if (maze[current_x+1][current_y] == '-'|| maze[current_x+1][current_y] != '#'){
                    maze[current_x+1][current_y] = '.';
                }
            }
        }

    }else if (currentOrientation == 2){
        if (current_x<ROWS-1){
            if (getDistanceFront()<8){
                if (maze[current_x+1][current_y] == '-'|| maze[current_x+1][current_y] != '#'){
                    maze[current_x+1][current_y] = '#';
                }
            }else{
                if (maze[current_x+1][current_y] == '-'|| maze[current_x+1][current_y] != '#'){
                    maze[current_x+1][current_y] = '.';
                }
            }
        }
        if (current_y<COLS-1){
            if (getDistanceLeft()<15){
                if (maze[current_x][current_y+1] == '-'|| maze[current_x][current_y+1] != '#'){
                    maze[current_x][current_y+1] = '#'; 
                }
            }else{
                if (maze[current_x][current_y+1] == '-'|| maze[current_x][current_y+1] != '#'){
                    maze[current_x][current_y+1] = '.'; 
                }
            }
        }
        if (current_y>0){
            if (getDistanceRight()<15){
                if (maze[current_x][current_y-1] == '-'|| maze[current_x][current_y-1] != '#'){
                    maze[current_x][current_y-1] = '#';
                }
            }else{
                if (maze[current_x][current_y-1] == '-'|| maze[current_x][current_y-1] != '#'){
                    maze[current_x][current_y-1] = '.';
                }
            }
        }
    
    }else if (currentOrientation == 3){
        if (current_y>0){
            if (getDistanceFront()<8){
                if (maze[current_x][current_y-1] == '-'|| maze[current_x][current_y-1] != '#'){
                    maze[current_x][current_y-1] = '#';
                }
            }else{
                if (maze[current_x][current_y-1] == '-'|| maze[current_x][current_y-1] != '#'){
                    maze[current_x][current_y-1] = '.';
                }
            }
        }
        if (current_x<ROWS-1){
            if (getDistanceLeft()<15){
                if (maze[current_x+1][current_y] == '-' || maze[current_x+1][current_y] != '#'){
                    maze[current_x+1][current_y] = '#'; 
                }
            }else{
                if (maze[current_x+1][current_y] == '-' || maze[current_x+1][current_y] != '#'){
                    maze[current_x+1][current_y] = '.'; 
                }
            }
        }
        if (current_x>0){
            if (getDistanceRight()<15){
                if (maze[current_x-1][current_y] == '-'|| maze[current_x-1][current_y] != '#'){
                    maze[current_x-1][current_y] = '#';
                }
            }else{
                if (maze[current_x-1][current_y] == '-'|| maze[current_x-1][current_y] != '#'){
                    maze[current_x-1][current_y] = '.';
                }
            }
        }
    }
}
void goStraightGyro(float targetDistance, float desiredAngle){
    targetDistance = 3.7 * targetDistance;
    float initialDistance = getMovingDistance();
    while ((getMovingDistance()-initialDistance)<targetDistance) {
    //   Serial.println(update());
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
    delay(100);
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
    delay(100);
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
    delay(100);
  }
// Movement functions (to be implemented for the actual robot)
void goForward(float currentAngle){
    // setPathTravelled("Forward");
    Serial.println("going Forward");
    // cout << " => Went Forward" << endl;
    // moves forward for 25 cm
    goStraightGyro(14, currentAngle);
  }
  
  void turnRight(){
    // setPathTravelled("Right");
    Serial.println("turning Right");
    // cout << " => turned Right" << endl;
    turnRightUntilCorrect();
    // delay(3000);
    // turn right 90 degree
  }
  
  void turnLeft(){
    // setPathTravelled("Left");
    Serial.println("turning Left");
    // cout << " => turned Left" << endl;
    turnLeftUntilCorrect();
    // delay(3000);
    // turn left 90 degree
  }

// Example function to print the maze (for debugging)
void printMaze(char maze[ROWS][COLS]) {
    // cout << endl;
    Serial.println();
    Serial.println("Your Maze Mapping: ");
    // cout << "Your Maze Mapping: " << endl;
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            // cout << maze[i][j] << " ";
            Serial.print(maze[i][j]);
            Serial.print(" ");
        }
        Serial.println();
        // cout << endl;
    }
    Serial.println();
    // cout << endl;
}
void printCurrentPositionInMaze(char maze[ROWS][COLS], int X, int Y) {
    // cout << endl;
    // cout << "Current Coordinate ( "<< currentX << " ," << currentY << ")" << endl;
    // cout << "Your Maze Mapping: " << endl;
    Serial.print("Current Coordinate ( ");
    Serial.print(currentX);
    Serial.print(", ");
    Serial.print(currentY);
    Serial.print(") ");
    Serial.println("Your Maze Mapping: ");
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (i==X && j==Y){
                // cout  << "X ";
                Serial.print("X ");
            }else{
                // cout << maze[i][j] << " ";
                Serial.print(maze[i][j]);
                Serial.print(" ");
            }
        }
        Serial.println();
        // cout << endl;
    }
    // cout << endl;
}
void printStack(const std::stack<Position>& stack) {
    std::stack<Position> tempStack = stack; // Create a copy of the stack
    // std::cout << "Stack contents (from top to bottom):" << std::endl;
    // Serial.println("Current stack: ");
    if (stack.empty() == true){
        Serial.println("Your stack is empty! ");
    }
    while (!tempStack.empty()) {
        Position pos = tempStack.top();
        // std::cout << "(" << pos.row << ", " << pos.col << ")" << std::endl;
        Serial.print("(");
        Serial.print(pos.row);
        Serial.print(" , ");
        Serial.print(pos.col);
        Serial.println(")");
        tempStack.pop(); // Remove the top element after printing
    }
    // std::cout << "End of stack." << std::endl;
}
// Function to print the contents of the queue
void printQueue(std::queue<Position> queue) {
    std::queue<Position> tempQueue = queue; // Create a copy of the stack
    // cout << endl;
    // cout << "Current Queue contents: ";
    if (queue.empty() == true){
        Serial.println("Your queue is empty! ");
    }
    Serial.println("Current queue: ");
    while (!tempQueue.empty()) {
        Position pos = tempQueue.front();
        // std::cout << "(" << pos.row << ", " << pos.col << ")" << std::endl;
        Serial.print("(");
        Serial.print(pos.row);
        Serial.print(" , ");
        Serial.print(pos.col);
        Serial.println(")");
        tempQueue.pop(); // Remove the top element after printing
    }
    // cout << endl;
}
// Adjust the robot's direction before moving forward
void checkAndUpdateOrientation(int requiredOrientation) { // 0 = UP, 1 = RIGHT, 2 = DOWN, 3 = LEFT
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
  void checkDestinationAndMove(int current_x, int current_y, int desiredX, int desiredY) {
    // Determine direction based on coordinate differences
    int desiredOrientation;
    int requiredAngle;

    Serial.print("currentX: ");
    Serial.println(current_x);
    Serial.print("currentY: ");
    Serial.println(current_y);
    
    Serial.print("desired x: ");
    Serial.println(desiredX);
    Serial.print("desired y: ");
    Serial.println(desiredY);
    
    
    if (current_y < desiredY) {
        desiredOrientation = 1; // Move East
        requiredAngle = 90;
    } else if (current_y > desiredY) {
        desiredOrientation = 3; // Move West
        requiredAngle = 270;
    } else if (current_x > desiredX) {
        desiredOrientation = 0; // Move North
        requiredAngle = 180;
    } else if (current_x < desiredX) {
        desiredOrientation = 2; // Move South
        requiredAngle = 0;
    } else {
        return; // Already at destination
    }
    
    Serial.print("Desired Orientation： ");
    Serial.println(desiredOrientation);
    // Adjust orientation before moving
    checkAndUpdateOrientation(desiredOrientation);

    // Move forward the required number of times
    int distance = abs(current_x - desiredX) + abs(current_y - desiredY);
    for (int i = 0; i < distance; i++) {
        goForward(requiredAngle);
    }
    currentX = desiredX;
    currentY = desiredY;
}
void backTrackToPreviousNode(std::stack<Position>& nodestack, std::stack<Position>& pathstack) {
    if (nodestack.empty()) {
        // std::cout << "No path to backtrack!" << std::endl;
        Serial.println("No node to backtrack");
        return;
    }
    
    Position currentNode = nodestack.top();

    bool latestCoordinateIsNode = false;
    Serial.println("Backtrack Node Stack: ");
    printStack(nodestack);
    Serial.println("Back Track Path Queue: ");
    printStack(pathstack);

    while (latestCoordinateIsNode == false){
        Position currentCoordinate = pathstack.top();
        checkDestinationAndMove(currentX, currentY, currentCoordinate.row, currentCoordinate.col);

        if (currentCoordinate.row == currentNode.row && currentCoordinate.col == currentNode.col){
            latestCoordinateIsNode = true;
            break;
        }
        pathstack.pop();
    }
    nodestack.pop();
}

void getMapping(char maze[][COLS], int endRow, int endCol) {
    bool visited[ROWS][COLS] = {false}; // Track visited positions
    std::stack<Position> dfsStack; // Stack for DFS
    std::stack<Position> nodeStack;
    std::stack<Position> pathstack; // Queue to store the path for backtracking

    // Find the starting position ('S')
    Position start;
    bool found = false;
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (maze[i][j] == 'S') {
                start = {i, j};
                found = true;
                break;
            }
        }
        if (found) break;
    }

    // Start DFS from the starting position
    dfsStack.push(start);
    pathstack.push(start); // Add the starting position to the path queue

    Serial.println("DFS Stack: ");
    printStack(dfsStack);
    Serial.println("Node Stack: ");
    printStack(nodeStack);
    Serial.println("Path Stack: ");
    printStack(pathstack);

    visited[start.row][start.col] = true;

    // Perform DFS
    while (!dfsStack.empty()) {
        
        bool isDeadEnd = true; 
        printCurrentPositionInMaze(maze, currentX, currentY);
        Position current = dfsStack.top();
        dfsStack.pop();
        
        int r = current.row;
        int c = current.col;
        
        // Check if the current position is the destination and move the robot
        checkDestinationAndMove(currentX, currentY, r, c);

        // Check if the current position is the destination
        if (r == endRow && c == endCol) {
            // std::cout << "Reached the end point!" << std::endl;
            Serial.println("Reached the Finish line");
            break; // Exit the loop if the end point is reached
        }

        setSymbol(maze, r, c, currentOrientation);

        int validBranchNum = 0;
        // Explore all 4 directions (up, right, down left)  
        int nr;
        int nc;
        for (int i = 0; i < 3; i++) {
            nr = r + dr[i];
            nc = c + dc[i];

            // Check if the new position is within bounds, not a wall, and not visited
            if (nr >= 0 && nr < ROWS && nc >= 0 && nc < COLS &&
                maze[nr][nc] != '#' && !visited[nr][nc]) {
                    // Mark the new position as visited and push it onto the stack
                    isDeadEnd = false;
                    visited[nr][nc] = true;
                    dfsStack.push({nr, nc});
                    pathstack.push({nr, nc}); // Add the new position to the path queue
                    validBranchNum++;
            }
        }

        if (validBranchNum>1){
            Serial.println("Node Detected!");
            nodeStack.push({r, c});
            nodePathNum = validBranchNum;
        }
        // If it's a dead end, backtrack to the previous node
        if (completedNodePathNum==nodePathNum-1){
            Serial.println("We have reached the dead end of the last node, mapping completed!");
            break;
        }
        if (isDeadEnd) {
            Serial.println("We have reached a Dead End:");
            Serial.println("Node Stack: ");
            printStack(nodeStack);
            Serial.println("Back Track Path Stack: ");
            printStack(pathstack);
            Serial.println("Executing Back Tracking... ");
            
            backTrackToPreviousNode(nodeStack, pathstack);
            completedNodePathNum++;
        }
        
    }

    // Restore the starting and ending positions
    maze[startingX][startingY] = 'S';
    maze[endingX][endingY] = 'E';
}

void setup() {
    Serial.begin(115200);
    motorSetup();
    ultrasonicSetup();
    encoderSetup();
    mpuSetup();

    setHash(unmappedMaze);
    setBorderHash(unmappedMaze);
    printMaze(unmappedMaze);

    getMapping(unmappedMaze, endingX, endingY);
}
  
void loop() {
    
}
