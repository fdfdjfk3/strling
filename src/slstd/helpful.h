#ifndef SL_HELPFUL_H
#define SL_HELPFUL_H

#include <stdio.h>

#define $EXPECT_STRID_ARG(varname, index)                                       \
    StrId varname;                                                             \
    if (index >= args.len) {                                                   \
        printf("Too few arguments passed into %s, argument " #index            \
               " not found.\n",                                                \
               __func__);                                                      \
        exit(EXIT_FAILURE);                                                    \
    } else if (args.list[index].is_ref) {                                      \
        printf("Expected argument " #index                                     \
               " of %s to be a String. Found a String Ref "                    \
               "instead.\n",                                                   \
               __func__);                                                      \
        exit(EXIT_FAILURE);                                                    \
    } else {                                                                   \
        varname = args.list[index].value;                                      \
    }

#define $EXPECT_STRID_REF_ARG(varname, index)                                   \
    StrId *varname;                                                            \
    if (index >= args.len) {                                                   \
        printf("Too few arguments passed into %s, argument " #index            \
               " not found.\n",                                                \
               __func__);                                                      \
        exit(EXIT_FAILURE);                                                    \
    } else if (!args.list[index].is_ref) {                                     \
        printf("Expected argument " #index                                     \
               " of %s to be a String Ref. Found a String "                    \
               "instead.\n",                                                   \
               __func__);                                                      \
        exit(EXIT_FAILURE);                                                    \
    } else {                                                                   \
        varname = args.list[index].ref;                                        \
    }

inline void clear_stdin(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
    }
}

#endif
