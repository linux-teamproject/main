#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <ctime>
#include <random>
#include <fstream>
#include <cmath>

// --- 상수 및 전역 변수 정의 ---
const int CELL_SIZE = 32;
const int MAX_ROWS = 40;
const int MAX_COLS = 40;
const int INVINCIBILITY_DURATION = 2000; // 무적 시간 (2초)

int ROWS = 20, COLS = 20;
int WIDTH = CELL_SIZE * COLS;
int HEIGHT = CELL_SIZE * (ROWS + 2);

// 미로 구성 요소
const int WALL = 1, PATH = 0, PLAYER = 2, EXIT = 3;

int maze[MAX_ROWS][MAX_COLS];
int player_x = 1, player_y = 1;
int exit_x = 0, exit_y = 0;

// 게임 오브젝트 텍스처
SDL_Texture* wallTexture = nullptr;
SDL_Texture* goalTexture = nullptr;
SDL_Texture* heartTexture = nullptr; 
SDL_Texture* playerTextures[5] = { nullptr};
SDL_Texture* enemyTexture = nullptr;
SDL_Texture* hardEnemyTexture = nullptr;

// 플레이어 상태 변수
int playerDirection = 0; 
int playerFrame = 0;
int selectedCharacter = 0;
int player_lives = 2; 
bool player_is_invincible = false;
Uint32 invincibility_start_time = 0;

// 적 상태 변수
const int ENEMY_COUNT = 2;
int enemy_x[ENEMY_COUNT], enemy_y[ENEMY_COUNT], enemyDirection[ENEMY_COUNT] = { 0 };
const int HARD_ENEMY_COUNT = 4;
int hard_enemy_x[HARD_ENEMY_COUNT], hard_enemy_y[HARD_ENEMY_COUNT], hard_enemy_dir[HARD_ENEMY_COUNT] = { 0 };

// 게임 상태 관리
enum GameState { START, SELECT_CHARACTER, SELECT_DIFFICULTY, PLAYING, GAME_OVER, SCORE_VIEW };
GameState gameState = START;

// 게임 진행 관련 변수
bool isGameWin = false;
int selectedDifficulty = 0;
int selectedOption = 0;
int animation_frame_counter = 0;
const int dx[] = { 0, 0, -1, 1 };
const int dy[] = { -1, 1, 0, 0 };

// 점수 및 시간 변수
int score(0);
int remainingTime = 0;
Uint32 last_tick = 0;

// UI 리소스
TTF_Font* font_large = nullptr, *font_medium = nullptr, *font_small = nullptr;
SDL_Color color_white = { 255, 255, 255, 255 }, color_yellow = { 255, 255, 0, 255 };
SDL_Texture* titleTexture = nullptr, *characterMenuTitle = nullptr, *difficultyMenuTitle = nullptr, *rankingTitle = nullptr;
SDL_Texture* startMenuTextures[2][3] = { {nullptr} }, *difficultyOptions[2][3] = { {nullptr} };
SDL_Texture* gameOverTextures[3] = { nullptr }, *gameOverOptions[2][2] = { {nullptr} };

// 점수 시스템 관련
struct ScoreEntry { std::string playerName; int score; std::string diff; };
std::vector<ScoreEntry> scores;
std::string currentPlayerName;
bool scoreSaved = false;
std::string nameInputBuffer;

// --- 함수 프로토타입 ---
SDL_Texture* createTextTexture(SDL_Renderer* renderer, TTF_Font* font, const char* text, SDL_Color color);
void loadResources(SDL_Renderer* renderer);
void destroyResources();
void check_player_enemy_collision();


// --- 점수 관리 함수 ---

// 파일에서 점수를 불러오는 함수
std::vector<ScoreEntry> load_scores(const std::string& filename) {
    std::vector<ScoreEntry> scores_vec;
    std::ifstream file(filename);
    if (!file.is_open()) return scores_vec;
    std::string name, diff_str;
    int score_val;
    while (file >> name >> score_val >> diff_str) {
        scores_vec.push_back({ name, score_val, diff_str });
    }
    file.close();
    return scores_vec;
}

// 파일에 점수를 저장하는 함수
void save_scores(const std::vector<ScoreEntry>& scores_vec, const std::string& filename) {
    std::ofstream file(filename);
    for (const auto& s : scores_vec) {
        file << s.playerName << " " << s.score << " " << s.diff << "\n";
    }
    file.close();
}


// --- 핵심 게임 로직 함수 ---

