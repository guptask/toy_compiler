/* Include section */
#include <unistd.h>
#include "parser.h"
#include "type_check.h"
#include "code_generation.h"


/* Macro section */
#define STACK_SIZE 0xFFF

/* Struct/Enum section */
typedef enum parserState_e
{
    PROGRAM = 0,
    PROGRAM_HEADER,
    PROGRAM_BODY,
    DECLARATION,
    PROCEDURE_DECLARATION,
    PROCEDURE_HEADER,
    PARAMETER_LIST,
    PARAMETER,
    PROCEDURE_BODY,
    VARIABLE_DECLARATION,
    STATEMENT,
    PROCEDURE_CALL,
    ASSIGNMENT_STATEMENT,
    IF_STATEMENT,
    LOOP_STATEMENT,
    ASSIGN_OR_PROC_CALL,
    IDENTIFIERS,
    EXPRESSION,
    ARITH_OP,
    RELATION,
    TERM,
    FACTOR,
    NAME,
    OUT_PARAM,
    ARGUMENT_LIST,
    NUMBERS,
    MAX_STATE_NUM

} parserState_t;

typedef struct stackState_s
{
    parserState_t eState;
    unsigned int uiCount;

} stackState_t;


/** Global variable(s) **/

/* Type checking variables */
program_t        *psProgram        = NULL;
unsigned int     uiNestingLevel    = 0;
bool_t           bIsCurrDeclGlobal = FALSE;
bool_t           bIsCurrProc       = FALSE;
tokenListEntry_t *psAuthToken      = NULL;


/** Static variable(s) **/

/* State stack variables */
static parserState_t eParserState        = PROGRAM;
static stackState_t  sStack[STACK_SIZE];
static unsigned int  uiTop               = 0;

/* Type checking variables */
static bool_t     bIsTypeCheckSucc = TRUE;
static dataType_t eExprEval        = UNDEFINED_TYPE;
static unsigned int uiExprEvalReg  = 0;
static dataType_t eAssignStatement = UNDEFINED_TYPE;
static unsigned char ucArgCnt      = 0;

/* Code generation variables */
static bool_t bIsUnaryNegative     = FALSE;
static unsigned int uiNameRegCnt   = 0;
static dataType_t eVarDataType     = UNDEFINED_TYPE;
static unsigned char ucVarSPDisp   = 0;
static bool_t bIsMemAllotted       = FALSE;
static unsigned char ucArgumentNum = MAX_PROC_PARAM_CNT+1;
static char *pcArrSize             = NULL;
static unsigned int uiArrIndexCnt  = 0;
static bool_t bIsGlobalVariable    = FALSE;
static bool_t arrbIsProcArgOut[MAX_PROC_PARAM_CNT] = {FALSE};
static unsigned char ucArgTempCnt  = 0;
static unsigned int uiLoopCount    = 0;
static tokenListEntry_t *arrpsLoopId[MAX_LOOP_NEST_CNT];
static unsigned int uiIfStmtCount  = 0;
static tokenListEntry_t *arrpsIfStmtId[MAX_IF_ELSE_NEST_CNT];
static unsigned int arruiArgRegCnt[MAX_PROC_PARAM_CNT];
static bool_t bIsArgOut            = FALSE;


/* Definition section */

/*Fetch token type from Token Table */
tokenType_t getTokenTypeFromTokTab( tokenListEntry_t *psNode )
{
    if(NULL == psNode)
    {
        printf("No token found for token type search.\n");
        return UNKNOWN_TYPE;
    }

    token_t *tempNode = arrcTokenTable[psNode->uiEntryHashIndex];

    while( NULL != tempNode )
    {
        if( 0 == strcmp(tempNode->pcToken, psNode->pcToken) )
        {
            return tempNode->eTokenType;
        }
        tempNode = tempNode->psNextChainEntry;
    }

    return UNKNOWN_TYPE;
}

/* Push a state into stack */
bool_t stackPush( parserState_t eParState )
{
    if( (uiTop > 0) && (sStack[uiTop-1].eState == eParState) )
    {
        sStack[uiTop-1].uiCount++;
        return TRUE;
    }

    if(uiTop >= STACK_SIZE)
    {
        printf("Stack overflow.\n");
        return FALSE;
    }

    sStack[uiTop].eState  = eParState;
    sStack[uiTop].uiCount = 1;
    uiTop++;

    return TRUE;
}

/* Pop state info from the stack */
stackState_t *stackPop()
{
    if(uiTop == 0)
    {
        return NULL;
    }

    uiTop--;
    if( uiTop > 0 )
    {
        eParserState = sStack[uiTop-1].eState;
    }
    else
    {
        eParserState = MAX_STATE_NUM;
    }

    return &sStack[uiTop];
}

/* State stack re-arrangement for re-sync */
void parseRuleReSync()
{
    /* Re-sync needed if current state is program_body or procedure_body */
    if( (PROGRAM_BODY   == sStack[uiTop-1].eState) || 
        (PROCEDURE_BODY == sStack[uiTop-1].eState)    )
    {
        if( 1 == sStack[uiTop-1].uiCount )
        {
            eParserState = DECLARATION;
        }
        else
        {
            eParserState = STATEMENT;
            sStack[uiTop-1].uiCount--;
        }
    }

    /* NOTE: Need to handle loop_assignment rule */

    return;
}

/* <array_size> ::= <number> and
   <number> ::= [0-9][0-9_]*[.[0-9_]*] (combined since no further expansion)
*/
bool_t numbers( tokenListEntry_t *psToken )
{
    if( NUMBER != getTokenTypeFromTokTab(psToken) )
    {
        //printf("'%s' not a number.\n", psToken->pcToken);
        return FALSE;
    }
    return TRUE;
}

/* <argument_list> ::=   <expression>','<argument_list>
                       | <expression>
*/
bool_t argument_list( tokenListEntry_t *psToken, bool_t *bIsTokIncrNeeded )
{
    dataType_t eDataType = UNDEFINED_TYPE;
    char arrcStr[LENGTH_OF_EACH_LINE] = {0};

    switch( (sStack[uiTop-1].uiCount)%2 )
    {
        case 0:
        {
            ucArgumentNum = MAX_PROC_PARAM_CNT+1;
            bIsArgOut = FALSE;
            eDataType = fetchParamDataType( (unsigned char)((sStack[uiTop-1].uiCount)/2) );
            if( !(eExprEval & eDataType) )
            {
                printf("Procedure '%s' signature mismatch.\n", fetchProcName());
                return FALSE;
            }

            /* Generate the runtime boolean check code */
            if(eDataType == BOOL_TYPE)
            {
                sprintf(arrcStr, "    booleanRuntimeChk( R[%u], %u );\n", uiExprEvalReg, psToken->uiLineNum);
                if( TRUE != genCodeInputString(arrcStr) )
                {
                    bCodeGenErr = TRUE;
                    return FALSE;
                }
            }

            ucArgTempCnt++;

            if( TRUE != writeProcArgs( uiExprEvalReg, (unsigned char)(((sStack[uiTop-1].uiCount)/2))-1) )
            {
                return FALSE;
            }
            
            if( TRUE == arrbIsProcArgOut[((sStack[uiTop-1].uiCount)/2)-1] )
            {
                arruiArgRegCnt[ucArgTempCnt-1] = uiRegCount;
            }

            if(0 != strcmp(psToken->pcToken, ","))
            {
                *bIsTokIncrNeeded = FALSE;
                (void) stackPop();
            }
        } break;

        case 1:
        {
            eParserState = EXPRESSION;
            ucArgumentNum = (unsigned char)((sStack[uiTop-1].uiCount + 1)/2);
            if(ucArgumentNum == 1) ucArgTempCnt = 0;
            if(arrbIsProcArgOut[ucArgTempCnt] == TRUE) bIsArgOut = TRUE;
            *bIsTokIncrNeeded = FALSE;
        } break;
    }

    return TRUE;
}

