#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>

#define MAX 5 //Nombre qu'il faut aligner au maximum pour gagner
typedef struct Board Board;
typedef struct Player Player;

struct Board {
    int size; // size*size = taille board
    int **board; // Matrice pour le board
    int stop; // Boolean si la partie est terminé
    int tour; // Le tour de la personne qui doit jouer, -1 si la partie est terminé
    int max; // Nombre maximum de coups consécutifs à remporter sur chaque ligne, colonne ou diagonale
    int *coups; // La liste des coups
    int cursor; // Le nombre de coup joué
    int *coupPossible; //liste des coups possibles
};

struct Player {
    int num; // nombre de joueur 1 ou 2
    int IA; // 0: humain, 1: IA simple, 2: IA difficile
    int simulation; // une simulation de l'historique du jeu ou un jeu en cours
    Board *board;
};

// Mutex pour l'accès au plateau
pthread_mutex_t board_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * Créer un plateau de jeu
 * @param size taille de la ligne
 * @return le plateau du jeu
 */
Board* createBoard(int size);
/**
 * Copier une partie (utilisé pour le minimax)
 * @param p la partie à copier
 * @return la copy de la partie passé en paramètre
 */
Board *copyBoard(Board *p);
/**
 * Supprimer en mémoire le plateau
 * @param p plateau du jeu à supprimer
 */
void deleteBoard(Board *p);
/**
 * Sauvegarder le plateau dans un fichier
 * @param pathname le fichier où sauvegarder le fichier
 * @param p le plateau qu'il faut sauvegarder
 */
void saveBoard(char *pathname, Board *p);
/**
 * Charger le plateau stocké dans un fichier
 * @param pathname fichier ou charger le plateau
 * @return le plateau
 */
Board *loadBoard(char *pathname);
/**
 * imprimer dans la console la plateau et indiquer le vainqueur si la partie est terminé
 * @param p le plateau à afficher
 */
void printBoard(Board *p);
/**
 * Créer un joueur
 * @return retourne le joueur crée
 */
Player *createPlayer(int num, int simulation, int IA, Board *partie);
/**
 * Demander au joueur humain de jouer un coup
 * @param p le plateau du jeu
 * @return le coup choisis par le joueur
 */
int playerMove(Board *p);
/**
 * Jouer un coup
 * @param p le plateau où il faut jouer le coup
 * @param coup le coup à joué
 * @param historique boolean, si true alors on sauvegarde dans l'historique, sinon on ne sauvegarde pas le coup dans l'historique
 * @return un boolean, true si le coup à été joué, false sinon
 */
int playMove(Board *p, int coup, int historique);
/**
 * vérifier si la partie est terminé -> vérifie la colonne
 * @param p le plateau
 * @param coup le coup qui à été joué par le joueur
 * @return un boolean, true si la partie est terminé, false sinon
 */
int checkColonne(Board *p, int coup);
/**
 * vérifier si la partie est terminé -> vérifie la ligne
 * @param p le plateau
 * @param coup le coup qui à été joué par le joueur
 * @return un boolean, true si la partie est terminé, false sinon
 */
int checkLine(Board *p, int coup);
/**
 * vérifier si la partie est terminé -> vérifie la diagonale
 * @param p le plateau
 * @param coup le coup qui à été joué par le joueur
 * @return un boolean, true si la partie est terminé, false sinon
 */
int checkDiagonal(Board *p, int coup);
/**
 * vérifier si la partie est terminé
 * @param p le plateau
 * @param coup le coup qui à été joué par le joueur
 * @return un boolean, true si la partie est terminé, false sinon
 */
int checkVictory(Board *p, int coup);
/**
 * supprime le coup sélectionné dans p->coupPossible
 * @param p la partie ou le coup doit etre supprimé
 * @param start l'emplacement du coup à supprimé
 */
void selectMoveID(Board *p, int start);
/**
 * Récupère l'emplacement du coup puis appel la fonction selectmoveID
 * @param p la partie ou le coup doit être sélectionné
 * @param coup le coup qu'il faut sélectionné
 */
void selectMove(Board *p, int coup);
/**
 * récuperer un coup aléatoire qui est jouable (utilisé pour l'ia de niveau facile)
 * @param p le plateau du jeu
 * @return le coup à jouer
 */