// DFS 알고리즘으로 미로를 절차적으로 생성하는 함수
void generate_maze() {
    srand(time(0));
    for (int y = 0; y < ROWS; y++) for (int x = 0; x < COLS; x++) maze[y][x] = WALL;
    std::vector<std::pair<int, int>> stack;
    stack.push_back({ 1, 1 });
    maze[1][1] = PATH;
    std::random_device rd;
    std::mt19937 g(rd());
    int l_dx[] = { 0, 0, -2, 2 }, l_dy[] = { -2, 2, 0, 0 };
    while (!stack.empty()) {
        auto [cx, cy] = stack.back();
        stack.pop_back();
        std::vector<int> dirs = { 0, 1, 2, 3 };
        std::shuffle(dirs.begin(), dirs.end(), g);
        for (int i : dirs) {
            int nx = cx + l_dx[i], ny = cy + l_dy[i];
            if (nx > 0 && ny > 0 && nx < COLS - 1 && ny < ROWS - 1 && maze[ny][nx] == WALL) {
                maze[ny][nx] = PATH;
                maze[cy + l_dy[i] / 2][cx + l_dx[i] / 2] = PATH;
                stack.push_back({ nx, ny });
            }
        }
    }
    player_x = 1; player_y = 1;
    for (int y = ROWS - 2; y > 0; y--) for (int x = COLS - 2; x > 0; x--) if (maze[y][x] == PATH) {
        maze[y][x] = EXIT;
        exit_x = x; exit_y = y;
        return;
    }
}

// 플레이어의 움직임을 처리하는 함수
void move_player(int l_dx, int l_dy) {
    int new_x = player_x + l_dx;
    int new_y = player_y + l_dy;
    playerFrame++;

    if (l_dx == 1) playerDirection = 2;
    else if (l_dx == -1) playerDirection = 1;
    else if (l_dy == -1) playerDirection = 3;
    else if (l_dy == 1) playerDirection = 0;
    if (maze[new_y][new_x] == PATH || maze[new_y][new_x] == EXIT) {
        player_x = new_x;
        player_y = new_y;
    }
    if (maze[player_y][player_x] == EXIT) {
        isGameWin = true;
        gameState = GAME_OVER;
    }
}

// 적의 움직임을 처리하는 함수
void move_enemy(bool isHard) {
    int count = isHard ? HARD_ENEMY_COUNT : ENEMY_COUNT;
    int* p_enemy_x = isHard ? hard_enemy_x : enemy_x;
    int* p_enemy_y = isHard ? hard_enemy_y : enemy_y;
    int* p_enemy_dir = isHard ? hard_enemy_dir : enemyDirection;
    for (int i = 0; i < count; ++i) {
        int prev_x = p_enemy_x[i], prev_y = p_enemy_y[i];
        std::vector<int> dirs = { 0, 1, 2, 3 };
        std::random_shuffle(dirs.begin(), dirs.end());
        for (int dir : dirs) {
            int nx = p_enemy_x[i] + dx[dir];
            int ny = p_enemy_y[i] + dy[dir];
            if (maze[ny][nx] == PATH || maze[ny][nx] == EXIT) {
                p_enemy_x[i] = nx; p_enemy_y[i] = ny;
                if (nx > prev_x) p_enemy_dir[i] = 2; else if (nx < prev_x) p_enemy_dir[i] = 1;
                else if (ny > prev_y) p_enemy_dir[i] = 0; else if (ny < prev_y) p_enemy_dir[i] = 3;
                break;
            }
        }
    }
}

// 플레이어와 적의 충돌을 감지하고 처리하는 함수 (무적 시스템 포함)
void check_player_enemy_collision() {
    if (player_is_invincible || selectedDifficulty == 0) return;
    
    bool isHard = (selectedDifficulty == 2);
    int count = isHard ? HARD_ENEMY_COUNT : ENEMY_COUNT;
    int* p_enemy_x = isHard ? hard_enemy_x : enemy_x;
    int* p_enemy_y = isHard ? hard_enemy_y : enemy_y;

    for (int i = 0; i < count; i++) {
        if (p_enemy_x[i] == player_x && p_enemy_y[i] == player_y) {
            player_lives--;
            player_is_invincible = true;
            invincibility_start_time = SDL_GetTicks();
            if (player_lives <= 0) {
                score = 0;
                isGameWin = false;
                gameState = GAME_OVER;
            }
            return; // 한 번에 한 번만 피격되도록 즉시 반환
        }
    }
}

