//
// Author: Lenovo
// Date: 15.01.2024
// Description: 
//

#ifndef FAV_PC_SP_23_24_BINARYDATAINIMG_TRIE_H
#define FAV_PC_SP_23_24_BINARYDATAINIMG_TRIE_H

//Lib includes

//region DEFINE, MACROS
#define CHILDREN_SIZE 256 // Alphabet size
//endregion

#include <stdint.h>
#include <stdbool.h>

//region STRUCTS

struct trienode {
    uint32_t value;
    struct trienode *children[CHILDREN_SIZE];
    bool is_leaf;
};
//endregion

//region FUNCTIONS DECLARATION
/**
 * Creates a new trie node.
 * @return  Pointer to the new trie node.
 */
struct trienode *trie_create_node();

/**
 * Frees the trie.
 * @param node  Pointer to the root of the trie.
 */
void trie_free(struct trienode *node);

/**
 * Inserts a new value into the trie.
 * @param ptr_root_trie  Pointer to the root of the trie.
 * @param char_value  Character value to be inserted.
 * @param value  Value to be inserted.
 * @return  true if the insertion is successful, false otherwise.
 */
bool trie_insert_char(struct trienode *ptr_root_trie, unsigned char char_value, int value);

/**
 * Searches for a specified character in the trie.
 * @param root  Pointer to the root of the trie.
 * @param char_value  Character value to be searched.
 * @param ptr_node_found  Pointer to store the found node.
 * @return  true if the character is found, false otherwise.
 */
bool trie_search_char(struct trienode *root, unsigned char char_value, struct trienode **ptr_node_found);

/**
 * Prints the trie.
 * @param root  Pointer to the root of the trie.
 * @param buffer  Buffer to store the word.
 * @param buffer_index  Index of the buffer.
 * @param tab  Number of tabs to be printed.
 * @return  true if the printing is successful, false otherwise.
 */
bool trie_print(struct trienode *root, unsigned char *buffer, unsigned int buffer_index, int tab);

//endregion


#endif //FAV_PC_SP_23_24_BINARYDATAINIMG_TRIE_H