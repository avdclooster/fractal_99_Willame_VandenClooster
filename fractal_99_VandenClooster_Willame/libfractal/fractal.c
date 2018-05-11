#include <stdlib.h>
#include "fractal.h"

struct fractal *fractal_new(char *name, int width, int height, double a, double b)
{
    struct fractal *nouvFract = (struct fractal *) malloc (sizeof (struct fractal));
    if(nouvFract==NULL)
    {
        return  NULL;
    }
    int i; 
    nouvFract -> name = name; 
    nouvFract -> width = width; 
    nouvFract -> height = height; 
    nouvFract -> a = a; 
    nouvFract -> b = b; 
    nouvFract -> matrice = (int **) malloc (sizeof(int *)*width); 
    for(i=0;i<width;i++){
      nouvFract -> matrice[i] = (int *) malloc (sizeof(int)*height);  
    }
    nouvFract -> sum = 0;
    return nouvFract; 
}

void fractal_free(struct fractal *f)
{
   int i; 
   for(i=0;i<(f->width);i++){
  free(f->matrice[i]);
 }
 free(f->matrice);
 free(f);
 f=NULL;
}

const char *fractal_get_name(const struct fractal *f)
{
    return (f->name);
}

int fractal_get_value(const struct fractal *f, int x, int y)
{
    return(f->matrice[x][y]);
}

void fractal_set_value(struct fractal *f, int x, int y, int val)
{
 f->matrice[x][y]=val;
}

int fractal_get_width(const struct fractal *f)
{
    return (f->width);
}


int fractal_get_height(const struct fractal *f)
{
    return (f->height);
}

double fractal_get_a(const struct fractal *f)
{
    return (f->a);
}
double fractal_get_b(const struct fractal *f)
{
    return (f->b);
}
