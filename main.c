#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#define MAX 5
typedef struct Partie Partie;
typedef struct Player Player;

struct Partie {
    int size; // size*size = taille board
    int **board; // Matrice pour le board
    int stop; // Boolean si la partie est terminé
    int tour; // Le tour de la personne qui doit jouer, -1 si la partie est terminé
    int max;
    int *coups;
    int cursor;
    int *coupPossible; //liste des coups possibles
};

struct Player {
    int num;
    int IA;
    int simulation;
    Partie *partie;
};



//Mutex pour l'accès au plateau
pthread_mutex_t board_mutex = PTHREAD_MUTEX_INITIALIZER;

Partie *creerPartie(int size){
    Partie *p;
    if((p = malloc(sizeof(Partie))) == NULL){
        setbuf(stdout, NULL);
        fprintf(stderr,"Malloc error\n");
        return NULL;
    }
    if((p->coups = malloc(sizeof(int)*size*size)) == NULL){
        free(p);
        setbuf(stdout, NULL);
        fprintf(stderr,"Malloc error\n");
        return NULL;
    }
    if((p->board = malloc(sizeof(int *) * size)) == NULL){
        free(p->coups);
        free(p);
        setbuf(stdout, NULL);
        fprintf(stderr,"Malloc error\n");
        return NULL;
    }
    if((p->coupPossible = malloc(sizeof(int *) * (size*size+1))) == NULL){
        free(p->coups);
        free(p);
        free(p->board);
        setbuf(stdout, NULL);
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
            setbuf(stdout, NULL);
            fprintf(stderr,"Malloc error\n");
            return NULL;
        }
        /*
         * Numero de position dans le plateau de jeux
         * Il commence par 1 -> size*size
         */
        for (int j = 0; j < size; ++j) {
            p->board[i][j] = (j + 1) + ((i) * size);
        }
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

void deletePartie(Partie *p){
    if( p == NULL) return;
    free(p->coups);
    for(int i = 0; i < p->size; i++)
        free((p->board[i]));
    free(p->board);
    free(p);

}

void savePartie(char *pathname, Partie *p){
    FILE *fd;
    if((fd = fopen(pathname, "w")) < 0){
        fprintf(stderr,"Open error\n");
        return;
    }
    fprintf(fd, "%d ", p->size);
    fprintf(fd, "%d ", p->cursor);
    for(int i = 0; i < p->cursor; i++){
        fprintf(fd, "%d ",p->coups[i]);
    }
    fclose(fd);
}

Partie *loadPartie(char *pathname){
    FILE *fd;
    int size;
    int cursor;
    if((fd = fopen(pathname, "r")) < 0){
        fprintf(stderr,"Open error\n");
        return NULL;
    }
    fscanf(fd,"%d", &size);
    Partie *p = creerPartie(size);
    fscanf(fd,"%d", &cursor);
    for(int i = 0; i < cursor; i++){
        fscanf(fd, "%d",&p->coups[i]);
    }
    printf("\n");
    fclose(fd);
    return p;
}


void afficherPartie(Partie *p){
    setbuf(stdout, NULL);
    printf("---\n");
    /*
     * Quand on termine une partie, la dernier joueur qui a gagné
     * Il correspond au dernier joueur avant l'arrêt de la partie.
     */
    if (p->stop){
        if(p->tour == -1){
            setbuf(stdout, NULL);
            printf("Il y a égalité !\n");
        } else{
            setbuf(stdout, NULL);
            printf("Le gagnant de la partie est : %d\n", p->tour);
        }

    } else {
        setbuf(stdout, NULL);
        printf("Au tour du joueur : %d\n", p->tour);
    }
    for (int i = 0; i < p->size; ++i) {
        for (int j = 0; j < p->size; ++j) {
            if(p->board[i][j] == -1) {
                setbuf(stdout, NULL);
                printf("X ");
            } else if (p->board[i][j] == -2) {
                setbuf(stdout, NULL);
                printf("O ");
            } else {
                setbuf(stdout, NULL);
                printf("%d ",p->board[i][j]);
            }
        }
        setbuf(stdout, NULL);
        printf("\n");
    }
    setbuf(stdout, NULL);
    printf("---\n");
}

Player *createPlayer(int num, int simulation, int IA, Partie *partie){
    Player *p;
    if((p = malloc(sizeof(Partie *))) == NULL){
        fprintf(stderr,"Malloc error\n");
        return NULL;
    }
    p->num = num;
    p->simulation = simulation;
    p->IA = IA;
    p->partie = partie;
    return p;
}

int coupJoueur(Partie *p){
    int coup = 0; //si 0 alors coup invalide
    while (!coup){
        setbuf(stdout, NULL);
        printf("Veuillez sélectionner un coup à joué: ");
        scanf("%d",&coup);
        if(coup<=0 || coup > p->size*p->size){
            setbuf(stdout, NULL);
            fprintf(stderr,"Coup Invalide, il est en dehors du plateau.\n");
            coup = 0;
        }
    }

    return coup;
}

// retourne boolean
int jouerCoup(Partie *p, int coup, int historique){
    coup -= 1;
    int ligne = coup / p->size , colonne = coup % p->size;

    if (p->board[ligne][colonne] < 0) {
        setbuf(stdout, NULL);
        fprintf(stderr,"Coup Invalide, quelqu'un à déjà joué ici.\n");
        return 0;
    } else {
        p->board[ligne][colonne] = p->tour * (-1);
        if(historique){
            p->coups[p->cursor] = coup+1;
            p->cursor+=1;
        }
        return 1;
    }
}

int checkVictory(Partie *p, int coup){
    if(p->size*p->size == p->cursor){
        p->tour = -1;
        return 1;
    }/**
    printf("Début\n");
    coup -= 1;
    int ligne = coup / p->size , colonne = coup % p->size;
    int counter = 1;
    //Ligne
    int i = ligne - 1;
    while ( i >=  (ligne - p->max + 1 < 0 ? 0 : ligne - p->max + 1)){
        if(p->board[i][colonne]==p->tour * (-1)){
            counter++;
            i--;
        } else{
            i = -1;
        }
    }
    i = ligne + 1;
    while ( i <= (ligne + p->max - 1 > p->size ? p->size - 1 : ligne + p->max - 1)){
        if(p->board[i][colonne]==p->tour* (-1)){
            counter++;
            i++;
        } else{
            i = p->size;
        }
    }
    if(counter >= p->max){
        return 1;
    }else{
        counter = 1;
    }
    printf("Mid\n");

    //Colonne

    int j = colonne - 1;
    while ( j >= (colonne - p->max + 1 < 0 ? 0 : colonne - p->max + 1) ){
        if(p->board[ligne][j]==p->tour* (-1)){
            counter++;
            j--;
        } else{
            j = 0;
        }
    }
    printf("Mid.5\n");
    j = colonne + 1;
    while ( j <= (colonne + p->max - 1 > p->size ? p->size - 1 : colonne + p->max - 1) ){
        if(p->board[ligne][j]== p->tour * (-1)){
            counter++;
            j++;
        } else{
            j = p->size;
        }
    }
    if(counter >= p->max){
        return 1;
    }else{
        counter = 1;
    }


    printf("Fin\n");

*/
    // Diagonale
    /*
    i = ligne - 1; j = colonne - 1;
    while (i > ((ligne - p->max) < 0 ? 0 : (ligne - p->max)) && j > ((colonne - p->max) < 0 ? 0 : (colonne - p->max))) {
        if (p->board[i][j] == p->tour) {
            counter++;
            i--; j--;
        } else { break; }
    }
    i = ligne + 1; j = colonne + 1;
    while (i < ((ligne + p->max) > p->size ? p->size : (ligne + p->max)) && j < ((colonne + p->max) > p->size ? p->size : (colonne + p->max))) {
        if (p->board[i][j] == p-> )
    }*/

    return 0;

}

int getCoupRandom(Partie *p){
    int size = (p->size * p->size) - p->cursor;
    int counter = rand() % size;
    int coup = p->coupPossible[counter];
    for(int i = counter; i < (p->size * p->size) - p->cursor ;i++){
        memcpy(&p->coupPossible[i],&p->coupPossible[i+1],sizeof(int *));
    }
    return coup;
}

void *play(void *playerarg){
    Player *player = (Player  *) playerarg;
    Partie *p = (Partie  *) player->partie;
    int coup;
    while (!p->stop){
        if(p->tour != player->num || p->stop){
            continue;
        }
        // On bloque l'accès au plateau pour ce joueur
        pthread_mutex_lock(&board_mutex);

        afficherPartie(p);
        if(player->simulation){
            jouerCoup(p,p->coups[p->cursor], 0);
            p->cursor+=1;
        }else {
            if (player->IA) {
                jouerCoup(p, getCoupRandom(p), 1);
            } else {
                coup = coupJoueur(p);
                while (!jouerCoup(p, coup, 1))
                    coup = coupJoueur(p);
            }
        }
        if(checkVictory(p, coup))
            p->stop = 1;
        else{
            if(p->tour == 1)
                p->tour = 2;
            else
                p->tour = 1;
        }
        pthread_mutex_unlock(&board_mutex);
    }

}

/**
 *
 * @param p
 * @param player
 * @param simulation
 * @return le numéro du gagant de la partie, -1 si null
 */
int startPartie(Partie *p, int player, int simulation){
    pthread_t t1, t2;
    Player *p1, *p2;
    p1 = createPlayer(1, simulation, player < 2, p);
    p2 = createPlayer(2, simulation, player == 0 , p);
    srand(time(NULL));
    if(pthread_create(&t1, NULL, play, p1) != 0){
        fprintf(stderr,"Erreur thread\n");
        return -1;
    }
    if(pthread_create(&t2, NULL, play, p2) != 0){
        fprintf(stderr,"Erreur thread\n");
        return -1;
    }

    // On attend que les threads se terminent
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    afficherPartie(p);
    return p->tour;
}



int main(){
    int size = 3;
    int nbPartie = 0;
    int choix = 0;
    int stop = 1;
    int player = 2;
    int tmp = -1;
    char s[50];
    Partie *p = NULL;
    while (stop){
        setbuf(stdout, NULL);
        fprintf(stdout,"Veuillez sélectionner votre choix: \n"
                       "1. Changer taille (actuellement %d)\n"
                       "2. Changer le nombre de joueur (actuellemement %d)\n"
                       "3. Démarrer partie\n"
                       "4. Sauvegarder dernière partie\n"
                       "5. Simuler partie\n"
                       "6. Lancer plusieurs parties\n"
                       "7. stop\n",size, player);

        scanf("%d",&choix);
        switch (choix) {
            case 1: // TAILLE
                setbuf(stdout, NULL);
                printf("Indiquer la taille.\n");
                memcpy(&size,&tmp, sizeof(int ));
                scanf("%d",&size);
                if(tmp == size){
                    fprintf(stderr,"Choix Invalide\n");
                    size = 3;
                    while (getchar() != '\n');
                } else if (size < 3){
                    fprintf(stderr,"Choix Invalide, la taille doit être supérieur à 2\n");
                    size = 3;
                } else{
                    printf("La taille à été modifié\n");
                }
                break;
            case 2: // COMBIEN DE JOUEUR
                setbuf(stdout, NULL);
                printf("Indiquez un nombre de joueur\n");
                memcpy(&player,&tmp, sizeof(int ));
                scanf("%d",&player);
                if(tmp == player){
                    fprintf(stderr,"Choix Invalide\n");
                    player = 2;
                    while (getchar() != '\n');
                } else if (player < 0 || player > 2){
                    fprintf(stderr,"Choix Invalide, le nombre de joueur doit être entre 0 et 2.\n");
                    player = 2;
                } else{
                    printf("Le nombre de joueur à été modifié.\n");
                }
                break;
            case 3: // START
                deletePartie(p);
                p = creerPartie(size);
                startPartie(p, player,0);
                break;
            case 4: //SAVE PARTIE
                printf("Indiquez le nom du fichier où sauvegarder la partie (Maximum 50 caractères).\n");
                scanf("%s",s);
                printf("Sauvegarde de la partie dans le fichier %s...\n",s);
                savePartie(s,p);
                break;
            case 5: //SIMULER PARTIE
                printf("Indiquez le nom du fichier de la partie à simuler (Maximum 50 caractères).\n");
                scanf("%s",s);
                printf("Chargement de la partie: %s...\n",s);
                p = loadPartie(s);
                startPartie(p, 0,1);
                break;
            case 6: //Lancer plusieurs parties et analyse
                printf("Indiquez le nombre de partie à lancer.\n");
                memcpy(&nbPartie,&tmp, sizeof(int ));
                scanf("%d",&nbPartie);
                if(tmp == nbPartie){
                    fprintf(stderr,"Choix Invalide\n");
                    while (getchar() != '\n');
                } else if (nbPartie < 0){
                    fprintf(stderr,"Choix Invalide, le nombre de joueur doit supérieur à 0.\n");
                } else{
                    printf("Lancement de %d parties...\n", nbPartie);
                    int g;
                    for (int i = 0; i < nbPartie; ++i) {
                        deletePartie(p);
                        p = creerPartie(size);
                        g = startPartie(p, player,0);
                    }
                }
                break;
            case 7: // STOP
                setbuf(stdout, NULL);
                printf("Arret du programme");
                stop = 0;
                break;
            default:
                setbuf(stdout, NULL);
                fprintf(stderr,"Choix Invalide\n");
                while (getchar() != '\n');

        }
    }
    deletePartie(p);

    return 0;
}
