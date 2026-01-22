#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

// Implemented larger playing area
#define WIDTH 120   
#define HEIGHT 35
#define INIT_SNAKE_SIZE 5
#define TROPHY_DURATION 10

typedef struct {
    int columns, rows;
} Position;

// Game variables
Position snake[WIDTH * HEIGHT];
int snake_size;
int game_over;
int trophy_columns, trophy_rows;
int trophy_active;
time_t trophy_timer;
int win_condition;

// Updated to center the game on users screen
int start_x = 0;
int start_y = 0;

// Initialize game
void init_colors() {
    if (has_colors()) {
        start_color();
        use_default_colors();
        init_pair(1, COLOR_GREEN, -1); 
        init_pair(2, COLOR_RED, -1);   
        init_pair(3, COLOR_YELLOW, -1); 
        init_pair(4, COLOR_CYAN, -1);   
    }
}

// Updated border logic for centering
void draw_borders() {
    attron(A_BOLD);
    for (int i = 0; i <= WIDTH; i++) {
        mvaddch(start_y, start_x + i, '-');
        mvaddch(start_y + HEIGHT, start_x + i, '-');
    }
    for (int i = 0; i <= HEIGHT; i++) {
        mvaddch(start_y + i, start_x, '|');
        mvaddch(start_y + i, start_x + WIDTH, '|');
    }
    mvaddch(start_y, start_x, '+');
    mvaddch(start_y, start_x + WIDTH, '+');
    mvaddch(start_y + HEIGHT, start_x, '+');
    mvaddch(start_y + HEIGHT, start_x + WIDTH, '+');
    attroff(A_BOLD);
}

// Added a start menu
void show_menu() {
    erase(); // CHANGED: Replaced clear() with erase() to reduce flicker
    draw_borders();
    attron(COLOR_PAIR(4) | A_BOLD);
    mvprintw(start_y + HEIGHT / 2 - 2, start_x + (WIDTH / 2) - 9, "S N A K E   G A M E");
    attroff(A_BOLD);
    mvprintw(start_y + HEIGHT / 2, start_x + (WIDTH / 2) - 11, "Press [SPACE] to Start");
    mvprintw(start_y + HEIGHT / 2 + 1, start_x + (WIDTH / 2) - 8, "Press [Q] to Quit");
    attroff(COLOR_PAIR(4));
    refresh();

    int ch;
    while (1) {
        ch = getch();
        if (ch == ' ') return;
        if (ch == 'q' || ch == 'Q') { endwin(); exit(0); }
    }
}

// Logic moved from init_game to handle resetting the game for restarts

void init_game_state() {
    
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    start_y = (max_y - HEIGHT) / 2;
    start_x = (max_x - WIDTH) / 2;

    snake_size = INIT_SNAKE_SIZE;
    trophy_active = 0;
    game_over = 0;
    win_condition = (WIDTH * HEIGHT) / 5; 

    int init_columns = WIDTH / 2;
    int init_rows = HEIGHT / 2;
    for (int i = 0; i < snake_size; i++) {
        snake[i].columns = init_columns + i;
        snake[i].rows = init_rows;
    }
}

void generate_trophy() {
    int valid = 0;
    while (!valid) {
        trophy_columns = rand() % (WIDTH - 1) + 1;
        trophy_rows = rand() % (HEIGHT - 1) + 1;
        valid = 1;
        for (int i = 0; i < snake_size; i++) {
            if (snake[i].columns == trophy_columns && snake[i].rows == trophy_rows) {
                valid = 0;
                break;
            }
        }
    }
    trophy_active = 1;
    time(&trophy_timer);
}

void update_snake(int d_columns, int d_rows) {
    Position new_head = {snake[0].columns + d_columns, snake[0].rows + d_rows};

    // Fixed collision boundaries to account for the larger borders

    if (new_head.columns <= 0 || new_head.columns >= WIDTH ||
        new_head.rows <= 0 || new_head.rows >= HEIGHT) {
        game_over = 1;
        return;
    }

    for (int i = 1; i < snake_size; i++) {
        if (snake[i].columns == new_head.columns && snake[i].rows == new_head.rows) {
            game_over = 1;
            return;
        }
    }

    for (int i = snake_size - 1; i > 0; i--) {
        snake[i] = snake[i - 1];
    }
    snake[0] = new_head;

    if (new_head.columns == trophy_columns && new_head.rows == trophy_rows) {
        snake_size += 1;
        trophy_active = 0;
    }
    if (snake_size >= win_condition) game_over = 2;
}

// Added a restart / quit prompt for user once at Game Over

int ask_restart(const char* message) {
    timeout(-1); // Stop the clock to wait for user input
    attron(COLOR_PAIR(2) | A_BOLD);
    mvprintw(start_y + HEIGHT / 2, start_x + (WIDTH / 2) - (strlen(message) / 2), "%s", message);
    attroff(A_BOLD);
    mvprintw(start_y + HEIGHT / 2 + 2, start_x + (WIDTH / 2) - 16, "Press [R] Restart or [Q] Quit");
    attroff(COLOR_PAIR(2));
    refresh();

    while (1) {
        int ch = getch();
        if (ch == 'r' || ch == 'R') return 1;
        if (ch == 'q' || ch == 'Q') return 0;
    }
}

int main() {
    srand(time(NULL));
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    init_colors();

    init_game_state();
    show_menu();

    // Wrapped the main loop in a restart loop
    int keep_playing = 1;
    while (keep_playing) {
        init_game_state();
        int d_columns = -1, d_rows = 0; 

        while (!game_over) {
            timeout(80); // CHANGED: Faster response time for the larger board
            int ch = getch();
            switch (ch) {
                case KEY_UP:    if (d_rows == 0) { d_rows = -1; d_columns = 0; } break;
                case KEY_DOWN:  if (d_rows == 0) { d_rows = 1; d_columns = 0; } break;
                case KEY_LEFT:  if (d_columns == 0) { d_columns = -1; d_rows = 0; } break;
                case KEY_RIGHT: if (d_columns == 0) { d_columns = 1; d_rows = 0; } break;
            }

            if (!trophy_active) generate_trophy();
            
            time_t current_time;
            time(&current_time);
            if (difftime(current_time, trophy_timer) >= TROPHY_DURATION) trophy_active = 0;

            update_snake(d_columns, d_rows);

            erase(); 
            draw_borders();
            
            if (trophy_active) {
                attron(COLOR_PAIR(3) | A_BOLD);
                mvaddch(start_y + trophy_rows, start_x + trophy_columns, '@');
                attroff(COLOR_PAIR(3) | A_BOLD);
            }

           
            attron(COLOR_PAIR(1));
            for (int i = 0; i < snake_size; i++) {
                mvaddch(start_y + snake[i].rows, start_x + snake[i].columns, (i == 0) ? 'O' : 'o');
            }
            attroff(COLOR_PAIR(1));

            mvprintw(start_y + HEIGHT + 1, start_x + 1, "Score: %d", snake_size - INIT_SNAKE_SIZE);
            refresh();
        }

        if (game_over == 2) keep_playing = ask_restart("WINNER!");
        else keep_playing = ask_restart("GAME OVER :(");
    }

    endwin();
    return 0;
}

