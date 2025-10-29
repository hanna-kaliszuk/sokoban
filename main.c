/**
 * HANNA KALISZUK "SOKOBAN"
 *
 * Program umożliwiający grę w Sokobana, rozgrywaną na dwuwymiarowej planszy. Na planszy znajduje się postać
 * sterowana przez gracza, która może przesuwać się na pola sąsiadujące z aktualnym w pionie lub poziomie, pod
 * warunkiem, że pole to jest wolne lub znajduje się na nim skrzynia, którą można popchnąć.
 *
 * Program czyta opis stanu początkowego planszy, a następnie odczytuje i wykonuje kolejne rozkazy. Rozpoznaje
 * polecenia wydruku aktualnego stanu planszy, pchnięcia skrzyni oraz cofnięcia wcześniej wykonanego pchnięcia.
 * Ignoruje dane wprowadzone po kropce.
 *
 * Program sam ustala, jak doprowadzić postać na pole, z którego możliwe będzie pchnięcie skrzyni we wskazanym kierunku.
 *
 * Oznaczenia na planszy:
 * - wolne, niedocelowe pole
 * + wolne, docelowe pole
 * # ściana
 * @ wolne pole niedocelowe, na którym znajduje się postać
 * * wolne pole docelowe, na którym znajduje się postać
 * [a...z] pole niedocelowe, na którym znajduje się skrzynia o podanej nazwie
 * [A...Z] pole docelowe, na którym znajduje się skrzynia o nazwie odpowiadającej małej literze
 *
 * Rozkazy:
 * - Koniec wiersza (wydruk planszy)
 * - Para: mała litera + 2/4/6/8 (pchnięcie skrzyni o podanej nazwie odpowiednio w dół, lewo, prawo, górę)
 * - 0 (cofnięcie ostatniego i jeszcze nie cofniętego skutecznego pchnięcia)
 *
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_LINE_LENGTH 1024
#define DEFAULT_TAB_SIZE 10
#define DOWN 2
#define UP 8
#define LEFT 4
#define RIGHT 6
#define DIRECTIONS 4
#define DIMENSIONS 2

typedef struct {
    char** grid;
    int rows;
} Board;

/*
 * Funkcja, która wczytuje opis planszy z wejścia. Wczytuje kolejne wiersze i dynamicznie alokuje pamięć na ich
 * przechowywanie w docelowej tablicy.
 */
int load_board(Board* board) {
    board->rows = 0;
    board->grid = NULL;

    char line[MAX_LINE_LENGTH]; // bufor do przechowywania wiersza

    while (fgets(line, sizeof(line), stdin) != NULL) {
        line[strcspn(line, "\n")] = 0; // usuwamy ewentualny znak nowej linii z końca wiersza

        if (line[0] == '\0') { // jeżeli cały wiersz jest pusty to oznacza to koniec opisu planszy
            return 1;
        }

        board->rows++;

        board->grid = realloc((board)->grid, (size_t)(board)->rows * sizeof(char*));
        if (board->grid == NULL) {
            for (int i = 0; i < board->rows; ++i) {
                free(board->grid[i]);
            }
            free(board->grid);
            board->rows = 0;
            return 1;
        }

        board->grid[board->rows - 1] = malloc((size_t)strlen(line) + 1);
        if (board->grid[board->rows - 1] == NULL) {
            for (int i = 0; i < board->rows; ++i) {
                free(board->grid[i]);
            }
            free(board->grid);
            board->rows = 0;
            return 1;
        }

        strcpy(board->grid[board->rows - 1], line);
    }
    return 0;
}

void print_board(Board* board) {
    if (board == NULL) return;

    for (int i = 0; i < board->rows; i++) {
        printf("%s\n", board->grid[i]);
    }
}

/*
 * Funkcja zwalnia zaalokowaną pamięć, najpierw dla każdego wiersza, następnie dla tablicy wskazników, a na
 * końcu dla całej planszy.
 */
void free_board(Board* board) {
    if (board == NULL || board->grid == NULL) return;

    for (int i = 0; i < board->rows; i++) {
        free(board->grid[i]);
    }

    free(board->grid);
    free(board);
}

void free_board_grid(Board* board) {
    for (int j = 0; j < board->rows; j++) {
        free(board->grid[j]);
    }
    free(board->grid);
}

