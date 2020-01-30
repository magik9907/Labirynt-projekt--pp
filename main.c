#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <conio.h>

// sluzy do definowania 2 wymiarowej tablicy w ktorej sa zapisane informacje o kazdym elemencie mapy
typedef struct {
    //okresla jaki znak jest zapisany
    char mark;
    //definiuje w ktore strony mozna sie poruszac t- mozna f- nie mozna
    char up;
    char down;
    char left;
    char right;
    char cross;
    //stosowany dosprawdzania czy pole zostalo wykorzystane
    int isUsed;
} Map;

//opsuje obecna mape
typedef struct {
    //rozmiary mapy
    short x;
    short y;
    //przechowuje punkty startowe i koncowe mapy
    short startX;
    short startY;
    short endX;
    short endY;
    //przechowuje informacje czy mapa posiada poczatek i koniec, slyzy do sprawdzania bledu mapy
    short isEnd;
    short isStart;
} Desc;

typedef struct  MOVE{
    char step;
    struct MOVE *prev;
    struct MOVE *next;
}   Move;

//zwraca wygenerowana mape o wymiarach (x,y)
Map** generateFromFile (FILE *f, Desc *desc );
//funckja wyszukujaca sciezke od startu do mety
void searchRoad (Map **map, Desc *desc);
//sluzy do opisu konkrentej pozycji na mapie
void mapDesc(Map **map,Desc *desc, int x, int y);
//tworzy liste Move
Move* createNode (char step);
//wyswietla wynik programu
void printRoad(Desc *desc, Map **map, Move *move);
//funkcja wykonywania ruchu
Move* step(char s,Move *move,Map **map, int *y, int *x);
//umozliwia tworzenie mapy przez uzytkowanika
Map** createMapByUser(Desc* desc);
//funkcja wyswietlaja wpomaga funkcje createMapByUser
void printForGenerator(Map** map, Desc* desc, int x, int y);
void saveToFile(Map** map, Desc* desc);

int main()
{
    int l;
    char *file;
    char opt;
    int status = 0;

    Map **map = NULL;
    Desc *desc = (Desc*)malloc (sizeof(Desc));

    do{
        file = (char* )malloc (256 * sizeof(char));
        printf("Podaj sciezke do pliku lub\n jesli chcesz podac recznie mape wpisz: MAP\n Do zakonczenia programu wpisz: EXIT.\n Potwierdz klikajac ENTER\n ");
        fflush(stdin);
        gets(file);
        system("cls");
        if(strcmp(file, "EXIT\0") == 0 || !strcmp(file, "exit\0") ){
            status = 1;
        }else if(!strcmp(file, "MAP\0") || !strcmp(file, "map\0") ){
            map = createMapByUser(desc);
            system("cls");
            printf("chcesz zapisac do pliku? t/n\n");
            fflush(stdin);
            scanf("%c",&opt);
            if(opt == 't' || opt == 'T')
                saveToFile(map, desc);
            system("cls");
            status = 0;
        }else{

            status = 0;
            FILE *fp = fopen(file , "r");

            if(fp == NULL){
                printf("nie znaleziono pliku\n\n");
                status = 3;
            }else{
                map = generateFromFile (fp, desc );
            }
            fclose(fp);
            free(file);
        }

        if(status != 1 && status != 3){
            if(desc -> isStart == 0 || desc -> isEnd == 0){
                printf("mapa posiada bledy\n");
            }else{
                searchRoad(map, desc);
            }
        }

    }while(status != 1);

    for(l=0; l<desc->y; l++){
        free(map[l]);
    }
    free(desc);
    free(map);
    return 0;
}


Map** generateFromFile (FILE *f, Desc *desc ){
    int x, y, i, l;
    fscanf(f, "%i %i", &x, &y);
    desc -> isStart = 0;
    desc -> isEnd = 0;
    desc -> x = x;
    desc -> y = y;
    Map **map;
    map = (Map**) malloc (x * y * sizeof(Map*));

    for (i = 0; i<y; i++)
        map[i] =(Map*) malloc(x * sizeof(Map));

    for (i = 0; i<y; i++)
        for (l = 0; l<x; l++){
            map[i][l].mark=fgetc(f);
            map[i][l].isUsed = 0;

            switch(map[i][l].mark){
            case '#':
                break;
            case '\n':
                l--;
                break;
            case '$':
                desc -> startX = l;
                desc -> startY = i;
                desc -> isStart = 1;
                break;
            case '@':
                desc -> endX = l;
                desc -> endY = i;
                desc -> isEnd = 1;
                break;
            }
        }
    return map;
}

