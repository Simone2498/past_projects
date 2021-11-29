#include <iostream>
#include "./search_engine.h"
#include <cstring>
#include <chrono>
#include <string>



int chess_to_sqr(char* move){
    int num; char ch;
    sscanf(move, "%c%d", &ch, &num);
    return (11-num)*11+(ch-97);
}
int chess_to_sqr_reverse(char* move){ //input for opponent with switched numbers
    int num; char ch;
    sscanf(move, "%c%d", &ch, &num);
    return (num-1)*11+(ch-97);
}
void sqr_to_chess(int sqr, char* move){
    char ch=(sqr%11)+97;
    int num=(11-sqr/11);
    sprintf(move, "%c%d", ch, num);
}
void sqr_to_chess_reverse(int sqr, char* move){ //output for opponent with switched numbers
    char ch=(sqr%11)+97;
    int num=1+sqr/11;
    sprintf(move, "%c%d", ch, num);
}
void move(STATE* s, char* str_move, bool Im_player, bool mode){
    char* m;
    int from, to;
    TABLE move={0,0}, res={0,0};
    m=strtok(str_move, "x");
    from=chess_to_sqr(m);
    m=strtok(NULL, "x");
    to=chess_to_sqr(m);
    apply_move(s, from, to, mode, Im_player);
}
void move_reverse(STATE* s, char* str_move, bool Im_player, bool mode){
    char* m;
    int from, to;
    TABLE move={0,0}, res={0,0};
    m=strtok(str_move, "x");
    from=chess_to_sqr_reverse(m);
    m=strtok(NULL, "x");
    to=chess_to_sqr_reverse(m);
    apply_move(s, from, to, mode, Im_player);
}