/* <name> ::= <identifier>['['<expression>']'] */
bool_t name( tokenListEntry_t *psToken, bool_t *bIsTokIncrNeeded )
{
    char arrcStr[LENGTH_OF_EACH_LINE] = {0};
    bool_t bIsGlobalVar = FALSE;

    switch( sStack[uiTop-1].uiCount )
    {
        case 1:
        {
            eParserState = IDENTIFIERS;
            *bIsTokIncrNeeded = FALSE;
            uiNameRegCnt = 0;
            eVarDataType = UNDEFINED_TYPE;
        } break;

        case 2:
        {
            if(TRUE != authVar(&bIsGlobalVar))
            {
                return FALSE;
            }

            /* Check for variable initialization */
            if( (TRUE != fetchMemAlloStatus()) && ((TRUE != bIsGlobalVar) || (uiNestingLevel == 1)) &&
                ( (ucArgumentNum >= MAX_PROC_PARAM_CNT+1) || (TRUE != fetchOutParamStatus(ucArgumentNum)) )
              )
            {
                if(TRUE == bIsGlobalVar)
                {
                    printf("Global variable '%s' needs to be initialized in program body before using anywhere.\n", fetchVarName());
                } else {
                    printf("Bad habit!! Variable '%s' needs to be initialized before use.\n", fetchVarName());
                }
                ucArgumentNum = MAX_PROC_PARAM_CNT+1;
                return FALSE;
            }

            /* Set the memory allocation status to true for out parameters */
            if( TRUE != bIsGlobalVar )
            {
                if( TRUE != fillMemAlloStatus() )
                {
                    return FALSE;
                }
            }

            /* Generate the code */
            sprintf(arrcStr, "    R[%u] = MM[SP+%u];\n", ++uiRegCount, (unsigned int)fetchVarSPDisp());
            if( TRUE != genCodeInputString(arrcStr) )
            {
                bCodeGenErr = TRUE;
                return FALSE;
            }

            if( TRUE != authDataType() )
            {
                return FALSE;
            }
            eVarDataType = fetchDataType();

            if(0 != strcmp(psToken->pcToken, "["))
            {
                (void) stackPop();
                *bIsTokIncrNeeded = FALSE;
                if( TRUE == authArr(TRUE) )
                {
                    return FALSE;
                }

                /* Generate the runtime boolean check code */
                if(eVarDataType == BOOL_TYPE)
                {
                    sprintf(arrcStr, "    booleanRuntimeChk( R[%u], %u );\n", uiRegCount, psToken->uiLineNum);
                    if( TRUE != genCodeInputString(arrcStr) )
                    {
                        bCodeGenErr = TRUE;
                        return FALSE;
                    }
                }

                if( TRUE != popuExprTreeOperand(eVarDataType) )
                {
                    return FALSE;
                }
            }
            else
            {
                if( TRUE != authArr(FALSE) )
                {
                    return FALSE;
                }
                uiNameRegCnt = uiRegCount;
                eParserState = EXPRESSION;
            }
        } break;

        case 3:
        {
            if(0 != strcmp(psToken->pcToken, "]"))
            {
                return FALSE;
            }
            if( ((INTEGER_TYPE+BOOL_TYPE) != eExprEval) && (INTEGER_TYPE != eExprEval) )
            {
                printf("Array index can only be bool (converted into integer) or integer.\n");
                return FALSE;
            }

            /* Generate the code */
            if( (eVarDataType == INTEGER_TYPE) || (eVarDataType == BOOL_TYPE) )
            {
                sprintf(arrcStr, "    R[%u] = *((int *)R[%u] + R[%u]);\n", 
                                    uiNameRegCnt, uiNameRegCnt, uiRegCount);
            } else if(eVarDataType == FLOAT_TYPE) {
                sprintf(arrcStr, "    R[%u] = *((float *)R[%u] + R[%u]);\n", 
                                    uiNameRegCnt, uiNameRegCnt, uiRegCount);
            } else if(eVarDataType == STRING_TYPE) {
                sprintf(arrcStr, "    R[%u] = *((char *)R[%u] + R[%u]);\n", 
                                    uiNameRegCnt, uiNameRegCnt, uiRegCount);
            } else {
                printf("Undefined data type of array.\n");
                return FALSE;
            }
            if( TRUE != genCodeInputString(arrcStr) )
            {
                bCodeGenErr = TRUE;
                return FALSE;
            }

            /* Offset the change in register count due to expression */
            sprintf(arrcStr, "    R[%u] = R[%u];\n", ++uiRegCount, uiNameRegCnt);
            if( TRUE != genCodeInputString(arrcStr) )
            {
                bCodeGenErr = TRUE;
                return FALSE;
            }

            /* Generate the runtime boolean check code */
            if(eVarDataType == BOOL_TYPE)
            {
                sprintf(arrcStr, "    booleanRuntimeChk( R[%u], %u );\n", uiRegCount, psToken->uiLineNum);
                if( TRUE != genCodeInputString(arrcStr) )
                {
                    bCodeGenErr = TRUE;
                    return FALSE;
                }
            }

            if( TRUE != popuExprTreeOperand(eVarDataType) )
            {
                return FALSE;
            }

            (void) stackPop();
        } break;

        default: return FALSE;
    }

    return TRUE;
}

/* <out_param> ::= <identifier> */
bool_t out_param( tokenListEntry_t *psToken, bool_t *bIsTokIncrNeeded )
{
    char arrcStr[LENGTH_OF_EACH_LINE] = {0};
    bool_t bIsGlobalVar = FALSE;

    switch( sStack[uiTop-1].uiCount )
    {
        case 1:
        {
            eParserState = IDENTIFIERS;
            *bIsTokIncrNeeded = FALSE;
            eVarDataType = UNDEFINED_TYPE;
        } break;

        case 2:
        {
            if(TRUE != authVar(&bIsGlobalVar))
            {
                return FALSE;
            }

            /* Check for variable initialization */
            if( (TRUE != fetchMemAlloStatus()) && ((TRUE != bIsGlobalVar) || (uiNestingLevel == 1)) &&
                ( (ucArgumentNum >= MAX_PROC_PARAM_CNT+1) || (TRUE != fetchOutParamStatus(ucArgumentNum)) )
              )
            {
                if(TRUE == bIsGlobalVar)
                {
                    printf("Global variable '%s' needs to be initialized in program body before using anywhere.\n", fetchVarName());
                } else {
                    printf("Bad habit!! Variable '%s' needs to be initialized before use.\n", fetchVarName());
                }
                ucArgumentNum = MAX_PROC_PARAM_CNT+1;
                return FALSE;
            }

            /* Set the memory allocation status to true for out parameters */
            if( TRUE != bIsGlobalVar )
            {
                if( TRUE != fillMemAlloStatus() )
                {
                    return FALSE;
                }
            }

            /* Generate the code */
            sprintf(arrcStr, "    R[%u] = (int)&MM[SP+%u];\n", ++uiRegCount, (unsigned int)fetchVarSPDisp());
            if( TRUE != genCodeInputString(arrcStr) )
            {
                bCodeGenErr = TRUE;
                return FALSE;
            }

            if( TRUE != authDataType() )
            {
                return FALSE;
            }
            eVarDataType = fetchDataType();

            if(0 != strcmp(psToken->pcToken, "["))
            {
                if( TRUE == authArr(TRUE) )
                {
                    return FALSE;
                }

                if( TRUE != popuExprTreeOperand(eVarDataType) )
                {
                    return FALSE;
                }
            }
            else
            {
                if( TRUE != authArr(FALSE) )
                {
                    return FALSE;
                }
                printf("Compiler does not have support for out param array. Sorry.\n");
                return FALSE;
            }
            (void) stackPop();
            *bIsTokIncrNeeded = FALSE;
        } break;

        default: return FALSE;
    }

    return TRUE;


}