typedef struct {
    int row;
    int column;
} Position;

/*
 * Struktura do przechowywania pary litera-cyfra reprezentujących rozkaz pchnięcia skrzyni
 */
typedef struct {
    char letter;
    int digit;
} Pair;

/*
 * Stos do przechowywanie kolejnych wykonanych rozkazów. Na górze stosu znajduje się ostatni wykonany rozkaz.
 */
typedef struct stack {
    Pair pair;
    struct stack* next;
} TStack;

bool empty_stack(TStack* stack) {
    return (stack == NULL);
}

void init_stack(TStack** stack) {
    *stack = NULL;
}

void push_stack(TStack** stack, Pair order) {
    TStack* temp = (TStack*)malloc(sizeof(TStack));
    temp->next = *stack;
    temp->pair = order;
    *stack = temp;
}

Pair pop_stack(TStack** stack) {
    TStack* temp = *stack;
    Pair x = temp->pair;
    *stack = (*stack)->next;
    free(temp);
    return x;
}

Pair top_stack(TStack* stack) {
    return stack->pair;
}

void clear_stack(TStack** stack) {
    while (!empty_stack(*stack)) {
        pop_stack(stack);
    }
}

/*
 * Stos do przechowywania kolejnych stanów planszy po wykonanych pchnięciach. Na górze stosu znajduje się aktualny
 * stan planszy.
 */
typedef struct board_stack {
    Board* board;
    struct board_stack* next;
} TBoardStack;

bool empty_board_stack(TBoardStack* board_stack) {
    return (board_stack == NULL);
}

void init_board_stack(TBoardStack** board_stack) {
    *board_stack = NULL;
}

void push_board_stack(TBoardStack** board_stack, Board* board) {
    TBoardStack* temp = (TBoardStack*)malloc(sizeof(TBoardStack));
    temp->next = *board_stack;
    temp->board = board;
    *board_stack = temp;
}

void pop_board_stack(TBoardStack** board_stack) {
    if (*board_stack == NULL) return;

    TBoardStack* temp = *board_stack;
    Board* board = temp->board;

    if (board != NULL) {
        int num_of_rows = board->rows;

        for (int i = 0; i < num_of_rows; i++) {
            free(board->grid[i]);
        }
        free(board->grid);
        free(board);
    }
    *board_stack = (*board_stack)->next;

    free(temp);
}

Board* top_board_stack(TBoardStack* board_stack) {
    if (board_stack == NULL) return NULL;
    return board_stack->board;
}

void clear_board_stack(TBoardStack** board_stack) {
    while (!empty_board_stack(*board_stack)) {
        pop_board_stack(board_stack);
    }
}

/*
 * Struktura reprezentująca dynamiczną kolejkę, w której odpowiednio znajdują się: tablica pozycji, początek kolejki,
 * jej koniec, aktualna liczba elementów oraz jej pojemność.
 */
typedef struct {
    Position* positions;
    int front;
    int rear;
    int size;
    int capacity;
} TDQueue;

void init_queue(TDQueue* queue, int initial_capacity) {
    queue->positions = malloc((size_t)initial_capacity * sizeof(Position));
    if (queue->positions == NULL) return;

    queue->size = 0;
    queue->capacity = initial_capacity;
    queue->front = 0;
    queue->rear = -1;
}

bool empty_queue(TDQueue* queue) {
    return (queue->size == 0);
}

void enqueue(TDQueue* queue, Position pos) {
    if (queue->size == queue->capacity) {
        queue->capacity *= 2;
        queue->positions = realloc(queue->positions, (size_t)queue->capacity * sizeof(Position));

        if (queue->positions == NULL) return;
    }

    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->positions[queue->rear] = pos;
    queue->size++;
}

