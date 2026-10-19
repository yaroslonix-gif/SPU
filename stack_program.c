#include "stack_program.h"

const uint64_t STRUCT_CANARY_VALUE = 0xEDA00ULL;
const stack_elem_t DATA_CANARY_VALUE = 0xBEDA00;

//============================================== BASIC FUNCTIONS ==============================================//
void _StackInit(struct stack_t* stk, const size_t capacity STACK_DEBUG_ARGUMENTS) {
    ALOGG;
    STACK_ASSERT_ONE_PARAM(stk, stk != NULL, STACK_NULL_PTR);

    if (stk->data != NULL) {
        StackDestroy(stk);
    }

    stack_elem_t* tmp_data = (stack_elem_t*)calloc(capacity + 2 * DATA_CANARY_SIZE, sizeof(stack_elem_t));
    if (tmp_data == NULL) {
        stk->status = STACK_CALLOC_ERROR;
        return;
    }
    
    // установка канареек
    UPDATE_DATA_CANARY(tmp_data, stk->data, capacity);

    SET_STRUCT_CANARY(stk);
    /////////////////////////////

    stk->capacity = capacity;
    stk->size = 0;
    stk->status = STACK_OK;
    
    STACK_RECalcHash(stk);
    STACK_HARD_ASSERT(stk);
}

void _StackPush(struct stack_t* stk, const stack_elem_t value STACK_DEBUG_ARGUMENTS) {
    STACK_HARD_ASSERT(stk);

    //============================================== Увеличение стека ==============================================//
    if (stk->size == stk->capacity) {
        stack_elem_t* tmp_data = (stack_elem_t*)_recalloc(stk->data - DATA_CANARY_SIZE, stk->capacity * 2 + 2 * DATA_CANARY_SIZE, sizeof(stack_elem_t));

        if (tmp_data == NULL) {
            stk->status = STACK_REALLOC_UP_ERROR;
            STACK_RECalcHash(stk);
            return;
        }

        stk->capacity *= 2;

        // обновление канареек data
        UPDATE_DATA_CANARY(tmp_data, stk->data, stk->capacity);
    }
    //============================================================================================//

    stk->data[stk->size++] = value;

    STACK_RECalcHash(stk);
    STACK_HARD_ASSERT(stk);
}

stack_elem_t _StackPop(struct stack_t* stk STACK_DEBUG_ARGUMENTS) {
    STACK_HARD_ASSERT(stk);

    if (stk->size == 0) {
        stk->status = STACK_NO_ELEMENTS;
        STACK_RECalcHash(stk);
        return STACK_NULL_ELEM;
    }

    //============================================== Уменьшение стека ==============================================//
    if (stk->size * 4 <= stk->capacity && stk->size >= MIN_STACK_CAPACITY_DOWN) {
        stack_elem_t* tmp_data = (stack_elem_t*)_recalloc(stk->data - DATA_CANARY_SIZE, stk->capacity / 2 + 2 * DATA_CANARY_SIZE, sizeof(stack_elem_t));
        
        if (tmp_data == NULL) {
            stk->status = STACK_REALLOC_DOWN_ERROR;
        } else {
            stk->capacity /= 2;

            // обновление канареек data
            UPDATE_DATA_CANARY(tmp_data, stk->data, stk->capacity);
        }
    }
    //============================================================================================//

    stk->size--;

    STACK_RECalcHash(stk);
    STACK_HARD_ASSERT(stk);

    return stk->data[stk->size];
}

void _StackDestroy(struct stack_t* stk STACK_DEBUG_ARGUMENTS) {
    STACK_HARD_ASSERT(stk);

    free(stk->data - DATA_CANARY_SIZE);
    stk->data = NULL;
    stk->capacity = 0;
    stk->size = 0;

    stk->status = STACK_DELETED;

    ALOGG;
}


//============================================== ADDITIONAL FUNCTIONS ==============================================//
enum stack_status _GetStackStatus(struct stack_t* stk STACK_DEBUG_ARGUMENTS) {
    STACK_HARD_ASSERT(stk);

    return stk->status;
}

void _StackPrint(FILE* out_stream, struct stack_t* stk STACK_DEBUG_ARGUMENTS) {
    SOFT_ASSERT(out_stream != NULL, NULL_ERROR);
    STACK_HARD_ASSERT(stk);

    fprintf(out_stream, "capacity = %zu\n", stk->capacity);
    fprintf(out_stream, "size = %zu\n", stk->size);
    fprintf(out_stream, "data[%zu] (0x%p)\n", stk->capacity, stk->data);
    fprintf(out_stream, "{\n");

    for (size_t i = 0; i < stk->capacity; i++) {
        if (i < stk->size)
            fprintf(out_stream, "*");
        else
            fprintf(out_stream, " ");
        
        fprintf(out_stream, "[%zu] = " STACK_ELEM_PRINT "\n", i, stk->data[i]);
    }
    fprintf(out_stream, "}\n");

    STACK_HARD_ASSERT(stk);
}

struct stack_t* _StackPtrInit(const size_t capacity STACK_DEBUG_ARGUMENTS) {
    struct stack_t* stk = (struct stack_t*)calloc(sizeof(struct stack_t), 1);
    StackInit(stk, capacity);

    return stk;
}