/* <factor> ::=   '(' <expression> ')'
                | ['-'] <name>
                | ['-'] <number>
                | <string>
                | 'true'
                | 'false'
*/
bool_t factor( tokenListEntry_t *psToken, bool_t *bIsTokIncrNeeded )
{
    char arrcStr[LENGTH_OF_EACH_LINE] = {0}, arrcTemp[LENGTH_OF_EACH_LINE] = {0};
    unsigned int uiIndex = 0;

    switch( sStack[uiTop-1].uiCount )
    {
        case 1:
        {
            if( (0 == strcmp(psToken->pcToken, "true" )) |
                (0 == strcmp(psToken->pcToken, "false"))  )
            {
                /* Generate the code */
                sprintf(arrcStr, "    R[%u] = %s;\n", ++uiRegCount, psToken->pcToken);
                if( TRUE != genCodeInputString(arrcStr) )
                {
                    bCodeGenErr = TRUE;
                    return FALSE;
                }

                if( TRUE != popuExprTreeOperand(BOOL_TYPE) )
                {
                    return FALSE;
                }

                (void) stackPop();
            }
            else if( STRING == getTokenTypeFromTokTab(psToken) )
            {
                /* Generate the code */
                /* Remove the quotes from the string */
                for( uiIndex = 1; uiIndex < strlen(psToken->pcToken)-1; uiIndex++ )
                {
                    arrcTemp[uiIndex-1] = psToken->pcToken[uiIndex];
                }
                arrcTemp[uiIndex] = '\0';

                sprintf(arrcStr, "    HP = HP - %u;\n", (unsigned int)(strlen(arrcTemp)+1));
                if( TRUE != genCodeInputString(arrcStr) )
                {
                    bCodeGenErr = TRUE;
                    return FALSE;
                }
                sprintf(arrcStr, "    memcpy( &MM[HP], %s, %u );\n", psToken->pcToken, (unsigned int)(strlen(arrcTemp)+1) );
                if( TRUE != genCodeInputString(arrcStr) )
                {
                    bCodeGenErr = TRUE;
                    return FALSE;
                }
                sprintf(arrcStr, "    R[%u] = (int)((char *)&MM[HP]);\n", ++uiRegCount);
                if( TRUE != genCodeInputString(arrcStr) )
                {
                    bCodeGenErr = TRUE;
                    return FALSE;
                }

                if( TRUE != popuExprTreeOperand(STRING_TYPE) )
                {
                    return FALSE;
                }

                (void) stackPop();
            }
            else if(0 == strcmp(psToken->pcToken, "("))
            {
                eParserState = EXPRESSION;
            }
            else 
            {
                if(0 != strcmp(psToken->pcToken, "-"))
                {
                    *bIsTokIncrNeeded = FALSE;
                }
                else
                {
                    if( TRUE != popuExprTreeOperator(psToken->pcToken, TRUE) )
                    {
                        return FALSE;
                    }
                    bIsUnaryNegative = TRUE;
                }
            }
        } break;

        case 2:
        {
            if( 0 == strcmp(psToken->pcToken, ")") )
            {
                sprintf(arrcStr, "    R[%u] = R[%u];\n", ++uiRegCount, uiExprEvalReg);
                if( TRUE != genCodeInputString(arrcStr) )
                {
                    bCodeGenErr = TRUE;
                    return FALSE;
                }

                if( TRUE != popuExprTreeOperand(eExprEval) )
                {
                    return FALSE;
                }

                (void) stackPop();
            }
            else if( TRUE == numbers(psToken) )
            {
                /* Check if number is integer or float */
                if( !strstr(psToken->pcToken, ".") )
                {
                    /* Generate the code */
                    if(bIsUnaryNegative)
                    {
                        sprintf(arrcStr, "    R[%u] = -%s;\n", ++uiRegCount, psToken->pcToken);
                    }
                    else
                    {
                        sprintf(arrcStr, "    R[%u] = %s;\n", ++uiRegCount, psToken->pcToken);
                    }
                    bIsUnaryNegative = FALSE;
                    if( TRUE != genCodeInputString(arrcStr) )
                    {
                        bCodeGenErr = TRUE;
                        return FALSE;
                    }

                    dataType_t eOptional = UNDEFINED_TYPE;
                    if( (0 == strcmp(psToken->pcToken, "0")) || (0 == strcmp(psToken->pcToken, "1")) )
                    {
                        eOptional = BOOL_TYPE;
                    }
                    if( TRUE != popuExprTreeOperand( INTEGER_TYPE + eOptional ) )
                    {
                        return FALSE;
                    }
                }
                else /* When number is float */
                {
                    /* Generate the code */
                    if(bIsUnaryNegative)
                    {
                        sprintf(arrcStr, "    FLOAT_VAR = -%s;\n", psToken->pcToken);
                    }
                    else
                    {
                        sprintf(arrcStr, "    FLOAT_VAR = %s;\n", psToken->pcToken);
                    }
                    bIsUnaryNegative = FALSE;
                    if( TRUE != genCodeInputString(arrcStr) )
                    {
                        bCodeGenErr = TRUE;
                        return FALSE;
                    }
                    sprintf(arrcStr, "    memcpy( &R[%u], &FLOAT_VAR, sizeof(float) );\n", ++uiRegCount);
                    if( TRUE != genCodeInputString(arrcStr) )
                    {
                        bCodeGenErr = TRUE;
                        return FALSE;
                    }

                    if( TRUE != popuExprTreeOperand(FLOAT_TYPE) )
                    {
                        return FALSE;
                    }
                }
                (void) stackPop();
            }
            else
            {
                eParserState = NAME;
                *bIsTokIncrNeeded = FALSE;
            }
        } break;

        case 3:
        {
            (void) stackPop();
            *bIsTokIncrNeeded = FALSE;
        } break;

        default: return FALSE;
    }

    return TRUE;
}

/* <term> ::=   <term> '*' <factor>
              | <term> '/' <factor>
              | <factor>
*/
bool_t term( tokenListEntry_t *psToken, bool_t *bIsTokIncrNeeded )
{
    switch( (sStack[uiTop-1].uiCount)%2 )
    {
        case 0:
        {
            if( (0 != strcmp(psToken->pcToken, "*")) &&
                (0 != strcmp(psToken->pcToken, "/"))   )
            {
                (void) stackPop();
                *bIsTokIncrNeeded = FALSE;
            }
            else
            {
                if( TRUE != popuExprTreeOperator(psToken->pcToken, FALSE) )
                {
                    return FALSE;
                }
            }
        } break;

        case 1:
        {
            eParserState = FACTOR;
            *bIsTokIncrNeeded = FALSE;
        } break;
    }

    return TRUE;
}

/* <relation> ::=   <relation> '<'  <term>
                  | <relation> '>=' <term>
                  | <relation> '<=' <term>
                  | <relation> '>'  <term>
                  | <relation> '==' <term>
                  | <relation> '!=' <term>
                  | <term>
*/
bool_t relation( tokenListEntry_t *psToken, bool_t *bIsTokIncrNeeded )
{
    switch( (sStack[uiTop-1].uiCount)%2 )
    {
        case 0:
        {
            if( (0 != strcmp(psToken->pcToken, "<" )) &&
                (0 != strcmp(psToken->pcToken, ">=")) &&
                (0 != strcmp(psToken->pcToken, "<=")) &&
                (0 != strcmp(psToken->pcToken, ">" )) &&
                (0 != strcmp(psToken->pcToken, "==")) &&
                (0 != strcmp(psToken->pcToken, "!="))   )
            {
                (void) stackPop();
                *bIsTokIncrNeeded = FALSE;
            }
            else
            {
                if( TRUE != popuExprTreeOperator(psToken->pcToken, FALSE) )
                {
                    return FALSE;
                }
            }
        } break;

        case 1:
        {
            eParserState = TERM;
            *bIsTokIncrNeeded = FALSE;
        } break;
    }

    return TRUE;
}

/* <arith_op> ::=   <arith_op> '+' <relation>
                  | <arith_op> '-' <relation>
                  | <relation>
*/
bool_t arith_op( tokenListEntry_t *psToken, bool_t *bIsTokIncrNeeded )
{
    switch( (sStack[uiTop-1].uiCount)%2 )
    {
        case 0:
        {
            if( (0 != strcmp(psToken->pcToken, "+")) &&
                (0 != strcmp(psToken->pcToken, "-"))   )
            {
                (void) stackPop();
                *bIsTokIncrNeeded = FALSE;
            }
            else
            {
                if( TRUE != popuExprTreeOperator(psToken->pcToken, FALSE) )
                {
                    return FALSE;
                }
            }
        } break;

        case 1:
        {
            eParserState = RELATION;
            *bIsTokIncrNeeded = FALSE;
        } break;
    }

    return TRUE;
}

