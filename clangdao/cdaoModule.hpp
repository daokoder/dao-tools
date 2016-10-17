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

#ifndef __CDAO_MODULE_H__
#define __CDAO_MODULE_H__

#include <clang/Basic/FileManager.h>
#include <clang/Lex/MacroInfo.h>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclGroup.h>
#include <clang/AST/DeclCXX.h>
#include <clang/Frontend/CompilerInstance.h>
#include <ostream>
#include <string>
#include <vector>
#include <map>

#include "cdaoFunction.hpp"
#include "cdaoUserType.hpp"
#include "cdaoNamespace.hpp"

using namespace std;
using namespace llvm;
using namespace clang;

extern string normalize_type_name( const string & name );
extern string cdao_qname_to_idname( const string & qname );

struct CDaoModuleInfo
{
	string      name;
	string      alias;
	string      path;
	FileEntry  *entry;
	CDaoModuleInfo(){ entry = NULL; }
};
struct CDaoHeaderInfo
{
	string      module;
	string      path;
	FileEntry  *entry;

	CDaoHeaderInfo( const string & p="", FileEntry *f = NULL, const string &mod="" ){
		path = p;
		entry = f;
		module = mod;
	}
};
struct CDaoInclusionInfo
{
	FileEntry  *includer;
	FileEntry  *includee;

	CDaoInclusionInfo( FileEntry *f1 = NULL, FileEntry *f2 = NULL ){
		includer = f1;
		includee = f2;
	}
	CDaoInclusionInfo( const CDaoInclusionInfo & other ){
		includer = other.includer;
		includee = other.includee;
	}
	bool operator<( const CDaoInclusionInfo & other )const{
		if( includer != other.includer ) return includer < other.includer;
		return includee < other.includee;
	}
};

struct CDaoModule
{
	bool  finalGenerating;
	bool  writeStringListConversion;
	bool  wrapExplicit;
	bool  skipVirtual;
	bool  skipProtected;
	bool  nullPointers;
	bool  variantNumber;
	bool  variantString;

	string onload;

	CompilerInstance  *compiler;
	CDaoModuleInfo     moduleInfo;
	CDaoNamespace      topLevelScope;

	vector<CDaoUserType*>     usertypes;
	vector<CDaoFunction*>     callbacks;
	vector<CDaoUserTypeDef*>  typedefs;

	map<const RecordDecl*,CDaoUserType*>         allUsertypes;
	map<const NamespaceDecl*,CDaoNamespace*>     allNamespaces;
	map<const FunctionProtoType*,CDaoFunction*>  allCallbacks;

	map<string,string> numericConsts;

	map<TypedefDecl*,int>  cxxTypedefs;
	map<string,int>        cxxTypedefs2;
	map<string,string>     daoTypedefs;

	map<FileEntry*,CDaoModuleInfo>  requiredModules; // directly required modules;
	map<FileEntry*,CDaoModuleInfo>  requiredModules2; // directly/indirectly required modules;

	vector<CDaoHeaderInfo> includes; // header files in the including order;

	map<FileEntry*,CDaoHeaderInfo>  headers; // header files from this module;
	map<FileEntry*,CDaoHeaderInfo>  headers2; // direct and indirect header files from this module;
	map<FileEntry*,CDaoHeaderInfo>  extHeaders; // header files from the required modules;
	map<FileEntry*,CDaoHeaderInfo>  extHeaders2; // direct and indirect header files from the required modules;
	map<CDaoInclusionInfo,int>      inclusions;
	map<string,vector<string> >     functionHints;

	static map<string,int>  mapExtensions;

	CDaoModule( CompilerInstance *com, const string & path );

	CDaoUserType* HandleUserType( QualType qtype, SourceLocation, TypedefDecl *TD=NULL );
	CDaoUserType* GetUserType( const RecordDecl *decl );
	CDaoUserType* NewUserType( const RecordDecl *decl );
	CDaoNamespace* GetNamespace( const NamespaceDecl *decl );
	CDaoNamespace* GetNamespace2( const NamespaceDecl *decl );
	CDaoNamespace* NewNamespace( const NamespaceDecl *decl );
	CDaoNamespace* AddNamespace( const NamespaceDecl *decl );

	CDaoHeaderInfo FindModuleInfo( SourceLocation loc );

	int Generate( const string & output = "" );

	int CheckFileExtension( const string & name );
	bool IsHeaderFile( const string & name );
	bool IsSourceFile( const string & name );
	bool IsFromModules( SourceLocation loc );
	bool IsFromMainModule( SourceLocation loc );
	bool IsFromMainModuleSource( SourceLocation loc );
	bool IsFromModuleSources( SourceLocation loc );
	bool IsFromRequiredModules( SourceLocation loc );
	bool CheckHeaderDependency();

	bool WrapExplicit() const { return wrapExplicit; }
	bool UseVariantNumber() const { return variantNumber; }
	bool UseVariantString() const { return variantString; }

	string GetFileName( SourceLocation );

	void HandleModuleDeclaration( const MacroInfo *macro );
	void HandleModuleAlias( const MacroInfo *macro );
	void HandleHeaderInclusion( SourceLocation loc, const string & name, const FileEntry *file );
	void HandleHintDefinition( const string & name, const MacroInfo *macro );
	void HandleNumericConstant( const string & name, const Token token );

	void HandleVariable( VarDecl *var );
	void HandleEnum( EnumDecl *decl );
	void HandleFunction( FunctionDecl *funcdec );
	void HandleUserType( RecordDecl *record );
	void HandleNamespace( NamespaceDecl *nsdecl );
	void HandleTypeDefine( TypedefDecl *decl );

	void WriteHeaderIncludes( std::ostream & stream );

	CDaoUserTypeDef* MakeTypeDefine( TypedefDecl *TD, const string &name );

	string MakeHeaderCodes( vector<CDaoUserType*> & usertypes );
	string MakeSourceCodes( vector<CDaoUserType*> & usertypes, CDaoNamespace *ns = NULL );
	string MakeSource2Codes( vector<CDaoUserType*> & usertypes );
	string MakeSource3Codes( vector<CDaoUserType*> & usertypes );
	string MakeOnLoadCodes( vector<CDaoUserType*> & usertypes, CDaoNamespace *ns = NULL );
	string MakeOnLoad2Codes( vector<CDaoUserType*> & usertypes );

	string MakeSourceCodes( vector<CDaoFunction*> & functions, CDaoNamespace *ns = NULL );
	string MakeOnLoadCodes( vector<CDaoFunction*> & functions, CDaoNamespace *ns = NULL );
	string MakeConstNumItems( vector<EnumDecl*> & enums, vector<VarDecl*> & vars, const string & name = "", bool nested = false );
	string MakeConstNumber( vector<EnumDecl*> & enums, vector<VarDecl*> & vars, const string & name = "", bool isCpp = false );
	string MakeConstStruct( vector<VarDecl*> & vars, const string & ns, const string & name = "" );

	string ExtractSource( SourceLocation & start, SourceLocation & end, bool original = true );
	string ExtractSource( const SourceRange & range, bool original = true );

	static string GetQName( const NamedDecl *D ){
		return D ? normalize_type_name( D->getQualifiedNameAsString() ) : "";
	}
	static string GetIdName( const NamedDecl *D ){
		return cdao_qname_to_idname( GetQName( D ) );
	}
};

#endif
