/**
 * @file data_structures.h
 * @author Filip Cerny
 * @brief Hlavickovy soubor s deklaracemi funkci, pro
 * zasobnik a binarni strom
 * @version 1.0
 * @date 2022-11-15
 */

#ifndef FAV_PC_SP_23_24_BINARYDATAINIMG_STACK_H
#define FAV_PC_SP_23_24_BINARYDATAINIMG_STACK_H

#include <stdbool.h>

/**
 * Vrchol grafu
 */
typedef void StackData;

/**
 * Zasobnik
 */
typedef struct Stack {
    int top;
    int capacity;
    int added_count;
    struct StackDAta** array;
}Stack;

/**
 * Vytiskne strom
 * @param root strom
 * @param deep pro odradkovani jako moc je hluboko
 */
void visualize_tree(StackData* root, int* deep);

/**
 * Vrati obraceny zasobnik
 * @param stack zasobnik na obraceni
 * @return obraceny zasobnik
 */
Stack* reverse_stack(Stack* stack);

/**
 * Vrati novy vrchol s hodnotou value
 * @param value hodnota vrcholu
 * @return vrchol
 */
StackData* new_node(char *value);

/**
 * Vytvori zasobnik s velikosti
 * @param capacity velikost zasobniku
 * @return Vytvoreny zasobnik
 */
Stack* create_stack(int capacity);

/**
 * Kontrola jestli je zasobnik prazdny
 * @param stack zasobnik na kontrolu
 * @return jestli je prazdny
 */
bool isempty_stack(Stack* stack);

/**
 * Ulozi hodnotu do zasobniku
 * @param stack zasobnik
 * @param node hodnota
 * @return jestli se ulozila
 */
bool push(Stack* stack, StackData* node);

/**
 * Smaze a vrati hodnotu na
 * vrcholu zasobniku
 * @param stack zasobnik
 * @return hodnota na vrcholu
 */
StackData* pop(Stack* stack);

/**
 * Vrati hodnotu na
 * vrcholu zasobniku
 * @param stack zasobnik
 * @return hodnota na vrcholu
 */
StackData* peek(Stack* stack);

/**
 * Vrati hodnotu ulozenou ve
 * vrcholu na vrcholu zasobniku
 * @param stack zasobnik
 * @return honota z node
 */
char* peek_value(Stack* stack);

/**
 * Vypise hodnoty v zasobniku
 * @param stack zasobnik
 */
void print_stack(Stack* stack);

/**
 * Uvolni pamet zasobniku
 * @param stack zasobnik
 */
void free_stack(Stack** stack);


#endif //FAV_PC_SP_23_24_BINARYDATAINIMG_STACK_H
