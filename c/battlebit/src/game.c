//
// Created by carson on 5/20/20.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "game.h"


// STEP 9 - Synchronization: the GAME structure will be accessed by both players interacting
// asynchronously with the server.  Therefore the data must be protected to avoid race conditions.
// Add the appropriate synchronization needed to ensure a clean battle.

static game * GAME = NULL;

void game_init() {
    if (GAME) {
        free(GAME);
    }
    GAME = malloc(sizeof(game));
    GAME->status = CREATED;
    game_init_player_info(&GAME->players[0]);
    game_init_player_info(&GAME->players[1]);
}

void game_init_player_info(player_info *player_info) {
    player_info->ships = 0;
    player_info->hits = 0;
    player_info->shots = 0;
}

int game_fire(game *game, int player, int x, int y) {
    // Step 5 - This is the crux of the game.  You are going to take a shot from the given player and
    // update all the bit values that store our game state.
    //
    //  - You will need up update the players 'shots' value
    //  - you You will need to see if the shot hits a ship in the opponents ships value.  If so, record a hit in the
    //    current players hits field
    //  - If the shot was a hit, you need to flip the ships value to 0 at that position for the opponents ships field
    //
    //  If the opponents ships value is 0, they have no remaining ships, and you should set the game state to
    //  PLAYER_1_WINS or PLAYER_2_WINS depending on who won.

    unsigned long long int mask = xy_to_bitval(x, y);

    if(mask == 0) {return 0;} //if x,y position off board, missed shot

    player_info *playerFiring = &game->players[player];
    player_info *playerFiredAt = &game->players[(player==0)?1:0];  //if player is 0, use player 1 info and vice versa


    playerFiring->shots = playerFiring->shots | mask;

    if(playerFiredAt->ships & mask) {

        playerFiredAt->ships = playerFiredAt->ships | mask;
        playerFiring->hits = playerFiring->hits | mask;

        //if playerFiredAt has no remaining ships on the board, playerFiring Wins. Otherwise, change player turn
        if(playerFiredAt->ships == 1){
            game->status = (player==0)?PLAYER_0_WINS:PLAYER_1_WINS; //if player is 0, player 0 wins and vice versa
        } else {
            game->status = (player==0)?PLAYER_1_TURN:PLAYER_0_TURN; //if player is 0, now player 1's turn and vice versa
        }

        return 1;
    }

    return 0;
}

unsigned long long int xy_to_bitval(int x, int y) {
    // Step 1 - implement this function.  We are taking an x, y position
    // and using bitwise operators, converting that to an unsigned long long
    // with a 1 in the position corresponding to that x, y
    //
    // x:0, y:0 == 0b00000...0001 (the one is in the first position)
    // x:1, y: 0 == 0b00000...10 (the one is in the second position)
    // ....
    // x:0, y: 1 == 0b100000000 (the one is in the eighth position)
    //
    // you will need to use bitwise operators and some math to produce the right
    // value.

    unsigned long long value;

    if ((x < 0) || (x > 7) || (y < 0) || (y >7))
        value = 0;
    else
        value = 1ull << (x + (y * 8));

    return value;
}

struct game * game_get_current() {
    return GAME;
}

