// ��ü �帧 ����
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
        // ���̵��� ���� �̷� ũ�� ����
        int choice = selectDifficulty();
        int width, height;
        getMazeSizeByDifficulty(choice, width, height);

        int sx, sy, ex, ey;
        auto maze = generateMaze(width, height, sx, sy, ex, ey);
        int px = sx, py = sy;

        srand((unsigned int)time(nullptr));

        // normal��� �� ��ȯ
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

        // �̷�, �÷��̾� ���
        clearScreen();
        hideCursor();
        renderMaze(maze, ex, ey, width, height);
        renderPlayer(px, py, "@");
        if (enemy) enemy->render();

        // �÷��̾� ����Ű �̵�
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

            // �� �̵� �� �浹 Ȯ��
            if (enemy) {
                enemy->moveRandom(maze, width, height, ex, ey);

                if (enemy->checkCollision(px, py)) {
                    gotoxy(0, height + 2);
                    cout << "\n������ �������ϴ�! Game Over\n";
                    delete enemy;
                    enemy = nullptr;

                    if (!askRestart()) {
                        cout << "\n������ �����մϴ�.\n";
                        return;
                    }
                    else {
                        cin.ignore();
                        clearScreen();
                        break;
                    }
                }
            }

            // �ⱸ ���� Ȯ��
            if (px == ex && py == ey) {
                gotoxy(0, height + 1);
                cout << "\nŻ�� ����!\n";
                break;
            }
        }

        // ���� ����� �� ����
        if (enemy) {
            delete enemy;
            enemy = nullptr;
        }

        // ����� ���� Ȯ��
        if (!askRestart()) {
            cout << "\n������ �����մϴ�.\n";
            break;
        }

        cin.ignore();
        clearScreen();
    }
}
