//
// Created by Lenovo on 15.01.2023.
//

#include "stack.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <search.h>


Stack* reverse_stack(Stack* stack){
    if(stack == NULL) return NULL;

    Stack* reversed = create_stack(stack->capacity);
    while (!isempty_stack(stack)){
        StackData* node_stack = pop(stack);
        push(reversed, node_stack);
    }

    return reversed;
}


StackData* new_node(char *value) {
    StackData* node = (StackData*)malloc(sizeof(StackData));
    register_alloc((void**)&node, "node", get_memory_alloc_count());

    node->value =  strdup(value);


    node->left = NULL;
    node->right = NULL;
    return node;
}

Stack* create_stack(int capacity) {
    Stack* stack = (Stack*)malloc(sizeof(Stack));
    register_alloc((void**)&stack, "stack", get_memory_alloc_count());
    stack->top = -1;
    stack->capacity = capacity;
    stack->array = (StackData**)malloc(capacity * sizeof(StackData*));
    stack->added_count = 0;
    register_alloc((void**)&stack->array, "stack array", get_memory_alloc_count());

    return stack;
}

void free_stack(Stack** stack){
    printf("\nFREESTACK\n");
    if(stack == NULL ) return;

    //int arr_size = sizeof((*stack->array) / sizeof(stack->array[0]);
    /*code_printf("sizeof(stack->array):%llu\n", sizeof(stack->array));
    code_printf("sizeof(stack->array[0]):%llu\n", sizeof(stack->array[0]));
    code_printf("sizeof(Node*):%llu\n", sizeof(Node*));
*/
    //code_printf("arrsize:%d\n", arr_size);
    printf("capacity:%d\n", ((*stack)->capacity));
    printf("added:%d\n", ((*stack)->added_count));

    for (int i = 0; i < ((*stack)->added_count); ++i) {
        printf("node_value:%s\n\n", ((*stack)->array[i])->value);
        free_alloc((void **)&((*stack)->array[i]), get_memory_alloc_count());
    }
    free_alloc((void **)&((*stack)->array), get_memory_alloc_count());

    free_alloc((void **)stack, get_memory_alloc_count());
}

bool isempty_stack(Stack* stack) {
    return stack->top < 0;
}

bool isfull_stack(Stack* stack) {
    return stack->top == stack->capacity - 1;
}

bool push(Stack* stack, StackData* node) {
    if (isfull_stack(stack)) {
        return false;
    }
    (stack->top)++;


    //memcpy((char*)stack->array + stack->top * sizeof(Node**), node, sizeof(Node*));
    stack->array[(stack->top)] = node;
    //memcpy((char*)stack->array + stack->top * sizeof(Node**), node, sizeof(Node));
    (stack->added_count)++;
    return true;
}

StackData* pop(Stack* stack) {
    if (isempty_stack(stack)) {
        return NULL;
    }
    //free(stack->array[stack->top]);
    //free_alloc((void**)(stack->array[stack->top]), get_memory_alloc_count());

    return stack->array[(stack->top)--];
}

StackData* peek(Stack* stack) {
    if (isempty_stack(stack) || stack->top < 0) {
        return NULL;
    }
    return stack->array[stack->top];
}

char* peek_value(Stack* stack) {
    if (isempty_stack(stack) || stack->top < 0) {
        return NULL;
    }
    StackData* node = stack->array[stack->top];
    if(node == NULL) return NULL;

    return node->value;
}

void print_stack(Stack* stack) {
    code_printf("\nPRINTSTACK top%d: ", stack->top);

    for (int i = 0; i <= stack->top; ++i) {
        code_printf("%s ", stack->array[i]->value);
    }

    code_printf("\n");
    //free(stack_copy);
    code_printf("END: PRINTSTACK\n");

}

void visualize_tree(StackData* root, int* deep) {
    if (root == NULL) return;

    int startdeep = *deep;


    for (int i = 0; i < *deep; ++i) {
        code_printf(" ");
    }
    code_printf("ROOT: %s {\n", root->value);
    (*deep)++;

    if(root->left != NULL) {
        for (int i = 0; i < *deep; ++i) {
            code_printf(" ");
        }
        code_printf("LEFT=>\n");

        int id = (*deep);
        visualize_tree(root->left, &id);
    } else{
        for (int i = 0; i < *deep; ++i) {
            code_printf(" ");
        }
        code_printf("LEFT=> ' '\n");
    }


    if(root->right != NULL) {
        for (int i = 0; i < *deep; ++i) {
            code_printf(" ");
        }
        code_printf("RIGHT=>\n");

        int id = (*deep);
        visualize_tree(root->right, &id);
        for (int i = 0; i < startdeep; ++i) {
            code_printf(" ");
        }
        code_printf("\n}\n");
    } else{
        for (int i = 0; i < *deep; ++i) {
            code_printf(" ");
        }
        code_printf("RIGHT=>' '\n");
        for (int i = 0; i < startdeep; ++i) {
            code_printf(" ");
        }
        code_printf("}\n");
    }
}
