" Vim syntax file
" Language:   mCRL
" Extension:  mcrl
" Maintainer: Aad Mathijssen <A.H.J.Mathijssen@tue.nl>
" Version:    1.0
" Date:       2004-10-15

syntax clear

syntax case match

syntax keyword mcrlSpecKW     sort func map var rew proc act comm init
syntax keyword mcrlProcCKW    delta tau
syntax keyword mcrlProcKW     encap hide rename sum
syntax keyword mcrlTodo       contained TODO FIXME XXX

syntax region mcrlComment     start=/%/ end='$' contains=mcrlTodo

syntax match mcrlIdentifier   /[a-zA-Z0-9^'_\-]\+/
syntax match mcrlOperator     /[.+|<>:=#@(),]/
syntax match mcrlOperator     /|_/
syntax match mcrlOperator     /->/

highlight link mcrlSpecKW     Special
highlight link mcrlProcCKW    Constant
highlight link mcrlProcKW     Statement
highlight link mcrlComment    Comment
highlight link mcrlTodo       Todo
highlight link mcrlIdentifier Identifier
highlight link mcrlOperator   Operator
