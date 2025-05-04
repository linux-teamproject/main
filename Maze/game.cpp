// 전체 흐름 관리
#include <iostream>
#include <conio.h>
#include <windows.h>
#include <vector>
#include <ctime>
#include <cstdlib>
#include "enemy.cpp"
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
void clearPlayer(int x, int y);


void runGame() {
    while (true) {
        // 난이도에 따른 미로 크기 설정
        int choice = selectDifficulty();
        int width, height;
        getMazeSizeByDifficulty(choice, width, height);

        int sx, sy, ex, ey;
        auto maze = generateMaze(width, height, sx, sy, ex, ey);
        int px = sx, py = sy;

        srand((unsigned int)time(nullptr));

        // normal모드 적 소환
        Enemy* enemy = nullptr;
        if (choice == 2) {
            int attempts = 0;
            while (attempts < 100) {
                int rx = rand() % (width - 2) + 1;
                int ry = rand() % (height - 2) + 1;
                int dist = abs(rx - ex) + abs(ry - ey);
                if (maze[ry][rx] == 0 && dist >= 4) {
                    enemy = new Enemy(rx, ry);
                    break;
                }
                ++attempts;
            }
        }

        // 미로, 플레이어 출력
        clearScreen();
        hideCursor();
        renderMaze(maze, ex, ey, width, height);
        renderPlayer(px, py, "@");
        if (enemy) enemy->render();

        // 플레이어 방향키 이동
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
                px = nx;
                py = ny;
                renderPlayer(px, py, "@");
            }

            // 적 이동 및 충돌 확인
            if (enemy) {
                enemy->moveRandom(maze, width, height, ex, ey);

                if (enemy->checkCollision(px, py)) {
                    gotoxy(0, height + 2);
                    cout << "\n적에게 잡혔습니다! Game Over\n";
                    delete enemy;
                    enemy = nullptr;

                    if (!askRestart()) {
                        cout << "\n게임을 종료합니다.\n";
                        return;
                    }
                    else {
                        cin.ignore();
                        clearScreen();
                        break;
                    }
                }
            }

            // 출구 도착 확인
            if (px == ex && py == ey) {
                gotoxy(0, height + 1);
                cout << "\n탈출 성공!\n";
                break;
            }
        }

        // 게임 종료시 적 삭제
        if (enemy) {
            delete enemy;
            enemy = nullptr;
        }

        // 재시작 여부 확인
        if (!askRestart()) {
            cout << "\n게임을 종료합니다.\n";
            break;
        }

        cin.ignore();
        clearScreen();
    }
}
