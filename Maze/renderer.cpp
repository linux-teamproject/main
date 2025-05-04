// 미로, 플레이어 콘솔에 출력
#include <iostream>
#include <vector>
#include <string>
using namespace std;

void gotoxy(int x, int y);

// 미로 출력 
void renderMaze(const vector<vector<int>>& maze, int ex, int ey, int WIDTH, int HEIGHT) {
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            if (x == ex && y == ey) cout << "E";
            else cout << (maze[y][x] == 1 ? "#" : " ");
        }
        cout << "\n";
    }
}

// 캐릭터 위치에 아이콘 표시 & 삭제
void renderPlayer(int x, int y, const string& icon) {
    gotoxy(x, y);
    cout << icon;
}

void clearPlayer(int x, int y) {
    gotoxy(x, y);
    cout << " ";
}

// 적 위치에 아이콘 표시 & 삭제
void renderEnemy(int x, int y) {
    renderPlayer(x, y, "X");
}

void clearEnemy(int x, int y) {
    clearPlayer(x, y);
}