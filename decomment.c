#include <stdio.h>
#include <stdlib.h>

/* Enum defining DFA states */
enum Statetype {
    NORMAL,
    READ_SLASH,
    IN_COMMENT,
    COMMENT_ASTERISK,
    STRING,
    STRING_ESCAPE,
    CHAR,
    CHAR_ESCAPE
};

/* State handler functions */

/* Handle NORMAL state: loops internally processing code until 
   a state-changing character ('/', '"', or '\'') or EOF is hit. 
*/
static enum Statetype handleNormal(int *pc, unsigned int *puiCurLine) {
    while (*pc != EOF) {
        if (*pc == '/') {
            return READ_SLASH;
        }
        if (*pc == '"') {
            putchar(*pc);
            *pc = getchar();
            return STRING;
        }
        if (*pc == '\'') {
            putchar(*pc);
            *pc = getchar();
            return CHAR;
        }

        putchar(*pc);
        if (*pc == '\n')
            (*puiCurLine)++;

        *pc = getchar();
    }
    return NORMAL;
}

/* Handle READ_SLASH state: a single '/' was previously encountered.
   Determines if a comment starts or if '/' was arithmetic/pointer op.
   Returns the next DFA state. */
static enum Statetype handleReadSlash(int *pc, unsigned int *puiCurLine,
                                     unsigned int *puiCommentStartLine) {
    if (*pc == '*') {
        putchar(' ');
        *puiCommentStartLine = *puiCurLine;
        *pc = getchar();
        return IN_COMMENT;
    }
    if (*pc == '/') {
        putchar('/');
        return READ_SLASH;
    }
    if (*pc == '"') {
        putchar('/');
        putchar(*pc);
        *pc = getchar();
        return STRING;
    }
    if (*pc == '\'') {
        putchar('/');
        putchar(*pc);
        *pc = getchar();
        return CHAR;
    }

    putchar('/');
    putchar(*pc);
    if (*pc == '\n')
        (*puiCurLine)++;

    *pc = getchar();
    return NORMAL;
}

/* Handle IN_COMMENT state: loops internally consuming comment body.
   Suppresses character output (except newlines), updates *puiCurLine, 
   and returns when '*' or EOF is hit. */
static enum Statetype handleInComment(int *pc, unsigned int *puiCurLine) {
    while (*pc != EOF) {
        if (*pc == '*') {
            *pc = getchar();
            return COMMENT_ASTERISK;
        }

        if (*pc == '\n') {
            putchar('\n');
            (*puiCurLine)++;
        }

        *pc = getchar();
    }
    return IN_COMMENT;
}

/* Handle COMMENT_ASTERISK state: an '*' was read inside a comment.
   Determines whether comment ends, continues, or hits another '*'.
   Suppresses printing (except newlines), updates *puiCurLine, 
   and returns next state. */
static enum Statetype handleCommentAsterisk(int *pc, 
                                           unsigned int *puiCurLine) {
    if (*pc == '/') {
        *pc = getchar();
        return NORMAL;
    }
    if (*pc == '*') {
        *pc = getchar();
        return COMMENT_ASTERISK;
    }

    if (*pc == '\n') {
        putchar('\n');
        (*puiCurLine)++;
    }

    *pc = getchar();
    return IN_COMMENT;
}

/* Handle STRING state: 
   Prints *pc to stdout, updates *puiCurLine on newlines, and returns 
   when '\\', '"', or EOF is encountered. */
static enum Statetype handleString(int *pc, unsigned int *puiCurLine) {
    while (*pc != EOF) {
        if (*pc == '\\') {
            putchar(*pc);
            *pc = getchar();
            return STRING_ESCAPE;
        }
        if (*pc == '"') {
            putchar(*pc);
            *pc = getchar();
            return NORMAL;
        }

        putchar(*pc);
        if (*pc == '\n')
            (*puiCurLine)++;

        *pc = getchar();
    }
    return STRING;
}

/* Handle STRING_ESCAPE state: handles an escaped character in a string.
 */
static enum Statetype handleStringEscape(int *pc, unsigned int *puiCurLine) {
    putchar(*pc);
    if (*pc == '\n')
        (*puiCurLine)++;

    *pc = getchar();
    return STRING;
}

/* Handle CHAR state: loops internally processing character literals.
*/
static enum Statetype handleChar(int *pc, unsigned int *puiCurLine) {
    while (*pc != EOF) {
        if (*pc == '\\') {
            putchar(*pc);
            *pc = getchar();
            return CHAR_ESCAPE;
        }
        if (*pc == '\'') {
            putchar(*pc);
            *pc = getchar();
            return NORMAL;
        }

        putchar(*pc);
        if (*pc == '\n')
            (*puiCurLine)++;

        *pc = getchar();
    }
    return CHAR;
}

/* Handle CHAR_ESCAPE state: handles an escaped character in char literal.
*/
static enum Statetype handleCharEscape(int *pc, unsigned int *puiCurLine) {
    putchar(*pc);
    if (*pc == '\n')
        (*puiCurLine)++;

    *pc = getchar();
    return CHAR;
}

/* Main driver function for decomment filter.
   Reads C source code from standard input,
   Returns EXIT_SUCCESS or EXIT_FAILURE if unconclusive 
*/
int main(void) {
    int c;
    enum Statetype state = NORMAL;
    unsigned int uiCurLine = 1;
    unsigned int uiCommentStartLine = 0;

    c = getchar();

    while (c != EOF) {
        switch (state) {
        case NORMAL:
            state = handleNormal(&c, &uiCurLine);
            break;
        case READ_SLASH:
            state = handleReadSlash(&c, &uiCurLine, &uiCommentStartLine);
            break;
        case IN_COMMENT:
            state = handleInComment(&c, &uiCurLine);
            break;
        case COMMENT_ASTERISK:
            state = handleCommentAsterisk(&c, &uiCurLine);
            break;
        case STRING:
            state = handleString(&c, &uiCurLine);
            break;
        case STRING_ESCAPE:
            state = handleStringEscape(&c, &uiCurLine);
            break;
        case CHAR:
            state = handleChar(&c, &uiCurLine);
            break;
        case CHAR_ESCAPE:
            state = handleCharEscape(&c, &uiCurLine);
            break;
        }
    }

    /* Edge case: trailing slash at end of file */
    if (state == READ_SLASH) {
        putchar('/');
    }

    /* Check for unterminated comment errors */
    if (state == IN_COMMENT || state == COMMENT_ASTERISK) {
        fprintf(stderr, "Error: line %u: unterminated comment\n", 
                uiCommentStartLine);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}