int main() {
        MOVE new_move;
        STATE s, last_state;

        s.flag_board[0]=  0b0000000000000000000000000000000000000000000000000000000000001000;
        s.flag_board[1]=  0b0000000000000000000000000000000000000000000000000000000000000000;
        //Inizio del programma di ricerca della mossa//
        char str_moves[8], str_move[4], str_move_[4], str_move1[4], str_move1_[4];
        char cmd; bool mode=false;
        std::cout<<"Your game's mode: gold(flag)->0 or silver->1: ";
        std::cin>>mode;
        std::cout<<"\n";
        if(!mode){ //I have the flag
            s.other_board[0]=    0b0000000000000011111000000000000000100000001001000000010010000000;
            s.other_board[1]=    0b1001000000010010000000100000000000000011111000000000000000000000;
            s.my_board[0]= 0b0000000000000000000000000000000000000111000000010001000000100010;
            s.my_board[1]= 0b0000010001000000011100000000000000000000000000000000000000000000;
            std::cout<<"You are the 0 pawns";
        }
        else{
            s.my_board[0]=    0b0000000000000011111000000000000000100000001001000000010010000000;
            s.my_board[1]=    0b1001000000010010000000100000000000000011111000000000000000000000;
            s.other_board[0]= 0b0000000000000000000000000000000000000111000000010001000000100010;
            s.other_board[1]= 0b0000010001000000011100000000000000000000000000000000000000000000;
            std::cout<<"You are the X pawns";
        }

        int horizontal_move[11][2048][10]; //sqr+this=to from=sqr
        horizontal_move_generator(horizontal_move);
        Zobrist_init();

        printTable(s, mode);
        std::chrono::system_clock::time_point start, end;
        std::chrono::duration<double> diff;
        double diff_;
        while (1) {
            std::cout<<"Game's menu:\n\t1. My turn\n\t2. Other turn\n\t3. Undo\n\t4. Exit\n->"; //	5. My turn manual6. My move reverse7. Other turn reverse

            std::cin>>cmd;
            switch(cmd){
                case '1': //my move

                    start = std::chrono::system_clock::now();

                    last_state=s;
                    AlphaBetaWrapper(s, mode, &new_move, horizontal_move);
                    if(new_move.sqr_from1!=-1&&new_move.sqr_to1!=-1) {
                        sqr_to_chess(new_move.sqr_from1, str_move);
                        sqr_to_chess(new_move.sqr_to1, str_move_);
                        std::cout<<"\nMove: "<<str_move<<"x"<<str_move_;
                        sprintf(str_moves, "%sx%s", str_move, str_move_);
                        move(&s, str_moves, 1, mode);
                    }
                    if(new_move.sqr_from2!=-1&&new_move.sqr_to2!=-1) {
                        sqr_to_chess(new_move.sqr_from2, str_move);
                        sqr_to_chess(new_move.sqr_to2, str_move_);
                        std::cout<<"\nMove: "<<str_move<<"x"<<str_move_;
                        sprintf(str_moves, "%sx%s", str_move, str_move_);
                        move(&s, str_moves, 1, mode);
                    }
                    printTable(s, mode);
                    if(Evaluate(s, mode)==inf){
                        std::cout<<"......You have WIN!......\n";
                    }
                    if(Evaluate(s, mode)==_inf){
                        std::cout<<"......You have LOSE!......\n";
                    }

                    end = std::chrono::system_clock::now();
                    diff = (end-start);
                    diff_=diff.count();
                    my_play_time+=diff_;//.count();
                    std::cout <<"Remaining time:"<< int((TIME-my_play_time)*100/TIME) <<"% "<< diff.count() <<"s\n";

                    break;
                case '2': //other move
                    std::cout << "Insert the move:";
                    std::cin >> str_moves;
                    last_state=s;
                    move(&s, str_moves, 0, mode);
                    printTable(s, mode);
                    if(Evaluate(s, mode)==inf){
                        std::cout<<"......You have WIN!......\n";
                    }
                    if(Evaluate(s, mode)==_inf){
                        std::cout<<"......You have LOSE!......\n";
                    }
                    diff_=-1;
                    break;
                case '3': //undo
                    s=last_state;
                    printTable(s, mode);
                    if(diff_!=-1){
                        my_play_time-=diff_;
                    }
                    break;
                case '4': //exit
                    return 0;
                    break;
                // utility to face abnormal situations
                case '5':
                    std::cout << "Insert the move:";
                    std::cin >> str_moves;
                    last_state=s;
                    move(&s, str_moves, 1, mode);
                    printTable(s, mode);
                    break;
                case '6': //only in case of an opponent with a switched number axis
                    last_state=s;
                    AlphaBetaWrapper(s, mode, &new_move, horizontal_move);
                    if(new_move.sqr_from1!=-1&&new_move.sqr_to1!=-1) {
                        sqr_to_chess(new_move.sqr_from1, str_move);
                        sqr_to_chess(new_move.sqr_to1, str_move_);
                         sqr_to_chess_reverse(new_move.sqr_from1, str_move1);
                         sqr_to_chess_reverse(new_move.sqr_to1, str_move1_);
                         std::cout<<"\nMove: "<<str_move1<<"x"<<str_move1_;
                        //std::cout<<"\nMove: "<<str_move<<"x"<<str_move_;
                        sprintf(str_moves, "%sx%s", str_move, str_move_);
                        move(&s, str_moves, 1, mode);
                    }
                    if(new_move.sqr_from2!=-1&&new_move.sqr_to2!=-1) {
                        sqr_to_chess(new_move.sqr_from2, str_move);
                        sqr_to_chess(new_move.sqr_to2, str_move_);
                         sqr_to_chess_reverse(new_move.sqr_from2, str_move1);
                         sqr_to_chess_reverse(new_move.sqr_to2, str_move1_);
                         std::cout<<"\nMove: "<<str_move1<<"x"<<str_move1_;
                        //std::cout<<"\nMove: "<<str_move<<"x"<<str_move_;
                        sprintf(str_moves, "%sx%s", str_move, str_move_);
                        move(&s, str_moves, 1, mode);
                    }
                    printTable(s, mode);
                    if(Evaluate(s, mode)==inf){
                        std::cout<<"......You have WIN!......\n";
                    }
                    if(Evaluate(s, mode)==_inf){
                        std::cout<<"......You have LOSE!......\n";
                    }
                    break;
                case '7': //only in case of an opponent with a switched number axis
                    std::cout << "Insert the move:";
                    std::cin >> str_moves;
                    last_state=s;
                    move_reverse(&s, str_moves, 0, mode);
                    printTable(s, mode);
                    break;
                case '8':

                    break;
                default:
                    std::cout<<"Comando errato\n";
            }
        }
}
