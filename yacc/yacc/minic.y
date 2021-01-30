%{
/*	minic.y(1.9)	17:46:21	97/12/10
*
*	Parser demo of simple symbol table management and type checking.
*/
#include	<stdio.h>	/* for (f)printf() */
#include	<stdlib.h>	/* for exit() */

#include	"symtab.h"
#include	"types.h"
#include	"check.h"

int		lineno	= 1;	/* number of current source line */
extern int	yylex();	/* lexical analyzer generated from lex.l */
extern char	*yytext;	/* last token, defined in lex.l  */
SYM_TAB 	*scope;		/* current symbol table, initialized in lex.l */
char		*base;		/* basename of command line argument */

void
yyerror(char *s)
{
fprintf(stderr,"Syntax error on line #%d: %s\n",lineno,s);
fprintf(stderr,"Last token was \"%s\"\n",yytext);
exit(1);
}

%}

%union	{
	char*		name;
	int		value;
	T_LIST*		tlist;
	T_INFO*		type;
	SYM_INFO*	sym;
	SYM_LIST*	slist;
	}

%token	INT FLOAT NAME STRUCT IF ELSE RETURN NUMBER LPAR RPAR LBRACE RBRACE epsilon
%token	LBRACE RBRACE ASSIGN SEMICOLON COMMA DOT PLUS MINUS TIMES DIVIDE EQUAL

%type	<name>	NAME
%type	<value>	NUMBER
%type	<type>	type parameter exp lexp
%type	<tlist>	parameters more_parameters exps
%type	<sym>	field var
%type	<slist>	fields

/*	associativity and precedence: in order of increasing precedence */

%nonassoc	LOW  /* dummy token to suggest shift on ELSE */
%nonassoc	ELSE /* higher than LOW */

%nonassoc	EQUAL
%left		PLUS	MINUS
%left		TIMES	DIVIDE
%left		UMINUS	/* dummy token to use as precedence marker */
%left		DOT	LBRACE	/* C compatible precedence rules */

%%
program		: declarations
		;

declarations	: declaration declarations
		| epsilon
		;

declaration	: fun_declaration
		| var_declaration
		;

fun_declaration	: type NAME 
		  LPAR parameters RPAR 
		  block	
		;

parameters	: more_parameters	
		| epsilon
		;

more_parameters	: parameter COMMA more_parameters					
		| parameter		
		;

parameter	: type NAME
		;

block		: LBRACE		
		  var_declarations statements RBRACE
		;

var_declarations : var_declaration var_declarations
		| epsilon
		;

var_declaration	: type NAME SEMICOLON	
		;

type		: INT		
		| FLOAT			
		| type TIMES		
		| STRUCT LBRACE fields RBRACE 			
		;

fields		: field fields		
		| epsilon		
		;

field		: type NAME SEMICOLON	
		;

statements	: statement SEMICOLON statements
		| epsilon
		;

statement	: IF LPAR exp RPAR statement 		
		| IF LPAR exp RPAR statement ELSE statement	
		| lexp ASSIGN exp	{=,exp.val(),@,lexp.val();}
		| RETURN exp 	
		| block
		;

lexp		: var			
		| lexp LBRACE exp RBRACE
		| lexp DOT NAME		
		;

exp		: exp DOT NAME		
		| exp LBRACE exp RBRACE	
		| exp PLUS exp		{res=newTemp();+,exp1.val(),exp2.val(),res;}
		| exp MINUS exp		{res=newTemp();-,exp1.val(),exp2.val(),res;}
		| exp TIMES exp		{res=newTemp();*,exp1.val(),exp2.val(),res;}
		| exp DIVIDE exp	{res=newTemp();/,exp1.val(),exp2.val(),res;}
		| exp EQUAL exp		{res=newTemp();==,exp1.val(),exp2.val(),res;}
		| LPAR exp RPAR
		| MINUS exp 	    {res=newTemp();-,exp1.val(),@,res;}			
		| var			
		| NUMBER 	  {+,@,@,res;}	
		| NAME LPAR RPAR	
		| NAME LPAR exps RPAR	
		;

exps		: exp 			
		| exp COMMA exps	
		;

var		: NAME 	         {+,@,@,res;}
                ;		
%%

int
main(int argc,char *argv[])
{
if (argc!=2) {
	fprintf(stderr,"Usage: %s base_file_name",argv[0]);
	exit(1);
	}
base = argv[1];
return yyparse();
}

