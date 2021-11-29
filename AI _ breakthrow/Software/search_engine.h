//
// Created by Simone on 25/09/2020.
//

#ifndef BREAKTHUV1_0_SEARCH_ENGINE_H
#define BREAKTHUV1_0_SEARCH_ENGINE_H


#include <iostream>
#include <cstdint>
#include <math.h>
#include <bitset>
#include <list>
#include <cstdlib>
#include <map>
#include <exception>
#include <random>       // std::default_random_engine


#define _inf -100000
#define inf 100000
#define table_size 11
#define DEEPTH 3
#define TIME 600


typedef uint64_t TABLE[2];
int8_t table_idx[table_size*table_size]= {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};
uint8_t row_shift_numbers[table_size*table_size]={
        55,55,55,55,55,55,55,55,55,55,55,
        44,44,44,44,44,44,44,44,44,44,44,
        33,33,33,33,33,33,33,33,33,33,33,
        22,22,22,22,22,22,22,22,22,22,22,
        11,11,11,11,11,11,11,11,11,11,11,
        0,0,0,0,0,0,0,0,0,55,55,
        44,44,44,44,44,44,44,44,44,44,44,
        33,33,33,33,33,33,33,33,33,33,33,
        22,22,22,22,22,22,22,22,22,22,22,
        11,11,11,11,11,11,11,11,11,11,11,
        0,0,0,0,0,0,0,0,0,0,0
};
typedef struct{
    int sqr_to1=-1;
    int sqr_from1=-1;
    int sqr_to2=-1;
    int sqr_from2=-1;
} MOVE;
typedef struct{
    TABLE my_board;
    TABLE other_board;
    TABLE flag_board;
    MOVE moves;
} STATE;

//** Utility Tables **//
void allZeros(TABLE move){ //set all bits at zeros
    move[0] = 0;
    move[1] = 0;
}
void AND (TABLE ret, TABLE table, TABLE mask){
    ret[0]=table[0]&mask[0];
    ret[1]=table[1]&mask[1];
    return;
}
void OR (TABLE ret, TABLE table, TABLE mask){
    ret[0]=table[0]|mask[0];
    ret[1]=table[1]|mask[1];
}
void XOR (TABLE ret, TABLE table, TABLE mask){
    ret[0]=table[0]^mask[0];
    ret[1]=table[1]^mask[1];
}
void COPY (TABLE to, TABLE from){
    TABLE zero;
    allZeros(zero);
    OR(to, from, zero);
}
void NOT (TABLE ret, TABLE table){
    ret[0]=~table[0];
    ret[1]=~table[1];
}
int countSetBits(uint64_t n){
    int count = 0;
    while (n) {
        count += n & 1;
        n >>= 1;
    }
    return count;
}
bool compare2moves(MOVE m1, MOVE m2){
    return (m1.sqr_from1==m2.sqr_from1)&&(m1.sqr_to1==m2.sqr_to1)&&(m1.sqr_from2==m2.sqr_from2)&&(m1.sqr_to2==m2.sqr_to2);
}

void generate_move(TABLE table, int from, int to){
    TABLE move;
    allZeros(move);
    int idx=0;
    if(from>=64) {from-=64; idx=1;}
    else idx=0;
    uint64_t num = pow(2, 64-from)/2;
    move[idx] |= num;
    if(to>=64) {to-=64; idx=1;}
    else idx=0;
    num = pow(2, 64-to)/2;
    move[idx] |= num;
    XOR(table, move, table);
}
uint16_t get_row_rank(TABLE tabler, int sqr){
    TABLE table;
    COPY(table, tabler);
    uint8_t shift = row_shift_numbers[sqr];
    uint16_t row;
    if(sqr>=55 && sqr<=65) {
        row = ((table[table_idx[sqr]] >> shift) & 0b11111111111);
        row = ((row<<2) | (table[1]>>(row_shift_numbers[64]+7) & 0b00000000011))&0b11111111111;
    }
    else {
        if (table_idx[sqr] == 1) {
            shift += 7;
            row = ((table[table_idx[sqr]] >> shift) & 0b11111111111);
        } else {
            row = ((table[table_idx[sqr]] >> (shift-2)) & 0b11111111111);
        }
    }
    row&=~(0b1<<(10-sqr%11));
    return row;
}
uint16_t get_col_rank(TABLE table, int sqr){
    uint16_t rank=0b0;
    uint64_t mask=0b1;
    mask=mask<<(10-(sqr%11)+7);
    char j=1;
    for(int i=0; i<11; i++){
        rank|=(((table[j]&mask)!=0)<<i);
        mask=mask<<11;
        if(mask==0){ //overflow
            j=0;
            mask=0b1;
            if(sqr%11<9)
                mask=mask<<(8-(sqr%11));
            else
                mask=mask<<(9+10-(sqr%11));
        }
    }
    rank&=~(0b1<<(10-sqr/11));
    return rank;
}
void horizontal_move_generator(int horizontal_move[][2048][10]){
    int j=0;
    for(uint16_t occupancy=0; occupancy<pow(2,11); occupancy++) {
        for (int pos = 0; pos < 11; pos++) {
            j=0;
            //std::cout << "\npos:" << pos << "\n";
            uint16_t d = occupancy >> (11 - pos);
            for (int i = 1; i <= pos; i++) {
                //std::cout << std::bitset<11>(d)<<"\n";
                if (!(d & 0b1)) {
                    horizontal_move[pos][occupancy][j]=(-i);
                    j++;
                    d = d >> 1;
                } else
                    break;
            }
            d = occupancy << pos + 1;
            for (int i = 1; i < (11 - pos); i++) {
                //std::cout << std::bitset<11>(d)<<"\n";
                if (!(d & 0b10000000000)) {
                        //std::cout << pos + i << "\t";
                        horizontal_move[pos][occupancy][j++]=i;
                    d = d << 1;
                } else
                    break;
            }
            for(int k=j; k<10; k++){
                horizontal_move[pos][occupancy][k]=0;
            }
        }
    }
    std::cout<<"\nMove's table generated\n";
}
uint8_t getMyPawns(uint8_t arr[], TABLE myboard){
    int j=0;
    int count=0;
    TABLE loc;
    COPY(loc, myboard);
    for(int i=0; i<20; i++){
        arr[i]=0;
    }
    for(int i=0; i<64; i++){
        if(loc[0]&0b1){
            arr[j]=63-i;
            count++; j++;
        }
        loc[0]=loc[0]>>1;
    }
    loc[1]=loc[1]>>7;
    for(int i=0; i<(64-7); i++){
        if(loc[1]&0b1){
            arr[j]=120-i;
            count++; j++;
        }
        loc[1]=loc[1]>>1;
    }
    return count;
}
uint8_t getFlag(TABLE flag_board){
    TABLE loc;
    COPY(loc, flag_board);
    for(int i=0; i<64; i++){
        if(loc[0]&0b1){
            return 63-i;
        }
        loc[0]=loc[0]>>1;
    }
    loc[1]=loc[1]>>7;
    for(int i=0; i<(64-7); i++){
        if(loc[1]&0b1){
            return 63-7-i+64;
        }
        loc[1]=loc[1]>>1;
    }
    return 0;
}
void printTable(STATE s_cpy, bool type){
    std::cout<<"\n";
    char me, other;
    char flag='@';
    if(!type){ //ho la bandiera =0
        me='O';
        other='X';
    }
    else{
        me='X';
        other='O';
    }
    std::cout<<"\t";
    for(int i=97; i<97+11; i++){
        std::cout<<char(i)<<"\t";
    }
    std::cout<<"\n";
    int idx=0;
    uint64_t mask=0b1000000000000000000000000000000000000000000000000000000000000000;
    for(int i=0; i<11; i++){
        std::cout<<(11-i)<<"\t";
        for(int j=0; j<11; j++){
            idx=table_idx[i*11+j];
            if(s_cpy.my_board[idx]&mask) std::cout<<me<<"\t";
            else if(s_cpy.other_board[idx]&mask) std::cout<<other<<"\t";
            else if(s_cpy.flag_board[idx]&mask) std::cout<<flag<<"\t";
            else std::cout<<"."<<"\t";
            s_cpy.my_board[idx]=s_cpy.my_board[idx]<<1;
            s_cpy.other_board[idx]=s_cpy.other_board[idx]<<1;
            s_cpy.flag_board[idx]=s_cpy.flag_board[idx]<<1;
        }
        std::cout<<"\n";
    }
}

