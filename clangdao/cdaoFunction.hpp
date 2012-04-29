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
