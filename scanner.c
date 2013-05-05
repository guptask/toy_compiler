/* Include section */
#include "scanner.h"

/* Macro section */
#define RES_WORD_NUM     21
#define RES_WORD_LEN     100
#define OPERATOR_NUM     20
#define OPERATOR_LEN     3
#define MAX_LINE_LEN     1000
#define INVALID_LINE_NUM 0

/* Static section */
static char arrcResWord[RES_WORD_NUM][RES_WORD_LEN] = 
                 { "string"   , "int"  , "bool"  , "float"  ,
                   "global"   , "in"   , "out"   , "if"     ,
                   "then"     , "else" , "case"  , "for"    ,
                   "and"      , "or"   , "not"   , "program",
                   "procedure", "begin", "return", "end"    ,
                   "is"
                 };

static char arrcOperator[OPERATOR_NUM][OPERATOR_LEN] = 
                 {
                   ":"  , ";"  , ","  , "+"  , "-" ,
                   "*"  , "/"  , "("  , ")"  , "=" ,
                   "<"  , "<=" , ">"  , ">=" , "!=" ,
                   ":=" , "{"  , "}"  , "["  , "]"
                 };

/* Definition section */

/* Compute the hash value of token */
bool_t isHashCalcSuccess( char *pcToken, unsigned int *uiHashValue )
{
    unsigned int uiIndex = 0;
    unsigned int uiSum   = 0;

    if( !strlen(pcToken) )
    {
        printf("Token is NULL.\n");
        return FALSE;
    }

    if( strlen(pcToken) >= MAX_TOKEN_LEN )
    {
        printf("Token \"%s\" exceeds max acceptable size.\n", pcToken);
        return FALSE;
    }

    while( pcToken[uiIndex] )
    {
        uiSum += (uiIndex+1)*pcToken[uiIndex]+2;
        uiIndex++;
    }

    *uiHashValue = uiSum % TOKEN_TABLE_SIZE;

    return TRUE;
}

/* Categorize token */
bool_t isCategorizeTokenSuccess( char *pcToken, tokenType_t *eTokenType, unsigned int uiLineNum )
{
    unsigned int uiIndex = 0;
    bool_t eIsErrDetected = FALSE;

    if(pcToken[0] == '"')
    {
        *eTokenType = STRING;
    }
    else if( (pcToken[0] >= '0') && (pcToken[0] <= '9') )
    {
        *eTokenType = NUMBER;
    }
    else if( ((pcToken[0] >= 'A') && (pcToken[0] <= 'Z')) || 
             ((pcToken[0] >= 'a') && (pcToken[0] <= 'z')) )
    {
        *eTokenType = IDENTIFIER;
    }
    else
    {
        *eTokenType = UNKNOWN_TYPE;
        eIsErrDetected = TRUE;
        printf("Error(line %u): Token '%s' could not be categorized.\n", uiLineNum, pcToken);
    }

    /* Verify the rule adherence */
    for(uiIndex = 1; (uiIndex < strlen(pcToken)) && (FALSE == eIsErrDetected); uiIndex++)
    {
        switch(*eTokenType)
        {
            case STRING:
            {
                if(uiIndex < strlen(pcToken)-1)
                {
                    if( !((pcToken[uiIndex] >= 'A') && (pcToken[uiIndex] <= 'Z')) &&
                        !((pcToken[uiIndex] >= 'a') && (pcToken[uiIndex] <= 'z')) &&
                        !((pcToken[uiIndex] >= '0') && (pcToken[uiIndex] <= '9')) &&
                        (pcToken[uiIndex] != ' ')   && (pcToken[uiIndex] != '_')  &&
                        (pcToken[uiIndex] != ',')   && (pcToken[uiIndex] != ';')  &&
                        (pcToken[uiIndex] != ':')   && (pcToken[uiIndex] != '.')  &&
                        (pcToken[uiIndex] != '\'')                                    )
                    {
                        eIsErrDetected = TRUE;
                        printf("Error(line %u): Invalid string '%s'.\n", uiLineNum, pcToken);
                    }
                } else {
                    if(pcToken[uiIndex] != '"') {
                        printf("Error(line %u): String '%s' not terminated.\n", uiLineNum, pcToken);
                        eIsErrDetected = TRUE;
                    }
                }
            } break;

            case NUMBER:
            {
                if( !((pcToken[uiIndex] >= '0') && (pcToken[uiIndex] <= '9')) &&
                    (pcToken[uiIndex] != '.') && 
                    (pcToken[uiIndex] != '_')                                    )
                {
                    eIsErrDetected = TRUE;
                    printf("Error(line %u): Invalid number '%s'.\n", uiLineNum, pcToken);
                }
            } break;

            case IDENTIFIER:
            {
                if( !((pcToken[uiIndex] >= 'A') && (pcToken[uiIndex] <= 'Z')) &&
                    !((pcToken[uiIndex] >= 'a') && (pcToken[uiIndex] <= 'z')) &&
                    !((pcToken[uiIndex] >= '0') && (pcToken[uiIndex] <= '9')) &&
                    (pcToken[uiIndex] != '_')                                    )
                {
                    eIsErrDetected = TRUE;
                    printf("Error(line %u): Invalid identifier '%s'.\n", uiLineNum, pcToken);
                }
            } break;

            default: {}
        }
    }

    /* If error has been detected - might need removal later on */
    if(TRUE == eIsErrDetected)
    {
        *eTokenType = UNKNOWN_TYPE;
    }

    return TRUE;
}