//**** Generation moves Engine ****//
void generatore_mosse(STATE s, std::list<STATE> &l, bool type, int horizontal_move[][2048][10]){
    TABLE occupancy={0,0};
    OR(occupancy, s.my_board, s.other_board);
    OR(occupancy, occupancy, s.flag_board);
    uint8_t mypawns[20];
    uint8_t num_pawns;
    num_pawns=getMyPawns(mypawns, s.my_board);
    uint8_t sqr_flag;
    sqr_flag=getFlag(s.flag_board);
    int count=0, sqr;
    STATE new_s;
    uint16_t occ1, occ2;
    uint8_t sol[2];
    //2 PAWNS MOVE
    allZeros(occupancy);
    OR(occupancy, s.my_board, s.other_board);
    OR(occupancy, occupancy, s.flag_board);
    for(int or1=0; or1<2; or1++){ //direction first move
        for(int or2=0; or2<2; or2++){ //direction second move
            //2 pawns move
            for(int i=0; i<num_pawns; i++){
                for(int j=0; j<num_pawns; j++){
                    if(i!=j){ //non-repeated provisions of pawns
                        for(int k=0; k<10; k++){ //for every possible moves
                            STATE new_s;
                            bool pass=true;
                            if(or1==false){
                                occ1=get_row_rank(occupancy, mypawns[i]);
                            }
                            else{
                                occ1=get_col_rank(occupancy, mypawns[i]);
                            }
                            TABLE move1={0,0};
                            if(or1==false){
                                if(horizontal_move[mypawns[i]%11][occ1][k]==0) pass=false;
                                else {
                                    generate_move(move1, mypawns[i], mypawns[i] + horizontal_move[mypawns[i] % 11][occ1][k]);
                                    new_s.moves.sqr_from1 = mypawns[i];
                                    new_s.moves.sqr_to1 = mypawns[i] + horizontal_move[mypawns[i] % 11][occ1][k];
                                }
                            }
                            else{
                                if(horizontal_move[mypawns[i]/11][occ1][k]==0) pass=false;
                                else {
                                    generate_move(move1, mypawns[i], mypawns[i] + ((horizontal_move[mypawns[i] / 11][occ1][k]) * 11));
                                    new_s.moves.sqr_from1 = mypawns[i];
                                    new_s.moves.sqr_to1 = mypawns[i] + ((horizontal_move[mypawns[i] / 11][occ1][k]) * 11);
                                }
                            }
                            if(pass) {
                                XOR(s.my_board, s.my_board, move1);
                                XOR(occupancy, occupancy, move1);
                                for(int w=0; w<10; w++){
                                    pass=true;
                                    TABLE move2={0,0}, res={0,0};
                                     if(or2==false){
                                         occ2=get_row_rank(occupancy, mypawns[j]);
                                         if(horizontal_move[mypawns[j]%11][occ2][w]==0) pass=false;
                                         else {
                                             generate_move(move2, mypawns[j], mypawns[j] + horizontal_move[mypawns[j] % 11][occ2][w]);
                                             new_s.moves.sqr_from2 = mypawns[j];
                                             new_s.moves.sqr_to2 = mypawns[j] + horizontal_move[mypawns[j] % 11][occ2][w];
                                         }
                                     }
                                     else{
                                         occ2=get_col_rank(occupancy, mypawns[j]);
                                         if(horizontal_move[mypawns[j]/11][occ2][w]==0) pass=false;
                                         else {
                                             generate_move(move2, mypawns[j], mypawns[j] + ((horizontal_move[mypawns[j] / 11][occ2][w]) * 11));
                                             new_s.moves.sqr_from2 = mypawns[j];
                                             new_s.moves.sqr_to2 = mypawns[j] + ((horizontal_move[mypawns[j] / 11][occ2][w]) * 11);
                                         }
                                     }
                                     AND(res, move1, move2);
                                     if(res[0]==0&&res[1]==0&&pass){ //delete moves overlay
                                             //APPLY
                                             COPY(new_s.my_board, s.my_board);
                                             XOR(new_s.my_board, s.my_board, move2);
                                             COPY(new_s.other_board, s.other_board);
                                             COPY(new_s.flag_board, s.flag_board);
                                             l.push_back(new_s);
                                             //printTable(new_s, false);
                                     }
                                 }
                               // printTable(s, false);
                                XOR(s.my_board, s.my_board, move1);
                                XOR(occupancy, occupancy, move1);
                            }
                        }
                    }
                }
            }
            /*
            //flag + 1 pawns move
            if(type==0) {
                for(int j=0; j<num_pawns; j++){
                    for(int k=0; k<10; k++){
                        STATE new_s;
                        bool pass=true;
                        if(or1==false){
                            occ1=get_row_rank(occupancy, sqr_flag);
                        }
                        else{
                            occ1=get_col_rank(occupancy, sqr_flag);
                        }
                        TABLE move1={0,0};
                        if(or1==false){
                            if(horizontal_move[sqr_flag%11][occ1][k]==0) pass=false;
                            else {
                                generate_move(move1, sqr_flag, sqr_flag + horizontal_move[sqr_flag % 11][occ1][k]);
                                new_s.moves.sqr_from1 = sqr_flag;
                                new_s.moves.sqr_to1 = sqr_flag + horizontal_move[sqr_flag % 11][occ1][k];
                            }
                        }
                        else{
                            if(horizontal_move[sqr_flag/11][occ1][k]==0) pass=false;
                            else {
                                generate_move(move1, sqr_flag, sqr_flag + ((horizontal_move[sqr_flag / 11][occ1][k]) * 11));
                                new_s.moves.sqr_from1 = sqr_flag;
                                new_s.moves.sqr_to1 = sqr_flag + ((horizontal_move[sqr_flag / 11][occ1][k]) * 11);
                            }
                        }
                        if(pass) {
                            XOR(s.flag_board, s.flag_board, move1);
                            XOR(occupancy, occupancy, move1);
                            for(int w=0; w<10; w++){
                                pass=true;
                                TABLE move2={0,0}, res={0,0};
                                if(or2==false){
                                    occ2=get_row_rank(occupancy, mypawns[j]);
                                    if(horizontal_move[mypawns[j]%11][occ2][w]==0) pass=false;
                                    else {
                                        generate_move(move2, mypawns[j], mypawns[j] + horizontal_move[mypawns[j] % 11][occ2][w]);
                                        new_s.moves.sqr_from2 = mypawns[j];
                                        new_s.moves.sqr_to2 = mypawns[j] + horizontal_move[mypawns[j] % 11][occ2][w];
                                    }
                                }
                                else{
                                    occ2=get_col_rank(occupancy, mypawns[j]);
                                    if(horizontal_move[mypawns[j]/11][occ2][w]==0) pass=false;
                                    else {
                                        generate_move(move2, mypawns[j], mypawns[j] + ((horizontal_move[mypawns[j] / 11][occ2][w]) * 11));
                                        new_s.moves.sqr_from2 = mypawns[j];
                                        new_s.moves.sqr_to2 = mypawns[j] + ((horizontal_move[mypawns[j] / 11][occ2][w]) * 11);
                                    }
                                }
                                AND(res, move1, move2);
                                if(res[0]==0&&res[1]==0&&pass){ //pruning n.2 => delete move that sovrappongono
                                    //APPLICA
                                    COPY(new_s.flag_board, s.flag_board);
                                    XOR(new_s.my_board, s.my_board, move2);
                                    COPY(new_s.other_board,s.other_board);
                                    //COPY(new_s.flag_board, s.flag_board);
                                    l.push_back(new_s);
                                    //printTable(new_s, false);
                                }
                            }
                            // printTable(s, false);
                            XOR(s.flag_board, s.flag_board, move1);
                            XOR(occupancy, occupancy, move1);

                        }
                    }
                }
                for(int j=0; j<num_pawns; j++){
                    for(int k=0; k<10; k++){
                        STATE new_s;
                        bool pass=true;
                        if(or1==false){
                            occ1=get_row_rank(occupancy, mypawns[j]);
                        }
                        else{
                            occ1=get_col_rank(occupancy, mypawns[j]);
                        }
                        TABLE move1={0,0};
                        if(or1==false){
                            if(horizontal_move[mypawns[j]%11][occ1][k]==0) pass=false;
                            else {
                                generate_move(move1, mypawns[j], mypawns[j] + horizontal_move[mypawns[j] % 11][occ1][k]);
                                new_s.moves.sqr_from1 = mypawns[j];
                                new_s.moves.sqr_to1 = mypawns[j] + horizontal_move[mypawns[j] % 11][occ1][k];
                            }
                        }
                        else{
                            if(horizontal_move[mypawns[j]/11][occ1][k]==0) pass=false;
                            else {
                                generate_move(move1, mypawns[j], mypawns[j] + ((horizontal_move[mypawns[j] / 11][occ1][k]) * 11));
                                new_s.moves.sqr_from1 = mypawns[j];
                                new_s.moves.sqr_to1 = mypawns[j] + ((horizontal_move[mypawns[j] / 11][occ1][k]) * 11);
                            }
                        }
                        if(pass) {
                            XOR(s.my_board, s.my_board, move1);
                            XOR(occupancy, occupancy, move1);
                            for(int w=0; w<10; w++){
                                pass=true;
                                TABLE move2={0,0}, res={0,0};
                                if(or2==false){
                                    occ2=get_row_rank(occupancy, sqr_flag);
                                    if(horizontal_move[sqr_flag%11][occ2][w]==0) pass=false;
                                    else {
                                        generate_move(move2, sqr_flag, sqr_flag + horizontal_move[sqr_flag % 11][occ2][w]);
                                        new_s.moves.sqr_from2 = sqr_flag;
                                        new_s.moves.sqr_to2 = sqr_flag + horizontal_move[sqr_flag % 11][occ2][w];
                                    }
                                }
                                else{
                                    occ2=get_col_rank(occupancy, sqr_flag);
                                    if(horizontal_move[sqr_flag/11][occ2][w]==0) pass=false;
                                    else {
                                        generate_move(move2, sqr_flag, sqr_flag + ((horizontal_move[sqr_flag / 11][occ2][w]) * 11));
                                        new_s.moves.sqr_from2 = sqr_flag;
                                        new_s.moves.sqr_to2 = sqr_flag + ((horizontal_move[sqr_flag / 11][occ2][w]) * 11);
                                    }
                                }
                                AND(res, move1, move2);
                                if(res[0]==0&&res[1]==0&&pass){
                                    //APPLICA
                                    COPY(new_s.my_board, s.my_board);
                                    XOR(new_s.flag_board, s.flag_board, move2);
                                    COPY(new_s.other_board,s.other_board);
                                    l.push_back(new_s);
                                    //printTable(new_s, false);
                                }
                            }
                            // printTable(s, false);
                            XOR(s.my_board, s.my_board, move1);
                            XOR(occupancy, occupancy, move1);

                        }
                    }
                }
            }
            */
        }
    }
    //PAWN CAPTURES
    TABLE move, move_other, res;
    for (int i = 0; i < num_pawns; i++) {
        sqr = mypawns[i];
        allZeros(move);
        allZeros(move_other);
        allZeros(res);
        allZeros(occupancy);
        if (type) { //type=0 I have the flag
            OR(occupancy, s.flag_board, s.other_board);
        } else {
            COPY(occupancy, s.other_board);
        }
        int sqr_to_capture = sqr - 11 - 1; //top-sx
        if (sqr_to_capture >= 0 && sqr_to_capture % 11 < sqr % 11) {
            allZeros(move);
            allZeros(move_other);
            generate_move(move_other, sqr_to_capture, sqr_to_capture);
            generate_move(move, sqr, sqr_to_capture);
            AND(res, occupancy, move_other);
            if (res[0] != 0 || res[1] != 0) { //there is a possible capture
                new_s.moves.sqr_from1 = sqr;
                new_s.moves.sqr_to1 = sqr_to_capture;
                new_s.moves.sqr_from2 = -1;
                new_s.moves.sqr_to2 = -1;
                AND(res, s.flag_board, move_other);
                if (res[0] != 0 || res[1] != 0) { //capture the flag
                    XOR(new_s.flag_board, s.flag_board, move_other);
                    COPY(new_s.other_board, s.other_board);//new_s.other_boad=s.other_board;
                } else {
                    XOR(new_s.other_board, s.other_board, move_other);
                    COPY(new_s.flag_board, s.flag_board);//new_s.flag_boad=s.flag_board;
                }
                XOR(new_s.my_board, s.my_board, move);
                l.push_back(new_s);
                //printTable(new_s, type);
            }
        }
        sqr_to_capture = sqr - 11 + 1; //top-dx
        if (sqr_to_capture >= 0 && sqr_to_capture % 11 > sqr % 11) {
            allZeros(move);
            allZeros(move_other);
            generate_move(move_other, sqr_to_capture, sqr_to_capture);
            generate_move(move, sqr, sqr_to_capture);
            AND(res, occupancy, move_other);
            if (res[0] != 0 || res[1] != 0) {
                new_s.moves.sqr_from1 = sqr;
                new_s.moves.sqr_to1 = sqr_to_capture;
                new_s.moves.sqr_from2 = -1;
                new_s.moves.sqr_to2 = -1;

                AND(res, s.flag_board, move_other);
                if (res[0] != 0 || res[1] != 0) {
                    XOR(new_s.flag_board, s.flag_board, move_other);
                    COPY(new_s.other_board, s.other_board);//new_s.other_boad=s.other_board;
                } else {
                    XOR(new_s.other_board, s.other_board, move_other);
                    COPY(new_s.flag_board, s.flag_board);//new_s.flag_boad=s.flag_board;
                }
                XOR(new_s.my_board, s.my_board, move);
                l.push_back(new_s);
                //printTable(new_s, type);
            }
        }
        sqr_to_capture = sqr + 11 - 1; //down-sx
        if (sqr_to_capture < table_size * table_size && sqr_to_capture % 11 < sqr % 11) {
            allZeros(move);
            allZeros(move_other);
            generate_move(move_other, sqr_to_capture, sqr_to_capture);
            generate_move(move, sqr, sqr_to_capture);
            AND(res, occupancy, move_other);
            if (res[0] != 0 || res[1] != 0) {
                new_s.moves.sqr_from1 = sqr;
                new_s.moves.sqr_to1 = sqr_to_capture;
                new_s.moves.sqr_from2 = -1;
                new_s.moves.sqr_to2 = -1;
                //todo cattura FLAG o OTHER
                AND(res, s.flag_board, move_other);
                if (res[0] != 0 || res[1] != 0) {
                    XOR(new_s.flag_board, s.flag_board, move_other);
                    COPY(new_s.other_board, s.other_board);//new_s.other_boad=s.other_board;
                } else {
                    XOR(new_s.other_board, s.other_board, move_other);
                    COPY(new_s.flag_board, s.flag_board);//new_s.flag_boad=s.flag_board;
                }
                XOR(new_s.my_board, s.my_board, move);
                l.push_back(new_s);
                //printTable(new_s, type);
            }
        }
        sqr_to_capture = sqr + 11 + 1; //down-dx
        if (sqr_to_capture < table_size * table_size && sqr_to_capture % 11 > sqr % 11) {
            allZeros(move);
            allZeros(move_other);
            generate_move(move_other, sqr_to_capture, sqr_to_capture);
            generate_move(move, sqr, sqr_to_capture);
            AND(res, occupancy, move_other);
            if (res[0] != 0 || res[1] != 0) {
                new_s.moves.sqr_from1 = sqr;
                new_s.moves.sqr_to1 = sqr_to_capture;
                new_s.moves.sqr_from2 = -1;
                new_s.moves.sqr_to2 = -1;
                //todo cattura FLAG o OTHER
                AND(res, s.flag_board, move_other);
                if (res[0] != 0 || res[1] != 0) {
                    XOR(new_s.flag_board, s.flag_board, move_other);
                    COPY(new_s.other_board, s.other_board);//new_s.other_boad=s.other_board;
                } else {
                    XOR(new_s.other_board, s.other_board, move_other);
                    COPY(new_s.flag_board, s.flag_board);//new_s.flag_boad=s.flag_board;
                }
                XOR(new_s.my_board, s.my_board, move);
                l.push_back(new_s);
                //printTable(new_s, type);
            }
        }
    }
    //FLAG CAPTURES
    sqr = sqr_flag;
    allZeros(move);
    allZeros(move_other);
    allZeros(res);
    allZeros(occupancy);
    if (type) { //type=0 I have the flag
        OR(occupancy, s.flag_board, s.other_board);
    } else {
        COPY(occupancy, s.other_board);
    }
    int sqr_to_capture = sqr - 11 - 1; //top-sx
    if (sqr_to_capture >= 0 && sqr_to_capture % 11 < sqr % 11) {
        allZeros(move);
        allZeros(move_other);
        generate_move(move_other, sqr_to_capture, sqr_to_capture);
        generate_move(move, sqr, sqr_to_capture);
        AND(res, occupancy, move_other);
        if (res[0] != 0 || res[1] != 0) { //there is a possible capture
            new_s.moves.sqr_from1 = sqr;
            new_s.moves.sqr_to1 = sqr_to_capture;
            new_s.moves.sqr_from2 = -1;
            new_s.moves.sqr_to2 = -1;
            AND(res, s.flag_board, move_other);
            if (res[0] != 0 || res[1] != 0) { //capture the flag
                XOR(new_s.flag_board, s.flag_board, move_other);
                COPY(new_s.other_board, s.other_board);//new_s.other_boad=s.other_board;
            } else {
                XOR(new_s.other_board, s.other_board, move_other);
                COPY(new_s.flag_board, s.flag_board);//new_s.flag_boad=s.flag_board;
            }
            XOR(new_s.my_board, s.my_board, move);
            l.push_back(new_s);
            //printTable(new_s, type);
        }
    }
    sqr_to_capture = sqr - 11 + 1; //top-dx
    if (sqr_to_capture >= 0 && sqr_to_capture % 11 > sqr % 11) {
        allZeros(move);
        allZeros(move_other);
        generate_move(move_other, sqr_to_capture, sqr_to_capture);
        generate_move(move, sqr, sqr_to_capture);
        AND(res, occupancy, move_other);
        if (res[0] != 0 || res[1] != 0) {
            new_s.moves.sqr_from1 = sqr;
            new_s.moves.sqr_to1 = sqr_to_capture;
            new_s.moves.sqr_from2 = -1;
            new_s.moves.sqr_to2 = -1;

            AND(res, s.flag_board, move_other);
            if (res[0] != 0 || res[1] != 0) {
                XOR(new_s.flag_board, s.flag_board, move_other);
                COPY(new_s.other_board, s.other_board);//new_s.other_boad=s.other_board;
            } else {
                XOR(new_s.other_board, s.other_board, move_other);
                COPY(new_s.flag_board, s.flag_board);//new_s.flag_boad=s.flag_board;
            }
            XOR(new_s.my_board, s.my_board, move);
            l.push_back(new_s);
            //printTable(new_s, type);
        }
    }
    sqr_to_capture = sqr + 11 - 1; //down-sx
    if (sqr_to_capture < table_size * table_size && sqr_to_capture % 11 < sqr % 11) {
        allZeros(move);
        allZeros(move_other);
        generate_move(move_other, sqr_to_capture, sqr_to_capture);
        generate_move(move, sqr, sqr_to_capture);
        AND(res, occupancy, move_other);
        if (res[0] != 0 || res[1] != 0) {
            new_s.moves.sqr_from1 = sqr;
            new_s.moves.sqr_to1 = sqr_to_capture;
            new_s.moves.sqr_from2 = -1;
            new_s.moves.sqr_to2 = -1;
            //todo cattura FLAG o OTHER
            AND(res, s.flag_board, move_other);
            if (res[0] != 0 || res[1] != 0) {
                XOR(new_s.flag_board, s.flag_board, move_other);
                COPY(new_s.other_board, s.other_board);//new_s.other_boad=s.other_board;
            } else {
                XOR(new_s.other_board, s.other_board, move_other);
                COPY(new_s.flag_board, s.flag_board);//new_s.flag_boad=s.flag_board;
            }
            XOR(new_s.my_board, s.my_board, move);
            l.push_back(new_s);
            //printTable(new_s, type);
        }
    }
    sqr_to_capture = sqr + 11 + 1; //down-dx
    if (sqr_to_capture < table_size * table_size && sqr_to_capture % 11 > sqr % 11) {
        allZeros(move);
        allZeros(move_other);
        generate_move(move_other, sqr_to_capture, sqr_to_capture);
        generate_move(move, sqr, sqr_to_capture);
        AND(res, occupancy, move_other);
        if (res[0] != 0 || res[1] != 0) {
            new_s.moves.sqr_from1 = sqr;
            new_s.moves.sqr_to1 = sqr_to_capture;
            new_s.moves.sqr_from2 = -1;
            new_s.moves.sqr_to2 = -1;
            //todo cattura FLAG o OTHER
            AND(res, s.flag_board, move_other);
            if (res[0] != 0 || res[1] != 0) {
                XOR(new_s.flag_board, s.flag_board, move_other);
                COPY(new_s.other_board, s.other_board);//new_s.other_boad=s.other_board;
            } else {
                XOR(new_s.other_board, s.other_board, move_other);
                COPY(new_s.flag_board, s.flag_board);//new_s.flag_boad=s.flag_board;
            }
            XOR(new_s.my_board, s.my_board, move);
            l.push_back(new_s);
            //printTable(new_s, type);
        }
    }
    /*
    //flag + 1 capture
    if(type==0) {
        for (int or1 = 0; or1 < 2; or1++) {
            for(int a=0; a<11; a++) {
                TABLE move = {0, 0};
                if (or1 == false) {
                    allZeros(occupancy);
                    OR(occupancy, s.my_board, s.other_board);
                    OR(occupancy, occupancy, s.flag_board);
                    occ1 = get_row_rank(occupancy, sqr_flag);
                    if((horizontal_move[sqr_flag % 11][occ1][a])!=0) {
                        generate_move(move, sqr_flag, sqr_flag + horizontal_move[sqr_flag % 11][occ1][a]);
                        new_s.moves.sqr_from1 = sqr_flag;
                        new_s.moves.sqr_to1 = sqr_flag + horizontal_move[sqr_flag % 11][occ1][a];
                        //APPLY
                        XOR(new_s.flag_board, s.flag_board, move);
                        for (int i = 0; i < num_pawns; i++) {
                            sqr = mypawns[i];
                            allZeros(move);
                            allZeros(move_other);
                            allZeros(res);
                            allZeros(occupancy);
                            COPY(occupancy, s.other_board);
                            int sqr_to_capture = sqr - 11 - 1; //top-sx
                            if (sqr_to_capture >= 0 && sqr_to_capture % 11 < sqr % 11) {
                                allZeros(move);
                                allZeros(move_other);
                                generate_move(move_other, sqr_to_capture, sqr_to_capture);
                                generate_move(move, sqr, sqr_to_capture);
                                AND(res, occupancy, move_other);
                                if (res[0] != 0 || res[1] != 0) {
                                    new_s.moves.sqr_from2 = sqr;
                                    new_s.moves.sqr_to2 = sqr_to_capture;
                                    XOR(new_s.other_board, s.other_board, move_other);
                                    XOR(new_s.my_board, s.my_board, move);
                                    l.push_back(new_s);
                                    //printTable(new_s, type);
                                }
                            }
                            sqr_to_capture = sqr - 11 + 1; //top-dx
                            if (sqr_to_capture >= 0 && sqr_to_capture % 11 > sqr % 11) {
                                allZeros(move);
                                allZeros(move_other);
                                generate_move(move_other, sqr_to_capture, sqr_to_capture);
                                generate_move(move, sqr, sqr_to_capture);
                                AND(res, occupancy, move_other);
                                if (res[0] != 0 || res[1] != 0) { //c'è una possibile cattura
                                    new_s.moves.sqr_from2 = sqr;
                                    new_s.moves.sqr_to2 = sqr_to_capture;
                                    XOR(new_s.other_board, s.other_board, move_other);
                                    XOR(new_s.my_board, s.my_board, move);
                                    l.push_back(new_s);
                                    //printTable(new_s, type);
                                }
                            }
                            sqr_to_capture = sqr + 11 - 1; //down-sx
                            if (sqr_to_capture < table_size * table_size && sqr_to_capture % 11 < sqr % 11) {
                                allZeros(move);
                                allZeros(move_other);
                                generate_move(move_other, sqr_to_capture, sqr_to_capture);
                                generate_move(move, sqr, sqr_to_capture);
                                AND(res, occupancy, move_other);
                                if (res[0] != 0 || res[1] != 0) { //c'è una possibile cattura
                                    new_s.moves.sqr_from2 = sqr;
                                    new_s.moves.sqr_to2 = sqr_to_capture;
                                    XOR(new_s.other_board, s.other_board, move_other);
                                    XOR(new_s.my_board, s.my_board, move);
                                    l.push_back(new_s);
                                    //printTable(new_s, type);
                                }
                            }
                            sqr_to_capture = sqr + 11 + 1; //down-dx
                            if (sqr_to_capture < table_size * table_size && sqr_to_capture % 11 > sqr % 11) {
                                allZeros(move);
                                allZeros(move_other);
                                generate_move(move_other, sqr_to_capture, sqr_to_capture);
                                generate_move(move, sqr, sqr_to_capture);
                                AND(res, occupancy, move_other);
                                if (res[0] != 0 || res[1] != 0) { //c'è una possibile cattura
                                    new_s.moves.sqr_from2 = sqr;
                                    new_s.moves.sqr_to2 = sqr_to_capture;
                                    XOR(new_s.other_board, s.other_board, move_other);
                                    XOR(new_s.my_board, s.my_board, move);
                                    l.push_back(new_s);
                                    //printTable(new_s, type);
                                }
                            }
                        }
                    }
                } else {
                    allZeros(occupancy);
                    OR(occupancy, s.my_board, s.other_board);
                    OR(occupancy, occupancy, s.flag_board);
                    occ1 = get_col_rank(occupancy, sqr_flag);
                    if(((horizontal_move[sqr_flag / 11][occ1][a]) * 11)!=0) {
                        generate_move(move, sqr_flag, sqr_flag + ((horizontal_move[sqr_flag / 11][occ1][a]) * 11));
                        new_s.moves.sqr_from1 = sqr_flag;
                        new_s.moves.sqr_to1 = sqr_flag + ((horizontal_move[sqr_flag / 11][occ1][a]) * 11);
                        //APPLICA
                        XOR(new_s.flag_board, s.flag_board, move);
                        for (int i = 0; i < num_pawns; i++) {
                            sqr = mypawns[i];
                            allZeros(move);
                            allZeros(move_other);
                            allZeros(res);
                            allZeros(occupancy);
                            COPY(occupancy, s.other_board);
                            int sqr_to_capture = sqr - 11 - 1; //in alto a sinista
                            if (sqr_to_capture >= 0 && sqr_to_capture % 11 < sqr % 11) {
                                allZeros(move);
                                allZeros(move_other);
                                generate_move(move_other, sqr_to_capture, sqr_to_capture);
                                generate_move(move, sqr, sqr_to_capture);
                                AND(res, occupancy, move_other);
                                if (res[0] != 0 || res[1] != 0) { //c'è una possibile cattura
                                    new_s.moves.sqr_from2 = sqr;
                                    new_s.moves.sqr_to2 = sqr_to_capture;
                                    XOR(new_s.other_board, s.other_board, move_other);
                                    XOR(new_s.my_board, s.my_board, move);
                                    l.push_back(new_s);
                                    //printTable(new_s, type);
                                }
                            }
                            sqr_to_capture = sqr - 11 + 1; //in alto a destra
                            if (sqr_to_capture >= 0 && sqr_to_capture % 11 > sqr % 11) {
                                allZeros(move);
                                allZeros(move_other);
                                generate_move(move_other, sqr_to_capture, sqr_to_capture);
                                generate_move(move, sqr, sqr_to_capture);
                                AND(res, occupancy, move_other);
                                if (res[0] != 0 || res[1] != 0) { //c'è una possibile cattura
                                    new_s.moves.sqr_from2 = sqr;
                                    new_s.moves.sqr_to2 = sqr_to_capture;
                                    XOR(new_s.other_board, s.other_board, move_other);
                                    XOR(new_s.my_board, s.my_board, move);
                                    l.push_back(new_s);
                                    //printTable(new_s, type);
                                }
                            }
                            sqr_to_capture = sqr + 11 - 1; //in basso a sinistra
                            if (sqr_to_capture < table_size * table_size && sqr_to_capture % 11 < sqr % 11) {
                                allZeros(move);
                                allZeros(move_other);
                                generate_move(move_other, sqr_to_capture, sqr_to_capture);
                                generate_move(move, sqr, sqr_to_capture);
                                AND(res, occupancy, move_other);
                                if (res[0] != 0 || res[1] != 0) { //c'è una possibile cattura
                                    new_s.moves.sqr_from2 = sqr;
                                    new_s.moves.sqr_to2 = sqr_to_capture;
                                    XOR(new_s.other_board, s.other_board, move_other);
                                    XOR(new_s.my_board, s.my_board, move);
                                    l.push_back(new_s);
                                    //printTable(new_s, type);
                                }
                            }
                            sqr_to_capture = sqr + 11 + 1; //in basso a destra
                            if (sqr_to_capture < table_size * table_size && sqr_to_capture % 11 > sqr % 11) {
                                allZeros(move);
                                allZeros(move_other);
                                generate_move(move_other, sqr_to_capture, sqr_to_capture);
                                generate_move(move, sqr, sqr_to_capture);
                                AND(res, occupancy, move_other);
                                if (res[0] != 0 || res[1] != 0) { //c'è una possibile cattura
                                    new_s.moves.sqr_from2 = sqr;
                                    new_s.moves.sqr_to2 = sqr_to_capture;
                                    XOR(new_s.other_board, s.other_board, move_other);
                                    XOR(new_s.my_board, s.my_board, move);
                                    l.push_back(new_s);
                                    //printTable(new_s, type);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    */
    //FLAG MOVES
    allZeros(occupancy);
    OR(occupancy, s.my_board, s.other_board);
    OR(occupancy, occupancy, s.flag_board);
    if(type==0) { //if I have the flag check
        for (int or1 = 0; or1 < 2; or1++) { //horizontal or vertical move
            for(int i=0; i<11; i++) { //for every possible move in horizontal_move
                TABLE move = {0, 0};
                new_s.moves.sqr_from2 = -1;
                new_s.moves.sqr_to2 = -1;
                if (or1 == false) {
                    occ1 = get_row_rank(occupancy, sqr_flag);
                    if((horizontal_move[sqr_flag % 11][occ1][i])!=0) {
                        generate_move(move, sqr_flag, sqr_flag + horizontal_move[sqr_flag % 11][occ1][i]);
                        new_s.moves.sqr_from1 = sqr_flag;
                        new_s.moves.sqr_to1 = sqr_flag + horizontal_move[sqr_flag % 11][occ1][i];
                        //APPLY
                        XOR(new_s.flag_board, s.flag_board, move);
                        COPY(new_s.my_board, s.my_board);//new_s.my_board=s.my_board;
                        COPY(new_s.other_board, s.other_board);//new_s.other_board=s.other_board;
                        l.push_back(new_s);
                        //printTable(new_s, type);
                    }
                } else {
                    occ1 = get_col_rank(occupancy, sqr_flag);
                    if(((horizontal_move[sqr_flag / 11][occ1][i]) * 11)!=0) {
                        generate_move(move, sqr_flag, sqr_flag + ((horizontal_move[sqr_flag / 11][occ1][i]) * 11));
                        new_s.moves.sqr_from1 = sqr_flag;
                        new_s.moves.sqr_to1 = sqr_flag + ((horizontal_move[sqr_flag / 11][occ1][i]) * 11);
                        //APPLY
                        XOR(new_s.flag_board, s.flag_board, move);
                        COPY(new_s.my_board, s.my_board);//new_s.my_board=s.my_board;
                        COPY(new_s.other_board, s.other_board);//new_s.other_board=s.other_board;
                        l.push_back(new_s);
                        //printTable(new_s, type);
                    }
                }


            }
        }
    }
}

