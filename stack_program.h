#pragma once

#include "base.h"

#define STACK_DEBUG_ON

/////////////////////////////////////////////////////////// STACK PARAMS ///////////////////////////////////////////////////////////
#define STACK_ELEM_PRINT "%lld"
#define STACK_ELEM_SCAN "%lld"
#define STACK_NULL_ELEM NAN
#define STACK_ELEM_SIZE sizeof(stack_elem_t)
typedef long long stack_elem_t;

enum stack_status {
    STACK_OK = 0,

    STACK_CALLOC_ERROR = 1,
    STACK_REALLOC_UP_ERROR = 2,
    STACK_REALLOC_DOWN_ERROR = 3,

    STACK_NO_ELEMENTS = 10,

    STACK_DELETED = 1000,
    STACK_NULL_PTR = 1001,
    STACK_OUT_OF_BUF = 1002,
    STACK_DATA_NULL = 1003,
    STACK_DATA_HASH_ERROR = 1004,
    STACK_STRUCT_T_HASH_ERROR = 1005,
    STRUCT_CANARY_ERROR = 1006,
    DATA_CANARY_ERROR = 1007,
};

#ifdef STACK_DEBUG_ON
    struct stack_t {
        uint64_t canary_left;

        stack_elem_t* data;
        size_t size;
        size_t capacity;

        enum stack_status status;

        uint64_t data_hash;
        uint64_t stack_t_hash;

        uint64_t canary_right;
    };
    #define DATA_CANARY_SIZE 1
    #define UPDATE_DATA_CANARY(tmp_data, targ_data, capacity) { \
        tmp_data[0] = DATA_CANARY_VALUE; \
        targ_data = tmp_data + 1; \
        tmp_data[capacity + 1] = DATA_CANARY_VALUE; \
    }
    #define SET_STRUCT_CANARY(ptr_stk) { \
        stk->canary_left = STRUCT_CANARY_VALUE; \
        stk->canary_right = STRUCT_CANARY_VALUE; \
    }
#else
    struct stack_t {
        stack_elem_t* data;
        size_t size;
        size_t capacity;

        enum stack_status status;
    };
    #define DATA_CANARY_SIZE 0
    #define UPDATE_DATA_CANARY(tmp_data, targ_data, capacity) targ_data = tmp_data;
    #define SET_STRUCT_CANARY(ptr_stk)
#endif

struct debug_params {
    const char* func_name;
    const char* file;
    const size_t line;
};


/////////////////////////////////////////////////////////// STACK FUNCTIONS' PARAMS AND MACROS ///////////////////////////////////////////////////////////
extern const uint64_t STRUCT_CANARY_VALUE;
extern const stack_elem_t DATA_CANARY_VALUE;

#define MIN_STACK_CAPACITY_DOWN 4

#define STACK_DEBUG_VALUES_FOR_DUMP (struct debug_params){.func_name = __FUNCTION__, .file = __FILE__, .line = __LINE__}
#define DUMP_PRINT_LIMIT 100

#ifdef STACK_DEBUG_ON
    #define STACK_DEBUG_ARGUMENTS , struct debug_params stk_debug_params
    #define STACK_DEBUG_VALUES , STACK_DEBUG_VALUES_FOR_DUMP
    #define _StackDumpIn(stk) _StackDump(stk, stk_debug_params)
    #define STACK_HARD_ASSERT_OUT(stk) { \
        struct debug_params stk_debug_params = STACK_DEBUG_VALUES_FOR_DUMP; \
        STACK_HARD_ASSERT(stk); \
    }
    #define STACK_HARD_ASSERT(stk) { \
            enum stack_status stk_err = _StackVerify(stk); \
            if (stk_err == STACK_NULL_PTR) { \
                _StackDumpIn(stk); \
                abort(); \
            } \
            if (stk_err != STACK_OK) { \
                stk->status = stk_err; \
                _StackDumpIn(stk); \
                abort(); \
            } \
        }
        #define STACK_ASSERT_ONE_PARAM(stk, value, stk_err) { if ((value) == 0) { \
            if (stk_err == STACK_NULL_PTR) { \
                _StackDumpIn(stk); \
                abort(); \
            } \
            stk->status = stk_err; \
            _StackDumpIn(stk); \
            abort(); \
        } \
    }
    #define STACK_RECalcHash(stk) _StackReCalcHash(stk STACK_DEBUG_VALUES)
    #define StackDump(stk)              _StackDump(stk, STACK_DEBUG_VALUES_FOR_DUMP)
    #define StackCalcStackthash(stk)    _StackCalcStackthash(stk STACK_DEBUG_VALUES)
#else
    #define STACK_DEBUG_ARGUMENTS
    #define STACK_DEBUG_VALUES
    #define _StackDumpIn(stk) _StackDump(stk, STACK_DEBUG_VALUES_FOR_DUMP)
    #define STACK_HARD_ASSERT(stk)
    #define STACK_ASSERT_ONE_PARAM(stk, value, stk_err)
    #define STACK_RECalcHash(stk)
#endif

#define StackInit(stk, capacity)    _StackInit(stk, capacity STACK_DEBUG_VALUES)
#define StackPush(stk, value)       _StackPush(stk, value STACK_DEBUG_VALUES)
#define StackPop(stk)               _StackPop(stk STACK_DEBUG_VALUES)
#define GetStackStatus(stk)         _GetStackStatus(stk STACK_DEBUG_VALUES)
#define StackDestroy(stk)           _StackDestroy(stk STACK_DEBUG_VALUES)
#define StackPrint(out_stream, stk) _StackPrint(out_stream, stk STACK_DEBUG_VALUES)
#define StackPtrInit(capacity)      _StackPtrInit(capacity STACK_DEBUG_VALUES)


/////////////////////////////////////////////////////////// ПРОТОТИПЫ ФУНКЦИЙ ///////////////////////////////////////////////////////////

//============================================== BASIC FUNCTIONS ==============================================//
void _StackInit(struct stack_t* stk, const size_t capacity STACK_DEBUG_ARGUMENTS);

void _StackPush(struct stack_t* stk, const stack_elem_t value STACK_DEBUG_ARGUMENTS);

stack_elem_t _StackPop(struct stack_t* stk STACK_DEBUG_ARGUMENTS);

void _StackDestroy(struct stack_t* stk STACK_DEBUG_ARGUMENTS);


//============================================== ADDITIONAL FUNCTIONS ==============================================//
enum stack_status _GetStackStatus(struct stack_t* stk STACK_DEBUG_ARGUMENTS);

void _StackPrint(FILE* out_stream, struct stack_t* stk STACK_DEBUG_ARGUMENTS);


//============================================== DEBUG AND OTHER FUNCTIONS ==============================================//
struct stack_t* _StackPtrInit(const size_t capacity STACK_DEBUG_ARGUMENTS);

#ifdef STACK_DEBUG_ON
    enum stack_status _StackVerify(struct stack_t* stk);

    void _StackDump(const struct stack_t* stk, const struct debug_params stk_debug_params);

    void _StackReCalcHash(struct stack_t* stk STACK_DEBUG_ARGUMENTS);

    uint64_t _StackCalcStackthash(struct stack_t* stk STACK_DEBUG_ARGUMENTS);

    uint64_t CalcHash(const void* data, const size_t size);
#endif