int getRandomMove(Board *p);
/**
 * Arbre Minimax pour IA difficile
 * @param p le plateau du jeu
 * @param idJoueur l'id de l'ia qui utilise MiniMax
 * @param deep la profondeur de l'arbre actuel
 * @return le coup à joué si deep = 0 sinon retour le score
 */
int MiniMax(Board *p, int idJoueur, int deep);
/**
 * cette fonction est appelé par chaque thread pour gérer chaque joueur et protéger avec un mutex le plateau
 * @param playerarg la structure player
 */
void *play(void *playerarg);
/**
 * fonction utilisé pour démarrer une partie, elle va créer 2 threads et chaque thread va joué l'un contre l'autre dans la fonction play
 * @param p la plateau ou la partie doit être jouer
 * @param typeP1 le type du joueur 1
 * @param typeP2 le type du joueur 2
 * @param simulation boolean, true si la partie est une simulation, false sinon
 * @param schedulerP1 type du scheduleur pour le joueur 1
 * @param schedulerP2 type du scheduleur pour le joueur 2
 * @param seed seed de la partie (-1 si seed aléatoire)
 * @param nb utilisé si plusieurs partie est lancé en meme temps pour éviter de jouer plusieurs fois la meme partie
 * @return le gagnant de la partie, -1 si égalité
 */
int startGame(Board *p, int typeP1, int typeP2, int simulation, int schedulerP1, int schedulerP2, int seed, int nb);
/**
 * Menu pour changer les paramètres sur un joueur
 * @param scheduler utilisé pour changer le scheduler du thread
 * @param type utilisé pour changer le type d'IA du joueur (humain, facile(IA), difficile(IA))
 */
void player_gui(int *scheduler, int *type);

/*
 * Fonction principale
 */
