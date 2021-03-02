#include <math.h>
#include <limits.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>


#define MAXLARGEUR 7         // nb max de coups possibles
#define STATS 1              // affichage des statss du dernier coup
#define AMELIORE 0           // simulation améliorée
#define STRATEGIE 0           // mode : robuste ou max
#define HEIGHT 6
#define WIDTH 7               


#define CHANGEJOUEUR(i) (1-(i)) // Macro changement de joueur
#define min(a, b)       ((a) < (b) ? (a) : (b)) // Macro retourne min entre a et b
#define max(a, b)       ((a) < (b) ? (b) : (a)) // Macro retourne max entre a et b

typedef enum {
    NON, DEFAITE, NUL, VICTOIRE
} PartieTerminee;

// type Etat 
typedef struct EtatSt {
    int joueur; // Joueur ici = Joueur actuel, pas utilisateur
    char plate[HEIGHT][WIDTH];
} Etat;

// type Coup
typedef struct {
    int lig;
    int col;
} Coup;

// type Noeud
typedef struct NoeudSt {
    int joueur; 
    Coup *coup; 
    Etat *etat; 
    struct NoeudSt *parent;
    struct NoeudSt *fils[MAXLARGEUR]; 
    int nbFils; 
    int nbSimus;
    float nbVictoire;
} Noeud;

void afficheJeu(Etat *etat) {
    fflush(stdout);
    int i;
    int j;
    printf("|");
    for (j = 0; j < WIDTH; j++)
        printf(" %d |", j);
    printf("\n");
    printf("-----------------------------");
    printf("\n");
    for (i = 0; i < HEIGHT; i++) {
        printf("|");
        for (j = 0; j < WIDTH; j++)
            printf(" %c |", etat->plate[i][j]);
        printf("\n");
        printf("-----------------------------");
        printf("\n");
    }
}

Etat *etatInitial(void) {
    Etat *etat = (Etat *) malloc(sizeof(Etat));
    int i;
    int j;
    for (i = 0; i < HEIGHT; i++)
        for (j = 0; j < WIDTH; j++)
            etat->plate[i][j] = ' ';

    return etat;
}


Etat *duplicateState(Etat *src) {
    Etat *etat = (Etat *) malloc(sizeof(Etat));
    etat->joueur = src->joueur;
    int i;
    int j;
    for (i = 0; i < HEIGHT; i++)
        for (j = 0; j < WIDTH; j++)
            etat->plate[i][j] = src->plate[i][j];


    return etat;
}


int jouerCoup(Etat *etat, Coup *coup) {
    if (etat->plate[coup->lig][coup->col] != ' ')
        return 0;
    else {
        etat->plate[coup->lig][coup->col] = etat->joueur ? 'O' : 'X';
        etat->joueur = CHANGEJOUEUR(etat->joueur);
        return 1;
    }
}


Coup *nouveauCoup(int i, int j) {
    Coup *coup = (Coup *) malloc(sizeof(Coup));
    coup->lig = i;
    coup->col = j;
    return coup;
}


Coup *coupSuivant(Etat *etat) {
    int i, j;
    printf(" Prochain coup :  ");
    scanf("%d", &j);
    for (i = 0; i < HEIGHT; ++i) {
        if (etat->plate[i][j] != ' '){
            break;
        }
    }

    return nouveauCoup(i-1, j);
}


Coup **coupsPossibles(Etat *etat) {
    Coup **coups = (Coup **) malloc((1 + MAXLARGEUR) * sizeof(Coup *));
    int i;
    int j;
    int k = 0;
    for (j = 0; j < WIDTH; j++) {
        for (i = 0; i < HEIGHT; ++i) {
            if (etat->plate[i][j] != ' '){
                break;
            }
        }
        if (i > 0){
            coups[k] = nouveauCoup(i-1, j);
            k++;
        }
    }
    coups[k] = NULL;
    return coups;
}


Noeud *newNoeud(Noeud *parent, Coup *coup) {
    Noeud *noeud = (Noeud *) malloc(sizeof(Noeud));
    if (coup != NULL && parent != NULL) {
        noeud->etat = duplicateState(parent->etat);
        jouerCoup(noeud->etat, coup);
        noeud->coup = coup;
        noeud->joueur = CHANGEJOUEUR(parent->joueur);
    } else {
        noeud->etat = NULL;
        noeud->coup = NULL;
        noeud->joueur = 0;
    }
    noeud->parent = parent;
    noeud->nbFils = 0;
    noeud->nbSimus = 0;
    noeud->nbVictoire = 0;
    return noeud;
}


