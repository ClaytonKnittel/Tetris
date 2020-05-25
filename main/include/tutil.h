#ifndef _TUTIL_H
#define _TUTIL_H

#include <assert.h>
#include <stdlib.h>

#include <print_colors.h>
#include <stdio.h>


#define P_FILE_LINE P_LGREEN __FILE__ P_DEFAULT ":" P_LCYAN "%d" P_DEFAULT


#define TETRIS_ASSERT(expr) \
    do { \
        if (__builtin_expect(!(expr), 0)) { \
            fprintf(stderr, P_FILE_LINE " " P_LRED "assert " P_LYELLOW "\"" \
                    #expr "\"" P_LRED " failed" P_RESET "\n", __LINE__); \
            assert(0); \
        } \
    } while (0)



#endif /* _TUTIL_H */
