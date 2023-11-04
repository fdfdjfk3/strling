#ifndef SLSTD_ESSENTIAL_H
#define SLSTD_ESSENTIAL_H
#include "../types.h"

StrId SLprint(BuiltinFnArgList args);
StrId SLconcat(StrId str1, StrId str2);
StrId SLremove_occurrences(StrId str, StrId search);
StrId SLintersect(StrId str1, StrId str2);
StrId SLdifference(StrId str1, StrId str2);
StrId SLpop(BuiltinFnArgList args);
StrId SLpopl(BuiltinFnArgList args);

#endif
