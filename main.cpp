// Editor65 by FallingSky65
// Terminal Text Editor inspired by Vi/Vim/Neovim

#include <clocale>
#include <cstdio>
#include <ncurses.h>
#include <fstream>
#include <string>
#include <vector>
//#include <regex>

#define max(a, b) (a > b) ? a : b
#define min(a, b) (a < b) ? a : b
#define clamp(x, low, high) x = min(max(x, low), high)

#define KEY_ESCAPE  27

#define RELATIVE_LINENUMBERS

#define COLOR_LINENUMBERS   8
#define COLOR_FOREGROUND    9
#define COLOR_BACKGROUND    10
#define COLOR_BARFOREGRND   11
#define COLOR_BARBACKGRND   12

#define CLRPR_TEXT        1
#define CLRPR_LINENUMBERS 2
#define CLRPR_BOTTOMBAR   3
#define CLRPR_COMMAND     4

enum {
    NORMAL,
    INSERT,
    VISUAL,
    COMMAND
} mode = NORMAL;

int row, col, ch, cx, cy, ox, oy;
std::string fileName;
std::vector<std::string> fileText;
std::string command = "";


void readFile() {
    fileText.clear();
    std::ifstream file(fileName);

    if (file.is_open()) {
        std::string line;
        while (file) {
            std::getline(file, line);
            fileText.push_back(line);
        }
        file.close();
        fileText.pop_back(); // remove EOF
        fileText[fileText.size()-1] += " ";
    } else {
        fileText.push_back(" ");
    }
}

void writeFile() {
    std::ofstream file(fileName);

    if (file.is_open()) {
        for (int i = 0; i < fileText.size() - 1; i++) {
            file << fileText[i] << "\n";
        }
        file << fileText[fileText.size()-1].substr(0, fileText[fileText.size()-1].size()-1);
        file.close();
    } else {
        printf("failed to write to file\n");
    }
}

void initColorHex(short color, int hex) {
    int r = ((hex & 0xFF0000) >> 16)*1000/256;
    int g = ((hex & 0x00FF00) >> 8)*1000/256;
    int b = ((hex & 0x0000FF))*1000/256;
    init_color(color, r, g, b);
}

void initColors() {
    start_color();
    initColorHex(COLOR_LINENUMBERS, 0x86888c);
    initColorHex(COLOR_FOREGROUND, 0xe0e2ea);
    initColorHex(COLOR_BACKGROUND, 0x13161b);
    initColorHex(COLOR_BARFOREGRND, 0x2c2e34);
    initColorHex(COLOR_BARBACKGRND, 0xc4c6cd);
    init_pair(CLRPR_TEXT, COLOR_FOREGROUND, COLOR_BACKGROUND);
    init_pair(CLRPR_LINENUMBERS, COLOR_LINENUMBERS, COLOR_BACKGROUND);
    init_pair(CLRPR_BOTTOMBAR, COLOR_BARFOREGRND, COLOR_BARBACKGRND);
    init_pair(CLRPR_COMMAND, COLOR_FOREGROUND, COLOR_BACKGROUND);
}

void printLineNumbers() {
    attron(COLOR_PAIR(CLRPR_LINENUMBERS));
    for (int i = 0; i < row-2; i++) {
        if (i+oy >= fileText.size()) {
            mvprintw(i, 0, "    ~ "); continue;
        }
        #ifdef RELATIVE_LINENUMBERS
        if (i < cy - oy) {
            mvprintw(i, 0, " %4i ", (cy-oy-i)%10000);
        } else if (i > cy - oy) {
            mvprintw(i, 0, " %4i ", (i-cy+oy)%10000);
        } else {
            mvprintw(i, 0, "      ");
            mvprintw(i, 0, " %i ", (cy+1)%10000);
        }
        #else
        mvprintw(i, 0, " %4i ", (i+oy+1)%10000);
        #endif
    }
    attroff(COLOR_PAIR(CLRPR_LINENUMBERS));
}

void printText() {
    attron(COLOR_PAIR(CLRPR_TEXT));
    for (int i = 0; i < row-2; i++) {
        mvprintw(i, 6, std::string(col-6, ' ').c_str());
    }
    for (int i = 0; i < row-2 && i + oy < fileText.size(); i++) {
        if (ox < fileText[i+oy].size()) {
            std::string line = fileText[i+oy];
            //line = std::regex_replace(line, std::regex("    |\t"), "│   ");
            mvprintw(i, 6, "%.*s", col-6, &line.c_str()[ox]);
        }
    }
    attron(COLOR_PAIR(CLRPR_TEXT));
}

void printBottomBar() {
    move(row-2, 0);
    attron(COLOR_PAIR(CLRPR_BOTTOMBAR));
    mvprintw(row-2, 0, std::string(col, ' ').c_str());
    mvprintw(row-2, 0, "%s", fileName.c_str());
    attroff(COLOR_PAIR(CLRPR_BOTTOMBAR));
    move(row-1, 0);
    clrtobot();
    attron(COLOR_PAIR(CLRPR_COMMAND));
    mvprintw(row-1, 0, std::string(col, ' ').c_str());
    mvprintw(row-1, 0, "%s", command.c_str());
    attroff(COLOR_PAIR(CLRPR_COMMAND));
}

