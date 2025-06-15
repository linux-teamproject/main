#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <ctime>
#include <random>
#include <fstream>
const int CELL_SIZE = 32;
const int MAX_ROWS = 40;
const int MAX_COLS = 40;

int ROWS = 20, COLS = 20;
int WIDTH = CELL_SIZE * COLS;
int HEIGHT = CELL_SIZE * ROWS;

const int WALL = 1, PATH = 0, PLAYER = 2, EXIT = 3;

int maze[MAX_ROWS][MAX_COLS];
int player_x = 1, player_y = 1;
int exit_x = 0, exit_y = 0;

SDL_Texture* wallTexture = nullptr;
SDL_Texture* playerTextures[5][4] = { nullptr };
int playerDirection = 0;
int selectedCharacter = 0;

// 적 선언 (normal mode/ 2마리)
const int ENEMY_COUNT = 2;
int enemy_x[ENEMY_COUNT];
int enemy_y[ENEMY_COUNT];
int enemyDirection[ENEMY_COUNT] = { 0 };
SDL_Texture* enemyTextures[4] = { nullptr };

// 적 선언 (hard mode/ 4마리)
const int HARD_ENEMY_COUNT = 4;
int hard_enemy_x[HARD_ENEMY_COUNT];
int hard_enemy_y[HARD_ENEMY_COUNT];
int hard_enemy_dir[HARD_ENEMY_COUNT] = { 0 };
SDL_Texture* hardEnemyTextures[4] = { nullptr };

enum GameState {
    START, // 게임 시작 화면
    SELECT_CHARACTER, // 캐릭터 선택 화면
    SELECT_DIFFICULTY, // 난이도 선택 화면
    PLAYING, // 게임 진행 중
    GAME_OVER, // 게임 종료 화면
    SCORE_VIEW // 점수 화면
};

GameState gameState = START;
bool isGameWin = false; // 승패 구분을 위해 추가함
int selectedDifficulty = 0;
int selectedOption = 0;
int enemy_frame_counter = 0;
const int dx[] = { 0, 0, -1, 1 };
const int dy[] = { -1, 1, 0, 0 };
int score(0);
bool isViewingScores = false;  // 추가: 점수 조회 모드

struct ScoreEntry {
    std::string playerName;
    int score;
    std::string diff;
};

int difficulty_priority(const std::string& diff){
    if(diff=="HARD")return 0;
    else if(diff=="NORMAL")return 1;
    else if(diff=="EASY")return 2;
    else return 3;
}

// 점수 로드 함수
std::vector<ScoreEntry> load_scores(const std::string& filename) {
    std::vector<ScoreEntry> scores;
    std::ifstream file(filename);
    if (!file.is_open()) return scores;

    std::string name;
    int score;
    std::string diff;
    while (file >> name >> score>>diff) {
        scores.push_back({name,score,diff});
    }
    file.close();
    return scores;
}

// 점수 저장 함수
void save_scores(const std::vector<ScoreEntry>& scores, const std::string& filename) {
    std::ofstream file(filename);
    
    for (const auto& s : scores) {
        file << s.playerName << " " << s.score << " "<< s.diff <<"\n";
    }
    file.close();
}

// 전역 변수
std::vector<ScoreEntry> scores;
std::string currentPlayerName;
bool isEnteringName = false;
bool scoreSaved=false;
std::string nameInputBuffer;

