#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "libfractal/fractal.h"
#include <sys/types.h>
#include <semaphore.h>
#include <sys/fcntl.h>
#include "libfractal/tools.c"
#include <getopt.h>


int  main(int argc, char *argv[]) {
    //recuperation des argumments
    int err;
    //maxthread
    int N;
    //vaut 1 si l'option -d a été activée
    int printAll = 0;



    if(strcmp(argv[1],"-d")){
        printAll=1;
    }

    N=atoi(argv[printAll+2]);
    //F contient le nombre de fichierIn passé en argument
    int F=argc-printAll-4;
    char **f=(char **) malloc(F*sizeof(char *));
    int j=0;
    while(j<F){
        int size=(int) strlen(argv[j+printAll+3]);
        char *argu=(char *) malloc(sizeof(char)*size);
        strcpy(argu,argv[j+printAll+3]);
        *(f+j)=argu;
	j+=1;
    }
    //nombre de thread à initialiser
    int NTHREADS = F + N + 1;


//commence le programme avec la decleration des variables globales
    struct list *list1 = (struct list *) malloc (sizeof (struct list));
    if (list1 == NULL){
        errorPrinter(-1,"malloc_list1");
    }
    list1->first = NULL;
    list1->size = 0;
    struct list *list2 = (struct list *) malloc (sizeof (struct list));
    if (list2 == NULL){
        errorPrinter(-1,"malloc_list2");
    }
    list2->first = NULL;
    list2->size = 0;


    //Pointeur vers des nodeName qui nous permettent de stocker tous les noms de fractales déjà créé afin de ne pas en créé deux identiques
    struct nodeName **head  = (struct nodeName **) malloc (sizeof (struct nodeName*));
    if (head == NULL){
        errorPrinter(-1,"malloc_head");
    }
    *head=NULL;
    pthread_mutex_t mutexHead;
    pthread_mutex_init(&mutexHead, NULL);
    struct fractal *tab[N];
    pthread_mutex_t mutexArg;
    int takeArg=0;
    pthread_mutex_init(&mutexArg, NULL);
    pthread_mutex_t mutexPC;//initialisation des sem et mutex pour le producteur/consommateur
    sem_t empty;
    sem_t full;
    pthread_mutex_init(&mutexPC, NULL);
    sem_init(&empty, 0, N);  // buffer vide
    int empty_value = N;
    sem_init(&full, 0, 0);  // buffer vide
    int full_value = 0;
    pthread_mutex_t mutexList1;
    pthread_mutex_init(&mutexList1, NULL);
    int reader_count = 0;  //compte le nombre de reader ayant terminé leur exécution
    int fractal_created = 0;  //compte le nombre de fractales crées par les readers
    int fractal_producted = 0;  //compte le nombre de fractales calculées par les calculators
    pthread_mutex_t mutexCount;  //protège la variable reader_count
    pthread_mutex_init(&mutexCount, NULL);
    int nombreDeFichier = F;  //variable qu on doit determiner en fonction des argumments

//fonction reader permertant de lire dans les fichiers, de créé les objets fractales et de les placer dans le tableau
    void *reader(void *arg) {
        int o;
        pthread_mutex_lock(&mutexArg);
        char *entree = *(f+takeArg);
        takeArg+=1;
        pthread_mutex_unlock(&mutexArg);

        o = open(entree, O_RDONLY);
        if (o == -1) {
            err = -1;
            errorPrinter(err, "open reader");
        }
        off_t l = 0;
        off_t size = file_size(entree);
        if (size == -1) {
            err = -1;
            errorPrinter(err, "file_size");
        }

        off_t sizeofchar = (off_t) sizeof(char);

        //Afin de récupèrer les différents paramètres nécessaires à la création d'une fractale
        while (l < size) {
            char *nom= (char *) malloc (64*sizeof(char));
            char *numberw = (char *) malloc (64*sizeof(char));
            char *numberh= (char *) malloc (64*sizeof(char));
            char *numbera= (char *) malloc (64*sizeof(char));
            char *numberb= (char *) malloc (64*sizeof(char));
            int width;
            int height;
            double a;
            double b;
            //Et on va lire caractères par caractère chaque ligne
            err = read(o, nom, sizeof(char));
            if (err == -1) {
                errorPrinter(err, "read");
            }
            l = l + sizeofchar;
            if (*nom == '#') {
                int i = 0;
                while (*(nom + i) != '\n') {
                    i++;
                    err = read(o, (nom + i), sizeof(char));
                    if (err == -1) {
                        errorPrinter(err, "read");
                    }
                    l = l + sizeofchar;
                }
            }
            else {
                int i = 0;
                while (*(nom + i) != ' ') {
                    i++;
                    err = read(o, nom + i, sizeof(char));
                    if (err == -1) {
                        errorPrinter(err, "read");
                    }
                    l = l + sizeofchar;
                }
                *(nom + i) = 0;
                i = 0;
                err = read(o, numberw, sizeof(char));
                if (err == -1) {
                    errorPrinter(err, "read");
                }
                l = l + sizeofchar;
                while (*(numberw + i) != ' ') {
                    i++;
                    err = read(o, (numberw + i), sizeof(char));
                    if (err == -1) {
                        errorPrinter(err, "read");
                    }
                    l = l + sizeofchar;
                }
                *(numberw + i) = 0;  //On passe d'une string à un int afin de l'utiliser dans fractal_new
                width = atoi(numberw);
                i = 0;
                err = read(o, numberh, sizeof(char));
                if (err == -1) {
                    errorPrinter(err, "read");
                }
                l = l + sizeofchar;
                while (*(numberh + i) != ' ') {
                    i++;
                    err = read(o, (numberh + i), sizeof(char));
                    if (err == -1) {
                        errorPrinter(err, "read");
                    }
                    l = l + sizeofchar;
                }
                *(numberh + i) = 0;
                height = atoi(numberh);
                i = 0;
                err = read(o, numbera, sizeof(char));
                if (err == -1) {
                    errorPrinter(err, "read");
                }
                l = l + sizeofchar;
                while (*(numbera + i) != ' ') {
                    i++;
                    err = read(o, (numbera + i), sizeof(char));
                    if (err == -1) {
                        errorPrinter(err, "read");
                    }
                    l = l + sizeofchar;
                }
                *(numbera + i) = 0;
                a = atof(numbera);
                i = 0;
                err = read(o, numberb, sizeof(char));
                if (err == -1) {
                    errorPrinter(err, "read");
                }
                l += sizeofchar;

                //on vérifie qu'on est à la fin d'une ligne ou à la fin du document
                while (*(numberb + i) != '\n' && *(numberb + i) != '\0') {
                    i++;
                    err = read(o, (numberb + i), sizeof(char));
                    if (err == -1) {
                        errorPrinter(err, "read");
                    }
                    l = l + sizeofchar;
                }
                *(numberb + i) = 0;
                b = atof(numberb);

                //lock nécessaire pour vérifier si le nom de fractal est unique

                pthread_mutex_lock(&mutexHead);
                int ins = insert(head, nom);
                pthread_mutex_unlock(&mutexHead);

                //si il est unique alors on crée la fractale
                if (ins == 0) {
                    struct fractal *nouvFract = fractal_new(nom, width, height, a, b);
                    if (nouvFract == NULL) {
                        err = -1;
                        errorPrinter(err, "fractal_new");
                    }
                    free(nom);
                    free(numberw);
                    free(numberh);
                    free(numbera);
                    free(numberb);
                    //Ici on entre dans la section critique pour aller inserrer la fractale dans le tableau
                    sem_wait(&empty);
                    pthread_mutex_lock(&mutexPC);
                    fractal_created += 1;
                    empty_value = empty_value - 1;
                    tab[empty_value] = nouvFract;
                    full_value = full_value + 1;
                    sem_post(&full);
                    pthread_mutex_unlock(&mutexPC);
                }
                if (ins == -2) {
                    errorPrinter(ins, "Deux fois le même nom de fractale");
                } else {
                    errorPrinter(ins, "malloc");
                }
            }

        }
        err = pthread_mutex_lock(&mutexCount);
        if (err != 0){
            errorPrinter(err,"pthread_mutex_lock");
        }
        reader_count += 1;
        err = pthread_mutex_unlock(&mutexCount);
        if (err != 0){
            errorPrinter(err,"pthread_mutex_unlock");
        }
        return NULL;

    }

    //fonction permettant de récuperer une fractale dans un tableau et de la calculer la valeur de chaque pixel.
    //Il met ensuite la fractale dans list

    void *calculator(void *arg) {
        int err;
        //La condition est "tant qu'il reste des fractale à calculer"
        while ((reader_count < nombreDeFichier) || (fractal_producted < fractal_created))
        {
            sem_wait(&full);
            pthread_mutex_lock(&mutexPC);
            full_value = full_value - 1;

            struct fractal *fractToCompute = (tab[empty_value]);
            if(fractToCompute==NULL)
            {
                err=-1;
                errorPrinter(err,"calculator");
            }
            tab[empty_value] = NULL;
            empty_value = empty_value + 1;
            sem_post(&empty);
            pthread_mutex_unlock(&mutexPC);
            int val;
            //on calcule la valeur de chaque pixel
            for (int i = 0; i < (fractToCompute->width); i++) {
                for (int j = 0; i < (fractToCompute->height); j++) {
                    val = fractal_compute_value(fractToCompute, i, j);
                    (fractToCompute->sum) += val;
                }
            }
            //on calcule la moyenne de chaque pixel
            double average = (fractToCompute->sum) / ((fractToCompute->width) * (fractToCompute->height));
            pthread_mutex_lock(&mutexList1);
            err = add_node(list1, average, fractToCompute);
            if(err==1)
            {
                errorPrinter(err,"add_node");
            }
            fractal_producted += 1;
            pthread_mutex_unlock(&mutexList1);
        }
        return NULL;
    }

    //fonction auxiliaire du writer qui permet de gérer les écritures dans les fichiers de sorties
    int fichierOut (struct list *list1, struct list *list2)
    {
        int o1;
        int o2;
        char *concat1 = (char *) malloc (64*sizeof(char));
        char *concat2 = (char *) malloc (64*sizeof(char));
        char *concat3 = (char *) malloc (64*sizeof(char));
        int w;

        struct node *runner1=list1->first;
        struct node *runner2=list2->first;

        if (printAll == 1){
            while (runner1 != NULL){
                char *fName = strcat(runner1->fract->name, ".BMP");
                o1 = open(fName, O_CREAT|O_TRUNC|O_RDWR);
                if(o1==-1)
                {
                    errorPrinter(o1,"open writer");
                }
                w = write_bitmap_sdl(runner1->fract, fName);
                if(w==-1)
                {
                    errorPrinter(w,"write_bitmap_sdl");
                }
                runner1 = runner1 -> next1;
            }
        }
        char *sufSortie = retireBMP(argv[argc-1]);
        if (list2->size == 1){
            verifSuf(sufSortie, ".bmp");
            w = write_bitmap_sdl(runner2->fract, sufSortie);
            if(w==-1)
            {
                errorPrinter(w,"write_bitmap_sdl");
            }
            runner2 = runner2 -> next2;
        }

        else {
            for (int i = 1; i <= (list2->size); i++){
                char *buff = (char *) malloc (64*sizeof(char));
                int s = sprintf(buff, "%d", i);

                if (s <= 0){
                    errorPrinter(s, "sprintf");
                }

                concat1 = strcat (sufSortie, "_");
                concat2 = strcat (concat1, buff);
                concat3 = strcat (concat2, ".bmp");
                o2 = open(concat3, O_CREAT|O_TRUNC|O_RDWR);
                w = write_bitmap_sdl(runner2->fract, concat3);
                runner2 = runner2 -> next2;
                if(o2==-1)
                {
                    errorPrinter(o2,"open writer2");
                }
                free(buff);
            }

        }
        free(concat1);
        free(concat2);
        free(concat3);

        return 0;

    }

    //la fonction présente dans le thread writer qui tient vérifie quelles fractales ont les plus grosses moyennes,
    //et crée ensuite les fichiers nécessaire en tenant compte du printAll
    void *writer(void *arg) {
        int err;
        int done = 0;
        while (done == 0) {
            if (reader_count == nombreDeFichier && fractal_producted == fractal_created) {
                err=bigList(list1,list2);
                if(err==1)
                {
                    errorPrinter(err,"il n'y avait pas de fractal à calculer");
                }
                err=fichierOut (list1, list2);
                done = 1;
            }

        }
        return NULL;
    }
//initialisation des threads

    pthread_t threads[NTHREADS];
    int i = 0;


//creation des threads reader
    while (i < F) {
            //char * entree = verifSuf(argv[i+4], ".txt");
            err = pthread_create(&(threads[i]), NULL,&reader, NULL);
            if(err!=0)
            {
                errorPrinter(err,"pthread_create");
            }
            i++;

    }

//creation des threads calculateur

    while (i<N+F)
    {
        err= pthread_create(&(threads[i]), NULL, &calculator, NULL);
        if(err!=0)
        {
            errorPrinter(err,"pthread_create");
        }
        i++;
    }

// creation du threads writer

    err = pthread_create(&(threads[i]), NULL, &writer, NULL);
    if(err!=0)
    {
        errorPrinter(err,"pthread_create");
    }
    void *r;
    pthread_join(threads[i],&r);


//on commence par les nodeNames
    while(*head!=NULL){
        struct nodeName *premier=*head;
        *head=(*head)->next;
        free(premier);
    }

//on free les fractales et les nodes
    while((list1->first)!=NULL){
        struct node *premier=list1->first;
        list1->first=premier->next1;
        fractal_free(premier->fract);
        free(premier);
    }

//on free les malloc qu'on avait ajouté
    int k=0;
    while(k<F){
        free(*(f+k));
    }
    free(f);

//On free les structures
    free(list1);
    free(list2);
    free(head);
    return (EXIT_SUCCESS);

}
