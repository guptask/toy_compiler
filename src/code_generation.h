/*---------------------------*
 |      CODE GENERATION      |
 *---------------------------*/

#ifndef CODE_GENERATION_H_
#define CODE_GENERATION_H_

/* Header section */
#include "scan_parse_common.h"

/** Global variables **/

/* Generated C code file parameters */
extern char *pcGenFileName;
extern FILE *fpGenCode;
extern bool_t bCodeGenErr;

/* Code parameters */
extern unsigned int uiRegCount;
extern unsigned int uiStackPtr;
extern unsigned int uiHeapPtr;
extern unsigned int uiFramePtr;


/* Macro section */
#define LENGTH_OF_EACH_LINE     100


/* Declaration section */

/* Write the header */
bool_t genCodeHeader();

/* Write the termination */
bool_t genCodeTermination();

/* Write the string sent as input */
bool_t genCodeInputString( char *pcStr );

/* Add the runtime definitions */
bool_t genCodeAddRuntimeDef( char *pcProcName );




#endif /* CODE_GENERATION_H_ */