// --- 렌더링 관련 함수 ---

// 이름 입력 UI를 그리는 함수
void draw_name_input(SDL_Renderer* renderer, int y_pos) {
    std::string prompt = "NAME: " + nameInputBuffer + "_";
    SDL_Texture* tex = createTextTexture(renderer, font_medium, prompt.c_str(), color_white);
    int w, h;
    SDL_QueryTexture(tex, NULL, NULL, &w, &h);
    SDL_Rect dst = { (WIDTH - w) / 2, y_pos, w, h };
    SDL_RenderCopy(renderer, tex, NULL, &dst);
    SDL_DestroyTexture(tex);
}

// 시작 화면을 그리는 함수
void draw_start_screen(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 10, 20, 40, 255);
    SDL_RenderClear(renderer);
    int w, h;
    SDL_QueryTexture(titleTexture, NULL, NULL, &w, &h);
    SDL_Rect titleDst = { (WIDTH - w) / 2, HEIGHT / 4, w, h };
    SDL_RenderCopy(renderer, titleTexture, NULL, &titleDst);
    int y_start = titleDst.y + h + 80;
    for (int i = 0; i < 3; ++i) {
        SDL_Texture* tex = startMenuTextures[i == selectedOption][i];
        SDL_QueryTexture(tex, NULL, NULL, &w, &h);
        SDL_Rect dst = { (WIDTH - w) / 2, y_start + i * 50, w, h };
        SDL_RenderCopy(renderer, tex, NULL, &dst);
    }
    SDL_RenderPresent(renderer);
}

// 점수판 화면을 그리는 함수
void draw_score_view(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 10, 20, 40, 255);
    SDL_RenderClear(renderer);
    int w, h;
    SDL_QueryTexture(rankingTitle, NULL, NULL, &w, &h);
    SDL_Rect dst = { (WIDTH - w) / 2, 50, w, h };
    SDL_RenderCopy(renderer, rankingTitle, NULL, &dst);
    const int table_width = 520;
    int y_start = HEIGHT / 4 + 40;
    int col_width[] = { 80, 180, 160, 100 };
    int initial_x_start = (WIDTH - table_width) / 2;
    std::string headers[] = { "RANK", "NAME", "SCORE", "MODE" };
    int current_header_x = initial_x_start;
    for (int j = 0; j < 4; ++j) {
        SDL_Texture* header_tex = createTextTexture(renderer, font_medium, headers[j].c_str(), color_yellow);
        SDL_QueryTexture(header_tex, NULL, NULL, &w, &h);
        SDL_Rect header_dst = { current_header_x, y_start - 50, w, h };
        SDL_RenderCopy(renderer, header_tex, NULL, &header_dst);
        SDL_DestroyTexture(header_tex);
        current_header_x += col_width[j];
    }
    for (size_t i = 0; i < scores.size() && i < 10; ++i) {
        int current_x = initial_x_start;
        std::string rows[] = { std::to_string(i + 1), scores[i].playerName, std::to_string(scores[i].score), scores[i].diff };
        for (int j = 0; j < 4; ++j) {
            SDL_Texture* row_tex = createTextTexture(renderer, font_small, rows[j].c_str(), color_white);
            SDL_QueryTexture(row_tex, NULL, NULL, &w, &h);
            SDL_Rect row_dst = { current_x, y_start + (int)i * 35, w, h };
            SDL_RenderCopy(renderer, row_tex, NULL, &row_dst);
            SDL_DestroyTexture(row_tex);
            current_x += col_width[j];
        }
    }
    SDL_Texture* tex = createTextTexture(renderer, font_small, "ESC to Return", color_white);
    SDL_QueryTexture(tex, NULL, NULL, &w, &h);
    dst = { (WIDTH - w) / 2, HEIGHT - h - 30, w, h };
    SDL_RenderCopy(renderer, tex, NULL, &dst);
    SDL_DestroyTexture(tex);
    SDL_RenderPresent(renderer);
}