int main() {
    struct timeval debut, fin;
    double temps = 0;
    int size = 3;
    int nbPartie = 0;
    int c1 = 0;
    int stop = 1;
    int tmp = -2;
    int player;
    char s[50];
    int schedulerP1 = SCHED_OTHER, schedulerP2 = SCHED_OTHER;
    int typeP1 = 1, typeP2 = 2;
    int seed = -1;
    Board *p = createBoard(size);
    while (stop) {
        setbuf(stdout, NULL);
        fprintf(stdout,"\nVeuillez sélectionner votre choix: \n"
                       "    1. Changer taille (actuellement %d)\n"
                       "    2. Changer seed\n"
                       "    3. Changer paramètres joueur (défault, typeJ1: %d; typeJ2: %d)\n"
                       "    4. Démarrer board\n"
                       "    5. Sauvegarder dernière board\n"
                       "    6. Simuler board\n"
                       "    7. Lancer plusieurs parties\n"
                       "    8. Stop\n", size, typeP1, typeP2);
        printf("-- Veuillez sélectionner votre choix: ");
        scanf("%d", &c1);
        switch (c1) {
            case 1: // TAILLE
                printf("----------------\n");
                printf("Indiquer la taille: ");
                memcpy(&size, &tmp, sizeof(int));
                scanf("%d", &size);
                if(tmp == size) {
                    fprintf(stderr,"    X [Choix Invalide!]\n");
                    size = 3;
                    while (getchar() != '\n');
                } else if (size < 3) {
                    fprintf(stderr,"    X [Choix Invalide, la taille doit être supérieur à 2!]\n");
                    size = 3;
                } else {
                    printf("    ==> [La taille à été modifié!]\n");
                }
                printf("----------------\n");
                break;
            case 2: //seed
                printf("----------------\n");
                setbuf(stdout, NULL);
                printf("Indiquer la seed (-1 pour mettre par défaut): ");
                memcpy(&seed, &tmp, sizeof(int ));
                scanf("%d", &seed);
                if (tmp == seed) {
                    fprintf(stderr,"    X [Choix Invalide, la seed doit être un nombre!]\n");
                    seed = -1;
                    while (getchar() != '\n');
                } else {
                    printf("    ==> [La seed à été modifié!]\n");
                }
                printf("----------------\n");
                break;
            case 3: // PARAMETRE JOUEUR
                printf("----------------\n");
                setbuf(stdout, NULL);
                printf("Indiquez le joueur à modifier: ");
                memcpy(&player, &tmp, sizeof(int));
                scanf("%d", &player);
                if (tmp == player) {
                    fprintf(stderr,"    X [Choix Invalide!]\n");
                    while (getchar() != '\n');
                } else if (player < 1 || player > 2) {
                    fprintf(stderr,"    X [Choix Invalide, le numéro du joueur doit être entre 1 et 2.]\n");
                } else {
                    printf("    ==> [Le joueur %d à été sélectionné!]\n", player);
                    if (player == 1)
                        player_gui(&schedulerP1, &typeP1);
                    else
                        player_gui(&schedulerP2, &typeP2);
                }
                printf("----------------\n");
                break;
            case 4: // START
                deleteBoard(p);
                p = createBoard(size);
                startGame(p, typeP1, typeP2, 0, schedulerP1, schedulerP2, seed, 0);
                break;
            case 5: //SAVE PARTIE
                printf("----------------\n");
                printf("Indiquez le nom du fichier où sauvegarder la board (Maximum 50 caractères): ");
                scanf("%s", s);
                printf("    [... Sauvegarde de la board dans le fichier %s...]\n", s);
                saveBoard(s, p);
                printf("----------------\n");
                break;
            case 6: //Simuler PARTIE
                printf("----------------\n");
                printf("Indiquez le nom du fichier de la board à simuler (Maximum 50 caractères): ");
                scanf("%s",s);
                printf("... Chargement de la board: %s...\n",s);
                p = loadBoard(s);
                startGame(p, 0, 0, 1, schedulerP1, schedulerP2, seed, 0);
                printf("----------------\n");
                break;
            case 7: // Lancer plusieurs parties et analyse
                printf("----------------\n");
                printf("Indiquez le nombre de board à lancer: ");
                memcpy(&nbPartie, &tmp, sizeof(int));
                scanf("%d", &nbPartie);
                if(tmp == nbPartie){
                    fprintf(stderr,"    X [Choix Invalide!]\n");
                    while (getchar() != '\n');
                } else if (nbPartie < 0){
                    fprintf(stderr,"    X [Choix Invalide, le nombre de joueur doit supérieur à 0.]\n");
                } else{
                    printf("Lancement de %d parties...\n", nbPartie);
                    int g, p1 = 0, p2 = 0, n = 0;
                    temps = 0;
                    for (int i = 0; i < nbPartie; ++i) {
                        deleteBoard(p);
                        p = createBoard(size);
                        gettimeofday(&debut, NULL);
                        if ((g = startGame(p, typeP1, typeP2, 0, schedulerP1, schedulerP2, seed, i)) == 1) {
                            p1++;
                        } else if(g == 2) {
                            p2++;
                        } else {
                            n++;
                        }
                        gettimeofday(&fin, NULL);
                        temps += (fin.tv_sec - debut.tv_sec) * 1000.0 + (fin.tv_usec - debut.tv_usec) / 1000.0; // Calculez le temps écoulé en millisecondes
                    }
                    printf("----------------------------------------------------------------\n");
                    printf("Paramètre de la board: \n"
                           " + Seed: %d\n"
                           " + Type Joueur 1: %d\n"
                           " + Type Joueur 2: %d\n"
                           " + Scheduler Joueur 1: %d\n"
                           " + Scheduler Joueur 2: %d\n", seed, typeP1, typeP2, schedulerP1, schedulerP2);
                    printf("-> Temps d'exécution des parties : %f millisecondes\n", temps);
                    printf("-> Moyenne du temps d'exécution : %f millisecondes\n", temps/nbPartie);
                    printf("-> Le joueur 1 à gagné %d board, le joueur 2 %d board et il y a eu %d board nulle\n", p1, p2, n);
                    printf("----------------------------------------------------------------\n");
                }
                break;
            case 8: // STOP
                setbuf(stdout, NULL);
                printf("\n    [Arret du programme!]\n");
                stop = 0;
                break;
            default:
                setbuf(stdout, NULL);
                fprintf(stderr,"    X [Choix Invalide!]\n");
                while (getchar() != '\n');

        }
    }
    deleteBoard(p);

    return 0;
}