// 시작화면에 점수 리스트 그리기
void draw_scores(SDL_Renderer* renderer, TTF_Font* font, const std::vector<ScoreEntry>& scores) {
    SDL_Color white = {255, 255, 255, 255};
    int y_start = HEIGHT / 4;
    int x_start = WIDTH / 2 - 200; // 시작 x 좌표
    int col_width[] = { 60, 120, 120, 120 }; // RANK, NAME, SCORE, MODE 너비

    // 헤더
    std::string headers[] = { "RANK", "NAME", "SCORE", "MODE" };
    for (int j = 0; j < 4; ++j) {
        SDL_Surface* surf = TTF_RenderText_Solid(font, headers[j].c_str(), white);
        SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_Rect dst = { x_start + col_width[j] * j, y_start - 40, surf->w, surf->h };
        SDL_RenderCopy(renderer, tex, NULL, &dst);
        SDL_FreeSurface(surf);
        SDL_DestroyTexture(tex);
    }

    // 점수 목록
    for (size_t i = 0; i < scores.size() && i < 10; ++i) {
        std::string rows[] = {
            std::to_string(i + 1),
            scores[i].playerName,
            std::to_string(scores[i].score),
            scores[i].diff
        };
        for (int j = 0; j < 4; ++j) {
            SDL_Surface* surf = TTF_RenderText_Solid(font, rows[j].c_str(), white);
            SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
            SDL_Rect dst = { x_start + col_width[j] * j, y_start + int(i) * 30, surf->w, surf->h };
            SDL_RenderCopy(renderer, tex, NULL, &dst);
            SDL_FreeSurface(surf);
            SDL_DestroyTexture(tex);
        }
    }


}




// 이름 입력창 그리기
void draw_name_input(SDL_Renderer* renderer, TTF_Font* font, const std::string& input) {
    SDL_Color white = {255, 255, 255, 255};
    std::string prompt = "Enter your name: " + input + "_";

    SDL_Surface* surface = TTF_RenderText_Solid(font, prompt.c_str(), white);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

    int texW = 0, texH = 0;
    SDL_QueryTexture(texture, NULL, NULL, &texW, &texH);
    SDL_Rect dst = { (WIDTH - texW) / 2, HEIGHT / 2 + 30, texW, texH };
    SDL_RenderCopy(renderer, texture, NULL, &dst);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

// 미로 생성 함수
void generate_maze() {
    srand(time(0));
    // 미로를 벽(WALL)로 초기화
    for (int y = 0; y < ROWS; y++)
        for (int x = 0; x < COLS; x++)
            maze[y][x] = WALL;

    std::vector<std::pair<int, int>> stack;
    stack.push_back({ 1, 1 });
    maze[1][1] = PATH; // 시작 위치는 PATH로 설정

    int dx[] = { 0, 0, -2, 2 };
    int dy[] = { -2, 2, 0, 0 };
    std::random_device rd;
    std::mt19937 g(rd());

    // 깊이 우선 탐색(DFS) 알고리즘으로 미로 생성
    while (!stack.empty()) {
        auto [cx, cy] = stack.back();
        stack.pop_back();

        std::vector<int> dirs = { 0, 1, 2, 3 };
        std::shuffle(dirs.begin(), dirs.end(), g);

        for (int i : dirs) {
            int nx = cx + dx[i];
            int ny = cy + dy[i];

            // 유효한 경로라면 해당 칸을 PATH로 설정하고 인접한 벽도 PATH로 변경
            if (nx > 0 && ny > 0 && nx < COLS - 1 && ny < ROWS - 1 && maze[ny][nx] == WALL) {
                maze[ny][nx] = PATH;
                maze[cy + dy[i] / 2][cx + dx[i] / 2] = PATH;
                stack.push_back({ nx, ny });
            }
        }
    }

    player_x = 1; // 플레이어 시작 위치
    player_y = 1;

    // EXIT 지점을 미로 안에 배치
    for (int y = ROWS - 2; y > 0; y--) {
        for (int x = COLS - 2; x > 0; x--) {
            if (maze[y][x] == PATH) {
                maze[y][x] = EXIT;
                exit_x = x;
                exit_y = y;
                return;
            }
        }
    }
}

// 텍스트를 화면 중앙에 출력하는 함수
void draw_text_center(SDL_Renderer* renderer, TTF_Font* font, const char* msg, int y_offset) {
    SDL_Color white = { 255, 255, 255, 255 };
    SDL_Surface* surface = TTF_RenderText_Solid(font, msg, white);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    int texW = 0, texH = 0;
    SDL_QueryTexture(texture, NULL, NULL, &texW, &texH);
    SDL_Rect dst = { (WIDTH - texW) / 2, (HEIGHT - texH) / 2 + y_offset, texW, texH };
    SDL_RenderCopy(renderer, texture, NULL, &dst);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

// 게임 시작 화면
void draw_start_screen(SDL_Renderer* renderer, TTF_Font* font) {
    
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    draw_text_center(renderer, font, "PRESS ENTER TO START", -140);
    draw_text_center(renderer,font,"PRESS 'S' TO VIEW SCORES",-70);
    draw_text_center(renderer,font,"PRESS 'Q' TO EXIT",0);
    if(isEnteringName){
        draw_name_input(renderer, font, nameInputBuffer);
    }
    SDL_RenderPresent(renderer);
    
    
}

void draw_score_view(SDL_Renderer* renderer, TTF_Font* font){
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    draw_text_center(renderer, font, "TOP SCORES", -250);
    draw_scores(renderer,font,scores);
    draw_text_center(renderer,font,"PRESS 'ESC' TO RETURN",HEIGHT/2-40);
    SDL_RenderPresent(renderer);
}

// 캐릭터 선택 화면
void draw_character_selection_screen(SDL_Renderer* renderer, TTF_Font* font) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    draw_text_center(renderer, font, "SELECT CHARACTER", -40);
    for (int i = 0; i < 5; ++i) {
        std::string text = (i == selectedCharacter ? "-> " : "   ") + std::string("CHARACTER ") + std::to_string(i + 1);
        draw_text_center(renderer, font, text.c_str(), 20 + i * 40);
    }
    SDL_RenderPresent(renderer);
}

// 난이도 선택 화면
void draw_difficulty_screen(SDL_Renderer* renderer, TTF_Font* font) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    draw_text_center(renderer, font, "SELECT DIFFICULTY", -40);
    draw_text_center(renderer, font, selectedDifficulty == 0 ? "-> EASY MODE" : "   EASY MODE", 20);
    draw_text_center(renderer, font, selectedDifficulty == 1 ? "-> NORMAL MODE" : "   NORMAL MODE", 60);
    draw_text_center(renderer, font, selectedDifficulty == 2 ? "-> HARD MODE" : "   HARD MODE", 100);
    SDL_RenderPresent(renderer);
}

