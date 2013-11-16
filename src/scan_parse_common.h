/*---------------------------*
 |  SCANNER & PARSER COMMON  |
 *---------------------------*/

#ifndef SCAN_PARSE_COMMON_H_
#define SCAN_PARSE_COMMON_H_

/* Header section */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Macro section */
#define TOKEN_TABLE_SIZE 0xFF
#define MAX_TOKEN_LEN    0xFF
#define DEBUG_FLAG       0

/* Enumeration section */
typedef enum bool_e
{
    FALSE = 0,
    TRUE

} bool_t;

typedef enum tokenType_e
{
    UNKNOWN_TYPE = 0,
    IDENTIFIER,
    STRING,
    NUMBER,
    OPERATOR,
    RESERVED_WORD,
    MAX_TOKEN_TYPE

} tokenType_t;

/* Structure/union section */
typedef struct token_s
{
    tokenType_t               eTokenType;
    char                      *pcToken;
    unsigned char             ucTokenLen;
    struct token_s            *psNextChainEntry;

} token_t;

typedef struct tokenListEntry_s
{
    unsigned int              uiEntryHashIndex;
    char                      *pcToken;
    unsigned int              uiLineNum;
    struct tokenListEntry_s   *psNextToken;

} tokenListEntry_t;

/* Global Section */
token_t *arrcTokenTable[TOKEN_TABLE_SIZE];


#endif /* SCAN_PARSE_COMMON_H_ */

