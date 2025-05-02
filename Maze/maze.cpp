// 난이도에 따른 미로 크기 조절
#include <vector>
#include <stack>
#include <random>
#include <utility>
using namespace std;

void getMazeSizeByDifficulty(int difficulty, int& width, int& height) {
    if (difficulty == 1) { width = 21; height = 21; }
    else if (difficulty == 2) { width = 31; height = 31; }
    else if (difficulty == 3) { width = 41; height = 41; }
    else { width = 21; height = 21; }
}

vector<vector<int>> generateMaze(int WIDTH, int HEIGHT, int& startX, int& startY, int& endX, int& endY) {
    vector<vector<int>> maze(HEIGHT, vector<int>(WIDTH, 1));
    stack<pair<int, int>> path;
    random_device rd;
    mt19937 rng(rd());
    int dx[4] = { 0, 0, -2, 2 };
    int dy[4] = { -2, 2, 0, 0 };

    startX = 1; startY = 1;
    endX = WIDTH - 2; endY = HEIGHT - 2;
    maze[startY][startX] = 0;
    maze[endY][endX] = 0;
    path.push({ startX, startY });

    while (!path.empty()) {
        int x = path.top().first;
        int y = path.top().second;

        vector<int> directions = { 0, 1, 2, 3 };
        shuffle(directions.begin(), directions.end(), rng);

        bool moved = false;
        for (int i = 0; i < 4; ++i) {
            int dir = directions[i];
            int nx = x + dx[dir];
            int ny = y + dy[dir];

            if (ny > 0 && ny < HEIGHT - 1 && nx > 0 && nx < WIDTH - 1 && maze[ny][nx] == 1) {
                maze[y + dy[dir] / 2][x + dx[dir] / 2] = 0;
                maze[ny][nx] = 0;
                path.push({ nx, ny });
                moved = true;
                break;
            }
        }

        if (!moved) path.pop();
    }

    bool connected = false;
    int adj[4][2] = { {0, -1}, {0, 1}, {-1, 0}, {1, 0} };
    for (int i = 0; i < 4; ++i) {
        int nx = endX + adj[i][0];
        int ny = endY + adj[i][1];
        if (maze[ny][nx] == 0) {
            connected = true;
            break;
        }
    }
    if (!connected) maze[endY][endX - 1] = 0;

    return maze;
}