int main(int argc, char** argv)
{
    if (argc != 2) return 1;
    fileName = argv[1];
    readFile();
    
    // init screen
    setlocale(LC_ALL, "");
    initscr();
    if (has_colors() == FALSE) {
        endwin();
        printf("This terminal does not support color\n");
        return 1;
    } else {
        initColors();
    }
    keypad(stdscr, TRUE);
    noecho();
    //addch('│');
    getmaxyx(stdscr, row, col);

    printLineNumbers();
    printText();
    printBottomBar();

    cx = 0; cy = 0; ox = 0; oy = 0;
    move(cy-oy, cx-ox+6);

    // refresh screen
    refresh();


    // pause screen, get output
    //getch();
    
    while (true) {
        ch = getch();
        if (mode == NORMAL) {
            if (ch == 'i') {
                mode = INSERT;
                command = "-- INSERT --";
                printBottomBar();
            } else if (ch == ';' || ch == ':') {
                mode = COMMAND;
                command = ":";
                printBottomBar();
                move(row-1, 1);
                continue;
            } else if (ch == KEY_RESIZE) {
                getmaxyx(stdscr, row, col);
                clear();
                printLineNumbers();
                printText();
            } else if (ch == 'w' || ch == KEY_UP) {
                cy--;
            } else if (ch == 'a' || ch == KEY_LEFT) {
                cx--;
            } else if (ch == 's' || ch == KEY_DOWN) {
                cy++;
            } else if (ch == 'd' || ch == KEY_RIGHT) {
                cx++;
            } else if (ch == 'W') {
                cy--; oy--;
            } else if (ch == 'A') {
                cx--; ox--;
            } else if (ch == 'S') {
                cy++; oy++;
            } else if (ch == 'D') {
                cx++; ox++;
            }
            if (cy < 0) cy = 0;
            if (cx < 0) cx = 0;
            if (cy >= fileText.size()) cy = fileText.size() - 1;
            if (cx >= fileText[cy].size()) cx = fileText[cy].size() - 1;
            if (fileText[cy].size() == 0) cx = 0;
            if (oy < 0) oy = 0;
            if (ox < 0) ox = 0;
            if (cy < oy) oy = cy;
            if (cx < ox) ox = cx;
            if (cy > oy + row - 3) oy = cy - row + 3;
            if (cx > ox + col - 7) ox = cx - col + 7;
            printLineNumbers();
            printText();
            move(cy-oy, cx-ox+6);
            refresh();
        } else if (mode == INSERT) {
            if (cy >= fileText.size()) cy = fileText.size() - 1;
            if (cx >= fileText[cy].size()) cx = fileText[cy].size();
            if (cy < 0) cy = 0;
            if (cx < 0) cx = 0;
            if (ch == KEY_ESCAPE) {
                mode = NORMAL;
                command = "";
                printBottomBar();
            } else if (ch == 9) { // tab
                fileText[cy].insert(cx, 4, ' ');
                cx += 4;
            } else if (ch == KEY_ENTER || ch == 10) {
                std::string left = fileText[cy].substr(0, cx);
                std::string right = fileText[cy].substr(cx);
                fileText.insert(fileText.begin() + cy, left); cy++; cx = 0;
                fileText[cy] = right;
            } else if (32 <= ch && ch < 127){
                fileText[cy].insert(cx, 1, (char)ch);
                cx++;
            } else if (ch == KEY_BACKSPACE || ch == KEY_DC || ch == 127) {
                if (cx == 0 && cy > 0) {
                    cx = fileText[cy-1].size();
                    fileText[cy-1].append(fileText[cy]);
                    fileText.erase(fileText.begin() + cy);
                    cy--;
                } else if (cx > 0){
                    fileText[cy].erase(cx-1, 1); cx--;
                }
            } else if (ch == KEY_UP) {
                cy--;
            } else if (ch == KEY_LEFT) {
                cx--;
            } else if (ch == KEY_DOWN) {
                cy++;
            } else if (ch == KEY_RIGHT) {
                cx++;
            }
            if (cy < 0) cy = 0;
            if (cx < 0) cx = 0;
            if (cy >= fileText.size()) cy = fileText.size() - 1;
            if (cx >= fileText[cy].size()) cx = fileText[cy].size();
            if (oy < 0) oy = 0;
            if (ox < 0) ox = 0;
            if (cy < oy) oy = cy;
            if (cx < ox) ox = cx;
            if (cy > oy + row - 3) oy = cy - row + 3;
            if (cx > ox + col - 7) ox = cx - col + 7;
            printLineNumbers();
            printText();
            move(cy-oy, cx-ox+6);
            refresh();
        } else if (mode == COMMAND) {
            if (ch == KEY_ESCAPE) {
                mode = NORMAL;
                command = "";
                printBottomBar();
                move(cy-oy, cx-ox+6);
                continue;
            } else if (ch == KEY_ENTER || ch == 10) {
                if (command.compare(":q") == 0) {
                    break;
                } else if (command.compare(":w") == 0) {
                    writeFile();
                } else if (command.compare(":x") == 0) {
                    writeFile();
                    break;
                }
                mode = NORMAL;
                command = "";
                printBottomBar();
                move(cy-oy, cx-ox+6);
                continue;
            } else if (('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || ('0' <= ch && ch <= '9')) {
                command = command + (char)ch;
                printBottomBar();
            } else if (ch == KEY_BACKSPACE || ch == KEY_DC || ch == 127) {
                if (command.size() > 1) {
                    command.pop_back();
                    printBottomBar();
                }
            }
            move(row-1, command.size());
            refresh();
        }
    }

    // dealloc mem, end ncurses
    endwin();

    return 0;
}