// 캐릭터 선택 화면을 그리는 함수
void draw_character_selection_screen(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 10, 20, 40, 255);
    SDL_RenderClear(renderer);
    int w, h;
    SDL_QueryTexture(characterMenuTitle, NULL, NULL, &w, &h);
    SDL_Rect titleDst = { (WIDTH - w) / 2, HEIGHT / 6, w, h };
    SDL_RenderCopy(renderer, characterMenuTitle, NULL, &titleDst);
    int sprite_size = 64;
    int total_width = 5 * sprite_size + 4 * 20;
    int x_start = (WIDTH - total_width) / 2;
    int y_pos = HEIGHT / 2 - sprite_size / 2;
    for (int i = 0; i < 5; ++i) {
        int x_pos_i = x_start + i * (sprite_size + 20);
        SDL_Rect dst = { x_pos_i, y_pos, sprite_size, sprite_size };
        int sheet_w, sheet_h;
        SDL_QueryTexture(playerTextures[i], NULL, NULL, &sheet_w, &sheet_h);
        SDL_Rect src = {0, 0, sheet_w/3, sheet_h/4};
        SDL_RenderCopy(renderer, playerTextures[i], &src, &dst);
        if (i == selectedCharacter) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            SDL_Rect highlightBox = { x_pos_i - 5, y_pos - 5, sprite_size + 10, sprite_size + 10 };
            SDL_RenderDrawRect(renderer, &highlightBox);
        }
    }
    draw_name_input(renderer, y_pos + sprite_size + 40);
    SDL_RenderPresent(renderer);
}

// 난이도 선택 화면을 그리는 함수
void draw_difficulty_screen(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 10, 20, 40, 255);
    SDL_RenderClear(renderer);
    int w, h;
    SDL_QueryTexture(difficultyMenuTitle, NULL, NULL, &w, &h);
    SDL_Rect titleDst = { (WIDTH - w) / 2, HEIGHT / 4, w, h };
    SDL_RenderCopy(renderer, difficultyMenuTitle, NULL, &titleDst);
    int y_start = titleDst.y + h + 80;
    for (int i = 0; i < 3; ++i) {
        SDL_Texture* tex = difficultyOptions[i == selectedDifficulty][i];
        SDL_QueryTexture(tex, NULL, NULL, &w, &h);
        SDL_Rect dst = { (WIDTH - w) / 2, y_start + i * 50, w, h };
        SDL_RenderCopy(renderer, tex, NULL, &dst);
    }
    SDL_RenderPresent(renderer);
}

// 게임 종료 화면을 그리는 함수
void draw_game_over_screen(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 10, 20, 40, 255);
    SDL_RenderClear(renderer);
    SDL_Texture* resultTexture = isGameWin ? gameOverTextures[0] : (remainingTime <= 0 ? gameOverTextures[2] : gameOverTextures[1]);
    int w, h;
    SDL_QueryTexture(resultTexture, NULL, NULL, &w, &h);
    SDL_Rect resultDst = { (WIDTH - w) / 2, HEIGHT / 4, w, h };
    SDL_RenderCopy(renderer, resultTexture, NULL, &resultDst);
    char scoreText[64];
    sprintf(scoreText, "Final Score: %d", score);
    SDL_Texture* scoreTexture = createTextTexture(renderer, font_medium, scoreText, color_white);
    SDL_QueryTexture(scoreTexture, NULL, NULL, &w, &h);
    SDL_Rect scoreDst = { (WIDTH - w) / 2, resultDst.y + h + 30, w, h };
    SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreDst);
    SDL_DestroyTexture(scoreTexture);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 100);
    int line_y = scoreDst.y + h + 40;
    SDL_RenderDrawLine(renderer, WIDTH / 4, line_y, WIDTH * 3 / 4, line_y);
    int y_start = line_y + 40;
    for (int i = 0; i < 2; i++) {
        SDL_Texture* tex = gameOverOptions[i == selectedOption][i];
        SDL_QueryTexture(tex, NULL, NULL, &w, &h);
        SDL_Rect dst = { (WIDTH - w) / 2, y_start + i * 50, w, h };
        SDL_RenderCopy(renderer, tex, NULL, &dst);
    }
    SDL_RenderPresent(renderer);
}

