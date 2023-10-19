#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>

#define MAX 5
typedef struct Partie Partie;

struct Partie {
    int size; // size*size = taille board
    int **board; // Matrice pour le board
    int stop; // Boolean si la partie est terminé
    int tour; // Le tour de la personne qui doit jouer, -1 si la partie est terminé
    int max;
};

/*
 * 1 2 3
 * 4 5 6
 * 7 8 9
 */

Partie *creerPartie(int size){
    Partie *p;
    if((p = malloc(sizeof(Partie))) == NULL){
        fprintf(stderr,"Malloc error\n");
        return NULL;
    }
    if((p->board = malloc(sizeof(int *) * size)) == NULL){
        free(p);
        fprintf(stderr,"Malloc error\n");
        return NULL;
    }
    for (int i = 0; i < size; ++i) {
        if((p->board[i] = malloc(sizeof(int) * size)) == NULL) {
            for(int j = 0 ; j < i; j++) {
                free(p->board[j]);
            }
            free(p->board);
            free(p);
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
    p->size = size;
    p->stop = 0;
    p->tour = 1;
    p->max = (p->size > MAX) ? MAX : p->size;
    return p;
}

void afficherPartie(Partie *p){
    printf("---\n");
    /*
     * Quand on termine une partie, la dernier joueur qui a gagné
     * Il correspond au dernier joueur avant l'arrêt de la partie.
     */
    if (p->stop){
        printf("Le gagnant de la partie est : %d\n", p->tour);
    } else {
        printf("Au tour du joueur : %d\n", p->tour);
    }
    for (int i = 0; i < p->size; ++i) {
        for (int j = 0; j < p->size; ++j) {
            if(p->board[i][j] == -1) {
                printf("X ");
            } else if (p->board[i][j] == -2) {
                printf("O ");
            } else {
                printf("%d ",p->board[i][j]);
            }
        }
        printf("\n");
    }
    printf("---\n");
}

int coupJoueur(Partie *p){
    int coup = 0; //si 0 alors coup invalide
    while (!coup){
        printf("Veuillez sélectionner un coup à joué: ");
        scanf("%d",&coup);
        if(coup<=0 || coup > p->size*p->size){
            fprintf(stderr,"Coup Invalide\n");
            coup = 0;
        }
    }

    return coup;
}

// retourne boolean
void jouerCoup(Partie *p, int coup){
    coup -= 1;
    int ligne = coup / p->size , colonne = coup % p->size;

    if (p->board[ligne][colonne] < 0) {
        fprintf(stderr,"Coup Invalide\n");
    } else {
        p->board[ligne][colonne] = p->tour * (-1);
    }
}

int checkVictory(Partie *p, int coup){
    coup -= 1;
    int ligne = coup / p->size , colonne = coup % p->size;
    int counter = 1;
    //Ligne
    int i = ligne - 1 < 0 ? 0 : ligne - 1;
    while ( i > (ligne - p->max - 1 < 0 ? 0 : ligne - p->max - 1) ){
        if(p->board[i][colonne]==p->tour){
            counter++;
            i--;
        } else{
            i = 0;
        }
    }
    i = ligne + 1 >= p->size ? p->size - 1 : ligne + 1;
    while ( i < (ligne + p->max < p->size ? p->size - 1 : ligne + p->max + 1) ){
        if(p->board[i][colonne]==p->tour){
            counter++;
            i++;
        } else{
            i = p->size;
        }
    }
    if(counter >= p->size){
        return 1;
    }else{
        counter = 1;
    }

    //Colonne

    int j = colonne - 1 < 0 ? 0 : colonne - 1;
    while ( j > (colonne - p->max - 1 < 0 ? 0 : colonne - p->max - 1) ){
        if(p->board[ligne][j]==p->tour){
            counter++;
            j--;
        } else{
            j = 0;
        }
    }
    j = colonne + 1 >= p->size ? p->size - 1 : colonne + 1;
    while ( j < (colonne + p->max < p->size ? p->size - 1 : colonne + p->max + 1) ){
        if(p->board[ligne][j]==p->tour){
            counter++;
            j++;
        } else{
            j = p->size;
        }
    }
    if(counter >= p->size){
        return 1;
    }else{
        counter = 1;
    }




    // Diagonale
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
    }



}


int main(){
    int size = 3;
    printf("Veuillez sélectionner votre choix: \n1. Changer taille (actuellement %d)\n2. Démarrer partie\n3. stop",size);
    int choix;
    scanf("%d",&choix);
    switch (choix) {
        case 1:
            break;
        case 2:
            Partie *p = creerPartie(size);
            int coup;
            while (!p->stop){
                afficherPartie(p);
                coup = coupJoueur(p);
                jouerCoup(p, coup);
                if(checkVictory(p, coup)){
                    p->stop = 1;
                } else{
                    if(p->tour == 1){
                        p->tour = 2;
                    }else{
                        p->tour = 1;
                    }
                }
            }
            break;
        case 3:
            break;
        default:
            fprintf(stderr,"Choix Invalide\n");
    }
    }

    return 0;
}