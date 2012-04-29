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

#ifndef __CDAO_NAMESPACE_H__
#define __CDAO_NAMESPACE_H__

#include <clang/AST/Decl.h>

#include "cdaoFunction.hpp"
#include "cdaoUserType.hpp"

using namespace std;
using namespace llvm;
using namespace clang;

struct CDaoModule;

struct CDaoNamespace
{
	string          varname;
	CDaoModule     *module;
	NamespaceDecl  *nsdecl;

	vector<CDaoNamespace*>  namespaces;
	vector<CDaoUserType*>   usertypes;
	vector<CDaoFunction*>   functions;
	vector<EnumDecl*>       enums;
	vector<VarDecl*>        variables;
	map<string,int>         overloads;
	map<CDaoUserType*,int>  utcheck;

	string  header;
	string  source;
	string  source2;
	string  source3;
	string  onload;
	string  onload2;
	string  onload3;

	CDaoNamespace( CDaoModule *mod = NULL, const NamespaceDecl *decl = NULL );

	int Generate( CDaoNamespace *outer = NULL );
	void HandleExtension( NamespaceDecl *nsdecl );
	void HandleDeclaration( Decl *D );

	void AddNamespace( CDaoNamespace *one ){ namespaces.push_back( one ); }
	void AddUserType( CDaoUserType *one ){
		if( one == NULL ) return;
		if( utcheck.find( one ) != utcheck.end() ) return;
		utcheck[ one ] = 1;
		usertypes.push_back( one );
		one->SetNamespace( this );
		one->Generate();
	}
	void AddFunction( CDaoFunction *one ){
		functions.push_back( one );
		one->index = ++overloads[ one->funcDecl->getNameAsString() ];
		one->Generate();
	}
	void AddEnumDecl( EnumDecl *one ){ enums.push_back( one ); }
	void AddVarDecl( VarDecl *one ){ variables.push_back( one ); }

	void Sort( vector<CDaoUserType*> & sorted, map<CDaoUserType*,int> & check );
	void Sort( CDaoUserType *UT, vector<CDaoUserType*> & sorted, map<CDaoUserType*,int> & check );
};

#endif
