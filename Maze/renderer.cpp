// �̷�, �÷��̾� �ֿܼ� ���
#include <iostream>
#include <vector>
#include <string>
using namespace std;

void gotoxy(int x, int y);

// �̷� ��� 
void renderMaze(const vector<vector<int>>& maze, int ex, int ey, int WIDTH, int HEIGHT) {
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            if (x == ex && y == ey) cout << "E";
            else cout << (maze[y][x] == 1 ? "#" : " ");
        }
        cout << "\n";
    }
}

// ĳ���� ��ġ�� ������ ǥ�� & ����
void renderPlayer(int x, int y, const string& icon) {
    gotoxy(x, y);
    cout << icon;
}

void clearPlayer(int x, int y) {
    gotoxy(x, y);
    cout << " ";
}

// �� ��ġ�� ������ ǥ�� & ����
void renderEnemy(int x, int y) {
    renderPlayer(x, y, "X");
}

void clearEnemy(int x, int y) {
    clearPlayer(x, y);
}