//**** Negamax search ****//
bool terminalNode(STATE s, std::list<STATE> l){
    if(l.size()==0) return true; //impossibilità di movimento è stato terminale
    TABLE victory_mask={0b1111111111110000000001100000000011000000000110000000001100000000, 0b0110000000001100000000011000000000110000000001111111111110000000};
    TABLE res;
    AND(res, s.flag_board, victory_mask);
    if((res[0]!=0 || res[1]!=0)||(s.flag_board[0]==0 && s.flag_board[1]==0)){ //la bandiera è arrivata a destinazione
        return true; //vittoria/sconfitta è stato terminale
    }
    return false;
}
int Evaluate(STATE s, bool type){
    TABLE victory_mask={0b1111111111110000000001100000000011000000000110000000001100000000, 0b0110000000001100000000011000000000110000000001111111111110000000};
    TABLE attack_mask={0b0000000000000000000000001111111000000000000000000000000000000000, 0b0000000000000000000000000011111110000000000000000000000000000000};
    TABLE res;
    int weight_flag=1;
    int weight_other=1;
    int weight_my=1;
    //todo tutti inf invertiti
    if(!type){ //io ho la bandiera
        AND(res, s.flag_board, victory_mask);
        if(res[0]!=0 || res[0]!=0){ //la bandiera è arrivata a destinazione
           return inf; //inf -> depth2
        }
        if(s.flag_board[0]==0 && s.flag_board[1]==0){ //la bandiera è stata catturata
            return _inf;
        }
    }
    else{ //io non ho la bandiera
        AND(res, s.flag_board, victory_mask);
        if(res[0]!=0 || res[0]!=0){ //la bandiera è arrivata a destinazione
            return _inf;
        }
        if(s.flag_board[0]==0 && s.flag_board[1]==0){ //la bandiera è stata catturata
            return inf;
        }
    }
    //todo ritorna il punteggio
    int my_pawns=countSetBits(s.my_board[0])+countSetBits(s.my_board[1]);
    int other_pawns=countSetBits(s.other_board[0])+countSetBits(s.other_board[1]);
    int eval=other_pawns*weight_other-my_pawns*weight_my+rand()%1;
    uint8_t mypawns[20];
    uint8_t num_pawns;
    int sum=0;
    num_pawns=getMyPawns(mypawns, s.other_board);
    for(int i=0; i<num_pawns; i++){
        sum+=abs(mypawns[i]-60);
    }
    sum=sum/60;
    if(!type){
        return (eval-sum);
    }
    else{
        //int dist=my
        return -(eval+sum);
    }


}