Board *createBoard(int size){
    Board *p;
    if((p =  (Board *)malloc(sizeof(Board))) == NULL){
        fprintf(stderr,"Malloc error\n");
        return NULL;
    }
    if((p->coups = malloc(sizeof(int)*size*size)) == NULL){
        free(p);
        fprintf(stderr,"Malloc error\n");
        return NULL;
    }
    if((p->board = malloc(sizeof(int *) * size)) == NULL){
        free(p->coups);
        free(p);
        fprintf(stderr,"Malloc error\n");
        return NULL;
    }
    if((p->coupPossible = malloc(sizeof(int *) * (size*size+1))) == NULL){
        free(p->coups);
        free(p->board);
        free(p);
        fprintf(stderr,"Malloc error\n");
        return NULL;
    }
    for (int i = 0; i < size; ++i) {
        if((p->board[i] = malloc(sizeof(int) * size)) == NULL) {
            for(int j = 0 ; j < i; j++) {
                free(p->board[j]);
            }
            free(p->coups);
            free(p->board);
            free(p);
            fprintf(stderr,"Malloc error\n");
            return NULL;
        }
        /*
         * Numero de position dans le plateau de jeux
         * Il commence par 1 -> size*size
         */
        for (int j = 0; j < size; ++j)
            p->board[i][j] = (j + 1) + ((i) * size);

    }
    for(int i = 1 ; i <= size*size; i++)
        p->coupPossible[i-1]=i;
    p->size = size;
    p->stop = 0;
    p->tour = 1;
    p->cursor = 0;
    p->max = (p->size > MAX) ? MAX : p->size;
    return p;
}

Board *copyBoard(Board *p){
    Board *result = createBoard(p->size);
    if(result == NULL)
        return NULL;
    memcpy(&result->stop, &p->stop, sizeof(int));
    memcpy(&result->tour, &p->tour, sizeof(int));
    memcpy(&result->cursor, &p->cursor, sizeof(int));
    memcpy(&result->max, &p->max, sizeof(int));
    for(int i = 0; i < p->cursor; i++)
        memcpy(&result->coups[i], &p->coups[i], sizeof(int));
    for(int i = 0; i < p->size * p->size - p->cursor; i++)
        memcpy(&result->coupPossible[i], &p->coupPossible[i], sizeof(int));
    for(int i = 0; i < p->size; i++)
        for(int j = 0; j < p->size; j++)
            memcpy(&result->board[i][j], &p->board[i][j], sizeof(int));
    return result;

}

void deleteBoard(Board *p){
    if( p == NULL) return;
    free(p->coups);
    for(int i = 0; i < p->size; i++)
        free((p->board[i]));
    free(p->board);
    free(p);
}

void saveBoard(char *pathname, Board *p){
    FILE *fd;
    if((fd = fopen(pathname, "w")) < 0){
        fprintf(stderr,"Open error\n");
        return;
    }
    fprintf(fd, "%d ", p->size);
    fprintf(fd, "%d ", p->cursor);
    for(int i = 0; i < p->cursor; i++)
        fprintf(fd, "%d ",p->coups[i]);
    fclose(fd);
}

Board *loadBoard(char *pathname){
    FILE *fd;
    int size;
    int cursor;
    if((fd = fopen(pathname, "r")) < 0){
        fprintf(stderr,"Open error\n");
        return NULL;
    }
    fscanf(fd,"%d", &size);
    Board *p = createBoard(size);
    fscanf(fd,"%d", &cursor);
    for(int i = 0; i < cursor; i++)
        fscanf(fd, "%d",&p->coups[i]);
    fclose(fd);
    return p;
}

void printSeperator(int n) {
    printf("\n");
    for (int i = 0; i < n; i++) {
        printf("-");
    }
    printf("\n");
}
void printBoard(Board *p) {
    printSeperator(p->size * 6);
    for (int i = 0; i < p->size; ++i) {
        printf("| ");
        for (int j = 0; j < p->size; ++j) {
            if(p->board[i][j] == -1) {
                printf("%5s", "X | ");
            } else if (p->board[i][j] == -2) {
                printf("%5s", "O | ");
            } else {
                printf("%2d%s", p->board[i][j], " | ");
            }
        }
        printSeperator(p->size * 6);
    }
    /*
     * Quand on termine une board, la dernier joueur qui a gagné
     * Il correspond au dernier joueur avant l'arrêt de la board.
     */
    if (p->stop) {
        if (p->tour == -1) {
            printf("Il y a égalité !\n");
        } else {
            printf("Le gagnant de la board est : %d\n", p->tour);
        }
    } else {
        printf("Au tour du joueur %d...\n", p->tour);
    }
}