// 인게임 화면(미로, HUD, 캐릭터 등)을 그리는 함수
void draw_maze(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    const int y_offset = 2 * CELL_SIZE;

    // 바닥 그리기
    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
            SDL_Rect floor_cell = { x * CELL_SIZE, y * CELL_SIZE + y_offset, CELL_SIZE, CELL_SIZE };
            if ((x + y) % 2 == 0) SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
            else SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
            SDL_RenderFillRect(renderer, &floor_cell);
        }
    }

    // 벽 및 도착지점 그리기
    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
            if (maze[y][x] == WALL) {
                double dist = sqrt(pow(x - player_x, 2) + pow(y - player_y, 2));
                Uint8 brightness = 255 / (1 + dist * 0.2);
                if (brightness < 30) brightness = 30;
                SDL_SetTextureColorMod(wallTexture, brightness, brightness, brightness);
                SDL_Rect cell = { x * CELL_SIZE, y * CELL_SIZE + y_offset, CELL_SIZE, CELL_SIZE };
                SDL_RenderCopy(renderer, wallTexture, NULL, &cell);
            } else if (maze[y][x] == EXIT) {
                 SDL_Rect cell = { x * CELL_SIZE, y * CELL_SIZE + y_offset, CELL_SIZE, CELL_SIZE };
                 SDL_SetTextureColorMod(goalTexture, 255, 255, 255); // 도착지점은 항상 밝게
                 SDL_RenderCopy(renderer, goalTexture, NULL, &cell);
            }
        }
    }
    SDL_SetTextureColorMod(wallTexture, 255, 255, 255); // 다음 프레임을 위해 텍스처 색상 복원

    // 플레이어 그리기 (무적 시 깜빡임)
    if (!player_is_invincible || (SDL_GetTicks() / 100) % 2 == 0) {
        int current_frame = (playerFrame / 6) % 3;
        SDL_Texture* pTex = playerTextures[selectedCharacter];
        int sheet_w, sheet_h;
        SDL_QueryTexture(pTex, NULL, NULL, &sheet_w, &sheet_h);
        SDL_Rect playerSrcRect = { current_frame * (sheet_w / 3), playerDirection * (sheet_h / 4), sheet_w / 3, sheet_h / 4 };
        SDL_Rect playerDestRect = { player_x * CELL_SIZE, player_y * CELL_SIZE + y_offset, CELL_SIZE, CELL_SIZE };
        SDL_RenderCopy(renderer, pTex, &playerSrcRect, &playerDestRect);
    }
    
    // HUD 그리기
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_Rect hud_panel = { 0, 0, WIDTH, y_offset };
    SDL_SetRenderDrawColor(renderer, 20, 20, 30, 200);
    SDL_RenderFillRect(renderer, &hud_panel);
    SDL_SetRenderDrawColor(renderer, 80, 80, 100, 255);
    SDL_RenderDrawRect(renderer, &hud_panel);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    char scoreText[32];
    sprintf(scoreText, "SCORE: %d", score);
    SDL_Texture* scoreTexture = createTextTexture(renderer, font_medium, scoreText, color_white);
    int scoreW, scoreH;
    SDL_QueryTexture(scoreTexture, NULL, NULL, &scoreW, &scoreH);
    SDL_Rect scoreDst = { 15, (y_offset - scoreH) / 2, scoreW, scoreH };
    SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreDst);
    SDL_DestroyTexture(scoreTexture);

    // 하트 UI 그리기
    int heart_size = 28;
    for (int i = 0; i < player_lives; i++) {
        SDL_Rect heart_dst = { (WIDTH / 2) - (2 * heart_size / 2) + (i * (heart_size + 5)), (y_offset - heart_size) / 2, heart_size, heart_size };
        SDL_RenderCopy(renderer, heartTexture, NULL, &heart_dst);
    }

    char timerText[32];
    sprintf(timerText, "TIME: %d:%02d", remainingTime / 60, remainingTime % 60);
    SDL_Texture* timerTexture = createTextTexture(renderer, font_medium, timerText, color_white);
    int timerW, timerH;
    SDL_QueryTexture(timerTexture, NULL, NULL, &timerW, &timerH);
    SDL_Rect timerDst = { WIDTH - timerW - 15, (y_offset - timerH) / 2, timerW, timerH };
    SDL_RenderCopy(renderer, timerTexture, NULL, &timerDst);
    SDL_DestroyTexture(timerTexture);
}

// 적을 그리는 함수
void draw_enemy(SDL_Renderer* renderer) {
    if (selectedDifficulty == 0) return;
    const int y_offset = 2 * CELL_SIZE;
    bool isHard = (selectedDifficulty == 2);
    int count = isHard ? HARD_ENEMY_COUNT : ENEMY_COUNT;
    int* p_enemy_x = isHard ? hard_enemy_x : enemy_x;
    int* p_enemy_y = isHard ? hard_enemy_y : enemy_y;
    int* p_enemy_dir = isHard ? hard_enemy_dir : enemyDirection;
    SDL_Texture* p_tex = isHard ? hardEnemyTexture : enemyTexture;
    int sheet_w, sheet_h;
    SDL_QueryTexture(p_tex, NULL, NULL, &sheet_w, &sheet_h);
    for (int i = 0; i < count; ++i) {
        SDL_Rect dest = { p_enemy_x[i] * CELL_SIZE, p_enemy_y[i] * CELL_SIZE + y_offset, CELL_SIZE, CELL_SIZE };
        SDL_Rect src = {0, p_enemy_dir[i] * (sheet_h/4), sheet_w/3, sheet_h/4};
        SDL_RenderCopy(renderer, p_tex, &src, &dest);
    }
}


