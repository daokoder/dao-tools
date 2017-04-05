/*
// ClangDao: the C/C++ library binding tool for Dao
// http://www.daovm.net
//
// Copyright (c) 2011-2017, Limin Fu
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

#include <llvm/Support/Host.h>
#include <llvm/Support/Path.h>
#include <clang/Lex/Token.h>
#include <clang/Lex/MacroInfo.h>
#include <clang/Lex/PPCallbacks.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/AST/DeclCXX.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/CompilerInvocation.h>
#include <clang/Parse/ParseAST.h>
#include <iostream>
#include <string>
#include <vector>

#include "cdaoModule.hpp"

using namespace llvm;
using namespace clang;

static bool clangdao_colons_conversion_simple = true;

struct CDaoPPCallbacks : public PPCallbacks
{
	CompilerInstance *compiler;
	CDaoModule *module;

	CDaoPPCallbacks( CompilerInstance *cinst, CDaoModule *mod ){
		compiler = cinst;
		module = mod;
	}

	void MacroDefined(const Token &MacroNameTok, const MacroDirective *MI);
	void MacroUndefined(const Token &MacroNameTok, const MacroDirective *MI);
	void InclusionDirective(SourceLocation Loc, const Token &Tok, StringRef Name, 
			bool Angled, CharSourceRange FilenameRange, const FileEntry *File,
			StringRef SearchPath, StringRef RelativePath, const Module *Imported );
};

void CDaoPPCallbacks::MacroDefined(const Token &MacroNameTok, const MacroDirective *MD)
{
	const MacroInfo *MI = MD->getMacroInfo();
	SourceLocation loc = MI->getDefinitionLoc();
	SourceManager & sourceman = compiler->getSourceManager();
	llvm::StringRef name = MacroNameTok.getIdentifierInfo()->getName();


	if( MI->getNumTokens() < 1 ){ // number of expansion tokens;
		if( not sourceman.isInMainFile( loc ) ) return;
		if( MI->isObjectLike() && name == "CLANGDAO_WRAP_EXPLICIT" ){
			module->wrapExplicit = true;
		}else if( MI->isObjectLike() && name == "CLANGDAO_SKIP_VIRTUAL" ){
			module->skipVirtual = true;
		}else if( MI->isObjectLike() && name == "CLANGDAO_SKIP_PROTECTED" ){
			module->skipProtected = true;
		}else if( MI->isObjectLike() && name == "CLANGDAO_NULLABLE_POINTERS" ){
			module->nullPointers = true;
		}else if( MI->isObjectLike() && name == "CLANGDAO_VARIANT_NUMBER" ){
			module->variantNumber = true;
		}else if( MI->isObjectLike() && name == "CLANGDAO_VARIANT_STRING" ){
			module->variantString = true;
		}else if( MI->isObjectLike() && name == "CLANGDAO_UNIQUE_NAME" ){
			clangdao_colons_conversion_simple = false;
		}
		return;
	}
	if( MI->isObjectLike() && name == "module_name" ){
		module->HandleModuleDeclaration( MI );
	}else if( MI->isObjectLike() && name == "module_alias" ){
		module->HandleModuleAlias( MI );
	}else if( MI->isObjectLike() && name == "module_onload" ){
		if( not sourceman.isInMainFile( loc ) ) return;
		module->onload = MI->getReplacementToken( 0 ).getIdentifierInfo()->getName();
	}else if( MI->isObjectLike() && MI->getNumTokens() == 1 ){
		if( not module->IsFromMainModule(loc) ) return;
		Token tok = *MI->tokens_begin();
		if( tok.getKind() == tok::numeric_constant ){
			if( name[0] != '_' ) module->HandleNumericConstant( name, tok );
		}
	}else if( MI->isFunctionLike() ){
		module->HandleHintDefinition( name, MI );
	}
}
void CDaoPPCallbacks::MacroUndefined(const Token &MacroNameTok, const MacroDirective *MI)
{
	llvm::StringRef name = MacroNameTok.getIdentifierInfo()->getName();
	module->numericConsts.erase( name );
}
void CDaoPPCallbacks::InclusionDirective(SourceLocation Loc, const Token &Tok, 
		StringRef Name, bool Angled, CharSourceRange FilenameRange, const FileEntry *File, 
		StringRef SearchPath, StringRef RelativePath, const Module *Imported )
{
	module->HandleHeaderInclusion( Loc, Name.str(), File );
}



struct CDaoASTConsumer : public ASTConsumer
{
	CompilerInstance *compiler;
	CDaoModule *module;

	CDaoASTConsumer( CompilerInstance *cinst, CDaoModule *mod ){
		compiler = cinst;
		module = mod;
	}
	CDaoASTConsumer( const CDaoASTConsumer & other ){
		compiler = other.compiler;
		module = other.module;
	}
	bool HandleTopLevelDecl(DeclGroupRef group);
	void HandleDeclaration( Decl *D );
};

bool CDaoASTConsumer::HandleTopLevelDecl(DeclGroupRef group)
{
	for (DeclGroupRef::iterator it = group.begin(); it != group.end(); ++it) {
		HandleDeclaration( *it );
	}
	return true;
}
void CDaoASTConsumer::HandleDeclaration( Decl *D )
{
	if( module->WrapExplicit() and not module->IsFromMainModuleSource(D->getLocation()) ) return;
	if( LinkageSpecDecl *TUD = dyn_cast<LinkageSpecDecl>(D) ){
		DeclContext::decl_iterator it, end;
		for(it=TUD->decls_begin(),end=TUD->decls_end(); it!=end; it++){
			HandleDeclaration( *it );
		}
	}
	if (VarDecl *var = dyn_cast<VarDecl>(D)) {
		module->HandleVariable( var );
	}else if (EnumDecl *e = dyn_cast<EnumDecl>(D)) {
		module->HandleEnum( e );
	}else if (FunctionDecl *func = dyn_cast<FunctionDecl>(D)) {
		module->HandleFunction( func );
	}else if (RecordDecl *record = dyn_cast<RecordDecl>(D)) {
		module->HandleUserType( record );
	}else if (NamespaceDecl *nsdecl = dyn_cast<NamespaceDecl>(D)) {
		module->HandleNamespace( nsdecl );
	}else if( TypedefDecl *decl = dyn_cast<TypedefDecl>(D) ){
		module->HandleTypeDefine( decl );
	}
}

string cdao_string_fill( const string & tpl, const map<string,string> & subs )
{
	string result;
	size_t i, rb, prev = 0, pos = tpl.find( '$', 0 );
	while( pos != string::npos ){
		string gap = tpl.substr( prev, pos - prev );
		result += gap;
		if( tpl[pos+1] == '(' and (rb = tpl.find( ')', pos ) ) != string::npos ){
			string key = tpl.substr( pos+2, rb - pos - 2 );
			map<string,string>::const_iterator it = subs.find( key );
			bool fill = it != subs.end();
			for(i=pos+2; fill and i<rb; i++) fill &= tpl[i] == '_' or isalnum( tpl[i] );
			if( fill ){
				result += it->second;
				prev = rb + 1;
				pos = tpl.find( '$', prev );
				continue;
			}
		}
		result += '$';
		prev = pos + 1;
		pos = tpl.find( '$', prev );
	}
	string gap = tpl.substr( prev, tpl.size() - prev );
	result += gap;
	return result;
}
void remove_type_prefix( string & name, const string & key )
{
	string key2 = key + " ";
	size_t klen = key2.size();
	size_t pos, from = 0;
	while( (pos = name.find( key2, from )) != string::npos ){
		if( pos == 0 || (isalnum( name[pos-1] ) ==0 && name[pos-1] != '_') ){
			name.replace( pos, klen, "" );
		}else{
			from = pos + klen;
		}
	}
}
string normalize_type_name( const string & name0 )
{
	string name = name0;
	size_t pos, from = 0;
	while( (pos = name.find( "\t" )) != string::npos ) name.replace( pos, 1, "" );
	while( (pos = name.find( "\n" )) != string::npos ) name.replace( pos, 1, "" );
	while( (pos = name.find( "  " )) != string::npos ) name.replace( pos, 2, " " );
	from = 0;
	while( (pos = name.find( " ", from )) != string::npos ){
		if( pos == 0 || (pos+1) == name.size() ){
			name.replace( pos, 1, "" );
			continue;
		}
		from = pos + 1;
		char prev = name[pos-1], next = name[pos+1];
		if( prev == '>' && next == '>' ) continue;
		if( (isalnum( prev ) || prev == '_') && (isalnum( next ) || next == '_') ) continue;
		name.replace( pos, 1, "" );
	}
	remove_type_prefix( name, "class" );
	remove_type_prefix( name, "struct" );
	remove_type_prefix( name, "union" );
	remove_type_prefix( name, "enum" );
	remove_type_prefix( name, "typename" );
	return name;
}
static bool is_invalid_dao_type_name( const string & name )
{
	int i, n = name.size();
	if( n == 0 ) return true;
	if( name[0] == '@' ){
		for(i=1; i<n; i++){
			char ch = name[i];
			if( isalnum( ch ) == 0 && ch != '_' ) return true;
		}
		return false;
	}
	for(i=0; i<n; i++){
		char ch = name[i];
		if( isalnum( ch ) == 0 && ch != '_' && ch != ':' ) return true;
	}
	return false;
}
map<string,int> type_for_quoting;
map<string,string> type_substitutions;
string cdao_make_dao_template_type_name( const string & name0, const map<string,string> & subs, const map<string,int> & type_for_quoting )
{
	map<string,string>::const_iterator it;
	string name = normalize_type_name( name0 );
	string result, part;
	int i, n;
	for(i=0, n = name.size(); i<n; i++){
		char ch = name[i];
		if( ch == '<' || ch == '>' || ch == ',' ){
			if( part != "" && part != " " ){
				string quote = is_invalid_dao_type_name( part ) ? "'" : "";
				it = subs.find( part );
				if( it != subs.end() ) part = it->second;
				if( part.find( "std::" ) == 0 ) part.replace( 0, 5, "_std::" );
				if( part.find( "io::" ) == 0 ) part.replace( 0, 4, "_io::" );
				if( type_for_quoting.find( part ) != type_for_quoting.end() ) quote = "'";
				if( type_for_quoting.size() == 0 ) quote = "";
				result += quote + part + quote;
			}
			if( ch == '>' && result[result.size()-1] == '>' ) result += ' ';
			result += ch;
			part = "";
		}else{
			part += ch;
		}
	}
	if( part == "" ) return result;
	string quote = is_invalid_dao_type_name( part ) ? "'" : "";
	it = subs.find( part );
	if( it != subs.end() ) part = it->second;
	if( part.find( "std::" ) == 0 ) part.replace( 0, 5, "_std::" );
	if( part.find( "io::" ) == 0 ) part.replace( 0, 4, "_io::" );
	if( type_for_quoting.find( part ) != type_for_quoting.end() ) quote = "'";
	return result + quote + part + quote;
}
string cdao_make_dao_template_type_name( const string & name0 )
{
	map<string,string>::const_iterator it;
	string name = normalize_type_name( name0 );
	string result, part;
	int i, n, istemplate = name.find( '<' ) != string::npos;
	for(i=0, n = name.size(); i<n; i++){
		char ch = name[i];
		if( ch == '<' || ch == '>' || ch == ',' ){
			if( part != "" && part != " " ){
				string quote = is_invalid_dao_type_name( part ) ? "'" : "";
				it = type_substitutions.find( part );
				if( it != type_substitutions.end() ) part = it->second;
				if( part.find( "std::" ) == 0 ) part.replace( 0, 5, "_std::" );
				if( part.find( "io::" ) == 0 ) part.replace( 0, 4, "_io::" );
				if( type_for_quoting.find( part ) != type_for_quoting.end() ) quote = "'";
				if( istemplate == 0 ) quote = "";
				result += quote + part + quote;
			}
			//if( ch == '>' && result[result.size()-1] == '>' ) result += ' ';
			result += ch;
			part = "";
		}else{
			part += ch;
		}
	}
	if( part == "" ) return result;
	string quote = is_invalid_dao_type_name( part ) ? "'" : "";
	it = type_substitutions.find( part );
	if( it != type_substitutions.end() ) part = it->second;
	if( part.find( "std::" ) == 0 ) part.replace( 0, 5, "_std::" );
	if( part.find( "io::" ) == 0 ) part.replace( 0, 4, "_io::" );
	if( type_for_quoting.find( part ) != type_for_quoting.end() ) quote = "'";
	if( istemplate == 0 ) quote = "";
	return result + quote + part + quote;
}
string cdao_substitute_typenames( const string & name0 )
{
	map<string,string>::const_iterator it;
	string name = normalize_type_name( name0 );
	string result, part;
	int i, n;
	for(i=0, n = name.size(); i<n; i++){
		char ch = name[i];
		if( ch == '<' || ch == '>' || ch == ',' ){
			while( part.size() && isspace( part[0] ) ) part.erase( 0, 1 );
			while( part.size() && isspace( part[part.size()-1] ) ) part.erase( part.size()-1, 1 );
			if( part != "" && part != " " ){
				it = type_substitutions.find( part );
				if( it != type_substitutions.end() ) part = it->second;
				result += part;
			}
			if( ch == '>' && result[result.size()-1] == '>' ) result += ' ';
			result += ch;
			part = "";
		}else{
			part += ch;
		}
	}
	if( part == "" ) return result;
	it = type_substitutions.find( part );
	if( it != type_substitutions.end() ) part = it->second;
	return result + part;
}
string cdao_remove_type_scopes( const string & qname )
{
	size_t i, n, colon = string::npos;
	string name = qname;
	for(i=0,n=name.size(); i<n; i++){
		char ch = name[i];
		if( ch == ':' ) colon = i;
		if( isalnum( ch ) ==0 && ch != '_' && ch != ':' ) break;
	}
	if( colon != string::npos ) name.erase( 0, colon+1 );
	return name;
}
const char *const conversions[] =
{
	"::", 
	"<", ">", ",", " ", "[", "]", "(", ")", "*", ".", 
	"=", "+", "-", "*", "/", "%", "&", "|", "^", "!", "~",
	NULL
};
const char *const conversions2[] =
{
	"+=", "-=", "*=", "/=", "%=", "&=", "^=",
	"==", "!=", "<=", ">=", "<<", ">>", "[]",
	NULL
};
// qualified name to single identifier name:
string cdao_qname_to_idname( const string & qname )
{
	string idname = normalize_type_name( qname );
	int i;
	if( clangdao_colons_conversion_simple ){
		size_t p = 0;
		while( (p = idname.find( "::", p )) != string::npos ) idname.replace( p, 2, "_" );
	}
	for(i=0; conversions2[i]; i++){
		size_t p = 0;
		string s = "_" + utostr( 30+i ) + "_";
		while( (p = idname.find( conversions2[i], p )) != string::npos ) idname.replace( p, 2, s );
	}
	for(i=0; conversions[i]; i++){
		size_t p = 0;
		string s = "_" + utostr( i ) + "_";
		while( (p = idname.find( conversions[i], p )) != string::npos ) idname.replace( p, 1+(i==0), s );
	}
	return idname;
}



static cl::list<std::string> preprocessor_definitions
("D", cl::value_desc("definitions"), cl::Prefix,
 cl::desc("Preprocessor definitions from command line arguments"));

static cl::list<std::string> include_paths
("I", cl::value_desc("includes"), cl::Prefix,
 cl::desc("Include paths from command line arguments"));

static cl::opt<std::string> main_input_file
(cl::Positional, cl::desc("<input file>"), cl::Required);

static cl::list<std::string> ignored_arguments(cl::Sink);

static llvm::cl::opt<std::string> output_dir("o", llvm::cl::desc("output directory"));

// Note:
// The follow path is needed for Objective-C:
// /Developer/SDKs/MacOSX10.5.sdk/usr/lib/gcc/i686-apple-darwin9/4.2.1/include

const string predefines = "";

void ClangDao_RemoveDefine( string & defines, const string & define )
{
	size_t pos = defines.find( define );
	if( pos != string::npos ){
		while( pos < defines.size() && defines[pos] != '\n' ){
			defines[pos] = ' ';
			pos += 1;
		}
	}
}


int main(int argc, char *argv[] )
{
	type_for_quoting[ "void" ] = 1;
	type_for_quoting[ "bool" ] = 1;
	type_for_quoting[ "char" ] = 1;
	type_for_quoting[ "wchar_t" ] = 1;
	type_for_quoting[ "short" ] = 1;
	type_for_quoting[ "long" ] = 1;
	type_for_quoting[ "size_t" ] = 1;
	type_for_quoting[ "int8_t" ] = 1;
	type_for_quoting[ "int16_t" ] = 1;
	type_for_quoting[ "int32_t" ] = 1;
	type_for_quoting[ "int64_t" ] = 1;
	type_for_quoting[ "uint8_t" ] = 1;
	type_for_quoting[ "uint16_t" ] = 1;
	type_for_quoting[ "uint32_t" ] = 1;
	type_for_quoting[ "uint64_t" ] = 1;
	type_substitutions[ "_Bool" ] = "bool";

	size_t i;
	cl::ParseCommandLineOptions( argc, argv, 
			"ClangDao: Clang-based automatic binding tool for Dao." );

	if (!ignored_arguments.empty()) {
		errs() << "Ignoring the following arguments:";
		copy(ignored_arguments.begin(), ignored_arguments.end(),
				std::ostream_iterator<std::string>(std::cerr, " "));
	}

	CompilerInstance compiler;
	CDaoModule module( & compiler, main_input_file );

	compiler.createDiagnostics();
	//compiler.getInvocation().setLangDefaults(IK_CXX);
	//compiler.getInvocation().setLangDefaults(IK_ObjC);
	CompilerInvocation::CreateFromArgs( compiler.getInvocation(),
			argv + 1, argv + argc, compiler.getDiagnostics() );

	std::shared_ptr<TargetOptions> taropts( new TargetOptions( compiler.getTargetOpts() ) ); 
	compiler.setTarget( TargetInfo::CreateTargetInfo(
				compiler.getDiagnostics(), taropts ) );

	compiler.createFileManager();
	compiler.createSourceManager(compiler.getFileManager());
	compiler.createPreprocessor( TU_Complete );
	compiler.createASTContext();

	std::unique_ptr<ASTConsumer> astConsumer( new CDaoASTConsumer( & compiler, & module ) );
	compiler.setASTConsumer( std::move(astConsumer) );
	//XXX compiler.createSema(false, NULL);
	//compiler.createSema(TU_Module, NULL);
	compiler.createSema(TU_Prefix, NULL);

	Preprocessor & pp = compiler.getPreprocessor();

	//outs()<<pp.getPredefines()<<"\n";
	string builtinDefines = pp.getPredefines();

#if 1
	ClangDao_RemoveDefine( builtinDefines, "#define __APPLE_CC__" );
	ClangDao_RemoveDefine( builtinDefines, "#define __APPLE__" );
	ClangDao_RemoveDefine( builtinDefines, "#define __MACH__" );
	ClangDao_RemoveDefine( builtinDefines, "#define __ENVIRONMENT_MAC_OS_X" );
	ClangDao_RemoveDefine( builtinDefines, "#define OBJC_" );
#endif

	//outs()<<builtinDefines<<"\n";

	std::unique_ptr<PPCallbacks> ppCallbacks( new CDaoPPCallbacks( & compiler, & module ) );
	pp.setPredefines( builtinDefines + "\n" + predefines + "\n#define __CLANGDAO__\n" );
	pp.addPPCallbacks( std::move( ppCallbacks ) );

	InputKind ik = FrontendOptions::getInputKindForExtension( main_input_file );
	compiler.InitializeSourceManager( FrontendInputFile( main_input_file, ik ) );
	compiler.getDiagnosticClient().BeginSourceFile( compiler.getLangOpts(), & pp );
	ParseAST( pp, &compiler.getASTConsumer(), compiler.getASTContext() );
	compiler.getDiagnosticClient().EndSourceFile();

	return module.Generate( output_dir );
}
