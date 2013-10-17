/* Include section */
#include "code_generation.h"


/* Macro section */
#define LENGTH_OF_EACH_LINE     100
#define NO_OF_HEADER_LINES        2
#define NO_OF_TERMINATION_LINES   2


/* Static declaration section */
static char arrcHeader[NO_OF_HEADER_LINES][LENGTH_OF_EACH_LINE] = {
    "#include <stdio.h>\n",
    "int main() {\n"
};

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

