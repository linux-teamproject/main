#include <iostream>
#include <conio.h>
#include <windows.h>
#include <vector>
using namespace std;

void gotoxy(int x, int y);
void hideCursor();
void clearScreen(); 
int selectDifficulty();
bool askRestart();
void getMazeSizeByDifficulty(int, int&, int&);
vector<vector<int>> generateMaze(int, int, int&, int&, int&, int&);
void renderMaze(const vector<vector<int>>&, int, int, int, int);
void renderPlayer(int, int, const string&);

void clearPlayer(int x, int y) {
    gotoxy(x, y);
    cout << " ";
}

void runGame() {
    while (true) {
        int choice = selectDifficulty();
        int WIDTH, HEIGHT;
        getMazeSizeByDifficulty(choice, WIDTH, HEIGHT);

        int sx, sy, ex, ey;
        auto maze = generateMaze(WIDTH, HEIGHT, sx, sy, ex, ey);
        int px = sx, py = sy;

        clearScreen(); 
        hideCursor();
        renderMaze(maze, ex, ey, WIDTH, HEIGHT);
        renderPlayer(px, py, "@");

        while (true) {
            char ch = _getch();
            int dx = 0, dy = 0;

            if (ch == -32) {
                ch = _getch();
                if (ch == 72) dy = -1;
                else if (ch == 80) dy = 1;
                else if (ch == 75) dx = -1;
                else if (ch == 77) dx = 1;
            }

            int nx = px + dx;
            int ny = py + dy;

            if (maze[ny][nx] == 0) {
                clearPlayer(px, py);
                px = nx; py = ny;
                renderPlayer(px, py, "@");
            }

            if (px == ex && py == ey) {
                gotoxy(0, HEIGHT + 1);
                cout << "\n 탈출 성공!\n";
                break;
            }
        }

        if (!askRestart()) {
            cout << "\n게임을 종료합니다.\n";
            break;
        }

        cin.ignore();
        clearScreen();
    }
}