// 게임 오버 화면
void draw_game_over_screen(SDL_Renderer* renderer, TTF_Font* font) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    draw_text_center(renderer, font, isGameWin ? "YOU WIN!" : "GAME OVER", -60); // 위쪽에 승패 문구
    draw_text_center(renderer, font, selectedOption == 0 ? "-> RESTART" : "   RESTART", 30);
    draw_text_center(renderer, font, selectedOption == 1 ? "-> QUIT" : "   QUIT", 70);

    // 점수 표시
    char scoreText[64];
    sprintf(scoreText, "Your score: %d", score);
    draw_text_center(renderer, font, scoreText, 0);  // 중앙에 점수 표시

    SDL_RenderPresent(renderer);
}

// 미로를 화면에 나타내는 함수
void draw_maze(SDL_Renderer* renderer,TTF_Font* font) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_Texture* goalTexture = IMG_LoadTexture(renderer, "exit.png");

    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
            SDL_Rect cell = { x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE };
            if (maze[y][x] == WALL && wallTexture) {
                SDL_RenderCopy(renderer, wallTexture, NULL, &cell); // 벽 그리기
            }
            else if (maze[y][x] == EXIT && goalTexture) {
                SDL_RenderCopy(renderer, goalTexture, NULL, &cell); // EXIT 그리기
            }
            
        }
    }

    SDL_Rect playerRect = { player_x * CELL_SIZE, player_y * CELL_SIZE, CELL_SIZE, CELL_SIZE };
    if (playerTextures[selectedCharacter][playerDirection]) {
        SDL_RenderCopy(renderer, playerTextures[selectedCharacter][playerDirection], NULL, &playerRect); // 플레이어 그리기
    }
    else {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &playerRect); // 플레이어가 없으면 기본 사각형 그리기
    }

    SDL_DestroyTexture(goalTexture);
    // 점수 출력
    char scoreText[32];
    sprintf(scoreText, "Score: %d", score);
    draw_text_center(renderer,font, scoreText, -(HEIGHT / 2) + 20);  // 상단 중앙

}

