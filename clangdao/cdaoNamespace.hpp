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

	bool unsupported;

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