Position dequeue(TDQueue* queue) {
    Position front_position = queue->positions[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size--;

    return front_position;
}

void free_queue(TDQueue* queue) {
    free(queue->positions);
    queue->size = 0;
}

/*
 * Funkcja przekazuje w wyniku ile kolumn ma najdłuższy wiersz planszy.
 */
int columns(Board* board) {
    int lenght = 0;
    int columns = 0;
    int rows = board->rows;

    for (int i = 0; i < rows; i++) {
        lenght = (int)strlen(board->grid[i]);
        if (lenght > columns) columns = lenght;
    }

    return columns;
}

/*
 * Funkcja, która szuka na planszy danego elementu, po kolei sprawdzając każdy wiersz. Gdy go znajdzie, aktualizuje
 * *row oraz *column. W przypadku, gdy nie zostanie on znaleziony, ustawia ich wartości na -1.
 */
void find_element(const Board* board, char element, int* row, int* column) {
    int i = 0;
    int k = 0;
    int found = 0;

    if (board == NULL) {
        *row = -1;
        *column = -1;
        return;
    }

    while (i < board->rows && !found) { // dopóki mamy wiersze i nie znaleziono szukanego elementu
        char* current_row = board->grid[i];

        k = 0;
        while (current_row[k] != '\0' && !found) {
            if (current_row[k] == element) {
                *row = i;
                *column = k;
                found = 1;
            }
            k++;
        }
        i++;
    }

    if (!found) {
        *row = -1;
        *column = -1;
    }
}

/*
 * Funkcja, która czyta dane wejściowe i wpisuje je do dynamicznej tablicy znaków dopóki nie trafi na znak kończący
 * "term". Zapisuje wczytane dane w tablicy jako napis.
 */
char* read_dynamic_orders(char term) {
    char* orders = NULL;
    size_t size = 0;
    size_t capacity = DEFAULT_TAB_SIZE;
    char ch = (char)getchar();

    orders = malloc(capacity * sizeof(char));
    if (orders == NULL) {
        return NULL;
    }

    while (ch != term && ch != EOF) { // wczutuje dane dopóki nie trafi na znak kończący lub koniec pliku
        if (size + 1 >= capacity) {
            capacity *= 2;
            char* new_buffer = realloc(orders, capacity * sizeof(char));
            if (new_buffer == NULL) {
                free(orders);
                return NULL;
            }
            orders = new_buffer;
        }

        orders[size++] = ch;
        ch = (char)getchar();
    }

    if (size + 1 >= capacity) { // zakończenie ciągu znaków
        char* new_buffer = realloc(orders, (size + 1) * sizeof(char));
        if (new_buffer == NULL) {
            free(orders);
            return NULL;
        }
        orders = new_buffer;
    }
    orders[size] = '\0';

    return orders;
}

/*
 * Funkcja przekazuje w wyniku wskaźnik na nowo utworzoną planszę będącą kopią wprowadzonej planszy źródłowej.
 * Odpowiednio kopiuje każdy wiersz i kończy go znakiem '\0'.
 */
Board* copy_board(const Board* source) {
    if (source == NULL) return NULL;

    Board* new_board = (Board*)malloc(sizeof(Board)); // alokuje pamięć na nową tablicę
    if (new_board == NULL) return NULL;

    new_board->rows = source->rows;
    new_board->grid = (char**)malloc((size_t)source->rows * sizeof(char*));

    if (new_board->grid == NULL) {
        free(new_board);
        return NULL;
    }

    for (int i = 0; i < source->rows; i++) {
        size_t len = strlen(source->grid[i]);
        new_board->grid[i] = (char*)malloc(len + 1); // +1 dla znaku null

        if (new_board->grid[i] == NULL) {
            for (int j = 0; j < i; j++) {
                free(new_board->grid[j]);
            }
            free(new_board->grid);
            free(new_board);
            return NULL;
        }

        strncpy(new_board->grid[i], source->grid[i], len);
        new_board->grid[i][len] = '\0';
    }

    return new_board;
}

/*
 * Funkcja przekazuje w wyniku wartość true, jeżeli możliwe jest przesunięcie skrzyni we wskazanym kierunku, false w
 * przeciwnym przypadku. Sprawdza ona, czy pole docelowe znajduje się w obrębie plaszy oraz czy jest ono wolne / stoi
 * na nim gracz.
 *
 * Jako argumenty przyjmuje aktualny stan planszy, kierunek, w którym ma zostać przesunięta oraz w którym
 * wierszu i której kolumnie obecnie znajduje się skrzynia.
 *
 * char target wskazuje na pole, na którym ma stanąć skrzynia, zgodnie z kierunkiem jej ruchu.
 */
bool can_move_the_box(Board* board, int direction, int row, int column) {
    int num_of_rows = board->rows;
    int current_num_of_columns = (int)strlen(board->grid[row]);
    char target = board->grid[row][column];

    if (direction == UP) {
        if (row == 0) return false;

        int num_of_columns_above = (int)strlen(board->grid[row - 1]);
        if (column > num_of_columns_above) return false;

        target = board->grid[row - 1][column];
    }

    else if (direction == DOWN) {
        if (row >= num_of_rows - 1) return false;

        int num_of_columns_below = (int)strlen(board->grid[row + 1]);
        if (column > num_of_columns_below) return false;

        target = board->grid[row + 1][column];
    }

    else if (direction == LEFT) {
        if (column == 0) return false;
        target = board->grid[row][column - 1];
    }

    else if (direction == RIGHT) {
        if (column >= current_num_of_columns) return false;
        target = board->grid[row][column + 1];
    }

    if (target == '-' || target == '+' || target == '@' || target == '*') return true;
    return false;
}

/*
 * Funkcja, która odpowiada za przesunięcie pudełka. Jako argumenty przyjmuje aktualny stan planszy, kierunek, w
 * którym ma zostać przesunięta skrzynia, wiersz i kolumnę, w którym jest obecnie oraz jej nazwę.
 *
 * char target wskazuje na pole, na którym ma stanąć skrzynia, zgodnie z kierunkiem jej ruchu.
 *
 * Sprawdzane jest, czy skrzynia stoi na polu docelowym (jest nazwana dużą literą) oraz aktualizowana jest
 * pozycja postaci poprzez ustawienie jej na polu, na którym stała skrzynia przed pchnięciem.
 */
void move_the_box(Board* board, int direction, int row, int column, char box) {
    if (direction == UP) {
        char target = board->grid[row - 1][column];

        if (box >= 'a' && box <= 'z') {
            if (target == '+') board->grid[row - 1][column] = (char)(box - ('a' - 'A'));
            else board->grid[row - 1][column] = box;

            board->grid[row][column] = '@';
        }
        else if (box >= 'A' && box <= 'Z') {
            if (target == '+') board->grid[row - 1][column] = box;
            else board->grid[row - 1][column] = (char)(box + ('a' - 'A'));

            board->grid[row][column] = '*';
        }
    }

    else if (direction == DOWN) {
        char target = board->grid[row + 1][column];

        if (box >= 'a' && box <= 'z') {
            if (target == '+') board->grid[row + 1][column] = (char)(box - ('a' - 'A'));
            else board->grid[row + 1][column] = box;

            board->grid[row][column] = '@';
        }
        else if (box >= 'A' && box <= 'Z') {
            if (target == '+') board->grid[row + 1][column] = box;
            else board->grid[row + 1][column] = (char)(box + ('a' - 'A'));

            board->grid[row][column] = '*';
        }
    }

    else if (direction == LEFT) {
        char target = board->grid[row][column - 1];
        if (box >= 'a' && box <= 'z') {
            if (target == '+') board->grid[row][column - 1] = (char)(box - ('a' - 'A'));
            else board->grid[row][column - 1] = box;

            board->grid[row][column] = '@';
        }
        else if (box >= 'A' && box <= 'Z') {
            if (target == '+') board->grid[row][column - 1] = box;
            else board->grid[row][column - 1] = (char)(box + ('a' - 'A'));

            board->grid[row][column] = '*';
        }
    }

    else if (direction == RIGHT) {
        char target = board->grid[row][column + 1];
        if (box >= 'a' && box <= 'z') {
            if (target == '+') board->grid[row][column + 1] = (char)(box - ('a' - 'A'));
            else board->grid[row][column + 1] = box;

            board->grid[row][column] = '@';
        }
        else if (box >= 'A' && box <= 'Z') {
            if (target == '+') board->grid[row][column + 1] = box;
            else board->grid[row][column + 1] = (char)(box + ('a' - 'A'));

            board->grid[row][column] = '*';
        }
    }
}

/*
 * Funkcja przekazuje w wyniku true, gdy możliwe jest dojście gracza do wskazanego pola, tzn istnieje taka ścieżka
 * wolnych pól oznaczonych '-' lub '+'. Sprawdza to wykorzystując algorytm BFS.
 *
 * Jako argumenty przyjmuje aktualny stan planszy, startową pozycję gracza oraz pole, na które gracz powinien się
 * dostać.
 *
 * Wykorzystuje także bool **visited, która jest tablicą zapisującą odwiedzone pozycje na planszy.
 */
bool find_a_path(Board* board, Position starting_position, Position where_to) {
    int rows = board->rows;
    int cols = columns(board);
    bool path_found = false;

    bool** visited = malloc((size_t)rows * sizeof(bool*));
    if (visited == NULL) return false;;

    for (int i = 0; i < rows; i++) {
        visited[i] = malloc((size_t)cols * sizeof(bool));
        if (visited[i] == NULL) {
            for (int j = 0; j < i; j++) {
                free(visited[j]);
            }
            free(visited);
            return false;
        }

        for (int k = 0; k < cols; k++) {
            visited[i][k] = false; // na początku żadne pole nie zostało odwiedzone
        }
    }

    TDQueue queue;
    init_queue(&queue, DEFAULT_TAB_SIZE);
    enqueue(&queue, starting_position); // dodanie pozycji startowej gracza do kolejki
    visited[starting_position.row][starting_position.column] = true;

    int directions[DIRECTIONS][DIMENSIONS] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}}; // góra, dół, lewo, prawo

    while (!empty_queue(&queue) && !path_found) {
        Position current = dequeue(&queue);

        if (current.row == where_to.row && current.column == where_to.column) path_found = true;

        for (int i = 0; i < DIRECTIONS; i++) {
            int new_row = current.row + directions[i][0]; // sprawdzam każde możliwe z pol sąsiednich
            int new_column = current.column + directions[i][1];

            if (new_row >= 0 && new_row < rows && new_column >= 0 && new_column < (int)strlen(board->grid[new_row])) {
                if (!visited[new_row][new_column] && (board->grid[new_row][new_column] == '+' || board->grid[new_row][
                    new_column] == '-')) {
                    visited[new_row][new_column] = true;
                    enqueue(&queue, (Position){new_row, new_column});
                }
            }
        }
    }

    for (int i = 0; i < rows; i++) {
        free(visited[i]);
    }
    free(visited);
    free_queue(&queue);

    return path_found;
}

