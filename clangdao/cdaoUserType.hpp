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

#ifndef __CDAO_USERTYPE_H__
#define __CDAO_USERTYPE_H__

#include <clang/AST/Decl.h>
#include <clang/AST/DeclCXX.h>

using namespace std;
using namespace llvm;
using namespace clang;

struct CDaoModule;
struct CDaoNamespace;

extern string cdao_qname_to_idname( const string & qname );

enum CDaoUserTypeWrapType
{
	CDAO_WRAP_TYPE_NONE ,   // none wrapping;
	CDAO_WRAP_TYPE_OPAQUE , // wrap as opaque type;
	CDAO_WRAP_TYPE_DIRECT , // wrap for direct member accessing;
	CDAO_WRAP_TYPE_PROXY    // wrap through a proxy struct or class;
};

struct CDaoUserTypeDef
{
	string  nspace;
	string  name;
	string  alias;
	TypedefDecl *td;

	CDaoUserTypeDef(){ td = NULL; }
};

struct CDaoUserType
{
	CDaoModule     *module;
	RecordDecl     *decl;
	SourceLocation  location;

	short  wrapType;
	short  wrapCount;
	short  wrapTypeHint;
	bool   forceOpaque;
	bool   typedefed;
	bool   dummyTemplate;
	bool   unsupported;
	bool   isRedundant;
	bool   isRedundant2;
	bool   isQObject;
	bool   isQObjectBase;
	bool   userItemOper;
	bool   userArithOper;
	bool   useTypeTag;
	bool   used;
	bool   isMBString;
	bool   isWCString;
	short  isNumber;

	string  name;  // just name: vector, SomeClass;
	string  name2; // name, with template arguments if any: vector<int>, SomeClass;
	string  qname; // qualified name: std::vector<int>, SomeNamespace::SomeClass;
	string  idname; // identification name: std_0_vector_1_int_2_, SomeNamespace_0_SomeClass;

	string  macroHeader;
	string  toValue;
	string  gcfields;
	string  hintMacro;
	string  hintDelete;

	string  set_fields;
	string  type_decls;
	string  type_codes;
	string  meth_decls;
	string  meth_codes;
	string  dao_meths;
	string  alloc_default;
	string  cxxWrapperVirt;
	string  typer_codes;
	string  set_bases;

	vector<string> baseFromHint;
	vector<string> extraMethods;

	vector<CDaoUserType*>    priorUserTypes;

	/* All virtual methods (including inherited but not overridden ones)
	// need to be wrapped by the DaoCxx_ classes:
	*/
	map<CXXMethodDecl*,CDaoUserType*>  virtualMethods;

	CDaoUserType( CDaoModule *mod = NULL, const RecordDecl *decl = NULL );

	void SetDeclaration( RecordDecl *decl );
	void SetNamespace( const CDaoNamespace *NS );
	void SearchHints();

	string GetName()const{ return decl ? decl->getNameAsString() : ""; }
	string GetQName()const{ return decl ? decl->getQualifiedNameAsString() : ""; }
	string GetIdName()const{ return cdao_qname_to_idname( GetQName() ); }
	string GetInputFile()const;

	void AddRequiredType( CDaoUserType *UT ){ priorUserTypes.push_back( UT ); }
	void MakeTyperCodes();

	bool IsFromMainModule();
	bool IsFromRequiredModules();
	void Clear();
	int Generate();
	int Generate( RecordDecl *decl );
	int Generate( CXXRecordDecl *decl );
	int GenerateSimpleTyper();
	void WrapField( CXXRecordDecl::field_iterator fit, map<string,string> kvmap );
	void SetupDefaultMapping( map<string,string> & kvmap );
};

#endif
