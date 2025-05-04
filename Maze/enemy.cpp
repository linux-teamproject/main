// 적 구현 (normal/ hard)
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <algorithm>
using namespace std;


void renderEnemy(int x, int y);
void clearEnemy(int x, int y);

class Enemy {
public:
    int x, y;
    int prevX, prevY;

    Enemy(int startX, int startY)
        : x(startX), y(startY), prevX(-1), prevY(-1) {}

    // 적 랜덤 이동 구현
    void moveRandom(const vector<vector<int>>& maze,
        int width, int height, int goalX, int goalY) {
        static const int dx[4] = { 0, 0, -1, 1 };
        static const int dy[4] = { -1, 1, 0, 0 };
        vector<int> dirOrder = { 0, 1, 2, 3 };
        random_shuffle(dirOrder.begin(), dirOrder.end());

        for (int i = 0; i < 4; ++i) {
            int dir = dirOrder[i];
            int nx = x + dx[dir];
            int ny = y + dy[dir];

            if (nx >= 0 && nx < width && ny >= 0 && ny < height &&
                maze[ny][nx] == 0 &&
                !(nx == goalX && ny == goalY)) {
                clear();
                prevX = x;
                prevY = y;
                x = nx;
                y = ny;
                break;
            }
        }

        render();
    }

    // 충돌 확인 
    bool checkCollision(int playerX, int playerY) const {
        return x == playerX && y == playerY;
    }

    // 적 생성 및 삭제 
    void render() const {
        renderEnemy(x, y);
    }

    void clear() const {
        clearEnemy(x, y);
    }
};
