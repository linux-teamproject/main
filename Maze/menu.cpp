// 난이도 선택 & 재시작 & 조작법 안내
#include <iostream>
#include <string>
using namespace std;

void clearScreen();  

// 조작법 안내
void showControls() {
    clearScreen();
    cout << "- 조작법 안내 - \n";
    cout << "────────────────────────────\n";
    cout << "↑ 방향키 : 위로 이동\n";
    cout << "↓ 방향키 : 아래로 이동\n";
    cout << "← 방향키 : 왼쪽 이동\n";
    cout << "→ 방향키 : 오른쪽 이동\n";
    cout << "\n출구(E)를 찾아 미로를 탈출하세요!\n";
    cout << "────────────────────────────\n";
    cout << "아무키나 누르면 메인 메뉴로 돌아갑니다...";
    cin.ignore(); 
    cin.get();    
    clearScreen();
}

// 난이도 선택 함수
int selectDifficulty() {
    int choice;
    while (true) {
        cout << "--- 미로찾기 게임 --- \n";
        cout << "1. Easy (21x21)\n";
        cout << "2. Normal (31x31)\n";
        cout << "3. Hard (41x41)\n";
        cout << "4. 조작법 보기\n";
        cout << "원하시는 난이도를 선택해주세요 (1~3): ";
        cin >> choice;

        if (cin.fail()) {
            cin.clear(); cin.ignore(1000, '\n');
            cout << "입력이 잘못되었습니다. 다시 선택해주세요.\n\n";
            continue;
        }

        if (choice == 4) {
            cin.ignore(); 
            showControls();
            continue;    
        }

        if (choice >= 1 && choice <= 3) {
            return choice;
        }
        else {
            cout << "1~4 중에서 선택해주세요.\n\n";
        }
    }
}

// 게임 재시작 여부 확인
bool askRestart() {
    string input;
    while (true) {
        cout << "\n게임을 다시 시작하시겠습니까? (Y/N): ";
        cin >> input;
        for (auto& c : input) c = tolower(c);
        if (input == "y" || input == "yes") return true;
        if (input == "n" || input == "no") return false;
        cout << "잘못된 입력입니다. 'Y' 또는 'N'으로 다시 입력해주세요.\n";
    }
}