/* <expression> ::=   <expression> '&' <arith_op>
                    | <expression> '|' <arith_op>
                    | ['not'] <arith_op>
*/
bool_t expression( tokenListEntry_t *psToken, bool_t *bIsTokIncrNeeded )
{
    switch( (sStack[uiTop-1].uiCount)%3 )
    {
        case 0:
        {
            if( (0 != strcmp(psToken->pcToken, "&")) &&
                (0 != strcmp(psToken->pcToken, "|"))   )
            {
                (void) stackPop();
                *bIsTokIncrNeeded = FALSE;

                if( UNDEFINED_TYPE == (eExprEval = evalExprTree(&uiExprEvalReg)) )
                {
                    return FALSE;
                }
                eExprEval = eExprEval | INTEGER_TYPE;

                if( TRUE != destroyExprTree() )
                {
                    return FALSE;
                }

                if(EXPR_DEBUG_FLAG)
                {
                    printf("Result: %d\n", eExprEval);
                    printf("End  : %s (%u)\n", psToken->pcToken, psToken->uiLineNum);
                }
            }
            else
            {
                if(bIsArgOut == TRUE)
                {
                    printf("Operator '%s' not allowed for out param.\n", psToken->pcToken);
                    return FALSE;
                }
                if( TRUE != popuExprTreeOperator(psToken->pcToken, FALSE) )
                {
                    return FALSE;
                }
            }
        } break;

        case 1:
        {
            if( 1 == sStack[uiTop-1].uiCount )
            {
                if( TRUE != createExprTree() )
                {
                    return FALSE;
                }
                eExprEval = UNDEFINED_TYPE;
                uiExprEvalReg = 0;

                if(EXPR_DEBUG_FLAG)
                {
                    printf("Begin: %s (%u)\n", psToken->pcToken, psToken->uiLineNum);
                }
            }

            if( 0 != strcmp(psToken->pcToken, "not") )
            {
                *bIsTokIncrNeeded = FALSE;
            }
            else
            {
                if(bIsArgOut == TRUE)
                {
                    printf("Operator '%s' not allowed for out param.\n", psToken->pcToken);
                    return FALSE;
                }
                if( TRUE != popuExprTreeOperator(psToken->pcToken, TRUE) )
                {
                    return FALSE;
                }
            }
        } break;

        case 2:
        {
            if(bIsArgOut == TRUE)
            {
                eParserState = OUT_PARAM;
            } else {
                eParserState = ARITH_OP;
            }
            *bIsTokIncrNeeded = FALSE;
        } break;
    }

    return TRUE;
}

/* <identifier> ::= [a-zA-Z][a-zA-Z0-9_]* */
bool_t identifiers( tokenListEntry_t *psToken, bool_t bIsStkPopNeed )
{
    psAuthToken = psToken;
    if( IDENTIFIER != getTokenTypeFromTokTab(psToken) )
    {
        psAuthToken = NULL;
        return FALSE;
    }

    if( TRUE == bIsStkPopNeed )
    {
        (void) stackPop();
    }

    return TRUE;
}

/* <assign_or_proc_call> ::=   <procedure_call>
                           | <assignment_statement>
*/
bool_t assign_or_proc_call( tokenListEntry_t *psToken, bool_t *bIsTokIncrNeeded )
{
    (void) stackPop();
    *bIsTokIncrNeeded = FALSE;

    if(0 == strcmp(psToken->pcToken, "("))
    {
        eParserState = PROCEDURE_CALL;
    }
    else
    {
        eParserState = ASSIGNMENT_STATEMENT;
    }

    return TRUE;
}

/* <loop_statement> ::=
     'for' '('<assignment_statement>';'
              <expression> ')'
               (<statement>';')*
     'end' 'for'
*/
bool_t loop_statement( tokenListEntry_t *psToken )
{
    char arrcStr[LENGTH_OF_EACH_LINE] = {0};

    switch( (sStack[uiTop-1].uiCount) )
    {
        case 1:
        {
            if(0 != strcmp(psToken->pcToken, "for")) return FALSE;
        } break;

        case 2:
        {
            if(0 != strcmp(psToken->pcToken, "(")) return FALSE;
        } break;

        case 3:
        {
            if (TRUE != identifiers(psToken, FALSE)) return FALSE;
            eParserState = ASSIGNMENT_STATEMENT;
        } break;

        case 4:
        {
            if(0 != strcmp(psToken->pcToken, ";")) return FALSE;
            arrpsLoopId[uiLoopCount++] = psToken;
            sprintf(arrcStr, "\n_loop_%p_ :\n", arrpsLoopId[uiLoopCount-1]);
            if( TRUE != genCodeInputString(arrcStr) )
            {
                bCodeGenErr = TRUE;
                return FALSE;
            }
            eParserState = EXPRESSION;
        } break;

        case 5:
        {
            if(0 != strcmp(psToken->pcToken, ")")) return FALSE;
            sprintf(arrcStr, "    if(!R[%u]) goto _exit_loop_%p_;\n", 
                                            uiExprEvalReg, arrpsLoopId[uiLoopCount-1]);
            if( TRUE != genCodeInputString(arrcStr) )
            {
                bCodeGenErr = TRUE;
                return FALSE;
            }
            eParserState = STATEMENT;
        } break;

        case 6:
        {
            if(0 != strcmp(psToken->pcToken, "end")) return FALSE;
        } break;

        case 7:
        {
            if(0 != strcmp(psToken->pcToken, "for")) return FALSE;
            sprintf(arrcStr, "    goto _loop_%p_;\n\n", arrpsLoopId[uiLoopCount-1]);
            if( TRUE != genCodeInputString(arrcStr) )
            {
                bCodeGenErr = TRUE;
                return FALSE;
            }

            sprintf(arrcStr, "_exit_loop_%p_ :\n", arrpsLoopId[--uiLoopCount]);
            if( TRUE != genCodeInputString(arrcStr) )
            {
                bCodeGenErr = TRUE;
                return FALSE;
            }

            (void) stackPop();
        } break;

        default: return FALSE;
    }
    return TRUE;
}

/* <if_statement> ::=
        'if' '('<expression>')' 'then' (<statement>';')+
         ['else' (<statement>';')+]
         'end' 'if'
 */
bool_t if_statement( tokenListEntry_t *psToken, bool_t *bIsTokIncrNeeded )
{
    char arrcStr[LENGTH_OF_EACH_LINE] = {0};

    switch( (sStack[uiTop-1].uiCount) )
    {
        case 1:
        {
            if(0 != strcmp(psToken->pcToken, "if")) return FALSE;
        } break;

        case 2:
        {
            if(0 != strcmp(psToken->pcToken, "(")) return FALSE;
            eParserState = EXPRESSION;
        } break;

        case 3:
        {
            if(0 != strcmp(psToken->pcToken, ")")) return FALSE;
        } break;

        case 4:
        {
            if(0 != strcmp(psToken->pcToken, "then")) return FALSE;
            arrpsIfStmtId[uiIfStmtCount++] = psToken;
            sprintf(arrcStr, "    if(!R[%u]) goto _else_%p_;\n", 
                        uiExprEvalReg, arrpsIfStmtId[uiIfStmtCount-1]);
            if( TRUE != genCodeInputString(arrcStr) )
            {
                bCodeGenErr = TRUE;
                return FALSE;
            }
            eParserState = STATEMENT;
        } break;

        case 5:
        {
            sprintf(arrcStr, "    goto _exit_if_else_%p_;\n", arrpsIfStmtId[uiIfStmtCount-1]);
            if( TRUE != genCodeInputString(arrcStr) )
            {
                bCodeGenErr = TRUE;
                return FALSE;
            }
            sprintf(arrcStr, "\n_else_%p_ :\n", arrpsIfStmtId[uiIfStmtCount-1]);
            if( TRUE != genCodeInputString(arrcStr) )
            {
                bCodeGenErr = TRUE;
                return FALSE;
            }
            if(0 == strcmp(psToken->pcToken, "else"))
            {
                eParserState = STATEMENT;
            }
            else
            {
                *bIsTokIncrNeeded = FALSE;
            }
        } break;

        case 6:
        {
            if(0 != strcmp(psToken->pcToken, "end")) return FALSE;
        } break;

        case 7:
        {
            if(0 != strcmp(psToken->pcToken, "if")) return FALSE;
            sprintf(arrcStr, "\n_exit_if_else_%p_ :\n", arrpsIfStmtId[--uiIfStmtCount]);
            if( TRUE != genCodeInputString(arrcStr) )
            {
                bCodeGenErr = TRUE;
                return FALSE;
            }
            (void) stackPop();
        } break;

        default: return FALSE;
    }

    return TRUE;
}