Player *createPlayer(int num, int simulation, int IA, Board *partie){
    Player *p;
    if((p = (Player *)malloc(sizeof(Player))) == NULL){
        fprintf(stderr,"Malloc error\n");
        return NULL;
    }
    p->num = num;
    p->simulation = simulation;
    p->IA = IA;
    p->board = partie;
    return p;
}

int playerMove(Board *p) {
    int coup = 0; //si 0 alors coup invalide
    while (!coup){
        setbuf(stdout, NULL);
        printf("-- Veuillez sélectionner un coup à joué: ");
        scanf("%d", &coup);
        if(coup<=0 || coup > p->size*p->size){
            setbuf(stdout, NULL);
            fprintf(stderr, "X [Coup Invalide, il est en dehors du plateau.]\n");
            coup = 0;
        }
    }

    return coup;
}

int playMove(Board *p, int coup, int historique) {
    coup -= 1;
    int ligne = coup / p->size, colonne = coup % p->size;

    if (p->board[ligne][colonne] < 0) {
        setbuf(stdout, NULL);
        fprintf(stderr, "X [Coup Invalide, quelqu'un à déjà joué ici.]\n");
        return 0;
    } else {
        p->board[ligne][colonne] = p->tour * (-1);
        if (historique) {
            p->coups[p->cursor] = coup;
            p->cursor += 1;
        }
        return 1;
    }
}

int checkColonne(Board *p, int coup) {
    int ligne = coup / p->size , colonne = coup % p->size;
    int counter = 1; // counter le nombre de coups consécutifs
    int k;
    int i = ligne - 1;
    while (i >= (k = (ligne - p->max + 1 < 0) ? 0 : ligne - p->max + 1)) {
        if (p->board[i][colonne] == p->tour * (-1)) {
            counter++;
            i--;
        } else {
            break;
        }
    }
    i = ligne + 1;
    while (i <= (k = (ligne + p->max - 1 > p->size - 1) ? p->size - 1 : ligne + p->max - 1) && counter < p->max) {
        if (p->board[i][colonne] == p->tour * (-1)) {
            counter++;
            i++;
        } else {
            break;
        }
    }
    if (counter < p->max) { return 0; }
    else { return 1; }
}
int checkLine(Board *p, int coup) {
    int ligne = coup / p->size, colonne = coup % p->size;
    int counter = 1;
    int k;
    int j = colonne - 1;
    while (j >= (k = (colonne - p->max + 1 < 0) ? 0 : colonne - p->max + 1)) {
        if (p->board[ligne][j] == p->tour * (-1)) {
            counter++;
            j--;
        } else {
            break;
        }
    }
    j = colonne + 1;
    while (j <= (k = (colonne + p->max - 1 > p->size - 1) ? p->size - 1 : colonne + p->max - 1) && counter < p->max) {
        if (p->board[ligne][j] == p->tour * (-1)) {
            counter++;
            j++;
        } else {
            break;
        }
    }
    if (counter < p->max) { return 0; }
    else { return 1; }
}
int checkDiagonal(Board *p, int coup) {
    int ligne = coup / p->size, colonne = coup % p->size;

#pragma region Diagonale de gauche a droite
    int counter = 1;
    int i = ligne - 1, j = colonne - 1;
    int limitLigne = (ligne - p->max + 1) < 0 ? 0 : ligne - p->max + 1;
    int limitColonne = (colonne - p->max + 1) < 0 ? 0 : colonne - p->max + 1;
    while (i >= limitLigne && j >= limitColonne) {
        if (p->board[i][j] == p->tour * (-1)) {
            counter++;
            i--; j--;
        } else {
            break;
        }
    }
    i = ligne + 1; j = colonne + 1;
    limitLigne = (ligne + p->max - 1) > p->size - 1 ? p->size - 1: ligne + p->max - 1;
    limitColonne = (colonne + p->max - 1) > p->size - 1 ? p->size - 1: colonne + p->max - 1;
    while (i <= limitLigne && j <= limitColonne && counter < p->max) {
        if (p->board[i][j] == p->tour * (-1)) {
            counter++;
            i++; j++;
        } else {
            break;
        }
    }
    if (counter >= p->max) { return 1; }
#pragma endregion

#pragma region Diagonale de droite a gauche
    counter = 1;
    i = ligne - 1, j = colonne + 1;
    limitLigne = (ligne - p->max + 1) < 0 ? 0 : ligne - p->max + 1;
    limitColonne = (colonne + p->max - 1) > p->size - 1 ? p->size - 1 : colonne + p->size - 1;
    while (i >= limitLigne && j <= limitColonne) {
        if (p->board[i][j] == p->tour * (-1)) {
            counter++;
            i--; j++;
        } else {
            break;
        }
    }
    i = ligne + 1; j = colonne - 1;
    limitLigne = (ligne + p->max - 1) > p->size - 1 ? p->size - 1 : ligne + p->max - 1;
    limitColonne = (colonne - p->max + 1) < 0 ? 0 : colonne - p->max + 1;
    while (i <= limitLigne && j >= limitColonne && counter < p->max) {
        if (p->board[i][j] == p->tour * (-1)) {
            counter++;
            i++; j--;
        } else {
            break;
        }
    }
    if (counter >= p->max) { return 1; }
#pragma endregion

    return 0;
}

