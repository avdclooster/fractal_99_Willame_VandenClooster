#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <SDL/SDL.h>
#include <stdlib.h>
#include "fractal.h"


#define MAX_ITER 4096
#define itoc(x) ((0x00ffffff/MAX_ITER)*(x))

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
#define RMASK 0xff000000
#define GMASK 0x00ff0000
#define BMASK 0x0000ff00
#define AMASK 0x000000ff
#else
#define RMASK 0x000000ff
#define GMASK 0x0000ff00
#define BMASK 0x00ff0000
#define AMASK 0xff000000
#endif

struct node{
    struct fractal *fract;
    struct node *next1;
    struct node *next2;
    double average;
};

struct list {
    struct node *first;
    int size;
};

struct nodeName{
    struct nodeName *next;
    char *name;
};

int write_bitmap_sdl(const struct fractal *f, const char *fname)
{
    SDL_Surface *back;
    SDL_Rect pix;
    int w, h, i, j, col;
    uint32_t pcol, rcol, gcol, bcol;
    w = fractal_get_width(f);
    h = fractal_get_height(f);

    back = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32, RMASK, GMASK, BMASK, AMASK);
    if (!back)
        return -1;

    for (i = 0; i < w; i++) {
        for (j = 0; j < h; j++) {
            col = itoc(fractal_get_value(f, i, j));
            rcol = col & 0xff;
            gcol = (col >> 8)& 0xff;
            bcol = (col >> 16) & 0xff;
            pcol = SDL_MapRGB(back->format, rcol, gcol, bcol);

            pix.w = pix.h = 1;
            pix.x = i;
            pix.y = j;
            SDL_FillRect(back, &pix, pcol);
        }
    }

    if (SDL_SaveBMP(back, fname) < 0)
        return -1;

    SDL_FreeSurface(back);

    return 0;
}

static int iter_julia(double zx, double zy, double a, double b, int it)
{
    /* prevent infinite loop */
    if (it > MAX_ITER)
        return 0;

    /* prevent leaving Julia set
     * if distance to origin >= 2
     */
    if (zx*zx + zy*zy >= 4.0)
        return it;

    /* compute next iteration
     * f(z) = z^2 + c
     * z = x + yi
     * c = a + bi
     */
    return iter_julia(zx*zx - zy*zy + a, 2*zx*zy + b, a, b, it+1);
}

int fractal_compute_value(struct fractal *f, int x, int y)
{
    double zx, zy;
    double a, b;
    int val;
    int w, h;

    a = fractal_get_a(f);
    b = fractal_get_b(f);

    w = fractal_get_width(f);
    h = fractal_get_height(f);

    zx = ((double)x / (double)w) * 3.0 - 1.5;
    zy = ((double)y / (double)h) * 3.0 - 1.5;

    val = iter_julia(zx, zy, a, b, 0);
    fractal_set_value(f, x, y, val);

    return val;
}

//fonction ajoutant la fractale venant  d'être calculée dans list1
int add_node (struct list *list, double value,struct fractal *fract)
{
    struct node *NewNode = (struct node *) malloc (sizeof (struct node));
    if (NewNode == NULL || list == NULL){
        return 1;
    }
    NewNode -> average = value;
    NewNode -> fract = fract;
    NewNode -> next1 = list-> first;
    NewNode -> next2 = NULL;
    list -> first = NewNode;
    list -> size = (list -> size) +1;
    return 0;
}
//fonction calculant la (ou les) plus grosse(s) moyenne(s) et la(ou les) sauvegardant dans list2
int bigList (struct list *list1, struct list *list2)
{
    if(list1->first==NULL)
    {
        return 1;
    }

    struct node *runner=list1->first;
    list2->first= runner;
    runner = runner-> next1;

    while (runner != NULL)
    {
        if ((runner-> average) < (list2->first->average))
        {
            runner = runner-> next1;
        }
        if ((runner -> average) > (list2->first->average))
        {
            list2 -> first = runner;
        }
        else
        {
            runner -> next2 = list2 -> first;
            list2 -> first = runner;
            runner = runner -> next1;
        }
    }
    return 0;
}

off_t file_size(const char *filename)
{
    struct stat s;

    if  (stat(filename,&s) != 0) {
        return -1;
    }

    return s.st_size;
}

//fonction permettant d'afficher nos erreurs lorsque l'on en detecte une
void errorPrinter(int err,char *msg)
{
    fprintf(stderr,"%s a retourné %d,message d'erreur: %s\n", msg,err,strerror(errno));
    exit(EXIT_FAILURE);
}

//fonction permettant d'inserer un nouveau nodeName dans la liste chainée. Celle-ci est utilisée afin de stocker tous
//les noms de fractal différent afin de ne pas en créer deux avec le même nom
int insert(struct nodeName **head, char *name){
    struct nodeName *premier=*head;
    if (*head==NULL){
        struct nodeName *nouveau = (struct nodeName *) malloc (sizeof(struct nodeName));
        if (nouveau == NULL){
            return -1;
        }
        nouveau->name=name;
        nouveau->next=NULL;
        *head=nouveau;
        return 0;
    }
    if(strcmp(name,(premier->name))<0){
        struct nodeName *nouveau = (struct nodeName *) malloc (sizeof(struct nodeName));
        if (nouveau == NULL){
            return -1;
        }
        nouveau->name=name;
        nouveau->next=premier;
        *head=nouveau;
        return 0;
    }
    while(premier!=NULL){
        if(name==(premier->name)){
            return -2;
        }
        if(premier->next==NULL){
            struct nodeName *nouveau = (struct nodeName *) malloc (sizeof(struct nodeName));
            if (nouveau == NULL){
                return -1;
            }
            nouveau->name=name;
            nouveau->next=NULL;
            premier->next=nouveau;
            return 0;
        }
        if(strcmp(name,premier->next->name)<0){
            struct nodeName *nouveau = (struct nodeName *) malloc (sizeof(struct nodeName));
            if (nouveau == NULL){
                return -1;
            }
            nouveau->name=name;
            nouveau->next=premier->next;
            premier->next=nouveau;
            return 0;
        }
        premier = premier->next;
    }
    return 0;
}

//fonction permettant de vérifier si un suffixe est présent dans un string. Nous l'utilisons afin de vérifier si les
//fichier sont bien fournit en format ".txt" et ".bmp"
char *verifSuf (char *string,char *extension){
    int size=(int) strlen(string);
    if(size<=4){
        return strcat(string,extension);
    }
    else{
        char *new = (char *) malloc (sizeof(char)*(size+4));
        int i=0;
        while(i<4){
            *(new+i)=*(string+size-4+i);
            i+=1;
        }
        i=strcmp(new,extension);
        if(i!=0){
	    free(new); 
            return strcat(string,extension);
        }
        else {
            free(new); 
            return string;
        }
    }
}

//fonction retirant, si elle est présente, l'extension bmp d'un fichier out fournit en argument
char *retireBMP(char *string){
    int size=(int) strlen(string);
    if(size<=4){
        return string;
    }
    else{
        char *new = (char *) malloc (sizeof(char)*(size+4));
        int i=0;
        while(i<4){
            *(new+i)=*(string+size-4+i);
            i+=1;
        }
        i=strcmp(new,".bmp");
        if(i==0){
            *(string+size-4)=0;
	    free(new);
            return string;
        }
	free(new);
        return string;
    }
}
