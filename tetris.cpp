#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>

using namespace std;

const int WIDTH = 10;
const int HEIGHT = 20;
vector<vector<int>> board(HEIGHT, vector<int>(WIDTH, 0));
bool gameOver = false;

struct Point {
    int x, y;
};

Point shapes[7][4] = {
    {{0,1},{1,1},{2,1},{3,1}},  // I
    {{0,0},{0,1},{1,1},{2,1}},  // J
    {{2,0},{0,1},{1,1},{2,1}},  // L
    {{1,0},{2,0},{1,1},{2,1}},  // O
    {{1,0},{0,1},{1,1},{2,1}},  // T
    {{0,0},{1,0},{1,1},{2,1}},  // S
    {{1,0},{2,0},{0,1},{1,1}},  // Z
};

Point block[4], temp[4];
int currentShape;
int dx = 0;
bool rotateFlag = false;

struct termios orig_termios;

void reset_terminal_mode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
}

void set_conio_terminal_mode() {
    struct termios new_termios;
    tcgetattr(STDIN_FILENO, &orig_termios);
    new_termios = orig_termios;
    new_termios.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
    atexit(reset_terminal_mode);
}

int kbhit() {
    timeval tv = {0L, 0L};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    return select(1, &fds, nullptr, nullptr, &tv);
}

char getch() {
    char c;
    if (read(STDIN_FILENO, &c, 1) < 0) return 0;
    return c;
}

bool check() {
    for (int i = 0; i < 4; ++i) {
        int x = block[i].x;
        int y = block[i].y;
        if (x < 0 || x >= WIDTH || y >= HEIGHT) return false;
        if (y >= 0 && board[y][x]) return false;
    }
    return true;
}

void spawnShape() {
    dx = 0;
    rotateFlag = false;
    currentShape = rand() % 7;
    for (int i = 0; i < 4; i++) {
        block[i] = shapes[currentShape][i];
        block[i].y -= 2;
    }
}

void fixToBoard() {
    for (int i = 0; i < 4; ++i) {
        int x = block[i].x;
        int y = block[i].y;
        if (y >= 0) board[y][x] = 1;
    }
}

void clearLines() {
    for (int i = HEIGHT - 1; i >= 0; --i) {
        bool full = true;
        for (int j = 0; j < WIDTH; ++j)
            if (!board[i][j]) full = false;
        if (full) {
            for (int k = i; k > 0; --k)
                board[k] = board[k - 1];
            board[0] = vector<int>(WIDTH, 0);
            i++;
        }
    }
}

void draw() {
    system("clear");
    for (int i = 0; i < HEIGHT; ++i) {
        for (int j = 0; j < WIDTH; ++j) {
            bool printed = false;
            for (int k = 0; k < 4; ++k)
                if (block[k].x == j && block[k].y == i) {
                    cout << "#";
                    printed = true;
                    break;
                }
            if (!printed)
                cout << (board[i][j] ? "*" : ".");
        }
        cout << endl;
    }
}

void input() {
    if (kbhit()) {
        char c = getch();
        if (c == 'a') dx = -1;
        else if (c == 'd') dx = 1;
        else if (c == 'w') rotateFlag = true;
        else if (c == 'q') gameOver = true;
    }
}

void logic() {
    for (int i = 0; i < 4; ++i) {
        temp[i] = block[i];
        block[i].x += dx;
    }
    if (!check())
        for (int i = 0; i < 4; ++i)
            block[i] = temp[i];

    if (rotateFlag) {
        Point p = block[1];
        for (int i = 0; i < 4; ++i) {
            int x = block[i].y - p.y;
            int y = block[i].x - p.x;
            block[i].x = p.x - x;
            block[i].y = p.y + y;
        }
        if (!check())
            for (int i = 0; i < 4; ++i)
                block[i] = temp[i];
    }

    for (int i = 0; i < 4; ++i) {
        temp[i] = block[i];
        block[i].y += 1;
    }

    if (!check()) {
        for (int i = 0; i < 4; ++i)
            block[i] = temp[i];
        fixToBoard();
        clearLines();
        spawnShape();
        if (!check()) gameOver = true;
    }
    dx = 0;
    rotateFlag = false;
}

int main() {
    srand(time(0));
    set_conio_terminal_mode();
    spawnShape();
    while (!gameOver) {
        draw();
        input();
        logic();
        usleep(200000);  // Adjust speed here (in microseconds)
    }
    cout << "Game Over!\n";
    return 0;
}