int checkVictory(Board *p, int coup) {
    if (p->size*p->size == p->cursor) {
        p->tour = -1;
        return 1;
    }
    coup -= 1;
    if (checkColonne(p, coup)) { return 1; }
    if (checkLine(p, coup)) { return 1; }
    if (checkDiagonal(p, coup)) { return 1; }
    return 0;
}

void selectMoveID(Board *p, int start) {
    for(int i = start; i < (p->size * p->size) - p->cursor ;i++)
        memcpy(&p->coupPossible[i], &p->coupPossible[i+1], sizeof(int *));
}

void selectMove(Board *p, int coup) {
    int id = 0;
    while (p->coupPossible[id] != coup) {
        id++;
        if(id > (p->size * p->size) - p->cursor){
            fprintf(stderr, "Erreur coup: %d non présent\n", coup);
            return;
        }
    }

    selectMoveID(p, id);
}

int getRandomMove(Board *p){
    int size = (p->size * p->size) - p->cursor;
    int counter = rand() % size;
    int coup = p->coupPossible[counter];
    return coup;
}

int MiniMax(Board *p, int idJoueur, int deep){
    Board *partie = NULL;
    int result = -1;
    int r;
    int m = 0;
    int j = p->tour == idJoueur;
    for(int i = 0 ; i < p->size * p->size - p->cursor; i++){
        partie = copyBoard(p);
        int coup = partie->coupPossible[i];
        selectMoveID(partie, i);
        playMove(partie, coup, 1);
        if(checkVictory(partie,coup))
            //Si l'ordinateur gagne r = 1 ; si égalité r = 0 ; si l'odinateur perd r = -1
            r = partie->tour == idJoueur ? 1 : partie->tour == -1 ? 0 : -1;
        else{
            if(partie->tour == 1)
                partie->tour = 2;
            else
                partie->tour = 1;
            r = MiniMax(partie, idJoueur, deep+1);
        }
        if(j){ //MAX
            if (r >= m || i == 0){
                m = r;
                result = coup;
            }
        } else { // MIN
            if (r <= m || i == 0){
                m = r;
                result = coup;
            }
        }
        deleteBoard(partie);
    }

    return deep == 0 ? result : m;
}

void *play(void *playerarg) {
    Player *player = (Player  *) playerarg;
    Board *p = (Board  *) player->board;
    int coup;
    while (!p->stop){
        if(p->tour != player->num || p->stop){
            continue;
        }
        // On bloque l'accès au plateau pour ce joueur
        pthread_mutex_lock(&board_mutex);

        printBoard(p);
        if (player->simulation) {
            coup = p->coups[p->cursor];
            playMove(p, coup, 0);
            p->cursor+=1;
        } else {
            if (player->IA == 0) {
                coup = playerMove(p);
                while (!playMove(p, coup, 1))
                    coup = playerMove(p);
            } else if(player->IA == 1) {
                coup = getRandomMove(p);
                playMove(p, coup, 1);
            } else {
                if (p->cursor == 0)
                    coup = 5;
                else
                    coup = MiniMax(p,player->num,0);
                playMove(p, coup, 1);
            }
            selectMove(p, coup);
        }
        if (checkVictory(p, coup))
            p->stop = 1;
        else {
            if(p->tour == 1)
                p->tour = 2;
            else
                p->tour = 1;
        }
        pthread_mutex_unlock(&board_mutex);
    }

    return NULL;
}