Move* step(char s,Move *move,Map **map, int *y, int *x){
    Move *current;
    Move *previous = move;

    switch(s){
    case 'G':
        current = createNode('G');
        map[*y][*x].isUsed = 1;
        (*y)--;
        break;
    case 'D':
        current = createNode('D');
        map[*y][*x].isUsed = 1;
        (*y)++;
        break;
    case 'L':
        current = createNode('L');
        map[*y][*x].isUsed = 1;
        (*x)--;
        break;
    case 'P':
        current = createNode('P');
        map[*y][*x].isUsed = 1;
        (*x)++;
        break;
    case 'K':
        current = createNode('K');
    }
    previous -> next = current;
    current -> prev = previous;
    return current;
}

void searchRoad (Map **map, Desc *desc){
    int x = desc -> startX;
    int y = desc -> startY;

    Move *move = createNode('S');
    Move *current = move;

    int dx;
    int dy;
    int makeStepBack = 0;
    do{
        if( !map[y][x].isUsed )
            mapDesc(map, desc, x, y);
        //move when position is on start
       if( map[y][x].isUsed == 0 && makeStepBack == 0){
            dx = desc->endX - x;
            dy = desc->endY - y;

        //making move forward
            if(dy<=0 && map[y][x].up ==  't' && map[y-1][x].isUsed != 1){
            //gora
               current = step('G', current, map, &y, &x);
            }else if(dy >=0 && map[y][x].down ==  't' && map[y+1][x].isUsed != 1){
           //  ruch w dol
                current = step('D', current, map, &y, &x);
            }else if(dx>=0 && map[y][x].right ==  't' && map[y][x+1].isUsed != 1){
             //   prawo
                current = step('P', current, map, &y, &x);
            }else if(dx<=0 && map[y][x].left ==  't' && map[y][x-1].isUsed != 1){
            //lewo
                current = step('L', current, map, &y, &x);
            }else{
              //  pokolei
                if(map[y][x].up ==  't' && map[y-1][x].isUsed != 1){
                    current = step('G', current, map, &y, &x);
                }else if(map[y][x].down == 't' && map[y+1][x].isUsed != 1){
                    current = step('D', current, map, &y, &x);
                }else if(map[y][x].right == 't' && map[y][x+1].isUsed != 1){
                    current = step('P', current, map, &y, &x);
                }else if(map[y][x].left == 't' && map[y][x-1].isUsed != 1){
                    current = step('L', current, map, &y, &x);
                }else{
                    if(map[y][x].mark == '$'){
                        printf("Brak przejscia\n");
                        return ;
                    }
                    makeStepBack = 1;
                    map[y][x].isUsed = 1;
                }
            }

        }else{
            //making move back to nearest cross
            do{
                current = current -> prev;
                switch(current -> next -> step){
                case 'G':
                    y++;
                    map[y-1][x].isUsed = 1;
                    break;
                case 'P':
                    x--;
                    map[y][x+1].isUsed = 1;
                    break;
                case 'L':
                    x++;
                    map[y][x-1].isUsed = 1;
                    break;
                case 'D':
                    y--;
                    map[y+1][x].isUsed = 1;
                    break;
                }
                free( current -> next);
                current -> next = NULL;

            }while(map[y][x].cross == 'f' && map[y][x].mark != '$');
            makeStepBack = 0;
            map[y][x].isUsed = 0;
        }
    }while(map[y][x].mark != '@');

    current = step('K', current, map, &y, &x);
    printRoad(desc, map, move);
    return ;
}

void mapDesc(Map **map, Desc *desc, int x, int y){
    int crossCount = 0;

    //setting which way can be next move
    map[y][x].left = (x > 0 && map[y][x - 1].mark != '#')?'t':'f';
    map[y][x].right = (x < desc -> x - 1  && map[y][x + 1].mark != '#')?'t':'f';
    map[y][x].up = (y > 0 && map[y - 1][x].mark != '#')?'t':'f';
    map[y][x].down = (y < desc -> y - 1 && map[y + 1][x].mark != '#')?'t':'f';

    //checking is map[y][x] is a crossing
    if(map[y][x].up == 't') crossCount++;
    if(map[y][x].down == 't') crossCount++;
    if(map[y][x].left == 't') crossCount++;
    if(map[y][x].right == 't') crossCount++;

    //t- is cross, f- isn't cross, e-end of road (move to the last cross)
    map[y][x].cross = (crossCount > 2)?'t':(crossCount == 1)?'e':'f';
}

Move* createNode(char nodeStep){

    Move *node = (Move*) malloc (sizeof(Move));
    node -> step = nodeStep;
    node -> prev = NULL;
    node -> next = NULL;
    return node;
}

