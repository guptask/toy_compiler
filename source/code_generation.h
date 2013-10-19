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
bool_t genCodeHeader( FILE *fpCode );

/* Write the termination */
bool_t genCodeTermination( FILE *fpCode );

/* Write the string sent as input */
bool_t genCodeInputString( char *arrcStr, FILE *fpCode );





#endif /* CODE_GENERATION_H_ */

