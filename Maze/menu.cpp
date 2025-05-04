// ���̵� ���� & ����� & ���۹� �ȳ�
#include <iostream>
#include <string>
using namespace std;

void clearScreen();  

// ���۹� �ȳ�
void showControls() {
    clearScreen();
    cout << "- ���۹� �ȳ� - \n";
    cout << "��������������������������������������������������������\n";
    cout << "�� ����Ű : ���� �̵�\n";
    cout << "�� ����Ű : �Ʒ��� �̵�\n";
    cout << "�� ����Ű : ���� �̵�\n";
    cout << "�� ����Ű : ������ �̵�\n";
    cout << "\n�ⱸ(E)�� ã�� �̷θ� Ż���ϼ���!\n";
    cout << "��������������������������������������������������������\n";
    cout << "�ƹ�Ű�� ������ ���� �޴��� ���ư��ϴ�...";
    cin.ignore(); 
    cin.get();    
    clearScreen();
}

// ���̵� ���� �Լ�
int selectDifficulty() {
    int choice;
    while (true) {
        cout << "--- �̷�ã�� ���� --- \n";
        cout << "1. Easy (21x21)\n";
        cout << "2. Normal (31x31)\n";
        cout << "3. Hard (41x41)\n";
        cout << "4. ���۹� ����\n";
        cout << "���Ͻô� ���̵��� �������ּ��� (1~3): ";
        cin >> choice;

        if (cin.fail()) {
            cin.clear(); cin.ignore(1000, '\n');
            cout << "�Է��� �߸��Ǿ����ϴ�. �ٽ� �������ּ���.\n\n";
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
            cout << "1~4 �߿��� �������ּ���.\n\n";
        }
    }
}

// ���� ����� ���� Ȯ��
bool askRestart() {
    string input;
    while (true) {
        cout << "\n������ �ٽ� �����Ͻðڽ��ϱ�? (Y/N): ";
        cin >> input;
        for (auto& c : input) c = tolower(c);
        if (input == "y" || input == "yes") return true;
        if (input == "n" || input == "no") return false;
        cout << "�߸��� �Է��Դϴ�. 'Y' �Ǵ� 'N'���� �ٽ� �Է����ּ���.\n";
    }
}



