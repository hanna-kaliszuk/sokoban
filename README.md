# SOKOBAN GAME
A console-based implementation of the classic Sokoban puzzle game written in C. Realized as a part of the Introduction to Informatics course realized in the winter semester of 2024/25 at the University of Warsaw. 

## DESCRIPTION
Unlike typical implementation, this version automalically moves the player to a valid position from which a push can be made - the used only issues cush commands, not step-by-step movements. The program supports:
- reading an initial board state form input
- executing push and undo commands
- printing the board state after each command
- maintaining full undo history for previous pushes

### BOARD REPRESENTATION
Each cell on the board is represented by a single ASCII character:

| Symbol | Meaning (English) |  
|:------:|:------------------|
| `-` | empty cell (not a goal) | 
| `+` | empty cell (a goal) | 
| `#` | wall | 
| `@` | player on a normal cell | 
| `*` | player on a goal cell | 
| `[a...z]` | player on a normal cell | 
| `[A...Z]` | crate on a goal cell | 

**RULES**
- exactly one player is present on the board
- each crate has a unique letter name

### COMMANDS
Comands are read from standard input, following the initial board description. The sequence of commands ends with a single dot `.` on a line by itself.

| Symbol | Meaning (English) |  
|:------:|:------------------|
| *empty line* | print current board state | 
| `[a...z][2/4/6/8]` | push crate with given letter in the given direction: `2` =  down, `8` = up, `4` = left, `6` = right | 
| `0` | undo the most resent push | 
| `.` | player on a normal cell | 
| `*` | end of command sequence | 

**AUTOMATIC PATHFINDING**
The player automatically finds a valid path (using only free spaces) to the position needed to push the chosen crate.
If a push cannot be performed (e.g. the crate can’t move or is blocked), the command has no effect.

**UNDO SYSTEM**
The `0` command undoes the last successful push, restoring:
- crate position
- player position prior to the push

If no successful push has been made yet, this command has no effect.

## EXAMPLE USAGE
If your initial board is saved in `plansza.txt`:
```bash
cat plansza.tzt <(echo) - | ./sokoban
```
To record an interactive session:
```bash
cat plansza.txt <(echo) - | tee test.in | ./sokoban
```
Then replay the same test:
```bash
< test.in ./sokoban > test.out
diff test.out przyklad1.out
```

**PRIVIDED EXAMPLE FILES**
This repository includes several example input and output files:
```
przyklad1.in
przyklad1.out
przyklad2.in
przyklad2.out
przyklad3.in
przyklad3.out
```

Run a test:
```bash
< przykladX.in ./sokoban > outputX.txt
```
and compare it with expected result:
```bash
diff outputX.txt przykladX.out
```

## IMPLEMENTATION ASSUMPTIONS
- the input data is valid
- the board can be any size but will always have at least one line