// --- 메인 함수 ---
int main(int argc, char* argv[]) {
    // 초기화
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    IMG_Init(IMG_INIT_PNG);
    SDL_Window* window = SDL_CreateWindow("MAZE GAME", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    loadResources(renderer);

    bool running = true;
    SDL_Event event;

    // 메인 게임 루프
    while (running) {
        animation_frame_counter++;
        if (gameState == PLAYING) {
            // 무적 시간 처리
            if (player_is_invincible && SDL_GetTicks() - invincibility_start_time > INVINCIBILITY_DURATION) {
                player_is_invincible = false;
            }
            // 시간 및 점수 감소 처리
            Uint32 current_tick = SDL_GetTicks();
            if (current_tick - last_tick >= 1000) {
                if (remainingTime > 0) {
                    remainingTime--;
                    if (selectedDifficulty == 0) score -= 50; else if (selectedDifficulty == 1) score -= 45; else score -= 40;
                }
                last_tick = current_tick;
                if (remainingTime <= 0) {
                    score = 0; isGameWin = false; gameState = GAME_OVER;
                }
            }
            // 적 이동 및 충돌 감지
            if (selectedDifficulty == 1 && (animation_frame_counter % 20 == 0)) { move_enemy(false); check_player_enemy_collision(); }
            else if (selectedDifficulty == 2 && (animation_frame_counter % 12 == 0)) { move_enemy(true); check_player_enemy_collision(); }
        }

        // 이벤트 처리
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_KEYDOWN) {
                switch (gameState) {
                    case START:
                        if (event.key.keysym.sym == SDLK_RETURN) {
                            if (selectedOption == 0) { nameInputBuffer.clear(); gameState = SELECT_CHARACTER; }
                            else if (selectedOption == 1) { 
                                scores = load_scores("scoreText.txt"); 
                                std::sort(scores.begin(), scores.end(), [](const ScoreEntry& a, const ScoreEntry& b) { return a.score > b.score; });
                                gameState = SCORE_VIEW; 
                            }
                            else if (selectedOption == 2) { running = false; }
                        }
                        else if (event.key.keysym.sym == SDLK_UP) selectedOption = (selectedOption - 1 + 3) % 3;
                        else if (event.key.keysym.sym == SDLK_DOWN) selectedOption = (selectedOption + 1) % 3;
                        break;
                    case SCORE_VIEW:
                        if (event.key.keysym.sym == SDLK_ESCAPE) gameState = START;
                        break;
                    case SELECT_CHARACTER:
                        if (event.key.keysym.sym == SDLK_RETURN && !nameInputBuffer.empty()) {
                            currentPlayerName = nameInputBuffer; scoreSaved = false; gameState = SELECT_DIFFICULTY;
                        } else if (event.key.keysym.sym == SDLK_BACKSPACE && !nameInputBuffer.empty()) {
                            nameInputBuffer.pop_back();
                        } else if (((event.key.keysym.sym >= SDLK_a && event.key.keysym.sym <= SDLK_z) || (event.key.keysym.sym >= SDLK_0 && event.key.keysym.sym <= SDLK_9)) && nameInputBuffer.size() < 12) {
                            nameInputBuffer += (char)event.key.keysym.sym;
                        } else if (event.key.keysym.sym == SDLK_LEFT) {
                            selectedCharacter = (selectedCharacter - 1 + 5) % 5;
                        } else if (event.key.keysym.sym == SDLK_RIGHT) {
                            selectedCharacter = (selectedCharacter + 1) % 5;
                        }
                        break;
                    case SELECT_DIFFICULTY:
                        if (event.key.keysym.sym == SDLK_UP) selectedDifficulty = (selectedDifficulty - 1 + 3) % 3;
                        else if (event.key.keysym.sym == SDLK_DOWN) selectedDifficulty = (selectedDifficulty + 1) % 3;
                        else if (event.key.keysym.sym == SDLK_RETURN) {
                            if (selectedDifficulty == 0) { ROWS = 21; COLS = 21; remainingTime = 60; score = 3000; }
                            else if (selectedDifficulty == 1) { ROWS = 21; COLS = 28; remainingTime = 120; score = 7000; }
                            else { ROWS = 21; COLS = 35; remainingTime = 180; score = 10000; }
                            player_lives = 2; player_is_invincible = false;
                            WIDTH = CELL_SIZE * COLS; HEIGHT = CELL_SIZE * (ROWS + 2);
                            SDL_SetWindowSize(window, WIDTH, HEIGHT);
                            generate_maze(); last_tick = SDL_GetTicks();
                            if (selectedDifficulty > 0) {
                                int count = (selectedDifficulty == 2) ? HARD_ENEMY_COUNT : ENEMY_COUNT;
                                int* p_x = (selectedDifficulty == 2) ? hard_enemy_x : enemy_x;
                                int* p_y = (selectedDifficulty == 2) ? hard_enemy_y : enemy_y;
                                for (int i = 0; i < count; ++i) {
                                    bool valid;
                                    do {
                                        valid = true;
                                        p_x[i] = rand() % COLS; p_y[i] = rand() % ROWS;
                                        if (maze[p_y[i]][p_x[i]] != PATH || (abs(p_x[i] - player_x) + abs(p_y[i] - player_y) < 12) || (abs(p_x[i] - exit_x) + abs(p_y[i] - exit_y) < 12)) valid = false;
                                        for (int j = 0; j < i; ++j) if (p_x[i] == p_x[j] && p_y[i] == p_y[j]) { valid = false; break; }
                                    } while (!valid);
                                }
                            }
                            gameState = PLAYING;
                        }
                        break;
                    case PLAYING:
                        if (event.key.keysym.sym == SDLK_LEFT) move_player(-1, 0);
                        else if (event.key.keysym.sym == SDLK_RIGHT) move_player(1, 0);
                        else if (event.key.keysym.sym == SDLK_UP) move_player(0, -1);
                        else if (event.key.keysym.sym == SDLK_DOWN) move_player(0, 1);
                        check_player_enemy_collision();
                        break;
                    case GAME_OVER:
                        if (!scoreSaved && !currentPlayerName.empty() && isGameWin) {
                            std::string diff_str = (selectedDifficulty == 0) ? "EASY" : (selectedDifficulty == 1) ? "NORMAL" : "HARD";
                            scores = load_scores("scoreText.txt");
                            scores.push_back({ currentPlayerName, score, diff_str });
                            std::sort(scores.begin(), scores.end(), [](const ScoreEntry& a, const ScoreEntry& b) { return a.score > b.score; });
                            save_scores(scores, "scoreText.txt");
                            scoreSaved = true;
                        }
                        if (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_DOWN) selectedOption = 1 - selectedOption;
                        else if (event.key.keysym.sym == SDLK_RETURN) {
                            if (selectedOption == 0) gameState = START; else running = false;
                            selectedOption = 0;
                        }
                        break;
                }
            }
        }

        // 현재 상태에 맞는 화면 그리기
        switch (gameState) {
            case START: draw_start_screen(renderer); break;
            case SCORE_VIEW: draw_score_view(renderer); break;
            case SELECT_CHARACTER: draw_character_selection_screen(renderer); break;
            case SELECT_DIFFICULTY: draw_difficulty_screen(renderer); break;
            case GAME_OVER: draw_game_over_screen(renderer); break;
            case PLAYING: draw_maze(renderer); draw_enemy(renderer); SDL_RenderPresent(renderer); break;
        }
        SDL_Delay(16);
    }

    destroyResources();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit(); TTF_Quit(); SDL_Quit();
    return 0;
}

