#ifndef __STRUCTS_H__
#define __STRUCTS_H__

/* Enums and structs for the program, the names speaks for themself */

typedef signed int Number;

typedef enum {
    FALSE = 0, TRUE
} BOOL;

typedef enum {
    ERROR_START_ERROR = 0,
    ERROR_LABEL_START_NONABC,
    ERROR_LABEL_CONTAINS_NONABC,
    ERROR_LABEL_TOO_LONG,
    ERROR_LABEL_ALREADY_DEFINED,
    ERROR_LABEL_IS_KEYWORD,
    ERROR_LABEL_UNKNOWN_ERROR,
    ERROR_LABEL_MISSING_NAME,
    ERROR_LABEL_DOESNT_EXIST,
    ERROR_LABEL_DECLARED_AS_ENTRY_BUT_NOT_DEFINED,
    ERROR_TOO_MANY_PARAMETERS,
    ERROR_MISSING_PARAMETERS,
    ERROR_MISSING_STRING,
    ERROR_BAD_NUMBER,
    ERROR_COMMAND_NULL,
    ERROR_COMMAND_NOT_FOUND,
    ERROR_COMMAND_OPERAND_ADDRESSING_METHOD_UNACCEPTABLE,
    ERROR_INVALID_STRUCT_FORMAT,
    ERROR_MISSING_COMB,
    ERROR_INVALID_COMB,
    ERROR_MAX_ERROR
} COMPILER_ERRORS;

typedef enum {
    LABEL_TYPE_REGULAR = 0,
    LABEL_TYPE_ENTRY,
    LABEL_TYPE_EXTERNAL,
    LABEL_TYPE_NUM
} LABEL_TYPES;

typedef enum {
    ADDRESS_METHOD_INSTANT = 0,
    ADDRESS_METHOD_DIRECT,
    ADDRESS_METHOD_ENTRY_ACCESS,
    ADDRESS_METHOD_REGISTER_DIRECT,
    ADDRESS_METHOD_NUM
} ADDRESSING_METHODS;


typedef struct {
    char *name;
    Number num_of_arguments;
    Number opcode;
    BOOL legal_source_addressing_methods[ADDRESS_METHOD_NUM];
    BOOL legal_dest_addressing_methods[ADDRESS_METHOD_NUM];
} command;

typedef struct {
    char *name; /* ".entry", ".data".. */
    Number directive_identifier; /* entry is 1 for example */
} directive;

typedef struct {
    char *name;
    Number address;
    LABEL_TYPES type;
    BOOL is_data_label; /* DC label or IC label */
    Number length;
} label;


#endif /* __STRUCTS__ */