void freeNoeud(Noeud *noeud) {
    if (noeud->etat != NULL)
        free(noeud->etat);

    while (noeud->nbFils > 0) {
        freeNoeud(noeud->fils[noeud->nbFils - 1]);
        noeud->nbFils--;
    }
    if (noeud->coup != NULL)
        free(noeud->coup);

    free(noeud);
}


Noeud *addFils(Noeud *parent, Coup *coup) {
    Noeud *enfant = newNoeud(parent, coup);
    parent->fils[parent->nbFils] = enfant;
    parent->nbFils++;
    return enfant;
}


PartieTerminee testFin(Etat *etat) {
    int n = 0;
    int k = 0;
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            if (etat->plate[i][j] != ' '){
                k = 0;
                n++;
                while(i + k < HEIGHT && k < 4 && etat->plate[i + k][j] == etat->plate[i][j]){
                    k++;
                }
                if(k == 4){
                    return (etat->plate[i][j] == 'O')? DEFAITE : VICTOIRE;
                }
                k = 0;
                while(j + k < WIDTH && k < 4 && etat->plate[i][j + k] == etat->plate[i][j]){
                    k++;
                }
                if(k == 4){
                    return (etat->plate[i][j] == 'O')? DEFAITE : VICTOIRE;
                }
                k = 0;
                while(i + k < HEIGHT && k < 4 && j + k < HEIGHT && etat->plate[i + k][j + k] == etat->plate[i][j]){
                    k++;
                }
                if(k == 4){
                    return (etat->plate[i][j] == 'O')? DEFAITE : VICTOIRE;
                }
                k = 0;
                while(i - k >= 0 && k < 4 && j + k < WIDTH && etat->plate[i - k][j + k] == etat->plate[i][j]){
                    k++;
                }
                if(k == 4){
                    return (etat->plate[i][j] == 'O')? DEFAITE : VICTOIRE;
                }
            }
        }
    }
    return (n >= WIDTH * HEIGHT)? NUL: NON;
}


int stats;
int tps;
int amelioration;
int strategie;