/* <assignment_statement> ::= [ '[' <expression> ']' ] ':=' <expression> */
bool_t assignment_statement( tokenListEntry_t *psToken, bool_t *bIsTokIncrNeeded )
{
    char arrcStr[LENGTH_OF_EACH_LINE] = {0}, arrcTmp[LENGTH_OF_EACH_LINE] = {0};

    switch( sStack[uiTop-1].uiCount )
    {
        case 1:
        {
            eAssignStatement = UNDEFINED_TYPE;

            /* Autheticate the variable scope */
            bIsGlobalVariable = FALSE;
            if(TRUE != authVar(&bIsGlobalVariable))
            {
                return FALSE;
            }

            if(0 == strcmp(psToken->pcToken, "["))
            {
                if( TRUE != authArr(FALSE) )
                {
                    return FALSE;
                }
                eParserState = EXPRESSION;
            }
            else if(0 == strcmp(psToken->pcToken, ":="))
            {
                if( TRUE == authArr(TRUE) )
                {
                    return FALSE;
                }
                *bIsTokIncrNeeded = FALSE;
            }
            else
            {
                return FALSE;
            }

            if( TRUE != authDataType() )
            {
                return FALSE;
            }
            eAssignStatement = fetchDataType();

            /* Set the memory allocation flag */
            pcArrSize = fetchArrSize();
            bIsMemAllotted = fetchMemAlloStatus();
            if(TRUE != fillMemAlloStatus())
            {
                return FALSE;
            }
            ucVarSPDisp = fetchVarSPDisp();

        } break;

        case 2:
        {
            if(0 == strcmp(psToken->pcToken, "]"))
            {
                if( ((INTEGER_TYPE+BOOL_TYPE) != eExprEval) && (INTEGER_TYPE != eExprEval) )
                {
                    printf("Array index can only be bool(converted into integer) or integer.\n");
                    return FALSE;
                }

                /* Store the array index for future use */
                if(pcArrSize)
                {
                    uiArrIndexCnt = uiRegCount;
                }

                /* Allocate the array memory */
                if( (TRUE != bIsMemAllotted) && pcArrSize )
                {
                    if(uiLoopCount != 0)
                    {
                        printf("Array has to be initialized before start of loop.\n");
                        return FALSE;
                    }
                    sprintf(arrcStr, "    HP = HP - %s;\n", pcArrSize);
                    if( TRUE != genCodeInputString(arrcStr) )
                    {
                        bCodeGenErr = TRUE;
                        return FALSE;
                    }

                    /* In case of global variable, SP = 0 */
                    if( TRUE == bIsGlobalVariable )
                    {
                        sprintf(arrcStr, "    MM[%u] = (int)&MM[HP];\n", (unsigned int)ucVarSPDisp);
                    } else {
                        sprintf(arrcStr, "    MM[SP+%u] = (int)&MM[HP];\n", (unsigned int)ucVarSPDisp);
                    }
                    if( TRUE != genCodeInputString(arrcStr) )
                    {
                        bCodeGenErr = TRUE;
                        return FALSE;
                    }
                }
            }
            else if(0 == strcmp(psToken->pcToken, ":="))
            {
                *bIsTokIncrNeeded = FALSE;
            }
            else
            {
                return FALSE;
            }
        } break;

        case 3:
        {
            if(0 == strcmp(psToken->pcToken, ":="))
            {
                eParserState = EXPRESSION;
            }
            else
            {
                return FALSE;
            }
        } break;

        case 4:
        {
            if( !(eAssignStatement & eExprEval) )
            {
                printf("Data type mismatch for left and right sides of assignment.\n");
                return FALSE;
            }

            if( !pcArrSize )
            {
                /* In case of global variable, SP = 0 */
                if( TRUE == bIsGlobalVariable )
                {
                    sprintf(arrcStr, "    MM[%u] = R[%u];\n", (unsigned int)ucVarSPDisp, uiExprEvalReg);
                    if(eAssignStatement == BOOL_TYPE)
                    {
                        sprintf(arrcTmp, "    booleanRuntimeChk( MM[%u], %u );\n", 
                                    (unsigned int)ucVarSPDisp, psToken->uiLineNum);
                    }
                } else {
                    sprintf(arrcStr, "    MM[SP+%u] = R[%u];\n", (unsigned int)ucVarSPDisp, uiExprEvalReg);
                    if(eAssignStatement == BOOL_TYPE)
                    {
                        sprintf(arrcTmp, "    booleanRuntimeChk( MM[SP+%u], %u );\n", 
                                    (unsigned int)ucVarSPDisp, psToken->uiLineNum);
                    }
                }
            }
            else
            {
                /* In case of global variable, SP = 0 */
                if( TRUE == bIsGlobalVariable )
                {
                    sprintf(arrcStr, "    *((int *)MM[%u]+R[%u]) = R[%u];\n", 
                            (unsigned int)ucVarSPDisp, uiArrIndexCnt, uiExprEvalReg);
                } else {
                    sprintf(arrcStr, "    *((int *)MM[SP+%u]+R[%u]) = R[%u];\n", 
                            (unsigned int)ucVarSPDisp, uiArrIndexCnt, uiExprEvalReg);
                }

                if(eAssignStatement == BOOL_TYPE)
                {
                    sprintf(arrcTmp, "    booleanRuntimeChk( R[%u], %u );\n", uiExprEvalReg, psToken->uiLineNum);
                }
            }
            if( TRUE != genCodeInputString(arrcStr) )
            {
                bCodeGenErr = TRUE;
                return FALSE;
            }

            if( (BOOL_TYPE == eAssignStatement) && (TRUE != genCodeInputString(arrcTmp)) )
            {
                bCodeGenErr = TRUE;
                return FALSE;
            }

            pcArrSize = NULL;
            uiArrIndexCnt = 0;
            ucVarSPDisp = 0;

            (void) stackPop();
            *bIsTokIncrNeeded = FALSE;
        } break;

        default: return FALSE;
    }

    return TRUE;
}

/* <procedure_call> ::= '(' [<argument_list>] ')' */
bool_t procedure_call( tokenListEntry_t *psToken, bool_t *bIsTokIncrNeeded )
{
    char arrcStr[LENGTH_OF_EACH_LINE] = {0};
    unsigned char ucIndex = 0;

    switch( sStack[uiTop-1].uiCount )
    {
        case 1:
        {
            /* Autheticate the procedure scope */
            if(TRUE != authProc())
            {
                return FALSE;
            }
            if(0 != strcmp(psToken->pcToken, "("))
            {
                return FALSE;
            }
        } break;

        case 2:
        {
            ucArgCnt = fetchParamCnt(arrbIsProcArgOut);
            if( (0 == strcmp(psToken->pcToken, ")")) && ucArgCnt )
            {
                return FALSE;
            }
            if(0 != strcmp(psToken->pcToken, ")"))
            {
                eParserState = ARGUMENT_LIST;
            }
            *bIsTokIncrNeeded = FALSE;
        } break;

        case 3:
        {
            if( (ucArgCnt != ucArgTempCnt) || (0 != strcmp(psToken->pcToken, ")")) )
            {
                return FALSE;
            }
            ucArgCnt = 0;
            if( TRUE != writeProcCall(psToken) )
            {
                return FALSE;
            }

            for(ucIndex = 0; ucIndex < ucArgTempCnt; ucIndex++)
            {
                if(arrbIsProcArgOut[ucIndex] == TRUE)
                {
                    sprintf(arrcStr, "    *((int *)R[%u]) = MM[SP+%u];\n", 
                            arruiArgRegCnt[ucIndex], (unsigned int)ucIndex);
                    if( TRUE != genCodeInputString(arrcStr) )
                    {
                        bCodeGenErr = TRUE;
                        return FALSE;
                    }
                }
            }

            if( TRUE != writeDecrementSP() )
            {
                return FALSE;
            }
            (void) stackPop();

        } break;

        default: return FALSE;
    }

    return TRUE;
}

