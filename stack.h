//  c++ library is called "stack", "not stack.h", so no naming conflicts

#include <limits.h>

#include "my_hash.h"

#define DEBUG_MODE

#define STACK_CANARY_PROTECT
#define DATA_CANARY_PROTECT

#define STACK_HASH_PROTECT
#define DATA_HASH_PROTECT

//-------------------------------------------------------------------------------------
typedef int    Elem_t;
typedef size_t Canary_t;

#define prcnt_el "%6d"
//-------------------------------------------------------------------------------------
#if (defined(DEBUG_MODE))

    #define STACK_DUMP(fname, stk, err_vector, debg_inf) StackDump((fname), (stk), (err_vector), (#stk), __FILE__, __LINE__, __FUNCTION__, debg_inf)

    #define ADD_ERR_MSG(prev, msg) strcat((*(prev)) ? strcat((prev), ", ") : (prev), (msg))

    // assume that err_vector declared before
    #define ASSERT_STACK(stk, call_from, debg_inf)           \
    {                                                        \
        err_vector = StackErr(stk, call_from);               \
        if (err_vector) {                                    \
            STACK_DUMP(LOG_FILE, stk, err_vector, debg_inf); \
            fprintf(stderr, "Stack corrupted. ABORTED\n");   \
            abort();                                         \
        }                                                    \
    }

    #define DEBUG_INFO(stk) (UpdDebugInfo((#stk), __FILE__, __LINE__))

#else
    #define STACK_DUMP(fname, stk, err_vector) ;
    #define ADD_ERR_MSG(prev, msg)             ;
    #define ASSERT_STACK(stk, call_from)       ;
    #define DEBUG_INFO(stk)                    {NULL, NULL, -1}
#endif // defined(DEBUG_MODE)

//-------------------------------------------------------------------------------------
#if (defined(STACK_CANARY_PROTECT) || defined(DATA_CANARY_PROTECT))
    const Canary_t LEFT_CHICK  = 0xB38F0F83F03F80AA; //  in 0b it looks like 101100111000...10101010
    const Canary_t RIGHT_CHICK = 0xB38F0F83F03F80AA; //  in 0b it looks like 101100111000...10101010
#else

#endif // defined(defined(STACK_CANARY_PROTECT) || defined(DATA_CANARY_PROTECT))

//-------------------------------------------------------------------------------------
const char * const LOG_FILE = "log.log";

const int    MX_STK             = 100;
const int    POISON             = 0xD00D1E;
const size_t MAX_ERR_MSG_STRING = 400;

//-------------------------------------------------------------------------------------
struct stk_data {

    Elem_t *buf;

    #if (defined(DATA_CANARY_PROTECT))

    Canary_t *l_canary;
    Canary_t *r_canary;

    #endif // defined(DATA_CANARY_PROTECT)

    #if (defined(DATA_HASH_PROTECT))

    Hash_t    hash_sum;

    #endif // defined(DATA_HASH_PROTECT)
};

//-------------------------------------------------------------------------------------
struct stack {

    #if (defined(STACK_CANARY_PROTECT))

    Canary_t        l_canary;

    #endif // defined(STACK_CANARY_PROTECT)

    stk_data        data;
    int             size;
    int             capacity;      // int instead of size_t in order to catch errors
    int             init_capacity;

    #if (defined(STACK_HASH_PROTECT))
    Hash_t    *     hash_sum;
    #endif // defined(STACK_HASH_PROTECT)

    #if (defined(STACK_CANARY_PROTECT))

    Canary_t        r_canary;

    #endif // defined(STACK_CANARY_PROTECT)
};

struct stk_debug_info {

    const char * stk_name;
    const char * filename;
    int line;

};

//-------------------------------------------------------------------------------------
enum CTOR_OUT {
    CTOR_NULL_STK = -1, // if function received already corrupted stack
    CTOR_NO_ERR   =  0,
    CTOR_ERR      =  1
};

enum REALLC_OUT {
    REALLC_ERR_SIDE = -1, // if function received already corrupted stack
    REALLC_NO_ERR   =  0,
    REALLC_ERR      =  1
};

enum DTOR_OUT {
    DTOR_ERR_SIDE  = -1, // if function received already corrupted stack
    DTOR_DESTR     =  0,
    DTOR_NOT_DESTR =  1

};