double my_play_time=0;
int get_depth_time(double curr_time){
    int rem = int((TIME-curr_time)*100/TIME);
    if(rem>=50){
        return DEEPTH;
    }
    else if(rem<50 && rem >=20){
        return 2;
    }
    else{
        return 1;
    }
}

void apply_move(STATE* s, int from, int to, bool mode, bool Im_player){
    TABLE move={0,0}, res={0,0};
    generate_move(move, from, to);
    AND(res, s->flag_board, move);
    if ((res[0] != 0 || res[1] != 0)&&(mode^Im_player)) { //ho la bandiera -> è una mossa bandiera
        XOR(s->flag_board, s->flag_board, move);
    }
    else { //non ho la bandiera o non è una mossa bandiera
        if (Im_player) {
            XOR(s->my_board, s->my_board, move);
            AND(res, move, s->other_board);
            XOR(s->other_board, s->other_board, res);
            AND(res, move, s->flag_board);
            XOR(s->flag_board, s->flag_board, res);
        } else {
            XOR(s->other_board, s->other_board, move);
            AND(res, move, s->my_board);
            XOR(s->my_board, s->my_board, res);
            AND(res, move, s->flag_board);
            XOR(s->flag_board, s->flag_board, res);
        }
    }

}

//**** Hashing and Transposition Table ****//
uint64_t Zobrist_table[120][3];
void Zobrist_init(){
    for(int i=0; i<120; i++){
        for(int j=0; j<3; j++){ //gold silver flag for every square
            Zobrist_table[i][j]=rand()%(120*3);
        }
    }
}
uint64_t Zobrist_hashing(STATE s, bool mode){
    TABLE occupancy={0,0};
    int h=0;
    uint64_t mask=0b1000000000000000000000000000000000000000000000000000000000000000;
    OR(occupancy, s.my_board, s.other_board);
    OR(occupancy, occupancy, s.flag_board);
    for(int i=0; i<120; i++){
        if(s.my_board[table_idx[i]]&mask){
            if(!mode) //mode=0 ho la flag-> sono gold
                h=h^Zobrist_table[i][0];
            else
                h=h^Zobrist_table[i][1];
        }
        else if(s.other_board[table_idx[i]]&mask){
            if(!mode) //mode=0 ho la flag-> sono gold
                h=h^Zobrist_table[i][1];
            else
                h=h^Zobrist_table[i][0];
        }
        else if(s.flag_board[table_idx[i]]&mask){
            h=h^Zobrist_table[i][2];
        }
        mask=mask>>1;
        if(mask==0){
            mask=0b1000000000000000000000000000000000000000000000000000000000000000;
        }
    }
    return h;
}

