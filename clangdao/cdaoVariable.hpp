/*
// This file is a part of Dao standard tools.
// Copyright (C) 2006-2012, Limin Fu. Email: daokoder@gmail.com
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy of this 
// software and associated documentation files (the "Software"), to deal in the Software 
// without restriction, including without limitation the rights to use, copy, modify, merge, 
// publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons 
// to whom the Software is furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all copies or 
// substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING 
// BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef __CDAO_VARIABLE_H__
#define __CDAO_VARIABLE_H__

#include <clang/AST/Type.h>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclGroup.h>
#include <string>

#include "cdaoVariable.hpp"

#define VAR_INDEX_RETURN  -1
#define VAR_INDEX_FIELD   -2

using namespace std;
using namespace llvm;
using namespace clang;

struct CDaoModule;
struct CDaoUserType;

struct CDaoVariable
{
	CDaoModule     *module;
	CDaoUserType   *hostype;
	QualType        qualtype;
	SourceLocation  location;
	const Expr     *initor;

	bool    extraReturn;
	bool    isArithmeticType;
	bool    isPointerType;
	bool    isObjectType;
	bool    isNullable;
	bool    isCallback;
	bool    isUserData; // callback userdata
	bool    hasArrayHint;
	bool    unsupported;
	bool    wrapOpaque;
	bool    wrapDirect;
	bool    useDefault;
	bool    useTypeTag;
	bool    useDaoString;
	bool    useUserWrapper;
	bool    argvLike;
	bool    ignore;
	bool    readonly; // readonly field;
	bool    ispixels; // field for image pixels
	bool    isbuffer;
	bool    hasDaoTypeHint;
	bool    hasBaseHint;
	bool    isMBS;
	bool    isWCS;
	string  name;
	string  cxxdefault;
	string  daodefault;
	string  hintDaoType;

	string  daotype;
	string  cxxtype; // original
	string  cxxtype2; // stripped off pointer, refernce, ...
	string  cxxtyper; // typer name for user type;
	string  cxxcall;
	string  daopar;
	string  cxxpar;
	string  dao2cxx;
	string  cxx2dao;
	string  ctxput;
	string  cacheReturn;
	string  cacheParam;
	string  parset;
	string  getres;
	string  getter;
	string  setter;
	string  dao_itemtype;
	string  get_item;
	string  set_item;
	string  pre_call;
	string  post_call;

	string          callback;
	string          userWrapper;
	vector<string>  sizes;
	vector<string>  names;
	vector<string>  scopes;

	CDaoVariable( CDaoModule *mod = NULL, const VarDecl *decl = NULL );

	void SetQualType( QualType qtype, SourceLocation loc = SourceLocation() );
	void SetDeclaration( const VarDecl *decl );
	void SetHints( const string & hints );

	int Generate( int daopar_index = 0, int cxxpar_index = 0 );
	int Generate2( int daopar_index = 0, int cxxpar_index = 0 );
	int GenerateForBuiltin( int daopar_index = 0, int cxxpar_index = 0 );
	int GenerateForPointer( int daopar_index = 0, int cxxpar_index = 0 );
	int GenerateForReference( int daopar_index = 0, int cxxpar_index = 0 );
	int GenerateForArray( int daopar_index = 0, int cxxpar_index = 0 );
	int GenerateForArray( QualType elemtype, string size, int daopar_index = 0, int cxxpar_index = 0, bool constsize = false );
	int GenerateForArray( QualType elemtype, string size, string size2, int dpid = 0, int cpid = 0 );
	int GenerateForArray2( QualType elemtype, string size, string size2, int dpid = 0, int cpid = 0 );

	void MakeCxxParameter( string & prefix, string & suffix );
	void MakeCxxParameter( QualType qtype, string & prefix, string & suffix );
	QualType GetStrippedType( QualType qtype );
	string GetStrippedTypeName( QualType qtype );
};

#endif