// 플레이어 움직임 처리 함수
void move_player(int dx, int dy) {
    int new_x = player_x + dx;
    int new_y = player_y + dy;

    // 방향에 따라 플레이어의 방향 설정
    if (dx == 1) playerDirection = 2;
    else if (dx == -1) playerDirection = 1;
    else if (dy == -1) playerDirection = 3;
    else if (dy == 1) playerDirection = 0;
    score++;
    // 이동할 수 있는 경로인 경우만 이동
    if (maze[new_y][new_x] == PATH || maze[new_y][new_x] == EXIT) {
        player_x = new_x;
        player_y = new_y;
    }

    

    if (maze[player_y][player_x] == EXIT) {
        isGameWin = true;           // 승리처리
        gameState = GAME_OVER;
    }
}

// normal mode 적 움직임 처리 함수
void move_enemy() {
    for (int i = 0; i < ENEMY_COUNT; ++i) {
        int prev_x = enemy_x[i];
        int prev_y = enemy_y[i];

        std::vector<int> dirs = { 0, 1, 2, 3 };
        std::random_shuffle(dirs.begin(), dirs.end());

        for (int dir : dirs) {
            int nx = enemy_x[i] + dx[dir];
            int ny = enemy_y[i] + dy[dir];

            if (maze[ny][nx] == PATH || maze[ny][nx] == EXIT) {
                enemy_x[i] = nx;
                enemy_y[i] = ny;

                if (nx > prev_x) enemyDirection[i] = 2;
                else if (nx < prev_x) enemyDirection[i] = 1;
                else if (ny > prev_y) enemyDirection[i] = 0;
                else if (ny < prev_y) enemyDirection[i] = 3;

                if (enemy_x[i] == player_x && enemy_y[i] == player_y) {
                    isGameWin = false;
                    gameState = GAME_OVER;
                }

                break;
            }
        }
    }

}

// hard mode 적 움직임 처리 함수 
void move_hard_enemy() {
    for (int i = 0; i < HARD_ENEMY_COUNT; ++i) {
        int prev_x = hard_enemy_x[i];
        int prev_y = hard_enemy_y[i];

        std::vector<int> dirs = { 0, 1, 2, 3 };
        std::random_shuffle(dirs.begin(), dirs.end());

        for (int dir : dirs) {
            int nx = hard_enemy_x[i] + dx[dir];
            int ny = hard_enemy_y[i] + dy[dir];

            if (maze[ny][nx] == PATH || maze[ny][nx] == EXIT) {
                hard_enemy_x[i] = nx;
                hard_enemy_y[i] = ny;

                if (nx > prev_x) hard_enemy_dir[i] = 2;   
                else if (nx < prev_x) hard_enemy_dir[i] = 1;
                else if (ny > prev_y) hard_enemy_dir[i] = 0; 
                else if (ny < prev_y) hard_enemy_dir[i] = 3;

                if (hard_enemy_x[i] == player_x && hard_enemy_y[i] == player_y) {
                    isGameWin = false;
                    gameState = GAME_OVER;
                }

                break;
            }
        }
    }
}