typedef struct{
    MOVE move;
    int score;
    int flag;
    int depth=0;
}TT;
std::map<uint64_t, TT> TT_table;
void get_from_TT(STATE s, bool mode, MOVE* TTMove, int* TTDepth, int* TTFlag, int* TTScore){
    TT tt;
    try {
        tt = TT_table.at(Zobrist_hashing(s, mode));
        *TTMove=tt.move;
        *TTDepth=tt.depth;
        *TTFlag=tt.flag;
        *TTScore=tt.score;
    }
    catch(std::exception &e){
        *TTDepth=-1;
    }
    return;
}
void put_into_TT(STATE s, bool mode, MOVE TTMove, int TTDepth, int TTFlag, int TTScore){
    TT tt, tt_;
    tt.move=TTMove;
    tt.depth=TTDepth;
    tt.flag=TTFlag;
    tt.score=TTScore;
    try {
        tt_ = TT_table.at(Zobrist_hashing(s, mode)); //if there is just an element -> choose the deepest
        if(TTDepth>tt_.depth){//the new is deeper -> insert, else do nothing
            TT_table.insert(std::pair<int, TT>(Zobrist_hashing(s, mode), tt));
        }
    }
    catch(std::exception &e){ //if it is a free position -> insert
        TT_table.insert(std::pair<int, TT>(Zobrist_hashing(s, mode), tt));
    }
    return;
}