SDL_Texture* createTextTexture(SDL_Renderer* renderer, TTF_Font* font, const char* text, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

void loadResources(SDL_Renderer* renderer) {
    font_large = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf", 48);
    font_medium = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf", 28);
    font_small = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf", 20);
    wallTexture = IMG_LoadTexture(renderer, "wall.png");
    goalTexture = IMG_LoadTexture(renderer, "exit.png");
    heartTexture = IMG_LoadTexture(renderer, "heart.png");
    for (int i = 0; i < 5; ++i) {
        SDL_Surface* spriteSheet = IMG_Load(("player" + std::to_string(i + 1) + ".png").c_str());
        if (spriteSheet) {
            SDL_SetColorKey(spriteSheet, SDL_TRUE, SDL_MapRGB(spriteSheet->format, 0, 0, 0));
            if (playerTextures[i]) SDL_DestroyTexture(playerTextures[i]);
            playerTextures[i] = SDL_CreateTextureFromSurface(renderer, spriteSheet);
            SDL_FreeSurface(spriteSheet);
        }
    }
    SDL_Surface* enemy_sheet_normal = IMG_Load("enemy.png");
    if (enemy_sheet_normal) {
        SDL_SetColorKey(enemy_sheet_normal, SDL_TRUE, SDL_MapRGB(enemy_sheet_normal->format, 0, 0, 0));
        enemyTexture = SDL_CreateTextureFromSurface(renderer, enemy_sheet_normal);
        SDL_FreeSurface(enemy_sheet_normal);
    }
    SDL_Surface* enemy_sheet_hard = IMG_Load("enemy2.png");
    if (enemy_sheet_hard) {
        SDL_SetColorKey(enemy_sheet_hard, SDL_TRUE, SDL_MapRGB(enemy_sheet_hard->format, 0, 0, 0));
        hardEnemyTexture = SDL_CreateTextureFromSurface(renderer, enemy_sheet_hard);
        SDL_FreeSurface(enemy_sheet_hard);
    }
    titleTexture = createTextTexture(renderer, font_large, "MAZE GAME", color_white);
    rankingTitle = createTextTexture(renderer, font_large, "RANKING", color_white);
    const char* start_texts[] = { "START GAME", "VIEW SCORES", "QUIT" };
    for (int i = 0; i < 3; ++i) {
        startMenuTextures[0][i] = createTextTexture(renderer, font_medium, start_texts[i], color_white);
        startMenuTextures[1][i] = createTextTexture(renderer, font_medium, start_texts[i], color_yellow);
    }
    characterMenuTitle = createTextTexture(renderer, font_large, "SELECT CHARACTER", color_white);
    difficultyMenuTitle = createTextTexture(renderer, font_large, "SELECT DIFFICULTY", color_white);
    const char* diff_texts[] = { "EASY", "NORMAL", "HARD" };
    for (int i = 0; i < 3; ++i) {
        difficultyOptions[0][i] = createTextTexture(renderer, font_medium, diff_texts[i], color_white);
        difficultyOptions[1][i] = createTextTexture(renderer, font_medium, diff_texts[i], color_yellow);
    }
    gameOverTextures[0] = createTextTexture(renderer, font_large, "YOU WIN!", color_yellow);
    gameOverTextures[1] = createTextTexture(renderer, font_large, "GAME OVER", color_white);
    gameOverTextures[2] = createTextTexture(renderer, font_large, "TIME OVER", color_white);
    const char* gameover_opts[] = { "RESTART", "QUIT" };
    for (int i = 0; i < 2; ++i) {
        gameOverOptions[0][i] = createTextTexture(renderer, font_medium, gameover_opts[i], color_white);
        gameOverOptions[1][i] = createTextTexture(renderer, font_medium, gameover_opts[i], color_yellow);
    }
}

void destroyResources() {
    SDL_DestroyTexture(wallTexture);
    SDL_DestroyTexture(goalTexture);
    SDL_DestroyTexture(heartTexture);
    SDL_DestroyTexture(enemyTexture);
    SDL_DestroyTexture(hardEnemyTexture);
    SDL_DestroyTexture(titleTexture);
    SDL_DestroyTexture(rankingTitle);
    SDL_DestroyTexture(characterMenuTitle);
    SDL_DestroyTexture(difficultyMenuTitle);
    for (int i = 0; i < 5; ++i) if (playerTextures[i]) SDL_DestroyTexture(playerTextures[i]);
    for (int i = 0; i < 3; ++i) {
        SDL_DestroyTexture(startMenuTextures[0][i]);
        SDL_DestroyTexture(startMenuTextures[1][i]);
        SDL_DestroyTexture(difficultyOptions[0][i]);
        SDL_DestroyTexture(difficultyOptions[1][i]);
        SDL_DestroyTexture(gameOverTextures[i]);
    }
    for (int i = 0; i < 2; ++i) {
        SDL_DestroyTexture(gameOverOptions[0][i]);
        SDL_DestroyTexture(gameOverOptions[1][i]);
    }
    TTF_CloseFont(font_large);
    TTF_CloseFont(font_medium);
    TTF_CloseFont(font_small);
}