// normal/hard 모드 적 렌더링   
void draw_enemy(SDL_Renderer* renderer) {
    if (selectedDifficulty == 1) {
        for (int i = 0; i < ENEMY_COUNT; ++i) {
            if (enemy_x[i] == player_x && enemy_y[i] == player_y && gameState == PLAYING) {
                isGameWin = false;
                gameState = GAME_OVER;
            }

            SDL_Rect dest = {
                enemy_x[i] * CELL_SIZE,
                enemy_y[i] * CELL_SIZE,
                CELL_SIZE,
                CELL_SIZE
            };
            SDL_RenderCopy(renderer, enemyTextures[enemyDirection[i]], NULL, &dest);
        }
    }
    else if (selectedDifficulty == 2) {
        for (int i = 0; i < HARD_ENEMY_COUNT; ++i) {
            if (hard_enemy_x[i] == player_x && hard_enemy_y[i] == player_y && gameState == PLAYING) {
                isGameWin = false;
                gameState = GAME_OVER;
            }

            SDL_Rect dest = {
                hard_enemy_x[i] * CELL_SIZE,
                hard_enemy_y[i] * CELL_SIZE,
                CELL_SIZE,
                CELL_SIZE
            };
            SDL_RenderCopy(renderer, hardEnemyTextures[hard_enemy_dir[i]], NULL, &dest);
        }
    }
}