#ifdef STACK_DEBUG_ON
    //============================================== DEBUG AND OTHER FUNCTIONS ==============================================//
    enum stack_status _StackVerify(struct stack_t* stk) { \
        if (stk == NULL) return STACK_NULL_PTR;
        
        if (stk->canary_left != STRUCT_CANARY_VALUE || stk->canary_right != STRUCT_CANARY_VALUE) return STRUCT_CANARY_ERROR;
        
        if (stk->status == STACK_DELETED) return STACK_DELETED;
        if (stk->data == NULL) return STACK_DATA_NULL;

        if (stk->size > stk->capacity) return STACK_OUT_OF_BUF;

        if (*(stk->data - 1) != DATA_CANARY_VALUE || *(stk->data + stk->capacity) != DATA_CANARY_VALUE) return DATA_CANARY_ERROR;

        // проверка хешей
        if (stk->data_hash != CalcHash(stk->data, stk->capacity * STACK_ELEM_SIZE)) return STACK_DATA_HASH_ERROR;
        if (stk->stack_t_hash != StackCalcStackthash(stk)) return STACK_STRUCT_T_HASH_ERROR;

        return STACK_OK;
    }

    void _StackDump(const struct stack_t* stk, const struct debug_params stk_debug_params) {
        ALOGG;
        LOGG("___STACK_DUMP___\n");
        LOGG("%s:%s:%zu\n", stk_debug_params.file, stk_debug_params.func_name, stk_debug_params.line);
        
        if (stk == NULL) {
            LOGG("struct stack_t* stk = NULL!\n");
            return;
        }

        if (stk->status == STACK_DELETED) {
            LOGG("stk is DELETED!\n");
            return;
        }

        if (stk->status == STRUCT_CANARY_ERROR) {
            LOGG("CANARY ERROR: struct canaries tripped!\n");
            return;
        }

        if (stk->status == DATA_CANARY_ERROR) {
            LOGG("CANARY ERROR: data canaries tripped!\n");
            return;
        }

        LOGG("stk [ 0x%p ] {\n", stk);
        LOGG("  status       = %d\n", stk->status);
        LOGG("  capacity     = %zu\n", stk->capacity);
        LOGG("  size         = %zu\n", stk->size);

        LOGG("  data_hash    = 0x%x\n", stk->data_hash);
        if (stk->status == STACK_DATA_HASH_ERROR) {
            LOGG("  WRONG HASH: data_hash != hash(data)!\n");
            LOGG("}\n");
            return;
        }

        LOGG("  stack_t_hash = 0x%x\n", stk->stack_t_hash);
        if (stk->status == STACK_STRUCT_T_HASH_ERROR) {
            LOGG("  WRONG HASH: stack_t_hash != hash(stk)!\n");
            LOGG("}\n");
            return;
        }

        LOGG("  data[0x%p] {\n", stk->data);

        if (stk->status == STACK_DATA_NULL) {
            LOGG("    data elements: (NULL_PTR, cannot print elements)\n");
            LOGG("  }\n");
            LOGG("}\n");
            return;
        }

        size_t print_limit = stk->size;
        if (print_limit > stk->capacity) {
            LOGG("    WARNING: size > capacity! Printing only elements, which index < capacity(%zu)\n", stk->capacity);
            print_limit = stk->capacity;
        }

        if (print_limit > stk->capacity) {
            LOGG("    WARNING: size > capacity! Printing only elements, which index < capacity(%zu)\n", stk->capacity);
            print_limit = stk->capacity;
        }

        if (print_limit > DUMP_PRINT_LIMIT) {
            LOGG("      Too many elements! Printing only %zu first\n", DUMP_PRINT_LIMIT);
            print_limit = DUMP_PRINT_LIMIT;
        }

        LOGG("    data elements {\n");
        for (size_t i = 0; i < stk->capacity; i++) {
            if (i < stk->size)
                LOGG("     *[%zu] = " STACK_ELEM_PRINT "\n", i, stk->data[i]);
            else
                LOGG("      [%zu] = " STACK_ELEM_PRINT "\n", i, stk->data[i]);
        }
        LOGG("    }\n");
        LOGG("  }\n");
        LOGG("}\n");
    }

    void _StackReCalcHash(struct stack_t* stk STACK_DEBUG_ARGUMENTS) {
        STACK_ASSERT_ONE_PARAM(stk, stk != NULL, STACK_NULL_PTR);
        STACK_ASSERT_ONE_PARAM(stk, stk->data != NULL, STACK_DATA_NULL);

        stk->data_hash = CalcHash(stk->data, stk->capacity * STACK_ELEM_SIZE);

        stk->stack_t_hash = StackCalcStackthash(stk);
    }

    uint64_t _StackCalcStackthash(struct stack_t* stk STACK_DEBUG_ARGUMENTS) {
        STACK_ASSERT_ONE_PARAM(stk, stk != NULL, STACK_NULL_PTR);

        uint64_t prev_hash = stk->stack_t_hash;
        stk->stack_t_hash = 0;
        uint64_t cur_hash = CalcHash(stk, sizeof(struct stack_t));
        stk->stack_t_hash = prev_hash;

        return cur_hash;
    }
#endif

uint64_t CalcHash(const void* data, const size_t size) {  // хеш djb2
    HARD_ASSERT(data != NULL, NULL_ERROR);

    const char* data_for_hash = (const char*)data;
    uint64_t hash = 5381;

    for (size_t i = 0; i < size; i++) {
        hash = ((hash << 5) + hash) + data_for_hash[i];
    }

    return hash;
}