//**** Version 1.0 simple ****//
int AlphaBeta(STATE s, int depth, int alpha, int beta, bool mode, int horizontal_move[][2048][10], MOVE* move){
    std::list<STATE> children;
    if(depth==0) return Evaluate(s, mode);
    TABLE tmp;
    MOVE bestMove;
    COPY(tmp, s.my_board);
    COPY(s.my_board, s.other_board);
    COPY(s.other_board, tmp);
    generatore_mosse(s, children, mode, horizontal_move);
    if(terminalNode(s, children)) return Evaluate(s, mode);
    //std::cout<<"Branching factor: "<<children.size()<<"\n";
    int score=_inf; //-inf
    int value;
    for(; children.size(); ){
        STATE child=children.front();
        children.pop_front();
        //std::cout<<s
        value=-AlphaBeta(child, depth-1, -beta, -alpha, !mode, horizontal_move, move);
        if(value>score) {
            score=value;
            bestMove=child.moves;
        }
        if(score>alpha) alpha=score;
        if(score>=beta) break;
    }
    *move=bestMove;
    return(score);
}
//**** Version 2.0 with TT table ****//
int AlphaBeta2(STATE s, int depth, int alpha, int beta, bool mode, int horizontal_move[][2048][10], MOVE* move){
    std::list<STATE> children;
    int old_alpha=alpha;
    TABLE tmp;
    int ttDepth, bestValue, ttFlag, ttScore, Done=0;
    MOVE ttMove, bestMove;
    get_from_TT(s, mode, &ttMove, &ttDepth, &ttFlag, &ttScore);
    if (ttDepth>=depth){
        if(ttFlag==0) return ttScore;
        else if(ttFlag==1) alpha=fmax(alpha, ttScore); //lower bound
        else if(ttFlag==2) beta=fmin(beta, ttScore);
        if (alpha>=beta) return ttScore;
    }
    if(depth==0) return Evaluate(s, mode);
    if(ttDepth>=0){ //ttMove as first
        //apply TTmove and undo
        apply_move(&s, ttMove.sqr_from1,ttMove.sqr_to1, mode, true);
        apply_move(&s, ttMove.sqr_from2,ttMove.sqr_to2, mode, true);
        COPY(tmp, s.my_board);
        COPY(s.my_board, s.other_board);
        COPY(s.other_board, tmp);
        bestValue= -AlphaBeta2(s, depth-1, -beta, -alpha, !mode, horizontal_move, move);
        COPY(tmp, s.my_board);
        COPY(s.my_board, s.other_board);
        COPY(s.other_board, tmp);
        apply_move(&s, ttMove.sqr_from2,ttMove.sqr_to2, mode, true);
        apply_move(&s, ttMove.sqr_from1,ttMove.sqr_to1, mode, true);
        bestMove=ttMove;
        if (bestValue>=beta) Done=1;
    }
    else bestValue=_inf;
    int score=_inf; //-inf
    if(!Done){
    COPY(tmp, s.my_board);
    COPY(s.my_board, s.other_board);
    COPY(s.other_board, tmp);
    generatore_mosse(s, children, mode, horizontal_move);
    //std::cout<<children.size()<<"\n";
    if(terminalNode(s, children)) return Evaluate(s, mode);
    int value;
    for(; children.size(); ){
        STATE child=children.front();
        children.pop_front();
        if(!compare2moves(ttMove, child.moves)){
            alpha=fmax(bestValue, alpha);
            value=-AlphaBeta2(child, depth-1, -beta, -alpha, !mode, horizontal_move, move);
            if(value>bestValue){
                bestValue=value;
                bestMove=child.moves;
                ttMove = child.moves;
                if(bestValue>=beta) {
                    break;
                }
            }
        }
    }
    }
    COPY(tmp, s.my_board);
    COPY(s.my_board, s.other_board);
    COPY(s.other_board, tmp);
    if(bestValue<=old_alpha) ttFlag=2; //upper bound
    else if(bestValue>=beta) ttFlag=1; //lowerbound
    else ttFlag=0; //exact value
    put_into_TT(s, mode, ttMove, ttDepth, ttFlag, ttScore);
    *move=bestMove;
    return(bestValue);
}


void AlphaBetaWrapper(STATE s, bool type, MOVE* move, int horizontal_move[][2048][10]){
    std::cout<<"Starting searching with depth "<<get_depth_time(my_play_time);
    TABLE tmp;
    COPY(tmp, s.my_board); //switch the board to change the player that move
    COPY(s.my_board, s.other_board);
    COPY(s.other_board, tmp);
    int beta=inf; int alpha=_inf, score; //iterative deepening with searching windows
    for(int depth=1; depth<=get_depth_time(my_play_time); depth++){
        score=AlphaBeta2(s, depth, alpha, beta, type, horizontal_move, move);
        beta=score+1; alpha=score-1; //searching windows lose or capture 1 pawns

    }

    //std::cout<<score<<"score\n";
    COPY(tmp, s.my_board);
    COPY(s.my_board, s.other_board);
    COPY(s.other_board, tmp);
}

#endif //BREAKTHUV1_0_SEARCH_ENGINE_H
