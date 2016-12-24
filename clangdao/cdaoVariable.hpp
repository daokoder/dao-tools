/*
// ClangDao: the C/C++ library binding tool for Dao
// http://www.daovm.net
//
// Copyright (c) 2011-2014, Limin Fu
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
// SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
// OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __CDAO_VARIABLE_H__
#define __CDAO_VARIABLE_H__

#include <clang/AST/Type.h>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclGroup.h>
#include <string>
#include <stdio.h>

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
	bool    wrapNone;
	bool    wrapOpaque;
	bool    wrapDirect;
	bool    useDefault;
	bool    useTypeTag;
	bool    useDaoString;
	bool    useUserWrapper;
	bool    userFieldCB;
	bool    argvLike;
	bool    ignore;
	bool    readonly; // readonly field;
	bool    ispixels; // field for image pixels
	bool    isbuffer;
	bool    hasDaoTypeHint;
	bool    hasCodeBlockHint;
	bool    hasBaseHint;
	bool    hasMacroHint;
	bool    hasMacro2Hint;
	bool    hasRefCountHint;
	bool    hasExternalUseHint;
	bool    hasDeleteHint;
	bool    hasUniThreadHint;
	bool    isMBS;
	bool    isWCS;
	bool    isNew;
	short   isNumber;
	string  name;
	string  cxxdefault;
	string  daodefault;
	string  hintImplicit;
	string  hintDaoType;
	string  hintCxxType;
	string  hintCxxBase;
	string  hintCodeBlock;
	string  hintMacro;
	string  hintMacro2;
	string  hintRefCount;
	string  hintExternalUse;
	string  hintDelete;

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
