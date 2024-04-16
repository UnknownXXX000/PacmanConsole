// PacmanConsole.cpp: ���������� ����� ����� ��� ����������.
//

#include "PacmanConsole.h"

using namespace std::chrono;


bool game_exit = false;

struct vector2di {
    int x = 0;
    int y = 0;
};

void show_map(const vector2di& apple, const int& eaten_apples);
void move_snake(vector2di& snake_head, vector2di& vector_step, std::vector<vector2di>& snake_tail, vector2di& apple, vector2di rnd, int& eaten_apples);

int main()
{
    // ����������� ����� ��� ����������� ���������
    system("chcp 1251");
    // �������������� ����� curses
    initscr();
    // ������ ������
    curs_set(0);
    // ��������� ������� �����
    start_color();
    // ������ ������ 1,2,3,4,5
    init_pair(1, COLOR_WHITE, COLOR_BLUE);
    init_pair(2, COLOR_MAGENTA, COLOR_BLUE);
    init_pair(3, COLOR_GREEN, COLOR_BLUE);
    init_pair(4, COLOR_RED, COLOR_BLUE);
    init_pair(5, COLOR_YELLOW, COLOR_BLACK);
    //���� ����, ����� 1
    bkgd(COLOR_PAIR(1));
    // ��������� ���������� ������ ������
    vector2di snake_head{ 10,10 };
    // ����������� � ��� ����������� ������
    vector2di vector_step{ 1,0 };
    // ���������� ���������� ������ 
    vector2di apple{ 15,15 };
    // ��������� ���������� ������
    vector2di rnd_apple;
    // ������������ ������ ��������� ������ ���� 
    std::vector<vector2di> snake_tail;
    // ������� ����� ������
    int frame_rate = 100;
    // ���������� ��������� �����
    int eaten_apples = 0;

    // **** ������ ������ ���������� ��������� ����� ****
    // ������ ���������� �������
    long long seed = system_clock::now().time_since_epoch().count();
    // ������ ���������� ��������� �����
    std::default_random_engine rnd(static_cast<unsigned int>(seed));
    // ��������� ��������� ��������� ��������� ������
    std::uniform_int_distribution<int> apple_x(10, 97);
    std::uniform_int_distribution<int> apple_y(5, 22);
    // **** ����� ������ ���������� ��������� ����� ****

    // ��������� ������������ ����������� ������� � ����� ������ ������� ��� ���������� �������   
    keypad(stdscr, true);
    // ��������� ����������� ������� ������ �� ������
    noecho();

    // **** ������� ���� ****
    while (!game_exit) {
        // ����������� ��������� �������� ���������� ��������� ��������� ������
        rnd_apple.x = apple_x(rnd);
        rnd_apple.y = apple_y(rnd);
        // ������ ������� ����
        show_map(apple, eaten_apples);
        // ���������� ������ �� �������� ����
        move_snake(snake_head, vector_step, snake_tail, apple, rnd_apple, eaten_apples);
        // ������� ������ �������� �����
        timeout(frame_rate);

        // **** ���������� ������� � ����� ****
        // �������� ������� �������
        switch (getch()) {
        case KEY_UP:
            if (vector_step.y == 0) {

                vector_step.y = -1;
                vector_step.x = 0;
                frame_rate = 170;
            }
            break;
        case KEY_DOWN:
            if (vector_step.y == 0) {

                vector_step.y = 1;
                vector_step.x = 0;
                frame_rate = 170;
            }
            break;
        case KEY_LEFT:
            if (vector_step.x == 0) {

                vector_step.x = -1;
                vector_step.y = 0;
                frame_rate = 100;
            }
            break;
        case KEY_RIGHT:
            if (vector_step.x == 0) {

                vector_step.x = 1;
                vector_step.y = 0;
                frame_rate = 100;
            }
            break;
        case 'q':
            game_exit = true;
            break;
        default:
            break;
        }
        // ****  ����� ���������� ������� � ����� **** 
    }
    // **** ����� �������� ����� ****
    endwin();
    return 0;
}

