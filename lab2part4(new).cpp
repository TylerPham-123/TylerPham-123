#include <iostream>
#include <vector>

bool isSafe(const std::vector<std::vector<int>>& board, int row, int col) {
    /**
     * Checks if placing a queen at board[row][col] is safe.
     *
     * @param board The current state of the chessboard.
     * @param row The row to check.
     * @param col The column to check.
     * @return True if safe, False otherwise.
     */

    int n = board.size();

    // Check the same row on the left side
    for (int i = 0; i < col; ++i) {
        if (board[row][i] == 1) {
            return false;
        }
    }

    // Check upper diagonal on left side
    for (int i = row, j = col; i >= 0 && j >= 0; --i, --j) {
        if (board[i][j] == 1) {
            return false;
        }
    }

    // Check lower diagonal on left side
    for (int i = row, j = col; i < n && j >= 0; ++i, --j) {
        if (board[i][j] == 1) {
            return false;
        }
    }

    return true;
}

bool solveNQUtil(std::vector<std::vector<int>>& board, int col) {
    /**
     * A recursive utility function to solve the N Queens problem.
     *
     * @param board The current state of the chessboard.
     * @param col The current column to place a queen.
     * @return True if a solution is found, False otherwise.
     */

    int n = board.size();

    // Base case: All queens are placed
    if (col >= n) {
        return true;
    }

    // Consider this column and try placing this queen in all rows one by one
    for (int i = 0; i < n; ++i) {
        if (isSafe(board, i, col)) {
            board[i][col] = 1; // Place the queen

            // Recur to place rest of the queens
            if (solveNQUtil(board, col + 1)) {
                return true;
            }

            // If placing queen in board[i][col] doesn't lead to a solution, then remove queen from board[i][col]
            board[i][col] = 0; // Backtrack
        }
    }

    // If the queen cannot be placed in any row in this column col, then return false
    return false;
}

std::vector<std::vector<int>> solveNQ(int n) {
    /**
     * Solves the N Queens problem.
     *
     * @param n The size of the chessboard (N x N).
     * @return The solved chessboard, or an empty vector if no solution exists.
     */

    std::vector<std::vector<int>> board(n, std::vector<int>(n, 0));

    if (!solveNQUtil(board, 0)) {
        std::cout << "Solution does not exist" << std::endl;
        return std::vector<std::vector<int>>(); // Return an empty vector
    }

    return board;
}

void printBoard(const std::vector<std::vector<int>>& board) {
    /**
     * Prints the chessboard.
     *
     * @param board The chessboard to print.
     */

    if (board.empty()) {
        return;
    }

    for (const auto& row : board) {
        for (int cell : row) {
            if (cell == 1) {
                std::cout << "Q ";
            } else {
                std::cout << ". ";
            }
        }
        std::cout << std::endl;
    }
}

int main() {
    /**
     * Main function to solve and display the 8 Queens problem.
     */
    int n = 8; // For the 8 Queens problem
    std::vector<std::vector<int>> solution = solveNQ(n);
    printBoard(solution);

    return 0;
}