int main() {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    IMG_Init(IMG_INIT_PNG);

    SDL_Window* window = SDL_CreateWindow("Maze Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    TTF_Font* font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf", 24);
    if (!font) return 1;

    // 벽 텍스처
    SDL_Surface* wallSurface = IMG_Load("wall.png");
    if (wallSurface) {
        wallTexture = SDL_CreateTextureFromSurface(renderer, wallSurface);
        SDL_FreeSurface(wallSurface);
    }

    // 플레이어 텍스처 
    for (int i = 0; i < 5; ++i) {
        SDL_Surface* spriteSheet = IMG_Load(("player" + std::to_string(i + 1) + ".png").c_str());
        if (spriteSheet) {
            int frameW = spriteSheet->w / 3;
            int frameH = spriteSheet->h / 4;
            for (int dir = 0; dir < 4; dir++) {
                SDL_Surface* dirSurface = SDL_CreateRGBSurface(0, frameW, frameH, 32, 0, 0, 0, 0);
                SDL_Rect src = { 0, dir * frameH, frameW, frameH };
                SDL_BlitSurface(spriteSheet, &src, dirSurface, NULL);
                playerTextures[i][dir] = SDL_CreateTextureFromSurface(renderer, dirSurface);
                SDL_FreeSurface(dirSurface);
            }
            SDL_FreeSurface(spriteSheet);
        }
    }

    

    // normal mode 적 텍스처
    SDL_Surface* spriteSheet = IMG_Load("enemy.png");
    if (spriteSheet) {
        int frameW = spriteSheet->w / 3;
        int frameH = spriteSheet->h / 4;

        for (int dir = 0; dir < 4; ++dir) {
            SDL_Surface* dirSurface = SDL_CreateRGBSurface(0, frameW, frameH, 32, 0, 0, 0, 0);
            SDL_Rect src = { 0, dir * frameH, frameW, frameH };
            SDL_BlitSurface(spriteSheet, &src, dirSurface, NULL);
            enemyTextures[dir] = SDL_CreateTextureFromSurface(renderer, dirSurface);
            SDL_FreeSurface(dirSurface);
        }

        SDL_FreeSurface(spriteSheet);
    }

    // Hard mode 적 텍스처
    SDL_Surface* hardSpriteSheet = IMG_Load("enemy2.png");
    if (hardSpriteSheet) {
        int frameW = hardSpriteSheet->w / 3;
        int frameH = hardSpriteSheet->h / 4;

        for (int dir = 0; dir < 4; ++dir) {
            SDL_Surface* dirSurface = SDL_CreateRGBSurface(0, frameW, frameH, 32, 0, 0, 0, 0);

            SDL_Rect src = { 0, dir * frameH, frameW, frameH };
            SDL_BlitSurface(hardSpriteSheet, &src, dirSurface, NULL);
            hardEnemyTextures[dir] = SDL_CreateTextureFromSurface(renderer, dirSurface);

            SDL_FreeSurface(dirSurface);
        }

        SDL_FreeSurface(hardSpriteSheet);
    }


    bool running = true;
    SDL_Event event;

    while (running) {
        // 각 게임 상태에 맞는 화면 그리기
        if (gameState == START)
            draw_start_screen(renderer, font);
        else if (gameState == SELECT_CHARACTER)
            draw_character_selection_screen(renderer, font);
        else if (gameState == SELECT_DIFFICULTY)
            draw_difficulty_screen(renderer, font);
        else if (gameState == PLAYING) {
            draw_maze(renderer,font);
            draw_enemy(renderer);
            SDL_RenderPresent(renderer);
        }
        else if(gameState==SCORE_VIEW){
            draw_score_view(renderer,font);
        }
        else
            draw_game_over_screen(renderer, font);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_KEYDOWN) {
                if (gameState == START) {
                    if (!isEnteringName) {
                        if (event.key.keysym.sym == SDLK_RETURN) {
                            isEnteringName = true;
                            nameInputBuffer.clear();
                        }
                        else if (event.key.keysym.sym == SDLK_s) {
                            // 점수 불러오기 및 toggle
                            scores = load_scores("scoreText.txt");
                            gameState=SCORE_VIEW;    
                        }
                        else if(event.key.keysym.sym==SDLK_q){
                            running=false;
                        }
                    }
                    else {
                        if (event.key.keysym.sym == SDLK_RETURN) {
                            if (!nameInputBuffer.empty()) {
                                currentPlayerName = nameInputBuffer;
                                isEnteringName = false;
                                gameState = SELECT_CHARACTER;
                                isViewingScores=false;
                                score=0;
                                scoreSaved=false;
                            }
                        }
                        else if (event.key.keysym.sym == SDLK_BACKSPACE && !nameInputBuffer.empty()) {
                            nameInputBuffer.pop_back();
                        }
                        else {
                            char c = (char)event.key.keysym.sym;
                            if ((c >= 32 && c <= 126) && nameInputBuffer.size() < 12) {
                                nameInputBuffer.push_back(c);
                            }
                        }
                    }
                }
                else if(gameState==SCORE_VIEW){
                    if(event.key.keysym.sym==SDLK_ESCAPE){
                        gameState=START;
                    }
                }
                else if (gameState == SELECT_CHARACTER) {
                    if (event.key.keysym.sym == SDLK_UP) selectedCharacter = (selectedCharacter - 1 + 5) % 5;
                    else if (event.key.keysym.sym == SDLK_DOWN) selectedCharacter = (selectedCharacter + 1) % 5;
                    else if (event.key.keysym.sym == SDLK_RETURN) gameState = SELECT_DIFFICULTY;
                }
                else if (gameState == SELECT_DIFFICULTY) {
                    if (event.key.keysym.sym == SDLK_UP) selectedDifficulty = (selectedDifficulty - 1 + 3) % 3;
                    else if (event.key.keysym.sym == SDLK_DOWN) selectedDifficulty = (selectedDifficulty + 1) % 3;
                    else if (event.key.keysym.sym == SDLK_RETURN) {
                        // 난이도에 따라 미로 크기 조정
                        if (selectedDifficulty == 0) {
                            ROWS = 21; COLS = 21;
                        }
                        else if (selectedDifficulty == 1) {
                            ROWS = 21; COLS = 28;
                        }
                        else {
                            ROWS = 21; COLS = 35;
                        }
                        WIDTH = CELL_SIZE * COLS;
                        HEIGHT = CELL_SIZE * ROWS;
                        generate_maze();
    
                        // 적 초기화 위치 설정 (출구, 입구 근처 생성 X, PATH 위에 생성)
                        if (selectedDifficulty == 1) {
                            for (int i = 0; i < ENEMY_COUNT; ++i) {
                                bool valid;
                                do {
                                    valid = true;
                                    enemy_x[i] = rand() % COLS;
                                    enemy_y[i] = rand() % ROWS;
    
                                    // 플레이어 또는 출구와 가깝게 출력 X
                                    if (maze[enemy_y[i]][enemy_x[i]] != PATH ||
                                        (enemy_x[i] == player_x && enemy_y[i] == player_y) ||
                                        (abs(enemy_x[i] - exit_x) + abs(enemy_y[i] - exit_y) < 6)) {
                                        valid = false;
                                    }
    
                                    // 다른 적과 겹치게 출력 X
                                    for (int j = 0; j < i; ++j) {
                                        if (enemy_x[i] == enemy_x[j] && enemy_y[i] == enemy_y[j]) {
                                            valid = false;
                                            break;
                                        }
                                    }
                                } while (!valid);
                            }
                        }
    
                        // Hard 모드 적 초기화 (4마리)
                        else if (selectedDifficulty == 2) {
                            for (int i = 0; i < HARD_ENEMY_COUNT; ++i) {
                                bool valid;
                                do {
                                    valid = true;
                                    hard_enemy_x[i] = rand() % COLS;
                                    hard_enemy_y[i] = rand() % ROWS;
    
                                    // 플레이어나 출구와 너무 가까운 위치 제외
                                    if (maze[hard_enemy_y[i]][hard_enemy_x[i]] != PATH ||
                                        (hard_enemy_x[i] == player_x && hard_enemy_y[i] == player_y) ||
                                        (abs(hard_enemy_x[i] - exit_x) + abs(hard_enemy_y[i] - exit_y) < 6)) {
                                        valid = false;
                                    }
    
                                    // 서로 겹치지 않게
                                    for (int j = 0; j < i; ++j) {
                                        if (hard_enemy_x[i] == hard_enemy_x[j] && hard_enemy_y[i] == hard_enemy_y[j]) {
                                            valid = false;
                                            break;
                                        }
                                    }
                                } while (!valid);
                            }
                        }
    
    
                        // 창 크기 조정
                        SDL_SetWindowSize(window, WIDTH, HEIGHT);
    
                        gameState = PLAYING;
                    }
                
                }
                else if (gameState == PLAYING) {
                    if (event.key.keysym.sym == SDLK_LEFT) move_player(-1, 0);
                    if (event.key.keysym.sym == SDLK_RIGHT) move_player(1, 0);
                    if (event.key.keysym.sym == SDLK_UP) move_player(0, -1);
                    if (event.key.keysym.sym == SDLK_DOWN) move_player(0, 1);
                }
                else if (gameState == GAME_OVER) {
                    if (!scoreSaved && !currentPlayerName.empty()&&isGameWin) {
                        std::string diff;
                        if(selectedDifficulty==0)diff="EASY";
                        else if(selectedDifficulty==1)diff="NORMAL";
                        else diff="HARD";
                        ScoreEntry newScore = { currentPlayerName, score, diff};
                        scores = load_scores("scoreText.txt");
                        scores.push_back(newScore);
                        std::sort(scores.begin(), scores.end(), [](const ScoreEntry& a, const ScoreEntry& b) {
                            int a_p=difficulty_priority(a.diff);
                            int b_p=difficulty_priority(b.diff);
                            if(a_p!=b_p) return a_p<b_p;
                            else return a.score<b.score;
                        });
                        save_scores(scores, "scoreText.txt");
                        scoreSaved = true;
                    }
                    if (event.key.keysym.sym == SDLK_UP) selectedOption = 0;
                    else if (event.key.keysym.sym == SDLK_DOWN) selectedOption = 1;
                    else if (event.key.keysym.sym == SDLK_RETURN) {
                        if (selectedOption == 0) {
                            gameState = START;
                        }
                        else {
                            running = false;
                        }
                    }
                }
            }
        }
        // 적 이동 속도 조정 (Normal / hard 모드에서)
        enemy_frame_counter++;

        if (gameState == PLAYING && selectedDifficulty == 1) {
            if (enemy_frame_counter > 15) {  // Normal
                move_enemy();
                enemy_frame_counter = 0;
            }
        }
        else if (gameState == PLAYING && selectedDifficulty == 2) {
            if (enemy_frame_counter > 8) {  // Hard
                move_hard_enemy();
                enemy_frame_counter = 0;
            }
        }


        SDL_Delay(16);

        
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
    return 0;
}