enum PUSH_OUT {
    PUSH_ERR_SIDE = -1,  // if function received already corrupted stack
    PUSH_NO_ERR   =  0,
    PUSH_ERR      =  1
};

enum POP_OUT {
    POP_ERR_SIDE = -1,  // if function received already corrupted stack
    POP_NO_ERR   =  0,
    POP_ERR      =  1
};

/**
 * this enum is for choosing when to compare hash and when to not
 * values are assigned to call_from variable
*/
enum CALL_FROM {
    F_BEGIN = -1,   // tells to calculate new hash and then to compare it with old one
    F_MID   =  0,   // tells to do nothing
    F_END   =  1    // tells to calculate new hash and set previous one to this freshly-calculated value
};

//-------------------------------------------------------------------------------------
/**
 * @param [out] stk stack-variable, not valid
 * @param [in] capacity required stack capacity
 * @param [in] debug_info contains info about place from which we got into function
 *
 * Stack Constructor: allocates memory for data, sets size to 0, sets canaries, allocates memory for hash,
 *                    poisons all the memory.
 *                    Calls static func StackDataCalloc in order to allocate memory for data and canaries
 *
*/
enum CTOR_OUT
StackCtor(stack          *stk,
          int            capacity,
          stk_debug_info debug_info);

//-------------------------------------------------------------------------------------
/**
 * @param [out] stk stack
 * @param [in]  new_capacity new required capacity of the stack (previously calculated in GetNewCapacity)
 * @param [in] debug_info contains info about place from which we got into function
 *
 * @brief reallocs data with canaries, poisons it, to do it
 *                                                 calls inner function StackDataRealloc
*/
enum REALLC_OUT
StackRealloc(stack          *stk,
             int            new_capacity,
             stk_debug_info debug_info);

//-------------------------------------------------------------------------------------
enum DTOR_OUT
/**
 * @param [out] stk stack
 * @param [in] debug_info contains info about place from which we got into function
 *
 * destructs stack
*/
StackDtor(stack *stk, stk_debug_info debug_info);

//-------------------------------------------------------------------------------------
/**
 * @param [out] stk  stack
 * @param [in]  value value to be pushed in stack
 * @param [in] debug_info contains info about place from which we got into function
*/
enum PUSH_OUT
StackPush(stack          *stk,
          Elem_t         value,
          stk_debug_info debug_info);

//-------------------------------------------------------------------------------------
/**
 * @param [in]  stk stack
 * @param [out] err error handling variable
 * @param [in] debug_info contains info about place from which we got into function
*/
Elem_t
StackPop(stack          *stk,
         enum POP_OUT   *err,
         stk_debug_info debug_info);

//-------------------------------------------------------------------------------------
/**
 * @param [in] fname      output file name
 * @param [in] stk        stack
 * @param [in] err_vector contains codes of errors
 * @param [in] err_file   file from which dump was called
 * @param [in] err_line   line from which dump was called
 * @param [in] err_func   func from which dump was called
 * @param [in] stk_debug  debug info
 * @param [in] debug_info contains info about place from which we got into function
 *
 * @brief prints whole info about stack to file
 *
 * WORKS ONLY IN DEBUG MODE
*/
void
StackDump(const char  * const fname,
          const stack *       stk,
          size_t              err_vector,
          const char  * const stk_name,
          const char  * const err_file,
          int                 err_line,
          const char  * const err_func,
          stk_debug_info      debug_info);

//-------------------------------------------------------------------------------------
/**
 * @param [in] stk stack
 * @param [out] call_from contains info for hash protect about from what part of func StackErr was called
 *
 * @brief forms error vector - number like 1011100000 where 1 - has error, 0 - does not have error,
 *        if call_from == F_BEGIN - calculates new hash and compares it with the old one (must be equal)
 *        if call_from == F_MID  - does nothing with hash
 *        if call_from == F_END  - calculates new hash and sets previous hash = new hash
*/
size_t StackErr(stack *stk, enum CALL_FROM call_from);

//-------------------------------------------------------------------------------------
/**
 * @param [in] stk_name string containing stack name
 * anscillary func for macros-wrapper, returns debug info about stack
*/
stk_debug_info UpdDebugInfo (const char *stk_name, const char *filename, int line);
