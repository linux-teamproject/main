// �ܼ� ��¿� �ʿ��� �Լ�. ���� ������ SDL2 ��ȯ �� ��ü ���� .
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

// ȭ�� ���� (�ܼ�/ ������ ���)
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}