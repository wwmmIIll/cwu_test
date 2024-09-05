#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     // Mac에서 sleep 함수 사용
#include <termios.h>    // 키보드 입력 처리
#include <fcntl.h>      // 파일 디스크립터 제어
#include <time.h>
#include <ncurses.h>    // 화면 제어를 위한 라이브러리

#define WIDTH 20
#define HEIGHT 10

// Map 구조체: 맵을 관리하고 그리기 기능을 담당
typedef struct {
    char layout[HEIGHT][WIDTH + 1];
} Map;

void drawMap(Map* map) {
    clear(); // 화면 지우기 (ncurses 함수)
    for (int i = 0; i < HEIGHT; i++) {
        printw("%s\n", map->layout[i]);  // 맵 출력 (ncurses 함수)
    }
    refresh(); // 화면 갱신 (ncurses 함수)
}

// Pacman 구조체: 팩맨의 위치 및 이동 기능
typedef struct {
    int x;
    int y;
    int score;
} Pacman;

void initPacman(Pacman* pacman, int startX, int startY) {
    pacman->x = startX;
    pacman->y = startY;
    pacman->score = 0;
}

void movePacman(Pacman* pacman, Map* map, int dx, int dy) {
    int newX = pacman->x + dx;
    int newY = pacman->y + dy;

    // 충돌 감지 및 점수 증가
    if (map->layout[newY][newX] == ' ') {
        map->layout[pacman->y][pacman->x] = ' ';
        pacman->x = newX;
        pacman->y = newY;
        map->layout[pacman->y][pacman->x] = 'P';
        pacman->score += 10; // 점수 증가
    }
}

// Ghost 구조체: 고스트의 위치 및 이동 기능
typedef struct {
    int x;
    int y;
} Ghost;

void initGhost(Ghost* ghost, int startX, int startY) {
    ghost->x = startX;
    ghost->y = startY;
}

void moveGhost(Ghost* ghost, Map* map, Pacman* pacman) {
    // 간단한 AI: 팩맨을 향해 이동 (랜덤으로 팩맨을 쫓는 방식)
    int dx = 0, dy = 0;
    if (rand() % 2) { // 50% 확률로 X축 방향 이동
        dx = (ghost->x < pacman->x) ? 1 : -1;
    } else { // 50% 확률로 Y축 방향 이동
        dy = (ghost->y < pacman->y) ? 1 : -1;
    }

    int newX = ghost->x + dx;
    int newY = ghost->y + dy;

    // 고스트의 이동 가능 여부 검사 (벽을 통과하지 않음)
    if (map->layout[newY][newX] == ' ') {
        map->layout[ghost->y][ghost->x] = ' ';
        ghost->x = newX;
        ghost->y = newY;
        map->layout[ghost->y][ghost->x] = 'G';
    }
}

// Game 구조체: 전체 게임 상태 관리
typedef struct {
    Map map;
    Pacman pacman;
    Ghost ghost;
    int isGameOver;
} Game;

void initGame(Game* game) {
    // 맵 초기화
    char initialMap[HEIGHT][WIDTH + 1] = {
        "###################",
        "#                 #",
        "#  ####  #######  #",
        "#  #        #     #",
        "#  ###  #######   #",
        "#                 #",
        "#  #######  ####  #",
        "#                 #",
        "#                 #",
        "###################"
    };
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            game->map.layout[i][j] = initialMap[i][j];
        }
        game->map.layout[i][WIDTH] = '\0'; // 문자열 끝 처리
    }

    // 팩맨과 고스트 초기화
    initPacman(&game->pacman, 1, 1);
    initGhost(&game->ghost, 10, 5);

    // 초기 위치 설정
    game->map.layout[game->pacman.y][game->pacman.x] = 'P';
    game->map.layout[game->ghost.y][game->ghost.x] = 'G';

    // 게임 상태 초기화
    game->isGameOver = 0;
}

void updateGame(Game* game) {
    drawMap(&game->map);
    printw("Score: %d\n", game->pacman.score);  // 점수 출력

    // 팩맨과 고스트 충돌 감지
    if (game->pacman.x == game->ghost.x && game->pacman.y == game->ghost.y) {
        game->isGameOver = 1;
        printw("Game Over! Final Score: %d\n", game->pacman.score);
    }
}

int kbhit(void) {
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}

int main() {
    Game game;
    initGame(&game);

    srand(time(NULL)); // 랜덤 시드 설정

    // ncurses 초기화
    initscr();
    cbreak();
    noecho();
    curs_set(0); // 커서 숨김

    while (!game.isGameOver) {
        updateGame(&game);

        // 키 입력 처리
        if (kbhit()) {
            char input = getchar();
            switch (input) {
                case 'w': movePacman(&game.pacman, &game.map, 0, -1); break;
                case 's': movePacman(&game.pacman, &game.map, 0, 1);  break;
                case 'a': movePacman(&game.pacman, &game.map, -1, 0); break;
                case 'd': movePacman(&game.pacman, &game.map, 1, 0);  break;
                case 'q': game.isGameOver = 1; break; // 'q'를 누르면 게임 종료
            }
        }

        // 고스트 움직임
        moveGhost(&game.ghost, &game.map, &game.pacman);

        // 간단한 딜레이
        usleep(200000); // 200ms
    }

    // ncurses 종료
    endwin();

    return 0;
}
