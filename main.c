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
        setbuf(stdout, NULL);
        fprintf(stderr,"Malloc error\n");
        return NULL;
    }
    if((p->board = malloc(sizeof(int *) * size)) == NULL){
        free(p);
        setbuf(stdout, NULL);
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
    p->size = size;
    p->stop = 0;
    p->tour = 1;
    p->max = (p->size > MAX) ? MAX : p->size;
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
        setbuf(stdout, NULL);
        printf("Le gagnant de la partie est : %d\n", p->tour);
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
int jouerCoup(Partie *p, int coup){
    coup -= 1;
    int ligne = coup / p->size , colonne = coup % p->size;

    if (p->board[ligne][colonne] < 0) {
        setbuf(stdout, NULL);
        fprintf(stderr,"Coup Invalide, quelqu'un à déjà joué ici.\n");
        return 0;
    } else {
        p->board[ligne][colonne] = p->tour * (-1);
        return 1;
    }
}

int checkVictory(Partie *p, int coup){
    coup -= 1;
    int ligne = coup / p->size , colonne = coup % p->size;
    int counter = 1;
    int k;
    //Ligne
    int i = ligne - 1 < 0 ? 0 : ligne - 1;
    while ( i >  (k = ligne - p->max + 1 < 0 ? 0 : ligne - p->max + 1) ){
        printf("%d \n",p->board[i][colonne]);
        if(p->board[i][colonne]==p->tour * (-1)){
            counter++;
            i--;
        } else{
            i = 0;
        }
    }
    i = ligne + 1 >= p->size ? p->size - 1 : ligne + 1;
    while ( i < (k = ligne + p->max - 1 > p->size ? p->size - 1 : ligne + p->max - 1) ){
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

    //Colonne

    int j = colonne - 1 < 0 ? 0 : colonne - 1;
    while ( j > (colonne - p->max - 1 < 0 ? 0 : colonne - p->max - 1) ){
        if(p->board[ligne][j]==p->tour* (-1)){
            counter++;
            j--;
        } else{
            j = 0;
        }
    }
    j = colonne + 1 >= p->size ? p->size - 1 : colonne + 1;
    while ( j < (colonne + p->max + 1 > p->size ? p->size - 1 : colonne + p->max + 1) ){
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


int main(){
    int size = 3;
    int choix = 0;
    int stop = 1;
    Partie *p;
    while (stop){
        setbuf(stdout, NULL);
        fprintf(stdout,"Veuillez sélectionner votre choix: \n"
                       "1. Changer taille (actuellement %d)\n"
                       "2. Démarrer partie\n"
                       "3. stop\n",size);

        scanf("%d",&choix);
        switch (choix) {
            case 1:
                setbuf(stdout, NULL);
                printf("Indiquer une taille\n");
                int tmp;
                memcpy(&size,&tmp, sizeof(int ));
                scanf("%d",&size);
                if(tmp == size){
                    setbuf(stdout, NULL);
                    fprintf(stderr,"Choix Invalide\n");
                    size = 3;
                    while (getchar() != '\n');
                } else if (size < 3){
                    setbuf(stdout, NULL);
                    fprintf(stderr,"Choix Invalide, la taille doit être supérieur à 2\n");
                    size = 3;
                } else{
                    setbuf(stdout, NULL);
                    printf("La taille à été modifié\n");
                }
                break;
            case 2:
                p = creerPartie(size);
                int coup;
                while (!p->stop){
                    afficherPartie(p);
                    coup = coupJoueur(p);
                    while (!jouerCoup(p, coup)){
                        coup = coupJoueur(p);
                    }
                    setbuf(stdout, NULL);
                    printf("v? :%d",checkVictory(p, coup));
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

    return 0;
}