/* Make a new entry into token table */
token_t* tokenEntryIntoTable( char *pcToken, unsigned int uiHashValue, 
                              tokenType_t initEntryType, unsigned int uiLineNum )
{
    token_t *psNewToken = (token_t *) malloc( sizeof(token_t) );
    if( !psNewToken )
    {
        printf("Could not create token entry for \"%s\".\n", pcToken);
        return NULL;
    }

    /* Check whether it is entry of reserved word, operator or unknown category */
    switch( initEntryType )
    {
        case UNKNOWN_TYPE: /* When token type is unknown */
        {
            /* Categorize tokens other than reserved word */
            if( FALSE == isCategorizeTokenSuccess( pcToken, &psNewToken->eTokenType, uiLineNum ) )
            {
                free(psNewToken);
                printf("Failed to categorize token \"%s\".\n", pcToken);
                return NULL;
            }
        } break;

        case RESERVED_WORD : /* When it is reserved word entry during init */
        case OPERATOR      : /* When it is operator entry during init */
        {
            psNewToken->eTokenType = initEntryType;
        } break;

        default:
        {
            printf("Invalid token type sent.\n");
        }
    }

    /* Populate the new token created */
    psNewToken->pcToken = (char *) malloc( strlen(pcToken) );
    strcpy(psNewToken->pcToken, pcToken);
    psNewToken->ucTokenLen = strlen(pcToken);

    psNewToken->psNextChainEntry = arrcTokenTable[uiHashValue];
    arrcTokenTable[uiHashValue] = psNewToken;

#if 0
    /* Print the lookup table nodes */
    printf("token='%s'\ttype=%d\n", psNewToken->pcToken, psNewToken->eTokenType);
#endif

    return psNewToken;
}

/* Initialize token table using reserved words */
bool_t resWordTableEntry()
{
    unsigned char ucIndex  = 0;
    unsigned int uiHashVal = 0;

    /* For each reserved word */
    for(ucIndex = 0; ucIndex < RES_WORD_NUM; ucIndex++ )
    {
        /* Calculate the hash value */
        if( FALSE == isHashCalcSuccess( arrcResWord[ucIndex], &uiHashVal ) )
        {
            printf("Invalid reserved word \"%s\" detected.\n", arrcResWord[ucIndex]);
            return FALSE;
        }

        /* Create a token table entry */
        if( !tokenEntryIntoTable( arrcResWord[ucIndex], uiHashVal, RESERVED_WORD, INVALID_LINE_NUM ) )
        {
            printf("Token table entry for reserved word \"%s\" failed.\n", arrcResWord[ucIndex]);
            return FALSE;
        }
    }

    return TRUE;
}

/* Initialize token table using operators */
bool_t operatorTableEntry()
{
    unsigned char ucIndex  = 0;
    unsigned int uiHashVal = 0;

    /* For each reserved word */
    for(ucIndex = 0; ucIndex < OPERATOR_NUM; ucIndex++ )
    {
        /* Calculate the hash value */
        if( FALSE == isHashCalcSuccess( arrcOperator[ucIndex], &uiHashVal ) )
        {
            printf("Invalid operator \"%s\" detected.\n", arrcOperator[ucIndex]);
            return FALSE;
        }

        /* Create a token table entry */
        if( !tokenEntryIntoTable( arrcOperator[ucIndex], uiHashVal, OPERATOR, INVALID_LINE_NUM ) )
        {
            printf("Token table entry for operator \"%s\" failed.\n", arrcOperator[ucIndex]);
            return FALSE;
        }
    }

    return TRUE;
}

