#ifndef STDBOOL_H
#define STDBOOL_H

#ifndef __bool_true_false_are_defined
    #define bool _Bool
    #define true   1
    #define false  0
    #define __bool_true_false_are_defined 1
#endif

#endif