int game_load_board(struct game *game, int player, char * spec) {
    // Step 2 - implement this function.  Here you are taking a C
    // string that represents a layout of ships, then testing
    // to see if it is a valid layout (no off-the-board positions
    // and no overlapping ships)

    // if it is valid, you should write the corresponding unsigned
    // long long value into the Game->players[player].ships data
    // slot and return 1

    // if it is invalid, you should return -1

    if(spec == NULL) { return -1; } // spec cannot be null

    //spec must have 15 characters
    if(strlen(spec) != 15 ){return -1;}

    player_info *playerInfo = &game->players[player];
    char * current = spec;

    //ships placed on board
    bool carrier = false;
    bool battleship = false;
    bool destroyer = false;
    bool submarine = false;
    bool patrol = false;

    for (int i = 0; i < 5; ++i) {
        char col =*(current + 1);
        char row =*(current + 2);

        int colInt = (int) (col - '0');
        int rowInt = (int) (row - '0');

        if ((carrier == false) && (*current == 'C' || *current == 'c')){           //Carrier - 5 spaces
            carrier = true;
            if (*current == 'C') {
                if(add_ship_horizontal(playerInfo, colInt, rowInt, 5) == -1) {return -1;}
            }
            else{
                if(add_ship_vertical(playerInfo, colInt, rowInt, 5) == -1) {return -1;}
            }
        }
        else if ((battleship == false) && (*current == 'B' || *current == 'b')){    //Battle Ship - 4 spaces
            battleship = true;
            if (*current == 'B') {
                if(add_ship_horizontal(playerInfo, colInt, rowInt, 4) == -1) {return -1;}
            }
            else{
                if(add_ship_vertical(playerInfo, colInt, rowInt, 4) == -1) {return -1;}
            }
        }
        else if ((destroyer == false) && (*current == 'D' || *current == 'd')){    //Destroyer - 3 spaces
            destroyer = true;
            if (*current == 'D') {
                if(add_ship_horizontal(playerInfo, colInt, rowInt, 3) == -1) {return -1;}
            }
            else{
                if(add_ship_vertical(playerInfo, colInt, rowInt, 3) == -1) {return -1;}
            }
        }
        else if ((submarine == false) && (*current == 'S' || *current == 's')){    //Submarine - 3 spaces
            submarine = true;
            if (*current == 'S') {
                if(add_ship_horizontal(playerInfo, colInt, rowInt, 3) == -1) {return -1;}
            }
            else{
                if(add_ship_vertical(playerInfo, colInt, rowInt, 3) == -1) {return -1;}
            }
        }
        else if ((patrol == false) && (*current == 'P' || *current == 'p')){   //Patrol - 2 spaces
            patrol = true;
            if (*current == 'P') {
                if(add_ship_horizontal(playerInfo, colInt, rowInt, 2) == -1) {return -1;}
            }
            else{
                if(add_ship_vertical(playerInfo, colInt, rowInt, 2) == -1) {return -1;}
            }
        }
        else {                                          //Not correct ship type
            return -1;
        }

        current += 3;
    }

    player_info *opponent = &game->players[(player==0)?1:0];
    if(opponent->ships != 0){
        game->status = PLAYER_0_TURN;
    }

    return 1;
}

int add_ship_horizontal(player_info *player, int x, int y, int length) {
    // implement this as part of Step 2
    // returns 1 if the ship can be added, -1 if not
    // hint: this can be defined recursively

    //Base case
    if(length == 0){return 1;} //return 1 if length is 0

    //check that x, y are on board between 0 and 7
    if(x<0 || x>7 || y<0 || y>7) { //(x,y) is off board
        return -1;
    }

    //check if there is already a ship at position x, y
    unsigned long long int mask = xy_to_bitval(x, y);

    if(player->ships & mask) {return -1;}

    //flip the players ships bit to 1
    player->ships = player->ships | mask;

    //recursive call
    return add_ship_horizontal(player, x+1, y, length-1); // Does the ship get added going right on board from start position? x+1?
}

int add_ship_vertical(player_info *player, int x, int y, int length) {
    // implement this as part of Step 2
    // returns 1 if the ship can be added, -1 if not
    // hint: this can be defined recursively

    //Base case
    if(length == 0){return 1;} //return 1 if length is 0

    //check that x, y are on board between 0 and 7
    if(x<0 || x>7 || y<0 || y>7) { return -1; } // x, y is off board

    //check if there is already a ship at position x, y
    unsigned long long int mask = xy_to_bitval(x, y);

    if(player->ships & mask) {return -1;}

    //flip the players ships bit to 1
    player->ships = player->ships | mask;

    //recursive call
    return add_ship_vertical(player, x, y+1, length-1); // Does the ship get added going up or down on board from start position? y+1 or y-1?
}