/*
 * Funkcja odpowiadająca za wykonanie ruchu, która jako argumenty przyjmuje aktualny stan planszy, nazwę pudełka
 * do przesunięcia oraz kierunek pchnięcia.
 *
 * Odpowiednio lokalizuje gracza i skrzynię oraz sprawdza, czy wykonanie pchnięcia jest możliwe.
 *
 * Jeżeli ruch jest wykonalny, zapisuje zagranie na planszy i przekazuje w wyniku 1. W przeciwnym wypadku przekazuje w
 * wyniku 0.
 */
int make_a_move(Board* board, char box, int direction) {
    Position player;
    Position target;

    find_element(board, '@', &player.row, &player.column);
    if (player.row == -1) find_element(board, '*', &player.row, &player.column); // jak nie ma @ to szukam *

    find_element(board, box, &target.row, &target.column); // szukam pudełka, które chcę przesunąć
    if (target.row == -1) { // nie ma skrzyni oznaczonej tą konkretną małą literką na planszy
        box = (char)(box - ('a' - 'A')); // patrzę, czy skrzynia nie jest nazwana dużą literką
        find_element(board, box, &target.row, &target.column);
        if (target.row == -1) return 0; // nie ma takiego pudełka na planszy
    }

    if (!can_move_the_box(board, direction, target.row, target.column)) return 0;

    if (can_move_the_box(board, direction, target.row, target.column)) {
        if (direction == UP) {
            if (find_a_path(board, player, (Position){target.row + 1, target.column})) {
                if (board->grid[player.row][player.column] == '@') board->grid[player.row][player.column] = '-';
                else if (board->grid[player.row][player.column] == '*') board->grid[player.row][player.column] = '+';
                move_the_box(board, UP, target.row, target.column, box);
                return 1;
            }
        }

        if (direction == DOWN) {
            if (find_a_path(board, player, (Position){target.row - 1, target.column})) {
                if (board->grid[player.row][player.column] == '@') board->grid[player.row][player.column] = '-';
                else if (board->grid[player.row][player.column] == '*') board->grid[player.row][player.column] = '+';
                move_the_box(board, DOWN, target.row, target.column, box);
                return 1;
            }
        }

        if (direction == LEFT) {
            if (find_a_path(board, player, (Position){target.row, target.column + 1})) {
                if (board->grid[player.row][player.column] == '@') board->grid[player.row][player.column] = '-';
                else if (board->grid[player.row][player.column] == '*') board->grid[player.row][player.column] = '+';
                move_the_box(board, LEFT, target.row, target.column, box);
                return 1;
            }
        }

        if (direction == RIGHT) {
            if (find_a_path(board, player, (Position){target.row, target.column - 1})) {
                if (board->grid[player.row][player.column] == '@') board->grid[player.row][player.column] = '-';
                else if (board->grid[player.row][player.column] == '*') board->grid[player.row][player.column] = '+';
                move_the_box(board, RIGHT, target.row, target.column, box);
                return 1;
            }
        }
    }
    return 0;
}

