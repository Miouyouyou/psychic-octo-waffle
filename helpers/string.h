/* I do not know how to inline this... It will stay as a macro until
   I figure out how the "inline" magic work */
#ifndef MYY_HELPERS_STRING_H
#define MYY_HELPERS_STRING_H 1
#define sh_pointToNextString(contiguous_strings) while (*contiguous_strings++)
#endif
