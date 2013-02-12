
%{
#include "material_parser.h"
#include <sstream>
#undef yywrap
#define yywrap() 1
#define YY_USER_ACTION  yylloc->columns (yyleng);
typedef yy::material_parser::token token;
#define YY_DECL											\
	yy::material_parser::token_type yylex(				\
		yy::material_parser::semantic_type * yylval,	\
		yy::material_parser::location_type * yylloc,	\
		my::Material & material)
YY_DECL;
#define yyterminate() return token::yytokentype(0)
%}

%option noyywrap nounistd never-interactive

intsuffix				([uU][lL]?)|([lL][uU]?)
fracconst				([0-9]*\.[0-9]+)|([0-9]+\.)
exppart					[eE][-+]?[0-9]+
floatsuffix				[fFlL]
stringtext				([^"])|(\\.)
blank					[ \t\f\v\r]

%%

%{
	yylloc->step();
%}
{blank}+							yylloc->step();
[\n]+								yylloc->lines(yyleng); yylloc->step();
"="									return token::yytokentype('=');
[a-zA-Z_][a-zA-Z_0-9]*				return token::IDENTIFIER;
"0"[xX][0-9a-fA-F]+{intsuffix}?		return token::INTEGER;
"0"[0-7]+{intsuffix}?				return token::INTEGER;
[0-9]+{intsuffix}?					return token::INTEGER;
{fracconst}{exppart}?{floatsuffix}?	return token::FLOATING;
[0-9]+{exppart}{floatsuffix}?		return token::FLOATING;
"\""{stringtext}*"\""				return token::STRING;
.									{
										std::ostringstream ostr;
										ostr << *yylloc;
									}

%%
