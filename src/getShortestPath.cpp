#include <Arduino.h>
#include <Memory.h>
#include <Motor.h>
#include <Ultrasonic.h>
#include <Encoder.h>
#include <Gyro.h>

// Define maze size
#define ROWS 17
#define COLS 17

char mappedMaze[][17] = {
  {'#', 'E', '#', '#','#','#','#','#','#','#','#','#','#','#','#','#','#'},
  {'#', '.', '#', '.','.','.','.','.','.','.','.','.','.','.','.','.','#'},
  {'#', '.', '#', '.','#','#','#','#','#','.','#','#','#','#','#','#','#'},
  {'#', '.', '#', '.','.','.','#','.','.','.','.','.','#','.','.','.','#'},
  {'#', '.', '#', '#','#','#','#','.','#','#','#','#','#','.','#','#','#'},
  {'#', '.', '.', '.','.','.','#','.','.','.','#','.','.','.','#','.','#'},
  {'#', '.', '#', '#','#','.','#','.','#','.','#','.','#','#','#','.','#'},
  {'#', '.', '#', '.','.','.','.','.','#','.','#','.','.','.','.','.','#'},
  {'#', '.', '#', '.','#','#','#','.','#','#','#','.','#','.','#','#','#'},
  {'#', '.', '#', '.','.','.','#','.','.','.','#','.','#','.','.','.','#'},
  {'#', '.', '#', '#','#','#','#','.','#','#','#','.','#','#','#','#','#'},
  {'#', '.', '#', '.','#','.','.','.','.','.','#','.','.','.','.','.','#'},
  {'#', '.', '#', '.','#','#','#','#','#','.','#','#','#','.','#','.','#'},
  {'#', '.', '#', '.','#','.','.','.','.','.','.','.','.','.','#','.','#'},
  {'#', '#', '#', '.','#','.','#','#','#','#','#','.','#','.','#','.','#'},
  {'#', '.', '.', '.','.','.','#','.','.','.','.','.','#','.','#','.','#'},
  {'#', '#', '#', '#','#','#','#','#','#','#','#','#','#','#','#','S','#'}

};

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
bool isValid(char maze[ROWS][COLS], int r, int c, uint8_t visited[ROWS][(COLS + 7) / 8]) {
  return (r >= 0 && r < ROWS && c >= 0 && c < COLS && maze[r][c] != '#' && !(visited[r][c / 8] & (1 << (c % 8))));
}

// Function to reconstruct the path
void reconstructPath(uint8_t came_from[ROWS][COLS], Cell start, Cell end, char maze[ROWS][COLS]) {
  Cell current = end;

  while (current.row != start.row || current.col != start.col) {
      if (maze[current.row][current.col] != 'S' && maze[current.row][current.col] != 'E') {
          maze[current.row][current.col] = '*'; // Mark shortest path
      }
      uint8_t dir = came_from[current.row][current.col];
      if (dir == 0) current.row++;
      else if (dir == 1) current.row--;
      else if (dir == 2) current.col++;
      else if (dir == 3) current.col--;
  }
}

// BFS function to find the shortest path
bool solveMaze(char maze[ROWS][COLS], Cell start, Cell end) {
  Queue q;
  uint8_t visited[ROWS][(COLS + 7) / 8] = {0};
  uint8_t came_from[ROWS][COLS] = {0};

  q.enqueue(start);
  visited[start.row][start.col / 8] |= (1 << (start.col % 8));
  came_from[start.row][start.col] = 255; // Mark as start

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
              visited[nr][nc / 8] |= (1 << (nc % 8));
              came_from[nr][nc] = i; // Store direction
          }
      }
  }

  return false; // No path found
}

// Function to get the shortest path modified maze
void getShortestPath(char maze[ROWS][COLS]) {
  Cell start = {16, 15}; // Example start position
  Cell end = {0, 1};   // Example end position

  if (!solveMaze(maze, start, end)) {
      Serial.println("No path found.");
  }
}

// Example function to print the maze (for debugging)
void printMaze(char maze[ROWS][COLS]) {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            Serial.print(maze[i][j]);
            // Serial.print(" ");
        }
        Serial.println();
    }
}
void setup() {
  Serial.begin(115200);

  getShortestPath(mappedMaze);
  
  // // check the resulting shortest path mapping
  Serial.println("Maze with Shortest Path:");
  printMaze(mappedMaze);
  
}


void loop() {
  // 

}
