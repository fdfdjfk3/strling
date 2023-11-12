#ifndef SLSTD_ESSENTIAL_H
#define SLSTD_ESSENTIAL_H
#include "../types.h"

StrId SLprint(BuiltinFnArgList args);
StrId SLprintln(BuiltinFnArgList args);
StrId SLgetchar(BuiltinFnArgList args);
StrId SLgetline(BuiltinFnArgList args);
StrId SLconcat(StrId str1, StrId str2);
StrId SLremove_occurrences(StrId str, StrId search);
StrId SLintersect(StrId str1, StrId str2);
StrId SLdifference(StrId str1, StrId str2);
StrId SLpop(BuiltinFnArgList args);
StrId SLpopl(BuiltinFnArgList args);
StrId SLpop_substr(BuiltinFnArgList args);
StrId SLpopl_substr(BuiltinFnArgList args);
StrId SLrev(BuiltinFnArgList args);
StrId SLreplace(BuiltinFnArgList args);

#endif
