// 난이도 선택 & 재시작 여부 확인하는 함수
#include <iostream>
#include <string>
using namespace std;

int selectDifficulty() {
    int choice;
    while (true) {
        cout << "    미로찾기 게임    \n";
        cout << "1. Easy (21x21)\n";
        cout << "2. Normal (31x31)\n";
        cout << "3. Hard (41x41)\n";
        cout << "난이도를 선택하세요 (1~3): ";
        cin >> choice;

        if (cin.fail() || choice < 1 || choice > 3) {
            cin.clear(); cin.ignore(1000, '\n');
            cout << "잘못된 입력입니다. 다시 선택해주세요.\n\n";
        }
        else {
            return choice;
        }
    }
}

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