int startGame(Board *p, int typeP1, int typeP2, int simulation, int schedulerP1, int schedulerP2, int seed, int nb) {
    if (p->size > 3 && (typeP1 == 2 || typeP2 == 2)) {
        fprintf(stderr,"    X [Impossible de lancer une partie avec l'ia de niveau 2 sur un plateau supérieur à 9.]\n");
        return 0;
    }
    struct sched_param param;
    param.sched_priority = 10;
    int err;
    pthread_t t1, t2;
    Player *p1, *p2;
    p1 = createPlayer(1, simulation, typeP1, p);
    p2 = createPlayer(2, simulation, typeP2, p);
    if(p1 == NULL || p2 == NULL || p == NULL)
        return -1;
    if(seed == -1)
        srand(time(NULL) + nb);
    else
        srand(seed);
    if(pthread_create(&t1, NULL, play, p1) != 0){
        fprintf(stderr,"    X [Erreur thread 1 create!]\n");
        return -1;
    }
    if(schedulerP1 != SCHED_OTHER) {
        if ((err = pthread_setschedparam(t1, schedulerP1, &param)) != 0) {
            fprintf(stderr, "    X [Erreur thread 1 setSched: %d]\n", err);
        }
    }
    if(pthread_create(&t2, NULL, play, p2) != 0){
        fprintf(stderr,"    X [Erreur thread 2 create!]\n");
        return -1;
    }
    if(schedulerP1 != SCHED_OTHER) {
        if ((err = pthread_setschedparam(t2, schedulerP2, &param)) != 0) {
            fprintf(stderr, "    X [Erreur thread 2 setSched: %d]\n", err);
        }
    }
    // On attend que les threads se terminent
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printBoard(p);
    return p->tour;
}

void player_gui(int *scheduler, int *type) {
    int choix = 0;
    int tmp;
    fprintf(stdout,"- Veuillez sélectionner l'option à modifier: \n"
                   "    1. Changer le scheduler\n"
                   "    2. Changer type de joueur\n"
                   "    3. Quitter le menu\n");
    scanf("%d",&choix);
    switch (choix) {
        case 1://SCHEDULER
            setbuf(stdout, NULL);
            printf("- Indiquer le scheduler:\n"
                   "    0. SCHED_OTHER (defaut)\n"
                   "    1. SCHED_FIFO\n"
                   "    2. SCHED_RR\n");
            memcpy(scheduler,&tmp, sizeof(int ));
            scanf("%d",scheduler);
            if (tmp == *scheduler) {
                fprintf(stderr,"    X [Choix Invalide!]\n");
                *scheduler = 0;
                while (getchar() != '\n');
            } else if (*scheduler < 0 || *scheduler > 2) {
                fprintf(stderr,"    X [Choix Invalide, la taille doit être supérieur à 2!]\n");
                *scheduler = 0;
            } else {
                printf("    ==> [Le scheduler à été modifié!]\n");
            }
            break;
        case 2: // TYPE Joueur
            setbuf(stdout, NULL);
            printf("- Indiquer le type du joueur:\n"
                   "    0. Joueur humain\n"
                   "    1. IA Simple\n"
                   "    2. IA Difficile\n");
            memcpy(type,&tmp, sizeof(int ));
            scanf("%d",type);
            if(tmp == *type){
                fprintf(stderr,"    X [Choix Invalide!]\n");
                *type = 0;
                while (getchar() != '\n');
            } else if (*type < 0 || *type > 2){
                fprintf(stderr,"    X [Choix Invalide, la taille doit être supérieur à 2!]\n");
                *type = 0;
            } else{
                printf("    ==> [Le joueur à été modifié!]\n");
            }
            break;
        case 3:
            fprintf(stdout,"...Retour au menu principal...\n");
            break;
        default:
            setbuf(stdout, NULL);
            fprintf(stderr,"    X [Choix Invalide!]\n");
            while (getchar() != '\n');
    }
}