void show_map(const vector2di& apple, const int& eaten_apples)
{

    // ������� ������
    clear();
    // ����������� �������
    move(2, 55);
    // ������������� �������� ��� ������ ������� � ����� �������� ����
    attrset(A_DIM | COLOR_PAIR(1));
    // ������� ����� � �������
    printw("������\t\t");
    attrset(A_BOLD | COLOR_PAIR(5));
    printw(" ���������� ��������� ����� < ");
    // �������� ������������� ���������� ��������� � ���������
    std::string s_eaten_apples = std::to_string(eaten_apples);
    // ������� �������� ����������
    printw(s_eaten_apples.c_str());
    printw(" > ");
    attrset(A_DIM | COLOR_PAIR(1));
    // ������ ������� ���� 
    for (int y = 4; y < 28; y++) {

        for (int x = 5; x < 112; x++) {

            if (y == 4 || y == 27 || x == 5 || x == 111) {

                move(y, x);
                printw("*");
            }
        }
    }
    // ������ ������
    move(apple.y, apple.x);
    attrset(A_BOLD | COLOR_PAIR(2));
    printw("@");
}

void move_snake(vector2di& snake_head, vector2di& vector_step, std::vector<vector2di>& snake_tail,
    vector2di& apple, vector2di rnd, int& eaten_apples)
{

    // ������������� ���� ������
    attrset(A_BOLD | COLOR_PAIR(3));

    // ����  ������ � �������, ������ �����
    if (!snake_tail.empty()) {

        for (auto const& mov : snake_tail) {

            move(mov.y, mov.x);

            printw("#");
        }
    }
    // �������� ���������� ������ ������
    snake_head.x += vector_step.x;
    snake_head.y += vector_step.y;
    // ���������� ������ � ���������� ������ ������
    move(snake_head.y, snake_head.x);
    // ��������� ������ � ������������� ����������� �������
    auto s = static_cast<char>(winch(stdscr));
    // *** ���� ������ ����������� � ������� ��� ��������� �������� ����
    //  ### ������� ������� ���� ###
    if (s == '*' || s == '#') {

        attrset(A_BOLD | COLOR_PAIR(4));
        move(13, 55);
        printw("����� ����");
        move(14, 42);
        printw("����� - < q >  ������ ������ - < n >");
        // *** ���� ������ �������� ���� ***
        do {
            if (getch() == 'q') {
                // ����� �� ����
                game_exit = true;
                return;
            }
            if (getch() == 'n')
            {
                // ������� ����� ��������� ���������� ����������
                snake_head = { 10,10 };
                vector_step = { 1,0 };
                snake_tail.clear();
                apple = { 15,15 };
                eaten_apples = 0;
                return;
            }
        } while (true);
        // *** ����� ����� ������ �������� ���� ***
    }
    // ### ����� �������� ���� ###

        //  ���� ������ ������� ������
    if (s == '@') {
        // ����������� ���������� ��������� �����
        eaten_apples++;
        // ��������� ����� ������
        snake_tail.push_back({ snake_head.x, snake_head.y });
        // ������ ������ ������
        printw("$");

        do {
            // ����� ����� ���������� ������
            apple.x = rnd.x; apple.y = rnd.y;
            move(apple.y, apple.x);
            auto s = static_cast<char>(winch(stdscr));
            // ��������� ���� ���� ���������� ������ ��������� � ������� ������
        } while (s == '#');
    }
    // ���� ������ ������������� � ��������� ����
    else {
        // ������ ������ ������
        printw("$");
        // ��������� ���������� ������ ����
        if (!snake_tail.empty()) {
            snake_tail.erase(snake_tail.begin());
            snake_tail.push_back({ snake_head.x, snake_head.y });
        }
    }
};