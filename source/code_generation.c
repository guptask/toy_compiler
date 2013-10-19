/* Include section */
#include "code_generation.h"


/* Macro section */
#define NO_OF_HEADER_LINES        4
#define NO_OF_TERMINATION_LINES   2

/** Global variables **/

/* Generated C code file parameters */
char *pcGenFileName = NULL;
FILE *fpGenCode     = NULL;
bool_t bCodeGenErr  = FALSE;

/* Code parameters */
unsigned int uiRegCount = 0;
unsigned int uiStackPtr = 0;
unsigned int uiHeapPtr  = 0;
unsigned int uiFramePtr = 0;


/** Static variables **/

/* Header details */
static char arrcHeader[NO_OF_HEADER_LINES][LENGTH_OF_EACH_LINE] = {
    "#include <stdio.h>\n",
    "#include \"run_time.h\"\n",
    "int main() {\n",
    "\tgoto _program_body_;\n"
};

/* Termination details */
static char arrcTermination[NO_OF_TERMINATION_LINES][LENGTH_OF_EACH_LINE] = {
    "\n\treturn 0;\n",
    "}\n"
};


/* Definition section */

/* API: Write the header */
bool_t genCodeHeader( FILE *fpCode )
{
    unsigned int uiIndex = 0;
    if( !fpCode )
    {
        return FALSE;
    }
    for(uiIndex = 0; uiIndex < NO_OF_HEADER_LINES; uiIndex++)
    {
        if( EOF == fputs(arrcHeader[uiIndex], fpCode) )
        {
            return FALSE;
        }
    }
    return TRUE;
}

/* API: Write the termination */
bool_t genCodeTermination( FILE *fpCode )
{
    unsigned int uiIndex = 0;
    if( !fpCode )
    {
        return FALSE;
    }
    for(uiIndex = 0; uiIndex < NO_OF_TERMINATION_LINES; uiIndex++)
    {
        if( EOF == fputs(arrcTermination[uiIndex], fpCode) )
        {
            return FALSE;
        }
    }
    return TRUE;
}

/* API: Write the string sent as input */
bool_t genCodeInputString( char *arrcStr, FILE *fpCode )
{
    if( !fpCode )
    {
        return FALSE;
    }
    if( EOF == fputs(arrcStr, fpCode) )
    {
        return FALSE;
    }
    return TRUE;
}

