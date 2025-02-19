#define _DEFAULT_SOURCE
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>

/* cst pour l'aléatoire */
#define MIN_C 0
#define MAX_C 1

/**
 * Tire et renvoie un entier pseudo-aléatoire entre min et max bornes incluses.
 *
 * @param min valeur minimale de l'entier
 * @param max valeur maximale de l'entier
 */
int gen_int_rand(int min, int max) {
    return (int)((random()/(INT_MAX+1.0))*(max-min+1)+min);
}

/**
 * Génère aléatoirement les chiffres binaires. 
 *
 * @param pt_ch tableau de chiffres binaires
 * @param size nombre de chiffres binaire
 */
void ch_gen(int* pt_ch, int size){
  srandom(time(NULL)+getpid());  // pour génération aléatoire des séquences
  for (int i=0; i<size; i++){
    pt_ch[i] = gen_int_rand(MIN_C, MAX_C);
  }  
}

/**
 * Affiche les chiffres binaire d'un tableau.
 *
 * @param pt_ch tableau de chiffres binaires
 * @param size nombre de chiffres
 */
void ch_print(const int* pt_ch, int size){
  for (int i=0;i<size;i++)
    printf("%d", pt_ch[i]);
}


/**
 * Calcule la valeur décimale d'une suite de chiffres binaires donnés dans un tableau.
 *
 * @param pt_ch tableau de chiffres binaires
 * @param size nombre de chiffres binaires
 * @param pt_val pointeur sur la valeur décimale
 */
void calc_dec(const int* pt_ch, int size, int* pt_val){
  int puiss = 1;
  *pt_val=pt_ch[size-1]*puiss;
  for (int i=size-2; i>=0; i--) {
    puiss *= 2;
    *pt_val = *pt_val + puiss*pt_ch[i];
  }
}

int main(int argc, char *argv[]){
  srandom(time(NULL)); // pour génération aléatoire de l
  return 0; 
}