/*
 * Funkcja przyjmuje jako argumenty tablicę rozkazów, aktualny stan planszy oraz wskaźniki na początek stosu z
 * rozkazami pchnięcia skrzyń oraz na górę stosu z kolejnymi stanami planszy.
 *
 * Dzięki zmiennej board_alloceted kontrolujemy, czy jakaś pamięć została zaalokowana dla planszy, dzięki czemu
 * możliwe jest jej poprawne zwalnianie, nawet jeśli nie doszło do wspomnianej alokacji.
 *
 * Zmienna can_reverse jest odpowiednio zmieniana w zależności od tego, czy na stosie ruchów jest taki, który można
 * cofnąć.
 *
 * Pusty stos plansz oznacza, że wszystkie ruchy zostały cofnięte, w związku z czym kolejny ruch będzie wykonywany na
 * planszy początkowej.
 *
 * W przypadku, gdy rozkazem jest '0', zdejmuje ze stosu ostatni wykonany rozkaz (jeśli to możliwe), a także ostatni
 * stan planszy. Aktualnym stanem będzie nowo ustalony pierwszy element stosu.
 */
void process_orders(char* orders, Board* board, TStack* stack, TBoardStack* board_stack) {
    if(orders == NULL) return;

    int i = 0;

    bool can_reverse = false;
    bool board_allocated = false;

    while (orders[i] != '\0') {
        if (empty_board_stack(board_stack)) {
            Board* need_to_add = copy_board(board);
            push_board_stack(&board_stack, need_to_add);
        }

        if (!empty_stack(stack)) can_reverse = true;
        else if (empty_stack(stack)) can_reverse = false; // bo nie mamy ruchu do cofnięcia

        if (orders[i] == '0' && can_reverse) {
            pop_stack(&stack);
            pop_board_stack(&board_stack); // wiadomo, że nie jest pusty

            if (!board_allocated) {
                free_board_grid(board);
                board_allocated = true;
            }
            else free_board(board);

            board = copy_board(top_board_stack(board_stack));
            board_allocated = true;
            i++;
        }

        else if (orders[i] == '0' && !can_reverse) i++;

        else if (orders[i] >= 'a' && orders[i] <= 'z' && orders[i + 1] >= '2' && orders[i + 1] <= '8') {
            int move_is_possible = make_a_move(board, orders[i], orders[i + 1] - '0');
            if (move_is_possible) {
                Pair pair = {orders[i], orders[i + 1] - '0'};
                Board* after_move = copy_board(board);
                push_board_stack(&board_stack, after_move); // dodaje kopię aktualnego stanu planszy na stos
                push_stack(&stack, pair); // dodaje wykonany rozkaz na stos
            }
            i += 2;
        }

        else if (orders[i] == '\n') {
            print_board(board);
            i++;
        }
    }

    if (board_allocated) free_board(board);
    else free_board_grid(board);
    free(orders);
    clear_stack(&stack);
    clear_board_stack(&board_stack);
}

int main(void) {
    Board board;
    TBoardStack* board_stack;
    TStack* stack;

    load_board(&board);

    init_board_stack(&board_stack);
    init_stack(&stack);

    Board* start = copy_board(&board);

    print_board(start);

    push_board_stack(&board_stack, start);

    char* orders = read_dynamic_orders('.');

    process_orders(orders, &board, stack, board_stack);

    return 0;
}