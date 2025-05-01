// 미로, 플레이어 콘솔에 출력하는 함수
#include <iostream>
#include <vector>
#include <string>
using namespace std;

void gotoxy(int x, int y);

void renderMaze(const vector<vector<int>>& maze, int ex, int ey, int WIDTH, int HEIGHT) {
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            if (x == ex && y == ey) cout << "E";
            else cout << (maze[y][x] == 1 ? "#" : " ");
        }
        cout << "\n";
    }
}

void renderPlayer(int x, int y, const string& icon) {
    gotoxy(x, y);
    cout << icon;
}