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
	bool   isMBString;
	bool   isWCString;
	bool   useTypeTag;
	bool   used;

	string  name;  // just name: vector, SomeClass;
	string  name2; // name, with template arguments if any: vector<int>, SomeClass;
	string  qname; // qualified name: std::vector<int>, SomeNamespace::SomeClass;
	string  idname; // identification name: std_0_vector_1_int_2_, SomeNamespace_0_SomeClass;

	string  toChars;
	string  gcfields;

	string  set_fields;
	string  type_decls;
	string  type_codes;
	string  meth_decls;
	string  meth_codes;
	string  dao_meths;
	string  alloc_default;
	string  cxxWrapperVirt;
	string  typer_codes;

	vector<string> baseFromHint;

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