/* Create and populate the next list node */
tokenListEntry_t* formNextListNode( char *pcToken, unsigned int uiLineNum )
{
    /* Calculate the hash value of token */
    unsigned int uiHashValue = 0;
    if( FALSE == isHashCalcSuccess( pcToken, &uiHashValue ) )
    {
        printf("Invalid token \"%s\" detected.\n", pcToken);
        return NULL;
    }

    /* Check for token in hash table */
    token_t *psToken = arrcTokenTable[uiHashValue];
    while ( psToken && strcmp( pcToken, psToken->pcToken ) )
    {
        psToken = psToken->psNextChainEntry;
    }

    /* If token not found in hash table */
    if( !psToken )
    {
        if ( !tokenEntryIntoTable( pcToken, uiHashValue, UNKNOWN_TYPE, uiLineNum ) )
        {
            printf("Token table entry for \"%s\" failed.\n", pcToken);
            return NULL;
        }
    }

    /* Create the new list node */
    tokenListEntry_t *psListEntry = (tokenListEntry_t *) malloc( sizeof(tokenListEntry_t) );
    if( !psListEntry )
    {
        printf("Parser list entry could not be created for \"%s\".\n", pcToken);
        return NULL;
    }

    /* Populate the new list node */
    psListEntry->pcToken = (char *) malloc( strlen(pcToken) );
    strcpy(psListEntry->pcToken, pcToken);
    psListEntry->uiEntryHashIndex = uiHashValue;
    psListEntry->uiLineNum = uiLineNum;
    psListEntry->psNextToken = NULL;

    return psListEntry;
}

