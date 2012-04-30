/*
// ClangDao: the C/C++ library binding tool for Dao
// http://www.daovm.net
//
// Copyright (c) 2011,2012, Limin Fu
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
// * Redistributions of source code must retain the above copyright notice, this list
//   of conditions and the following disclaimer.
// 
// * Redistributions in binary form must reproduce the above copyright notice, this list
//   of conditions and the following disclaimer in the documentation and/or other materials
//   provided with the distribution.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
// SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
// OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
// TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
// EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __CDAO_FUNCTION_H__
#define __CDAO_FUNCTION_H__

#include <llvm/ADT/StringExtras.h>
#include <clang/AST/Type.h>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclGroup.h>
#include <map>

#include "cdaoVariable.hpp"

using namespace std;
using namespace llvm;
using namespace clang;

struct CDaoModule;

struct CDaoFunction
{
	CDaoModule           *module;
	FunctionDecl         *funcDecl;
	FunctionProtoType    *funcType;
	FieldDecl            *fieldDecl;
	SourceLocation        location;
	CDaoVariable          retype;
	vector<CDaoVariable>  parlist;

	int   index; // overloading index
	bool  excluded;
	bool  generated;
	bool  constQualified;

	string  signature;
	string  signature2;

	string  cxxName;
	string  cxxWrapName; // such as: dao_host_meth;
	string  cxxWrapper; // wrapper function definition;

	// implement method in DaoCxxVirt_$(type) to call appropriate Dao function:
	string  cxxWrapperVirt;
	// reimplement virtual function in DaoCxx_$(type)
	// by calling method from parent DaoCxxVirt_$(type):
	string  cxxWrapperVirt2;
	// reimplement virtual function in DaoCxxVirt_$(type)
	// by calling method from parent DaoCxxVirt_$(type):
	string  cxxWrapperVirt3;

	string  cxxWrapperVirtProto; // virtual method prototype;

	// Dao routine prototype for the wrapping function:
	// { dao_func_wrapping, "func( vec : array<int>, N : int )" },
	string  daoProtoCodes;

	// C prototype for the wrapping function:
	// void dao_func_wrapping( DaoContext *_ctx, DValue *_p[], int N )
	string  cxxProtoCodes;

	string  cxxCallCodes;
	string  cxxProtoParam;
	string  cxxProtoParamDecl;
	string  cxxProtoParamVirt;
	string  cxxCallParam;
	string  cxxCallParamV;

	string qtSlotSignalDecl;
	string qtSignalSignalDecl;
	string qtSlotSlotDecl;
	string qtSlotSlotCode;
	string qtSignalSlotDecl;
	string qtSignalSlotCode;

	CDaoFunction( CDaoModule *mod = NULL, FunctionDecl *decl = NULL, int idx = 1 );

	void SetDeclaration( FunctionDecl *decl );
	void SetCallback( FunctionProtoType *func, FieldDecl *decl, const string & name="" );
	void SetHints( const vector<string> & hints, const string & sig = "" );

	bool ConstQualified()const{ return constQualified; }
	bool IsFromMainModule();
	string GetInputFile()const;

	int Generate();
};

struct CDaoProxyFunction
{
	bool    used;
	string  name;
	string  codes;

	static int  proxy_function_index;
	static map<string,CDaoProxyFunction>  proxy_functions;

	CDaoProxyFunction( int u=0, const string & n = "", const string & c = "" ){
		used = u;
		name = n;
		codes = c;
	}

	static string NewProxyFunctionName(){
		return "DaoPF" + utohexstr( proxy_function_index ++ );
	}
	static bool IsDefined( const string & signature ){
		return proxy_functions.find( signature ) != proxy_functions.end();
	}
	static bool IsNotDefined( const string & signature ){
		return proxy_functions.find( signature ) == proxy_functions.end();
	}
	static void Add( const string & signature, const string & name, const string & codes ){
		proxy_functions[ signature ] = CDaoProxyFunction( 0, name, codes );
	}
	static void Use( const string & signature ){
		if( IsDefined( signature ) ) proxy_functions[ signature ].used = true;
	}
};

#endif
