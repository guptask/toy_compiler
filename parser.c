/* Include section */
#include "parser.h"
#include "type_check.h"


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
    DEST_OR_PROC_CALL,
    IDENTIFIERS,
    EXPRESSION,
    ARITH_OP,
    RELATION,
    TERM,
    FACTOR,
    NAME,
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
parserState_t eParserState = PROGRAM;

/* State stack variables */
stackState_t sStack[STACK_SIZE];
unsigned int uiTop = 0;

/* Type checking variables */
program_t    *psProgram        = NULL;
unsigned int uiNestingLevel    = 0;
bool_t       bIsCurrDeclGlobal = FALSE;


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

    (void) stackPop();

    return TRUE;
}

/* <argument_list> ::=   <expression>','<argument_list>
                       | <expression>
*/
bool_t argument_list( tokenListEntry_t *psToken, bool_t *bIsTokIncrNeeded )
{
    switch( (sStack[uiTop-1].uiCount)%2 )
    {
        case 0:
        {
            if(0 != strcmp(psToken->pcToken, ","))
            {
                *bIsTokIncrNeeded = FALSE;
                (void) stackPop();
            }
        } break;

        case 1:
        {
            eParserState = EXPRESSION;
            *bIsTokIncrNeeded = FALSE;
        } break;
    }

    return TRUE;
}

/* <name> ::= <identifier>['['<expression>']'] */
bool_t name( tokenListEntry_t *psToken, bool_t *bIsTokIncrNeeded )
{
    switch( sStack[uiTop-1].uiCount )
    {
        case 1:
        {
            eParserState = IDENTIFIERS;
            *bIsTokIncrNeeded = FALSE;
        } break;

        case 2:
        {
            if(0 != strcmp(psToken->pcToken, "["))
            {
                (void) stackPop();
                *bIsTokIncrNeeded = FALSE;
            }
            else
            {
                eParserState = EXPRESSION;
            }
        } break;

        case 3:
        {
            if(0 != strcmp(psToken->pcToken, "]"))
            {
                return FALSE;
            }
            (void) stackPop();
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
    switch( sStack[uiTop-1].uiCount )
    {
        case 1:
        {
            if( (0 == strcmp(psToken->pcToken, "true" ))   |
                (0 == strcmp(psToken->pcToken, "false"))   |
                (STRING == getTokenTypeFromTokTab(psToken)) )
            {
                (void) stackPop();
            }
            else if(0 == strcmp(psToken->pcToken, "("))
            {
                eParserState = EXPRESSION;
            }
            else 
            {
                if(0 != strcmp(psToken->pcToken, "-")) *bIsTokIncrNeeded = FALSE;
            }
        } break;

        case 2:
        {
            if( (0 == strcmp(psToken->pcToken, ")")) || (TRUE == numbers(psToken)) )
            {
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
            }
        } break;

        case 1:
        {
            if( 0 != strcmp(psToken->pcToken, "not") )
            {
                *bIsTokIncrNeeded = FALSE;
            }
        } break;

        case 2:
        {
            eParserState = ARITH_OP;
            *bIsTokIncrNeeded = FALSE;
        } break;
    }

    return TRUE;
}

/* <identifier> ::= [a-zA-Z][a-zA-Z0-9_]* */
bool_t identifiers( tokenListEntry_t *psToken, bool_t bIsStkPopNeed )
{
    if( IDENTIFIER != getTokenTypeFromTokTab(psToken) )
    {
        //printf("'%s' not an identifier.\n", psToken->pcToken);
        return FALSE;
    }

    if( TRUE == bIsStkPopNeed )
    {
        (void) stackPop();
    }

    return TRUE;
}

/* <dest_or_proc_call> ::=   <procedure_call>
                           | <assignment_statement>
*/
bool_t dest_or_proc_call( tokenListEntry_t *psToken, bool_t *bIsTokIncrNeeded )
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
            eParserState = EXPRESSION;
        } break;

        case 5:
        {
            if(0 != strcmp(psToken->pcToken, ")")) return FALSE;
            eParserState = STATEMENT;
        } break;

        case 6:
        {
            if(0 != strcmp(psToken->pcToken, "end")) return FALSE;
        } break;

        case 7:
        {
            if(0 != strcmp(psToken->pcToken, "for")) return FALSE;
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
            eParserState = STATEMENT;
        } break;

        case 5:
        {
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
            (void) stackPop();
        } break;

        default: return FALSE;
    }

    return TRUE;
}

