#include <iostream>
#include <vector>
#include <stack>
#include <utility>
#include <cstdlib>
#include <ctime>
#include <random>
#include <algorithm>
#include <conio.h>
#include <windows.h>

// 벡터
int dx[4] = { 0, 0, -2, 2 };
int dy[4] = { -2, 2, 0, 0 };

// 플레이어 캐릭터 이동 (이전 위치를 지우고 새 위치에 출력하면서 이동 표현)
void gotoxy(int x, int y) {
    COORD pos = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

// 이동시 깜빡임 제거 
void hideCursor() {
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = FALSE;
    SetConsoleCursorInfo(console, &info);
}

// 난이도 선택
int selectDifficulty() {
    int choice;
    while (true) {
        std::cout << "    미로찾기 게임    \n";
        std::cout << "1. Easy (21x21)\n";
        std::cout << "2. Normal (31x31)\n";
        std::cout << "3. Hard (41x41)\n";
        std::cout << "난이도를 선택하세요 (1~3): ";
        std::cin >> choice;

        if (std::cin.fail() || choice < 1 || choice > 3) {
            std::cin.clear();
            std::cin.ignore(1000, '\n');
            std::cout << "잘못된 입력입니다. 다시 선택해주세요.\n\n";
        }
        else {
            return choice;
        }
    }
}

// 게임 재시작
bool askRestart() {
    std::string input;
    while (true) {
        std::cout << "\n게임을 다시 시작하시겠습니까? (Y/N): ";
        std::cin >> input;
        for (auto& c : input) c = std::tolower(c);
        if (input == "y" || input == "yes") return true;
        if (input == "n" || input == "no") return false;
        std::cout << "잘못된 입력입니다. 'Y' 또는 'N'으로 다시 입력해주세요.\n";
    }
}

// random_device, mt19937을 통해 미로 랜덤 생성 구현
std::vector<std::vector<int>> generateMaze(int WIDTH, int HEIGHT, int& startX, int& startY, int& endX, int& endY) {
    std::vector<std::vector<int>> maze(HEIGHT, std::vector<int>(WIDTH, 1));
    std::stack<std::pair<int, int>> path;
    std::random_device rd;
    std::mt19937 rng(rd());

    startX = 1; startY = 1;
    endX = WIDTH - 2; endY = HEIGHT - 2;

    maze[startY][startX] = 0;
    maze[endY][endX] = 0;
    path.push({ startX, startY });

    while (!path.empty()) {
        int x = path.top().first;
        int y = path.top().second;

        std::vector<int> directions = { 0, 1, 2, 3 };
        std::shuffle(directions.begin(), directions.end(), rng);

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

    // 출구 설정
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

// 미로 출력 (벽, 출구)
void printMaze(const std::vector<std::vector<int>>& maze, int ex, int ey, int WIDTH, int HEIGHT) {
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            if (x == ex && y == ey) std::cout << "E";
            else std::cout << (maze[y][x] == 1 ? "#" : " ");
        }
        std::cout << "\n";
    }
}

// 난이도 선택에 따른 미로 크기 설정
int main() {
    while (true) {
        int choice = selectDifficulty();
        int WIDTH = 21, HEIGHT = 21;
        if (choice == 2) { WIDTH = 31; HEIGHT = 31; }
        else if (choice == 3) { WIDTH = 41; HEIGHT = 41; }

        // 플레이어 조작 및 모양
        int startX, startY, endX, endY;
        auto maze = generateMaze(WIDTH, HEIGHT, startX, startY, endX, endY);
        int px = startX, py = startY;
        std::string playerChar = "@"; 

        system("cls");
        hideCursor();
        printMaze(maze, endX, endY, WIDTH, HEIGHT);
        gotoxy(px, py); std::cout << playerChar;

        while (true) {
            char ch = _getch();
            int nx = px, ny = py;

            if (ch == -32) {
                ch = _getch();
                switch (ch) {
                case 72: ny--; break; // ↑
                case 80: ny++; break; // ↓
                case 75: nx--; break; // ←
                case 77: nx++; break; // →
                }
            }

            // 출구 도착
            if (maze[ny][nx] == 0) {
                gotoxy(px, py); std::cout << " ";
                px = nx; py = ny;
                gotoxy(px, py); std::cout << playerChar;
            }

            if (px == endX && py == endY) {
                gotoxy(0, HEIGHT + 1);
                std::cout << "\n 탈출 성공! \n";
                break;
            }
        }

        // 재시작 여부 확인
        if (!askRestart()) {
            std::cout << "\n게임을 종료합니다.\n";
            break;
        }

        std::cin.ignore();
        system("cls");
    }

    return 0;
}