/* <statement> ::=   <assign_or_proc_call>
                   | <if_statement>
                   | <loop_statement>
                   | 'return'
*/
bool_t statement( tokenListEntry_t *psToken, bool_t *bIsTokIncrNeeded, bool_t bIsPlusRule )
{
    switch( (sStack[uiTop-1].uiCount)%2 )
    {
        case 0:
        {
            if(0 != strcmp(psToken->pcToken, ";"))
            {
                return FALSE;
            }
        } break;

        case 1:
        {
            if(0 == strcmp(psToken->pcToken, "if"))
            {
                eParserState = IF_STATEMENT;
                *bIsTokIncrNeeded = FALSE;
            }
            else if(0 == strcmp(psToken->pcToken, "for"))
            {
                eParserState = LOOP_STATEMENT;
                *bIsTokIncrNeeded = FALSE;
            }
            else if(0 == strcmp(psToken->pcToken, "return"))
            {
                if( TRUE != writeProcReturn() )
                {
                    return FALSE;
                }
            }
            else if (TRUE == identifiers(psToken, FALSE))
            {
                eParserState = ASSIGN_OR_PROC_CALL;
            }
            else
            {
                if( (TRUE == bIsPlusRule) && (1 == sStack[uiTop-1].uiCount) )
                {
                    printf("At least one statement needed.\n");
                    return FALSE;
                }
                (void) stackPop();
                *bIsTokIncrNeeded = FALSE;
            }
        } break;
    }
    return TRUE;
}

/* <type_mark> ::= 'integer' | 'float' | 'bool' | 'string' */
bool_t type_mark( tokenListEntry_t *psToken )
{
    if( (0 != strcmp(psToken->pcToken, "integer")) &&
        (0 != strcmp(psToken->pcToken, "float"))   &&
        (0 != strcmp(psToken->pcToken, "bool"))    &&
        (0 != strcmp(psToken->pcToken, "string"))     )
    {
        return FALSE;
    }
    return TRUE;
}

/* <variable_declaration> ::=
       <type_mark><identifier> [ '['<array_size>']' ]
*/
bool_t variable_declaration( tokenListEntry_t *psToken, bool_t *bIsTokIncrNeeded )
{
    switch(sStack[uiTop-1].uiCount)
    {
        case 1:
        {
            if( TRUE != type_mark(psToken) )
            {
                return FALSE;
            }
            bIsTypeCheckSucc &= fillVarType(psToken);
        } break;

        case 2:
        {
            eParserState = IDENTIFIERS;
            *bIsTokIncrNeeded = FALSE;
            bIsTypeCheckSucc &= fillVarName(psToken);
        } break;

        case 3:
        {
            if( 0 != strcmp(psToken->pcToken, "[") )
            {
                (void) stackPop();
                *bIsTokIncrNeeded = FALSE;
            }
            else
            {
                if(STRING_TYPE == fetchDataType())
                {
                    printf("Sorry! The compiler currently does not support array of strings.\n");
                    return FALSE;
                }
            }
        } break;

        case 4:
        {
            if( TRUE != numbers(psToken) )
            {
                return FALSE;
            }
            bIsTypeCheckSucc &= fillArrSize(psToken);
        } break;

        case 5:
        {
            if( 0 != strcmp(psToken->pcToken, "]") )
            {
                return FALSE;
            }
            (void) stackPop();
        } break;
    }
    return TRUE;
}

/* <procedure_body> ::=
                ( <declaration> ';' )*
        'begin'
                ( <statement> ';' )*
        'end' 'procedure'
*/
bool_t procedure_body( tokenListEntry_t *psToken, bool_t *bIsTokIncrNeeded )
{
    switch( sStack[uiTop-1].uiCount )
    {
        case 1:
        {
            if( 0 != strcmp(psToken->pcToken, "begin") )
            {
                eParserState = DECLARATION;
            }
            *bIsTokIncrNeeded = FALSE;
        } break;

        case 2:
        {
            if( 0 != strcmp(psToken->pcToken, "begin") )
            {
                printf("'begin' reserved word missing on line no. %u.\n", psToken->uiLineNum);
                return FALSE;
            }
            if( TRUE != writeProcLabel() )
            {
                return FALSE;
            }
        } break;

        case 3:
        {
            eParserState = STATEMENT;
            *bIsTokIncrNeeded = FALSE;
        } break;

        case 4:
        {
            if( 0 != strcmp(psToken->pcToken, "end") )
            {
                printf("'end' reserved word missing on line no. %u.\n'", psToken->uiLineNum);
                return FALSE;
            }
        } break;

        case 5:
        {
            if( 0 != strcmp(psToken->pcToken, "procedure") )
            {
                printf("'program' reserved word missing on line no. %u.\n'", psToken->uiLineNum);
                return FALSE;
            }
            if( TRUE != writeProcReturn() )
            {
                return FALSE;
            }
            uiNestingLevel--;
        } break;

        default:
        {
            (void) stackPop();
            *bIsTokIncrNeeded = FALSE;
        }
    }

    return TRUE;
}

/* <parameter> ::= <variable_declaration> ('in'|'out') */
bool_t parameter( tokenListEntry_t *psToken, bool_t *bIsTokIncrNeeded )
{
    switch( sStack[uiTop-1].uiCount )
    {
        case 1:
        {
            eParserState = VARIABLE_DECLARATION;
            *bIsTokIncrNeeded = FALSE;
        } break;

        case 2:
        {
            if( (0 != strcmp(psToken->pcToken,"in")) &&
                (0 != strcmp(psToken->pcToken,"out"))   )
            {
                return FALSE;
            }
            bIsTypeCheckSucc &= fillParamType(psToken);
            (void) stackPop();
        } break;

        default: return FALSE;
    }
    return TRUE;
}

/* <parameter_list> ::= <parameter>','<parameter_list> | <parameter> */
bool_t parameter_list( tokenListEntry_t *psToken, bool_t *bIsTokIncrNeeded )
{
    switch( (sStack[uiTop-1].uiCount)%2 )
    {
        case 0:
        {
            if( 0 != strcmp(psToken->pcToken, ",") )
            {
                (void) stackPop();
                *bIsTokIncrNeeded = FALSE;
            }
        } break;

        case 1:
        {
            eParserState = PARAMETER;
            *bIsTokIncrNeeded = FALSE;
        } break;
    }

    return TRUE;
}

/* <procedure_header> ::=
       'procedure' <identifier> '('[<array_size>]')'
*/
bool_t procedure_header( tokenListEntry_t *psToken, bool_t *bIsTokIncrNeeded )
{
    switch(sStack[uiTop-1].uiCount)
    {
        case 1:
        {
            if( 0 != strcmp( psToken->pcToken, "procedure" ) )
            {
                return FALSE;
            }
            uiNestingLevel++;
        } break;

        case 2:
        {
            eParserState = IDENTIFIERS;
            *bIsTokIncrNeeded = FALSE;
            bIsTypeCheckSucc &= fillProcName(psToken);
        } break;

        case 3:
        {
            if( 0 != strcmp(psToken->pcToken, "(") )
            {
                return FALSE;
            }
        } break;

        case 4:
        {
            if( TRUE == type_mark(psToken) )
            {
                eParserState = PARAMETER_LIST;
            }
            *bIsTokIncrNeeded = FALSE;
        } break;

        case 5:
        {
            if( 0 != strcmp(psToken->pcToken, ")") )
            {
                return FALSE;
            }
            (void) stackPop();
        } break;

        default: return FALSE;
    }
    return TRUE;
}

/* <procedure_declaration> ::= <procedure_header><procedure_body> */
bool_t procedure_declaration( tokenListEntry_t *psToken, bool_t *bIsTokIncrNeeded )
{
    switch(sStack[uiTop-1].uiCount)
    {
        case 1:
        {
            eParserState = PROCEDURE_HEADER;
        } break;

        case 2:
        {
            eParserState = PROCEDURE_BODY;
        } break;

        case 3:
        {
            (void) stackPop();
        } break;

        default:
        {
            printf("'%s' token not part of procedure declaration.\n", psToken->pcToken);
            return FALSE;
        }
    }
    *bIsTokIncrNeeded = FALSE;

    return TRUE;
}

