//
// Author: Filip Cerny
// Date: 15.01.2024
// Description: Represents a trie data structure for dictionary in compression.
//


//Local includes
#include "trie.h"
#include "utils.h"

//Lib includes
#include <stdlib.h>
#include <stdbool.h>


//region FUNCTIONS DEFINITIONS

//region PUBLIC FUNCTIONS

/**
 * Creates a new trie node with CHILDREN_SIZE children initialized to NULL.
 * @return  Pointer to the new trie node.
 */
struct trienode *trie_create_node() {
    struct trienode *node = (struct trienode *) TRACKED_MALLOC(sizeof(struct trienode));
    if (!node) {
        LOG_MESSAGE(ERROR, "Failed to allocate memory for trie node.");
        return NULL;
    }
    for (unsigned int i = 0; i < CHILDREN_SIZE; i++) {
        node->children[i] = NULL;
    }
    node->is_leaf = false;
    return node;
}

/**
 * Frees the trie.
 * @param node  Pointer to the root of the trie.
 */
void trie_free(struct trienode *node) {
    if (!node) {
        return;
    }

    for (unsigned int i = 0; i < CHILDREN_SIZE; i++) {
        if (node->children[i] != NULL) {
            trie_free(node->children[i]);
        } else {
            continue;
        }
    }
    TRACKED_FREE(node);
}

/**
 * Inserts a new value into the trie.
 * @param ptr_root_trie  Pointer to the root of the trie.
 * @param char_value  Character value to be inserted.
 * @param value  Value to be inserted.
 * @return  true if the insertion is successful, false otherwise.
 */
bool trie_insert_char(struct trienode *ptr_root_trie, unsigned char char_value, int value) {
    if (!ptr_root_trie) {
        LOG_MESSAGE(ERROR, "Trie is not initialized.");
        return false;
    }
    if (value >= UINT16_MAX){
        LOG_MESSAGE(ERROR, "Value is too big for trie.");
        return false;
    }

    unsigned int index = (unsigned int) char_value;
    if (ptr_root_trie->children[index] == NULL) {
        ptr_root_trie->children[index] = trie_create_node();
        if (ptr_root_trie->children[index] == NULL) {
            LOG_MESSAGE(ERROR, "Failed to allocate memory for trie node.");
            return false;
        }

        ptr_root_trie->children[index]->is_leaf = true;
        ptr_root_trie->children[index]->value = value;
    }

    return true;
}

/**
 * Searches for a specified character in the trie.
 * @param root  Pointer to the root of the trie.
 * @param char_value  Character value to be searched.
 * @param ptr_node_found  Pointer to store the found node.
 * @return  true if the character is found, false otherwise.
 */
bool trie_search_char(struct trienode *root, unsigned char char_value, struct trienode **ptr_node_found) {
    if (!root) {
        LOG_MESSAGE(ERROR, "Trie is not initialized.");
        return false;
    }
    if (!ptr_node_found) {
        LOG_MESSAGE(ERROR, "Pointer to store the found node is not initialized.");
        return false;
    }

    struct trienode *temp = root->children[(unsigned int) char_value];

    if (temp != NULL && temp->is_leaf == true) {
        *ptr_node_found = temp;
        return true;
    }
    return false;
}

/**
 * Prints the trie.
 * @param root  Pointer to the root of the trie.
 * @param buffer  Buffer to store the word.
 * @param buffer_index  Index of the buffer.
 * @param tab  Number of tabs to be printed.
 * @return  true if the printing is successful, false otherwise.
 */
bool trie_print(struct trienode *root, unsigned char *buffer, unsigned int buffer_index, int tab) {
    if (root == NULL) {
        return false;
    }
    if (!buffer){
        LOG_MESSAGE(ERROR, "Buffer is not initialized.");
        return false;
    }

    if (root->is_leaf == true) {
        buffer[buffer_index] = '\0';
        /*LOG_MESSAGE(INFO, "Value: %d, Word: %s", root->value, buffer);*/
        printf("Value: %d, Word: %s\n", root->value, buffer);
    }

    for (unsigned int i = 0; i < CHILDREN_SIZE; i++) {
        if (root->children[i] != NULL) {
            buffer[buffer_index] = (unsigned char) i;
            for (int j = 0; j < tab; ++j) {
                printf("\t");
            }
            trie_print(root->children[i], buffer, buffer_index + 1, tab + 1);
        }
    }
    return true;
}
//endregion

//region PRIVATE FUNCTIONS

//endregion

//endregion