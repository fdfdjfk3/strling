" Language: Strling
" Maintainer: fdfdjfk3
" Latest Revision: 25 October 2023

if exists("b:strling_syntax")
    finish
endif

syntax keyword strlingKw fun
syntax keyword strlingQualifier ref
syntax keyword strlingControlFlow while if elif
syntax keyword strlingControlFlow2 return continue break
syntax keyword strlingComparator and or
syntax match strlingComment "\v\?.*$"
syntax match strlingOperator "\v\+"
syntax match strlingOperator "\v\!\="
syntax match strlingOperator "\v\!\&|\&"
syntax match strlingOperator "\v\="
syntax match strlingOperator "\v\-"
syntax region strlingString start=/\v"/ skip=/\v\\./ end=/\v"/

highlight link strlingOperator Operator
highlight link strlingComment Comment
highlight link strlingString String
highlight link strlingQualifier Special
highlight link strlingKw Keyword
highlight link strlingControlFlow Repeat
highlight link strlingControlFlow2 Statement
highlight link strlingComparator Boolean

let b:strling_syntax = "strling"