/* <declaration> ::=   ['global'] <procedure_declaration>  | ['global'] <variable_declaration> */
bool_t declaration( tokenListEntry_t *psToken, bool_t bIsGlobal, bool_t *bIsTokIncrNeeded )
{
    switch( (sStack[uiTop-1].uiCount)%3 )
    {
        case 0:
        {
            if( 0 != strcmp(psToken->pcToken, ";") )
            {
                return FALSE;
            }
        } break;

        case 1:
        {
            if( 0 == strcmp(psToken->pcToken, "global") )
            {
                if(FALSE == bIsGlobal)
                {
                    return FALSE;
                }
                bIsCurrDeclGlobal = TRUE;
            }
            else
            {
                *bIsTokIncrNeeded = FALSE;
                bIsCurrDeclGlobal = FALSE;
            }
        } break;

        case 2:
        {
            if( 0 == strcmp(psToken->pcToken, "procedure") )
            {
                bIsCurrProc = TRUE;
                eParserState = PROCEDURE_DECLARATION;
            }
            else if( TRUE == type_mark(psToken) )
            {
                bIsCurrProc = FALSE;
                eParserState = VARIABLE_DECLARATION;
            }
            else
            {
                (void) stackPop();
            }
            *bIsTokIncrNeeded = FALSE;
        }
    }

    return TRUE;
}

/* <program_body> ::=
                ( <declaration> ';' )*
        'begin'
                ( <statement> ';' )*
        'end' 'program'
*/
bool_t program_body( tokenListEntry_t *psToken, bool_t *bIsTokIncrNeeded )
{
    char arrcStr[LENGTH_OF_EACH_LINE] = {0};

    switch( sStack[uiTop-1].uiCount )
    {
        case 1:
        {
            if( 0 != strcmp(psToken->pcToken, "begin") )
            {
                eParserState = DECLARATION;
            }
            *bIsTokIncrNeeded = FALSE;
        } break;

        case 2:
        {
            if( 0 != strcmp(psToken->pcToken, "begin") )
            {
                printf("'begin' reserved word missing on line no. %u.\n", psToken->uiLineNum);
                return FALSE;
            }

            /* Generate the program body tag */
            sprintf(arrcStr, "_program_body_ :\n");
            if( TRUE != genCodeInputString(arrcStr) )
            {
                bCodeGenErr = TRUE;
                return FALSE;
            }
        } break;

        case 3:
        {
            eParserState = STATEMENT;
            *bIsTokIncrNeeded = FALSE;
        } break;

        case 4:
        {
            if( 0 != strcmp(psToken->pcToken, "end") )
            {
                printf("'end' reserved word missing on line no. %u.\n'", psToken->uiLineNum);
                return FALSE;
            }
        } break;

        case 5:
        {
            if( 0 != strcmp(psToken->pcToken, "program") )
            {
                printf("'program' reserved word missing on line no. %u.\n'", psToken->uiLineNum);
                return FALSE;
            }
            uiNestingLevel--;
            (void) stackPop();
            *bIsTokIncrNeeded = FALSE;

            /* Close the main loop */
            if( TRUE != genCodeTermination() )
            {
                bCodeGenErr = TRUE;
                return FALSE;
            }
        } break;

        default:
        {
            printf("Something is wrong in program_body rule handler.\n");
            return FALSE;
        }
    }
    return TRUE;
}

/* <program_header> ::= 'program' <identifier> 'is' */
bool_t program_header( tokenListEntry_t *psToken, bool_t *bIsTokIncrNeeded )
{
    switch( sStack[uiTop-1].uiCount )
    {
        case 1:
        {
            if( 0 != strcmp(psToken->pcToken, "program") )
            {
                printf("'program' reserved word missing on line no. %u.\n", psToken->uiLineNum);
                return FALSE;
            }
            uiNestingLevel++;
        } break;

        case 2:
        {
            eParserState = IDENTIFIERS;
            *bIsTokIncrNeeded = FALSE;
            bIsTypeCheckSucc &= fillProgName(psToken);

            /* Create the generated C code file */
            if( NULL == (pcGenFileName = fetchProgName()) )
            {
                bCodeGenErr = TRUE;
                return FALSE;
            }
            if( NULL == (pcGenFileName = strdup(pcGenFileName)) )
            {
                bCodeGenErr = TRUE;
                return FALSE;
            }
            if( !strcat(pcGenFileName, ".c") )
            {
                bCodeGenErr = TRUE;
                return FALSE;
            }
            if( !(fpGenCode = fopen(pcGenFileName, "w")) )
            {
                bCodeGenErr = TRUE;
                return FALSE;
            }
        } break;

        case 3:
        {
            if( 0 != strcmp(psToken->pcToken, "is") )
            {
                printf("'is' reserved word missing on line no. %u.\n'", psToken->uiLineNum);
                return FALSE;
            }

            /* Initialize the gen file */ 
            if( TRUE != genCodeHeader() )
            {
                bCodeGenErr = TRUE;
                return FALSE;
            }

            if( TRUE != genRuntimeProcCode() )
            {
                bCodeGenErr = TRUE;
                return FALSE;
            }
        } break;

        default:
        {
            (void) stackPop();
            *bIsTokIncrNeeded = FALSE;
        }
    }

    return TRUE;
}

/* <program> ::= <program_header> <program_body> */
bool_t program( tokenListEntry_t *psToken, bool_t *bIsTokIncrNeeded )
{
    switch( sStack[uiTop-1].uiCount )
    {
        case 1:
        {
            eParserState = PROGRAM_HEADER;
            *bIsTokIncrNeeded = FALSE;
        } break;

        case 2:
        {
            eParserState = PROGRAM_BODY;
            *bIsTokIncrNeeded = FALSE;
        } break;

        default:
        {
            (void) stackPop();
            *bIsTokIncrNeeded = FALSE;
            if( uiTop > 0 )
            {
                return FALSE;
            }
        }
    }

    return TRUE;
}