/* API: Scan the code */
tokenListEntry_t* scan( FILE *fpCode )
{
    tokenListEntry_t *psListStart = NULL, *psListNext = NULL, *psListTemp = NULL;
    unsigned int uiLineNum = 0, uiTempIndex = 0, uiLineIndex = 0;
    bool_t eIsWithinQuote = FALSE, eHasCmtStarted = FALSE, eIsOperator = FALSE;
    char arrcString[MAX_LINE_LEN], *pcTemp = NULL, arrcToken[MAX_TOKEN_LEN], arrcTemp[MAX_TOKEN_LEN];

    /* Initialize the token table with reserved words */
    if( FALSE == resWordTableEntry() )
    {
        printf("Failed to initialize scanner. Quitting compilation.\n");
        return NULL;
    }

    /* Initialize the token table with operator */
    if( FALSE == operatorTableEntry() )
    {
        printf("Failed to initialize scanner. Quitting compilation.\n");
        return NULL;
    }

    /* Read the entire code file one line at a time in to a temp list */
    while(1)
    {
        arrcString[0] = 0;
        (void) fgets(arrcString, MAX_LINE_LEN, fpCode);
        if(0 == arrcString[0])
        {
            break;
        }

        uiLineIndex++;

        /* Search for operators, etc. */
        pcTemp = strdup(arrcString);
        uiTempIndex = uiLineNum = 0;
        eIsWithinQuote = eHasCmtStarted = eIsOperator = FALSE;

        while(0 != pcTemp[uiTempIndex])
        {
            /* Form the string */
            switch( pcTemp[uiTempIndex] )
            {
                case ' ': case '\t': case '\n':
                {
                    if(FALSE == eIsWithinQuote)
                    {
                        if(uiLineNum > 0)
                        {
                            arrcToken[uiLineNum] = 0;
                            strcpy(arrcTemp, arrcToken);
                            uiLineNum = 0;
                        }
                        while( (pcTemp[uiTempIndex] == ' ')  || 
                               (pcTemp[uiTempIndex] == '\t') || 
                               (pcTemp[uiTempIndex] == '\n')   )
                        {
                            uiTempIndex++;
                        }
                        uiTempIndex--;
                    }
                    else
                    {
                        if(uiLineNum > 0)
                        {
                            arrcToken[uiLineNum++] = pcTemp[uiTempIndex];
                        }
                        else
                        {
                            printf("1. Something wrong with the string detection.\n");
                        }
                    }
                } break;

                case '/':
                {
                    if(FALSE == eIsWithinQuote)
                    {
                        if(uiLineNum > 0)
                        {
                            arrcToken[uiLineNum] = 0;
                            strcpy(arrcTemp, arrcToken);
                            uiLineNum = 0;
                        }

                        if(pcTemp[uiTempIndex+1] == '/')
                        {
                            eHasCmtStarted = TRUE;
                        }
                        else
                        {
                            arrcToken[uiLineNum++] = '/';
                            arrcToken[uiLineNum] = 0;
                            eIsOperator = TRUE;
                        }
                    }
                    else
                    {
                        arrcToken[uiLineNum++] = pcTemp[uiTempIndex];
                    }
                } break;

                case '"':
                {
                    if(TRUE == eIsWithinQuote)
                    {
                        if(uiLineNum > 0)
                        {
                            arrcToken[uiLineNum++] = pcTemp[uiTempIndex];
                            arrcToken[uiLineNum] = 0;
                            strcpy(arrcTemp, arrcToken);
                            uiLineNum = 0;
                        }
                        else
                        {
                            printf("2. Something wrong with the string detection.\n");
                        }
                        eIsWithinQuote = FALSE;
                    }
                    else /* if new string starts */
                    {
                        if(uiLineNum > 0)
                        {
                            arrcToken[uiLineNum] = 0;
                            strcpy(arrcTemp, arrcToken);
                            uiLineNum = 0;
                        }
                        arrcToken[uiLineNum++] = pcTemp[uiTempIndex];
                        eIsWithinQuote = TRUE;
                    }
                } break;

                case ';': case ',': case '+': case '-': case '*': case '(':
                case ')': case '=': case '{': case '}': case '[': case ']':
                {
                    if(FALSE == eIsWithinQuote)
                    {
                        if(uiLineNum > 0)
                        {
                            arrcToken[uiLineNum] = 0;
                            strcpy(arrcTemp, arrcToken);
                            uiLineNum = 0;
                        }
                        arrcToken[uiLineNum++] = pcTemp[uiTempIndex];
                        arrcToken[uiLineNum] = 0;
                        eIsOperator = TRUE;
                    }
                    else /* if part of string */
                    {
                        if(uiLineNum > 0)
                        {
                            arrcToken[uiLineNum++] = pcTemp[uiTempIndex];
                        }
                        else
                        {
                            printf("3. Something wrong with the string detection.\n");
                        }
                    }
                } break;

                case ':': case '<': case '>': case '!':
                {
                    if(FALSE == eIsWithinQuote)
                    {
                        if(uiLineNum > 0)
                        {
                            arrcToken[uiLineNum] = 0;
                            strcpy(arrcTemp, arrcToken);
                            uiLineNum = 0;
                        }
                        arrcToken[uiLineNum++] = pcTemp[uiTempIndex];
                        if(pcTemp[uiTempIndex+1] == '=')
                        {
                            arrcToken[uiLineNum++] = pcTemp[++uiTempIndex];
                        }
                        arrcToken[uiLineNum] = 0;
                        eIsOperator = TRUE;
                    }
                    else /* if part of string */
                    {
                        if(uiLineNum > 0)
                        {
                            arrcToken[uiLineNum++] = pcTemp[uiTempIndex];
                        }
                        else
                        {
                            printf("4. Something wrong with the string detection.\n");
                        }
                    }
                } break;

                default:
                {
                    arrcToken[uiLineNum++] = pcTemp[uiTempIndex];
                }
            }

            /* Create a new token node */
            while( (0 != arrcTemp[0]) || (TRUE == eIsOperator) || (uiTempIndex >= strlen(pcTemp)-1))
            {
                if(0 != arrcTemp[0])
                {
                    /* Create the token node */
                    if( NULL == (psListNext = formNextListNode( arrcTemp, uiLineIndex )) )
                    {
                        printf("Failed to generate scan tokens.\n");
                        return NULL;
                    }
                    arrcTemp[0] = 0;
                    if(NULL == psListStart)
                    {
                        psListStart = psListNext;
                        psListTemp = psListNext;
                    }
                    else
                    {
                        psListTemp->psNextToken = psListNext;
                        psListTemp = psListNext;
                    }
                }

                /* Handle operators */
                if(TRUE == eIsOperator)
                {
                    if(uiLineNum > 0)
                    {
                        strcpy(arrcTemp, arrcToken);
                        uiLineNum = 0;
                        eIsOperator = FALSE;
                    }
                    else
                    {
                        printf("Something wrong with the operator detection.\n");
                        eIsOperator = FALSE;
                    }
                }
                else /* Terminate the last token */
                {
                    if(uiLineNum > 0)
                    {
                        arrcToken[uiLineNum] = 0;
                        strcpy(arrcTemp, arrcToken);
                        uiLineNum = 0;
                    }
                    else
                    {
                        break;
                    }
                }
            }

            /* Handle comment */
            if(TRUE == eHasCmtStarted)
            {
                break;
            }

            uiTempIndex++;
        }

        free(pcTemp);
    }

    return psListStart;
}