void printRoad(Desc *desc, Map **map,Move *move){
    int i,l;
    int x = desc -> x, y = desc -> y;
    for(i=0; i<y; i++){
        for(l=0; l<x; l++){
            printf("%c", map[i][l].mark);
        }
        printf("\n");
    }

    Move *temp = move;

    printf("wykonane ruchy\n");

    do{
        printf("%c", temp -> step);
        temp = temp -> next;
    }while(temp->next != NULL);

    printf("%c\n\n", temp->step);

    while(temp -> prev != NULL){
            temp = temp ->prev;
            free(temp->next);
            temp->next = NULL;
    }
    free(temp);
}

Map** createMapByUser(Desc* desc){
    printf("Podaj wymiary mapy w formacie: SZEROKOSC WYSOKOSC\n");
    int x, y, i, l, m = 0, n = 0, end = 0;
    char s;
    scanf("%d %d",&x, &y);
    system("cls");
    desc -> isStart = 0;
    desc -> isEnd = 0;
    desc -> x = x;
    desc -> y = y;

    Map **map;
    map = (Map**) malloc (x * y * sizeof(Map*));

    for (i = 0; i<y; i++)
        map[i] =(Map*) malloc(x * sizeof(Map));

    for (i = 0; i<y; i++){
        for (l = 0; l<x; l++){
            map[i][l].mark = '#';
            map[i][l].isUsed = 0;
        }
    }

    while(end == 0){
        printForGenerator(map, desc, n , m);
        printf("\nX - obecna pozycja do zmiany;\nW - ruch w gore;\nS - ruch w dol;\nA - ruch w lewo;\nD  - ruch w prawo\n \nE - wyjscie z kreatora;\n $ -start;\n @ - koniec;\n . -droga;\n # lub (pole puste) -sciana\n");
        fflush(stdin);
        s=getc(stdin);
        system("cls");
        switch(s){
        case 'W':
        case 'w':
            if(m>0)
                m--;
            break;
        case 'S':
        case 's':
            if(m<y-1)
                m++;
            break;
        case 'd':
        case 'D':
            if(n<x-1)
                n++;
            break;
        case 'A':
        case 'a':
            if(n>0)
                n--;
            break;
        case '\n':
        case ' ':
            if(n<x-1)
                n++;
            else {
                n=0;
                if(m<y-1)
                    m++;
            }
            break;
        case '#':
            map[m][n].mark = '#';
            if(n<x-1)
                n++;
            else {
                n=0;
                if(m<y-1)
                    m++;
            }
            break;
        case '$':
            if(desc -> isStart == 1){
                printf("start juz istnieje");
                break;
            }
            desc -> startX = n;
            desc -> startY = m;
            desc -> isStart = 1;
            map[m][n].mark = s;
            if(n<x-1)
                n++;
            else {
                n=0;
                if(m<y-1)
                    m++;
            }
            break;
        case '@':
            if(desc -> isEnd == 1){
                printf("koniec juz istnieje");
                break;
            }
            desc -> endX = n;
            desc -> endY = m;
            desc -> isEnd = 1;
            map[m][n].mark = s;
            if(n<x-1)
                n++;
            else {
                n=0;
                if(m<y-1)
                    m++;
            }
            break;
        case '.':
            map[m][n].mark = s;
            if(n<x-1)
                n++;
            else {
                n=0;
                if(m<y-1)
                    m++;
            }
            break;
        case 'E':
        case 'e':

            if(desc ->isEnd == 1 && desc ->isStart == 1){
                end = 1;
                break;
            }
            printf("mapa musi miec start ($) i mete (@) \n Czy chcesz wyjsc: t/n\n");
            fflush(stdin);
            scanf("%c",&s);
            if(s == 't' || s == 'T') end = 1;
            break;
        default:
            printf("Nieprawidlowy znak\n");
            break;
        }

    }
    return map;
}

void printForGenerator(Map** map, Desc* desc, int x, int y){
int m, n, my = desc->y, nx = desc->x;
    for(m = 0; m < my ; m++){
        printf("\n");
        for(n = 0; n < nx; n++){
            if(m == y && n == x) printf("X");
            else printf("%c", map[m][n].mark);
        }
    }
}

void saveToFile(Map** map, Desc* desc){
    printf("Podaj nazwe pliku do zapisu:\n");
    char file[100];
    fflush(stdin);
    scanf("%s",file);
    FILE *f = fopen(file,"w");
    if(f == NULL){
        printf("blad otwarcia pliku");
        system("pause");
        return ;
    }
    fprintf(f,"%i %i", desc->x, desc->y);

    int m, n, my = desc->y, nx = desc->x;
    for(m = 0; m < my ; m++){
        fprintf(f,"\n");
        for(n = 0; n < nx; n++){
            fprintf(f,"%c", map[m][n].mark);
        }
    }
    fclose(f);
}