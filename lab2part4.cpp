#include <iostream>
#include <vector>
#include <stack>

using namespace std;

// Function to check if a queen can be placed at a given position
bool isSafe(const vector<int>& queens, int row, int col) {
    for (int i = 0; i < row; ++i) {
        if (queens[i] == col || queens[i] - col == i - row || queens[i] - col == row - i) {
            return false; // Conflict found
        }
    }
    return true; // No conflict
}

// Function to solve the Eight Queens problem using a stack
void solveEightQueens() {
    stack<vector<int>> solutions;
    vector<int> currentQueens(8, -1); // Initialize to -1 (no queen placed)
    int row = 0;

    while (row >= 0) {
        int col = currentQueens[row] + 1; // Start from the next column

        while (col < 8) {
            if (isSafe(currentQueens, row, col)) {
                currentQueens[row] = col; // Place the queen
                if (row == 7) {
                    solutions.push(currentQueens); // Solution found
                    currentQueens[row] = -1; // Reset for finding other solutions
                    break;
                } else {
                    row++; // Move to the next row
                    break;
                }
            }
            col++;
        }

        if (col == 8) { // No safe column found in this row
            currentQueens[row] = -1; // Reset the current row
            row--; // Backtrack to the previous row
        }
    }

    // Print the solutions
    while (!solutions.empty()) {
        vector<int> solution = solutions.top();
        solutions.pop();

        for (int row = 0; row < 8; ++row) {
            for (int col = 0; col < 8; ++col) {
                if (solution[row] == col) {
                    cout << "Q ";
                } else {
                    cout << ". ";
                }
            }
            cout << endl;
        }
        cout << endl;
    }
}

int main() {
    solveEightQueens();
    return 0;
}