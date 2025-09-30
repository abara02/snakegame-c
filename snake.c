// Alexis Baranauskas, Edmond Cani 
// CS 355
// Snake Game Project

#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#define WIDTH 100
#define HEIGHT 25
#define INIT_SNAKE_SIZE 4
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
// Initialize game
void init_game() {
initscr(); // Start ncurses
cbreak();
noecho();
curs_set(0); // Hide cursor
keypad(stdscr, TRUE); // Enable arrow keys
timeout(300); // Initial delay
// Set colors
if (has_colors()) {
start_color();
use_default_colors();
init_pair(1, COLOR_GREEN, -1); // Snake
init_pair(2, COLOR_RED, -1); // Game Over message
init_pair(3, COLOR_RED, -1); // Apple trophy
}
// Center & initialize the snake
int init_columns = WIDTH / 2 - INIT_SNAKE_SIZE / 2;
int init_rows = HEIGHT / 2;
for (int i = 0; i < INIT_SNAKE_SIZE; i++) {


snake[i].columns = init_columns + i;
snake[i].rows = init_rows;
}
snake_size = INIT_SNAKE_SIZE;
trophy_active = 0;
int total_area = (WIDTH - 1) * (HEIGHT - 1);
win_condition = total_area / 2;
game_over = 0;
}
// Draw borders
void draw_borders() {
attron(A_BOLD);
for (int i = 0; i <= WIDTH; i++) {
mvprintw(0, i, "-");
mvprintw(HEIGHT, i, "-");
}
for (int i = 0; i <= HEIGHT; i++) {
mvprintw(i, 0, "|");
mvprintw(i, WIDTH, "|");
}
// Add corners
mvprintw(0, 0, "+");
mvprintw(0, WIDTH, "+");
mvprintw(HEIGHT, 0, "+");
mvprintw(HEIGHT, WIDTH, "+");
attroff(A_BOLD);
}
// Draw snake
void draw_snake() {
attron(COLOR_PAIR(1));
for (int i = 0; i < snake_size; i++) {
mvprintw(snake[i].rows, snake[i].columns, "o");
}
attroff(COLOR_PAIR(1));
}
// Draw apple (trophy)
void draw_trophy() {
attron(COLOR_PAIR(3) | A_BOLD);
mvprintw(trophy_rows, trophy_columns, "@");
attroff(COLOR_PAIR(3) | A_BOLD);
}


// Generate apple
void generate_trophy() {
trophy_columns = rand() % (WIDTH - 2) + 1;
trophy_rows = rand() % (HEIGHT - 2) + 1;
trophy_active = 1;
time(&trophy_timer);
}
// Move and update snake state
void update_snake(int d_columns, int d_rows) {
Position new_head = {snake[0].columns + d_columns, snake[0].rows + d_rows};
// Border collision
if (new_head.columns == 0 || new_head.columns == WIDTH ||
new_head.rows == 0 || new_head.rows == HEIGHT) {
game_over = 1;
return;
}
// Self collision
for (int i = 1; i < snake_size; i++) {
if (snake[i].columns == new_head.columns && snake[i].rows == new_head.rows) {
game_over = 1;
return;
}
}
// Move snake
for (int i = snake_size - 1; i > 0; i--) {
snake[i] = snake[i - 1];
}
snake[0] = new_head;
// Eat apple
if (new_head.columns == trophy_columns && new_head.rows == trophy_rows) {
snake_size += 1;
trophy_active = 0;
}
// Win
if (snake_size >= win_condition) {
game_over = 2;
}
}
// Display end game message
void display_game_over_message(const char* message) {
clear();
draw_borders();


int message_length = strlen(message);
int center_x = WIDTH / 2;
int center_y = HEIGHT / 2;
attron(COLOR_PAIR(2) | A_BOLD);
mvprintw(center_y, center_x - (message_length / 2), "%s", message);
attroff(COLOR_PAIR(2) | A_BOLD);
refresh();
napms(5000);
getch();
}
// Main loop
int main() {
srand(time(NULL));
int random_direction = rand() % 4;
int d_columns = 0, d_rows = 0;
switch (random_direction) {
case 0: d_columns = 1; break;
case 1: d_columns = -1; break;
case 2: d_rows = 1; break;
case 3: d_rows = -1; break;
}
init_game();
while (!game_over) {
int ch = getch();
// Changing snake direction
switch (ch) {
case KEY_UP: if (d_rows == 0) { d_rows = -1; d_columns = 0; } break;
case KEY_DOWN: if (d_rows == 0) { d_rows = 1; d_columns = 0; } break;
case KEY_LEFT: if (d_columns == 0) { d_columns = -1; d_rows = 0; } break;
case KEY_RIGHT: if (d_columns == 0) { d_columns = 1; d_rows = 0; } break;
}
clear();
draw_borders();
timeout(130 - (snake_size * 1.3)); // dynamic speed
if (!trophy_active) {
generate_trophy();
}
draw_trophy();



update_snake(d_columns, d_rows);
draw_snake();
mvprintw(HEIGHT + 1, 1, "Score: %d", snake_size - INIT_SNAKE_SIZE);
refresh();
// Game end
if (game_over == 2) {
display_game_over_message("WINNER!");
break;
}
if (game_over == 1) {
display_game_over_message("GAME OVER :(");
break;
}
// Apple lasts for 10 seconds
time_t current_time;
time(&current_time);
double diff = difftime(current_time, trophy_timer);
if (trophy_active && diff >= TROPHY_DURATION) {
trophy_active = 0;
}
}
endwin(); // cleanup ncurses
return 0;
}