/* API: Parse the code */
bool_t parse( tokenListEntry_t *psTokenList )
{
    tokenListEntry_t *psTempList = psTokenList;
    bool_t bIsIncrNeeded = TRUE, bIsRetSucc = TRUE, bIsReSyncNeeded = FALSE;
    char cGenCodeCompile = 0;

    if(NULL == psTempList)
    {
        printf("No token(s) found for parser.\n");
        return FALSE;
    }

    /* Initialize the type check structure */
    if( FALSE == initTypeChecking() )
    {
        return FALSE;
    }

    while( psTempList )
    {
        bIsIncrNeeded = TRUE;
        if( TRUE != stackPush(eParserState) )
        {
            if( fpGenCode )
            {
                fclose(fpGenCode);
                if( remove(pcGenFileName) )
                {
                    printf("Could not remove invalid file '%s'.\n", pcGenFileName);
                }
            }
            return FALSE;
        }

        switch(eParserState)
        {
            case PROGRAM:
            {
                if(DEBUG_FLAG) printf("program: %s\n", psTempList->pcToken);
                bIsRetSucc = program( psTempList, &bIsIncrNeeded );
            } break;

            case PROGRAM_HEADER:
            {
                if(DEBUG_FLAG) printf("program_header: %s\n", psTempList->pcToken);
                bIsRetSucc = program_header( psTempList, &bIsIncrNeeded );
            } break;

            case PROGRAM_BODY:
            {
                if(DEBUG_FLAG) printf("program_body: %s\n", psTempList->pcToken);
                bIsRetSucc = program_body( psTempList, &bIsIncrNeeded );
            } break;

            case DECLARATION:
            {
                bIsRetSucc = FALSE;
                if(DEBUG_FLAG) printf("declaration: %s\n", psTempList->pcToken);
                if( PROGRAM_BODY == sStack[uiTop-2].eState )
                {
                    bIsRetSucc = declaration( psTempList, TRUE, &bIsIncrNeeded );
                }
                else if( PROCEDURE_BODY == sStack[uiTop-2].eState )
                {
                    bIsRetSucc = declaration( psTempList, FALSE, &bIsIncrNeeded );
                }
                else
                {
                    printf("Declaration rule error on line no. %u.\n", psTempList->uiLineNum);
                }
            } break;

            case PROCEDURE_DECLARATION:
            {
                if(DEBUG_FLAG) printf("procedure_declaration: %s\n", psTempList->pcToken);
                bIsRetSucc = procedure_declaration( psTempList, &bIsIncrNeeded );
            } break;

            case PROCEDURE_HEADER:
            {
                if(DEBUG_FLAG) printf("procedure_header: %s\n", psTempList->pcToken);
                bIsRetSucc = procedure_header( psTempList, &bIsIncrNeeded );
            } break;

            case PARAMETER_LIST:
            {
                if(DEBUG_FLAG) printf("parameter_list: %s\n", psTempList->pcToken);
                bIsRetSucc = parameter_list( psTempList, &bIsIncrNeeded );
            } break;

            case PARAMETER:
            {
                if(DEBUG_FLAG) printf("parameter: %s\n", psTempList->pcToken);
                bIsRetSucc = parameter( psTempList, &bIsIncrNeeded );
            } break;

            case PROCEDURE_BODY:
            { 
                if(DEBUG_FLAG) printf("procedure_body: %s\n", psTempList->pcToken);
                bIsRetSucc = procedure_body( psTempList, &bIsIncrNeeded );
            } break;

            case VARIABLE_DECLARATION:
            {
                if(DEBUG_FLAG) printf("variable_declaration: %s\n", psTempList->pcToken);
                bIsRetSucc = variable_declaration( psTempList, &bIsIncrNeeded );
            } break;

            case STATEMENT:
            {
                if(DEBUG_FLAG) printf("statement: %s\n", psTempList->pcToken);
                if( IF_STATEMENT == sStack[uiTop-2].eState )
                {
                    bIsRetSucc = statement( psTempList, &bIsIncrNeeded, TRUE );
                }
                else
                {
                    bIsRetSucc = statement( psTempList, &bIsIncrNeeded, FALSE );
                }
            } break;

            case PROCEDURE_CALL:
            {
                if(DEBUG_FLAG) printf("procedure_call: %s\n", psTempList->pcToken);
                bIsRetSucc = procedure_call( psTempList, &bIsIncrNeeded );
            } break;

            case ASSIGNMENT_STATEMENT:
            {
                if(DEBUG_FLAG) printf("assignment_statement: %s\n", psTempList->pcToken);
                bIsRetSucc = assignment_statement( psTempList, &bIsIncrNeeded );
            } break;

            case IF_STATEMENT:
            {
                if(DEBUG_FLAG) printf("if_statement: %s\n", psTempList->pcToken);
                bIsRetSucc = if_statement( psTempList, &bIsIncrNeeded );
            } break;

            case LOOP_STATEMENT:
            {
                if(DEBUG_FLAG) printf("loop_statement: %s\n", psTempList->pcToken);
                bIsRetSucc = loop_statement( psTempList );
            } break;

            case ASSIGN_OR_PROC_CALL:
            {
                if(DEBUG_FLAG) printf("assign_or_proc_call: %s\n", psTempList->pcToken);
                bIsRetSucc = assign_or_proc_call( psTempList, &bIsIncrNeeded );
            } break;

            case IDENTIFIERS:
            {
                if(DEBUG_FLAG) printf("identifiers: %s\n", psTempList->pcToken);
                bIsRetSucc = identifiers( psTempList, TRUE );
            } break;

            case EXPRESSION:
            {
                if(DEBUG_FLAG) printf("expression: %s\n", psTempList->pcToken);
                bIsRetSucc = expression( psTempList, &bIsIncrNeeded );
            } break;

            case ARITH_OP:
            {
                if(DEBUG_FLAG) printf("arith_op: %s\n", psTempList->pcToken);
                bIsRetSucc = arith_op( psTempList, &bIsIncrNeeded );
            } break;

            case RELATION:
            {
                if(DEBUG_FLAG) printf("relation: %s\n", psTempList->pcToken);
                bIsRetSucc = relation( psTempList, &bIsIncrNeeded );
            } break;

            case TERM:
            {
                if(DEBUG_FLAG) printf("term: %s\n", psTempList->pcToken);
                bIsRetSucc = term( psTempList, &bIsIncrNeeded );
            } break;

            case FACTOR:
            {
                if(DEBUG_FLAG) printf("factor: %s\n", psTempList->pcToken);
                bIsRetSucc = factor( psTempList, &bIsIncrNeeded );
            } break;

            case NAME:
            {
                if(DEBUG_FLAG) printf("name: %s\n", psTempList->pcToken);
                bIsRetSucc = name( psTempList, &bIsIncrNeeded );
            } break;

            case OUT_PARAM:
            {
                if(DEBUG_FLAG) printf("out_param: %s\n", psTempList->pcToken);
                bIsRetSucc = out_param( psTempList, &bIsIncrNeeded );
            } break;

            case ARGUMENT_LIST:
            {
                if(DEBUG_FLAG) printf("argument_list: %s\n", psTempList->pcToken);
                bIsRetSucc = argument_list( psTempList, &bIsIncrNeeded );
            } break;

            case NUMBERS:
            {
                if(DEBUG_FLAG) printf("numbers: %s\n", psTempList->pcToken);
                bIsRetSucc = numbers( psTempList );
            } break;

            case MAX_STATE_NUM:
            {
                if( psTempList->psNextToken )
                {
                    if(FALSE == bCodeGenErr)
                    {
                        printf("Error. Parsing ends after token '%s' on line no. %u.\n", 
                                          psTempList->pcToken, psTempList->uiLineNum);
                    }
                    if( fpGenCode )
                    {
                        fclose(fpGenCode);
                        if( remove(pcGenFileName) )
                        {
                            printf("Could not remove invalid file '%s'.\n", pcGenFileName);
                        }
                    }
                    return FALSE;
                }
            } break;

            default:
            {
                if(DEBUG_FLAG) printf("Rule no. %d still remains unhandled.\n", eParserState);
                if( fpGenCode )
                {
                    fclose(fpGenCode);
                    if( remove(pcGenFileName) )
                    {
                        printf("Could not remove invalid file '%s'.\n", pcGenFileName);
                    }
                }
                return FALSE;
            }
        }

        if( TRUE != bIsRetSucc )
        {
            if( TRUE == bCodeGenErr )
            {
                printf("Code generation error before or for token '%s' on line no. %u. Exiting.\n", 
                                                        psTempList->pcToken, psTempList->uiLineNum);
                if( fpGenCode )
                {
                    fclose(fpGenCode);
                    if( remove(pcGenFileName) )
                    {
                        printf("Could not remove invalid file '%s'.\n", pcGenFileName);
                    }
                }
                return FALSE;
            }
            printf("Parsing error before or for token '%s' on line no. %u. Re-syncing...\n", 
                                                psTempList->pcToken, psTempList->uiLineNum);

            /* Re-sync */
            bIsReSyncNeeded = TRUE;
            bIsIncrNeeded = TRUE;
            parseRuleReSync();
            do
            {
                psTempList = psTempList->psNextToken;
            }
            while( (NULL != psTempList) && (0 != strcmp(psTempList->pcToken, ";")) );
        }

        if( (TRUE == bIsIncrNeeded) && (NULL != psTempList) )
        {
            psTempList = psTempList->psNextToken;
        }
    }

    if(MAX_STATE_NUM != eParserState)
    {
        printf("Program body not terminated properly.\n");
        if( fpGenCode )
        {
            fclose(fpGenCode);
            if( remove(pcGenFileName) )
            {
                printf("Could not remove invalid file '%s'.\n", pcGenFileName);
            }
        }
        return FALSE;
    }

    if(TRUE == bIsReSyncNeeded)
    {
        printf("Resyncing done. Parsing not entirely successful.\n");
        if( fpGenCode )
        {
            fclose(fpGenCode);
            if( remove(pcGenFileName) )
            {
                printf("Could not remove invalid file '%s'.\n", pcGenFileName);
            }
        }
        return FALSE;
    }

    if(FALSE == bIsTypeCheckSucc)
    {
        printf("Type checking not entirely successful.\n");
        if( fpGenCode )
        {
            fclose(fpGenCode);
            if( remove(pcGenFileName) )
            {
                printf("Could not remove invalid file '%s'.\n", pcGenFileName);
            }
        }
        return FALSE;
    }

    /* Close the generated code file */
    fclose(fpGenCode);
    printf("Generated code file is '%s'.\n", pcGenFileName);

    /* Compile the generated code file */
    printf("Do you want to compile '%s' ? [y/n]: ", pcGenFileName);
    scanf("%c", &cGenCodeCompile);
    system("rm -f a.out");
    if( ('y' == cGenCodeCompile) || ('Y' == cGenCodeCompile) )
    {
        printf("'a.out' will be created on successful compilation.\n");
        if( -1 == execl("/usr/bin/gcc", "gcc", "-w", pcGenFileName, NULL) )
        {
            printf("Compilation of '%s' failed. Try manual compilation.\n", pcGenFileName);
        }
    }
    else
    {
        printf("You chose not to auto-compile '%s' :(\n", pcGenFileName);
    }

    return TRUE;
}