/* <assignment_statement> ::= [ '[' <expression> ']' ] ':=' <expression> */
bool_t assignment_statement( tokenListEntry_t *psToken, bool_t *bIsTokIncrNeeded )
{
    switch( sStack[uiTop-1].uiCount )
    {
        case 1:
        {
            if(0 == strcmp(psToken->pcToken, "["))
            {
                eParserState = EXPRESSION;
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

        case 2:
        {
            if(0 == strcmp(psToken->pcToken, "]")) {}
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
    switch( sStack[uiTop-1].uiCount )
    {
        case 1:
        {
            if(0 != strcmp(psToken->pcToken, "("))
            {
                return FALSE;
            }
        } break;

        case 2:
        {
            if(0 != strcmp(psToken->pcToken, ")"))
            {
                eParserState = ARGUMENT_LIST;
            }
            *bIsTokIncrNeeded = FALSE;
        } break;

        case 3:
        {
            if(0 != strcmp(psToken->pcToken, ")"))
            {
                return FALSE;
            }
            (void) stackPop();
        } break;

        default: return FALSE;
    }

    return TRUE;
}

/* <statement> ::=   <dest_or_proc_call>
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
            else if(0 == strcmp(psToken->pcToken, "return")) {}
            else if (TRUE == identifiers(psToken, FALSE))
            {
                eParserState = DEST_OR_PROC_CALL;
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
            (void) fillVarType(psToken->pcToken);
        } break;

        case 2:
        {
            eParserState = IDENTIFIERS;
            *bIsTokIncrNeeded = FALSE;
            (void) fillVarName(psToken->pcToken);
        } break;

        case 3:
        {
            if( 0 != strcmp(psToken->pcToken, "[") )
            {
                (void) stackPop();
                *bIsTokIncrNeeded = FALSE;
            }
        } break;

        case 4:
        {
            eParserState = NUMBERS;
            *bIsTokIncrNeeded = FALSE;
            (void) fillArrSize(psToken->pcToken);
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
            (void) fillParamType(psToken->pcToken);
            (void) stackPop();
        } break;

        default: return FALSE;
    }
    return TRUE;
}

/* <paramter_list> ::= <parameter>','<parameter_list> | <parameter> */
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
            (void) fillProcName(psToken->pcToken);
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
                eParserState = PROCEDURE_DECLARATION;
            }
            else if( TRUE == type_mark(psToken) )
            {
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
        } break;

        default:
        {
            (void) stackPop();
            *bIsTokIncrNeeded = FALSE;
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
            (void) fillProgName(psToken->pcToken);
        } break;

        case 3:
        {
            if( 0 != strcmp(psToken->pcToken, "is") )
            {
                printf("'is' reserved word missing on line no. %u.\n'", psToken->uiLineNum);
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
void parse( tokenListEntry_t *psTokenList )
{
    tokenListEntry_t *psTempList = psTokenList;
    bool_t bIsIncrNeeded = TRUE, bIsRetSucc = TRUE;
    if(NULL == psTempList)
    {
        printf("No token(s) found for parser.\n");
        return;
    }

    /* Initialize the type check structure */
    typeChkInit();
    if(!psProgram)
    {
        printf("Type check initialization failed.\n");
        return;
    }

    while( psTempList )
    {
        bIsIncrNeeded = TRUE;
        if( TRUE != stackPush(eParserState) )
        {
            return;
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

            case DEST_OR_PROC_CALL:
            {
                if(DEBUG_FLAG) printf("dest_or_proc_call: %s\n", psTempList->pcToken);
                bIsRetSucc = dest_or_proc_call( psTempList, &bIsIncrNeeded );
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
                    printf("Error. Parsing ends after token '%s' on line no. %u.\n", 
                                          psTempList->pcToken, psTempList->uiLineNum);
                }
                return;
            } break;

            default:
            {
                if(DEBUG_FLAG) printf("Rule no. %d still remains unhandled.\n", eParserState);
                return;
            }
        }

        if( TRUE != bIsRetSucc )
        {
            printf("Parsing error before or for token '%s' on line no. %u. Re-syncing...\n", 
                            psTempList->pcToken, psTempList->uiLineNum);

            /* Re-sync */
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

    if( (PROGRAM_BODY != eParserState) || 
        ( (PROGRAM_BODY == eParserState) && (sStack[uiTop-1].uiCount != 5) ) )
    {
        printf("Program body not terminated properly.\n");
    }
}