void iaPlay(Etat *etat, int tempsmax) {
    clock_t clock1;
    clock_t clock2;

    int temps;
    int k = 0;
    Noeud *enfant;

    clock1 = clock();
    srand(time(NULL));
    Coup **coups;
    Coup *meilleurCoup;
    Noeud *racine = newNoeud(NULL, NULL);
    racine->etat = duplicateState(etat);
    coups = coupsPossibles(racine->etat);
    
    while (coups[k] != NULL) {
        enfant = addFils(racine, coups[k]);
        k++;
    }

    int it = 0;	// iteration

    do {
        float maxBValue = 0;
        Noeud *noeudMBV;
        bool trouve = false;
        Noeud *currNoeud = racine;
        Noeud *filsTrouve[MAXLARGEUR];
        int nbfilsTrouve = 0;
		int a = 1;

        while(!trouve){
			a++;
			maxBValue = INT_MIN;
            if (testFin(currNoeud->etat) == NON){
                for (int i = 0; i < currNoeud->nbFils; ++i){
                    Noeud *noeudCurrFils = currNoeud->fils[i];
                    if (noeudCurrFils->nbSimus == 0) {
                        trouve = true;
                        filsTrouve[nbfilsTrouve] = noeudCurrFils;
                        nbfilsTrouve++;
                    } else {
                        float muI = (float) noeudCurrFils->nbVictoire / (float) noeudCurrFils->nbSimus;
                        if (noeudCurrFils->joueur == 0){
                            muI = -muI;
                        }
                        float BValue = muI + sqrt(2) * sqrt(log(currNoeud->nbSimus) / noeudCurrFils->nbSimus);
						if (BValue > maxBValue) {
							maxBValue = BValue;
                            noeudMBV = noeudCurrFils;
                        }
                    }
                }
                if (!trouve){
                    currNoeud = noeudMBV;
                    if (currNoeud->nbFils == 0) {
                        coups = coupsPossibles(currNoeud->etat);
                        k = 0;
                        while (coups[k] != NULL) {
                            addFils(currNoeud, coups[k]);
                            k++;
                        }
                    }
                }
            } else {
                filsTrouve[nbfilsTrouve] = currNoeud;
                nbfilsTrouve++;
                trouve = true;
            }
        }

        Noeud *noeudPicked = filsTrouve[rand() % nbfilsTrouve];
        Etat *etatDepart = duplicateState(noeudPicked->etat);

		if (amelioration == AMELIORE){
			while(testFin(etatDepart) == NON){
				Coup **coupsL = coupsPossibles(etatDepart);
				int nbCoups = 0;
				while (coupsL[nbCoups] != NULL){
					nbCoups++;
				}
				Coup* coupChoisi = coupsL[rand() % nbCoups];
				jouerCoup(etatDepart, coupChoisi);
			}
		} else { 
			Etat *nextState = duplicateState(etatDepart);
			bool nextStateFinal = false;
			while(testFin(etatDepart) == NON){
				nextStateFinal = false;
				Coup **coupsL = coupsPossibles(etatDepart);
				int nbCoups = 0;
				while (coupsL[nbCoups] != NULL){
					nbCoups++;
				}
				for (int i = 0; i < nbCoups; i++){
					nextState = duplicateState(etatDepart);
					jouerCoup(nextState, coupsL[i]);
					if (testFin(nextState) == DEFAITE){
						nextStateFinal = true;
						jouerCoup(etatDepart, coupsL[i]);
						break;
					}
				}
				if (!nextStateFinal){
					Coup* coupChoisi = coupsL[rand() % nbCoups];
					jouerCoup(etatDepart, coupChoisi);
				}
			}
		}
        PartieTerminee resultat = testFin(etatDepart);
        currNoeud = noeudPicked;
        while(currNoeud != NULL){
            if (resultat == DEFAITE){
                currNoeud->nbVictoire += 1;
            }
            if (resultat == NUL){
                currNoeud->nbVictoire += 0.5f;
            }
            currNoeud->nbSimus++;
            currNoeud = currNoeud->parent;
        }

        free(etatDepart);
        clock2 = clock();
        temps = (int)( ((double) (clock2 - clock1)) / CLOCKS_PER_SEC );
        it ++;
    } while (temps < tempsmax);

    int maxN = 0;
    for (int j = 0; j < racine->nbFils; j++) {
		int nb;
		if(strategie == STRATEGIE) nb = racine->fils[j]->nbSimus;
		else nb = racine->fils[j]->nbVictoire;
        if (nb > maxN){
            meilleurCoup = racine->fils[j]->coup;
            maxN = racine->fils[j]->nbSimus;
        }
    }

    jouerCoup(etat, meilleurCoup);
	
	if(stats == STATS) {
        printf("\nSTATISTIQUES :\n");
		printf("Nombre de simulations réalisées : %d\n", it);
		printf("Probabilité de victoire pour l'ordinateur : %.2f \n", racine->nbVictoire/(float)racine->nbSimus);
	}
    freeNoeud(racine);
    free(coups);
}

int main(void) {
    Coup *coup;
    PartieTerminee etatF;

    Etat *etat = etatInitial();

    printf("     1er joueur -> Joueur : 0 // Ordinateur : 1  :  ");
    scanf("%d", &(etat->joueur));
    
    printf("Q1 ) Affichage des statss du dernier coup -> Oui : 1 // Non : 0  :  ");
    scanf("%d", &stats);
    
    printf("Q2 ) Temps de calcul en secondes par coup de l'ordinateur  :  ");
    scanf("%d", &tps);

    printf("Q3 ) Simulation améliorée -> Oui : 1 // Non : 0  :  ");
    scanf("%d", &amelioration);

    printf("Q5 ) Stratégie -> Robuste : 0 // Max : 1  :  ");
    scanf("%d", &strategie);

    do {
        printf("\n");
        afficheJeu(etat);
        Coup** coup2 = coupsPossibles(etat);
        if (etat->joueur == 0) {
            do {
                coup = coupSuivant(etat);
            } while (!jouerCoup(etat, coup));
        } else {
            iaPlay(etat, tps);
        }

        etatF = testFin(etat);
    } while (etatF == NON);

    printf("\n");
    afficheJeu(etat);

    if (etatF == DEFAITE)
        printf("\nFin de partie : Défaite\n");
    else if (etatF == NUL)
        printf("\nFin de partie : Nul\n");
    else
        printf("\nFin de partie : Victoire\n");
    return 0;
}