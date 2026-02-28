#include <iostream>
#include <array>
#include <string>
#include <chrono>
#include <limits>

using Board = std::array<std::array<int, 9>, 9>;

void print_board(const Board& board) {
    std::cout << "+-------+-------+-------+\n";
    for (int r = 0; r < 9; ++r) {
        std::cout << "| ";
        for (int c = 0; c < 9; ++c) {
            if (board[r][c] == 0)
                std::cout << ". ";
            else
                std::cout << board[r][c] << " ";
            if (c % 3 == 2) std::cout << "| ";
        }
        std::cout << "\n";
        if (r % 3 == 2) std::cout << "+-------+-------+-------+\n";
    }
}

bool is_valid(const Board& board, int row, int col, int num) {
    for (int c = 0; c < 9; ++c)
        if (board[row][c] == num) return false;

    for (int r = 0; r < 9; ++r)
        if (board[r][col] == num) return false;

    int box_r = (row / 3) * 3, box_c = (col / 3) * 3;
    for (int r = box_r; r < box_r + 3; ++r)
        for (int c = box_c; c < box_c + 3; ++c)
            if (board[r][c] == num) return false;

    return true;
}

bool solve(Board& board) {
    for (int r = 0; r < 9; ++r) {
        for (int c = 0; c < 9; ++c) {
            if (board[r][c] == 0) {
                for (int num = 1; num <= 9; ++num) {
                    if (is_valid(board, r, c, num)) {
                        board[r][c] = num;
                        if (solve(board)) return true;
                        board[r][c] = 0;
                    }
                }
                return false;
            }
        }
    }
    return true;
}

bool validate_board(const Board& board) {
    for (int r = 0; r < 9; ++r) {
        for (int c = 0; c < 9; ++c) {
            int val = board[r][c];
            if (val < 0 || val > 9) {
                std::cout << "Invalid value " << val << " at row " << r + 1 << ", col " << c + 1 << "\n";
                return false;
            }
            if (val != 0) {
                Board temp = board;
                temp[r][c] = 0;
                if (!is_valid(temp, r, c, val)) {
                    std::cout << "Conflict: " << val << " at row " << r + 1 << ", col " << c + 1 << "\n";
                    return false;
                }
            }
        }
    }
    return true;
}

void solve_and_display(Board board, const std::string& name) {
    std::cout << "\n--- " << name << " ---\n\n";
    std::cout << "Puzzle:\n";
    print_board(board);

    if (!validate_board(board)) {
        std::cout << "Invalid puzzle!\n";
        return;
    }

    auto start = std::chrono::high_resolution_clock::now();
    bool solved = solve(board);
    auto end = std::chrono::high_resolution_clock::now();
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    if (solved) {
        std::cout << "\nSolution:\n";
        print_board(board);
        if (us < 1000)
            std::cout << "Solved in " << us << " us\n";
        else
            std::cout << "Solved in " << us / 1000.0 << " ms\n";
    } else {
        std::cout << "No solution exists.\n";
    }
}

Board get_easy_puzzle() {
    return {{
        {5,3,0, 0,7,0, 0,0,0},
        {6,0,0, 1,9,5, 0,0,0},
        {0,9,8, 0,0,0, 0,6,0},
        {8,0,0, 0,6,0, 0,0,3},
        {4,0,0, 8,0,3, 0,0,1},
        {7,0,0, 0,2,0, 0,0,6},
        {0,6,0, 0,0,0, 2,8,0},
        {0,0,0, 4,1,9, 0,0,5},
        {0,0,0, 0,8,0, 0,7,9},
    }};
}

Board get_medium_puzzle() {
    return {{
        {0,0,0, 6,0,0, 4,0,0},
        {7,0,0, 0,0,3, 6,0,0},
        {0,0,0, 0,9,1, 0,8,0},
        {0,0,0, 0,0,0, 0,0,0},
        {0,5,0, 1,8,0, 0,0,3},
        {0,0,0, 3,0,6, 0,4,5},
        {0,4,0, 2,0,0, 0,6,0},
        {9,0,3, 0,0,0, 0,0,0},
        {0,2,0, 0,0,0, 1,0,0},
    }};
}

Board get_hard_puzzle() {
    return {{
        {8,0,0, 0,0,0, 0,0,0},
        {0,0,3, 6,0,0, 0,0,0},
        {0,7,0, 0,9,0, 2,0,0},
        {0,5,0, 0,0,7, 0,0,0},
        {0,0,0, 0,4,5, 7,0,0},
        {0,0,0, 1,0,0, 0,3,0},
        {0,0,1, 0,0,0, 0,6,8},
        {0,0,8, 5,0,0, 0,1,0},
        {0,9,0, 0,0,0, 4,0,0},
    }};
}

Board read_custom_puzzle() {
    Board board{};
    std::cout << "\nEnter the puzzle row by row (use 0 for empty cells).\n";
    std::cout << "Enter 9 digits per row, separated by spaces.\n\n";

    for (int r = 0; r < 9; ++r) {
        std::cout << "Row " << r + 1 << ": ";
        for (int c = 0; c < 9; ++c) {
            int val;
            if (!(std::cin >> val)) {
                if (std::cin.eof()) return board;
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Invalid input. Please enter numbers 0-9.\n";
                return read_custom_puzzle();
            }
            if (val < 0 || val > 9) {
                std::cout << "Values must be 0-9. Try again.\n";
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                return read_custom_puzzle();
            }
            board[r][c] = val;
        }
    }
    return board;
}

int main() {
    while (true) {
        std::cout << "\n================================\n";
        std::cout << "       SUDOKU SOLVER\n";
        std::cout << "================================\n\n";
        std::cout << "  1. Easy puzzle\n";
        std::cout << "  2. Medium puzzle\n";
        std::cout << "  3. Hard puzzle\n";
        std::cout << "  4. Enter custom puzzle\n";
        std::cout << "  5. Solve all built-in puzzles\n";
        std::cout << "  0. Exit\n";
        std::cout << "\nChoice: ";

        int choice;
        if (!(std::cin >> choice)) {
            if (std::cin.eof()) {
                std::cout << "\nGoodbye!\n";
                return 0;
            }
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input.\n";
            continue;
        }

        switch (choice) {
            case 1:
                solve_and_display(get_easy_puzzle(), "Easy Puzzle");
                break;
            case 2:
                solve_and_display(get_medium_puzzle(), "Medium Puzzle");
                break;
            case 3:
                solve_and_display(get_hard_puzzle(), "Hard Puzzle");
                break;
            case 4: {
                Board custom = read_custom_puzzle();
                solve_and_display(custom, "Custom Puzzle");
                break;
            }
            case 5:
                solve_and_display(get_easy_puzzle(), "Easy Puzzle");
                solve_and_display(get_medium_puzzle(), "Medium Puzzle");
                solve_and_display(get_hard_puzzle(), "Hard Puzzle");
                break;
            case 0:
                std::cout << "Goodbye!\n";
                return 0;
            default:
                std::cout << "Invalid choice.\n";
        }
    }
}
