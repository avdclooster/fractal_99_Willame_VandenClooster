#include <CUnit/CUnit.h>
#include <stdlib.h>
#include"../libfractal/fractal.h"
#include"../tools/tools.h"

void test_fratal_new(){
	char *nom="fractal";
	struct fractal *f= fractal_new(nom,800,500,0.8,0.4);
	CU_ASSERT_TRUE(strcmp(f->name,"fractal")==0);
	CU_ASSERT__TRUE(f->width==800);
	CU_ASSERT__TRUE(f->height==500);
	CU_ASSERT__TRUE(f->a==0.8);
	CU_ASSERT__TRUE(f->b==0.4);
	fractal_free(f); 
}

void test_add_node(){
	struct list *list1 = (struct list *) malloc (sizeof (struct list));
	struct fractal *fract1 = fractal_new("hello", 12, 12, 1, 1)
	
	list1 -> size = 0; 
	list1 -> first = NULL; 
	int i = add_node(list1, 14, fract1); 
	CU_ASSERT__TRUE(list1->size==1);
	CU_ASSERT__TRUE(lis1-> first != NULL);
	free(list1); 
	fractal_free(fract1); 
}

void test_verifSuf (){
	char *nom1 = "fichier"; 
	char *nom2 = "fichier.txt"; 

	CU_ASSERT__TRUE(strcmp(verifSuf(nom1), "fichier.txt");
	CU_ASSERT__TRUE(strcmp(verifSuf(nom2), "fichier.txt");

}

void test_big_list() {
    struct list *list1 = (struct list *) malloc(sizeof(struct list));
    list1->size = 0;
    list1->first = NULL;
    struct list *list2 = (struct list *) malloc(sizeof(struct list));
    list2->size = 0;
    list2->first = NULL;
    struct fractal *f1 = fractal_new("f1", 800, 700, 0.5, 0.5);
    struct fractal *f2 = fractal_new("f2", 800, 700, 0.5, 0.5);
    struct fractal *f3 = fractal_new("f3", 1000, 1200, 0.01, 0.01);
    struct fractal *f4 = fractal_new("f4", 800, 700, 0.5, 0.5);
    int i = add_node(list1, 100, f1);
    i = add_node(list1, 100, f2);
    i = add_node(list1, 20, f3);
    i = add_node(list1, 100, f4);
    i = bigList(list1, list2);
    CU_ASSERT_TRUE(list2->first->average == 100);
    CU_ASSERT_TRUE(list2->first->next2->average == 100);
    CU_ASSERT_TRUE(list2->first->next2->next2->average == 100);
    CU_ASSERT_TRUE(list2->first->next2->next2->next2 == NULL);
    free(list1); 
    free(list2); 
    fractal_free(f1);
    fractal_free(f2);
    fractal_free(f3);
    fractal_free(f4);
}

void test_retireBMP(){
    char *nom1="out.bmp";
    char *nom2="out";
    CU_ASSERT_EQUAL(retireBMP(nom1),nom2);
    CU_ASSERT_EQUAL(retireBMP(nom1),retireBMP(nom2));
}






