
%skeleton "lalr1.cc"
%defines
%define parser_class_name "material_parser"

%code requires {
#include <algorithm> 
namespace my { class Material; };
}

%parse-param { my::Material & material }
%lex-param   { my::Material & material }
%locations

%code {
#include "myUtility.h"
yy::material_parser::token_type yylex(
	yy::material_parser::semantic_type * yylval,
	yy::material_parser::location_type * yylloc,
	my::Material & material);
}

%token IDENTIFIER INTEGER FLOATING STRING

%%

%start assignment_expression_seq;

assignment_expression_seq:
	assignment_expression assignment_expression_seq
	| /*empty*/
	;

assignment_expression:
	IDENTIFIER '=' INTEGER
	| IDENTIFIER '=' FLOATING
	| IDENTIFIER '=' STRING
	;
	
%%

void my::Material::CreateMaterialFromFile(
			LPCSTR pFilename)
{
}

void my::Material::CreateMaterialFromFileInMemory(
			LPCVOID Memory,
			DWORD SizeOfMemory)
{
}
