// 콘솔 출력에 필요한 함수. 추후 리눅스 SDL2 전환 시 교체 예정 .
#include <windows.h>

void gotoxy(int x, int y) {
    COORD pos = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void hideCursor() {
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = FALSE;
    SetConsoleCursorInfo(console, &info);
}

// 화면 제거 (콘솔/ 리눅스 대비)
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}