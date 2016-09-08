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

#include <clang/Lex/Preprocessor.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/Sema/DeclSpec.h>
#include <clang/Sema/Sema.h>
#include <clang/Sema/Template.h>
#include <map>
#include <algorithm>

#include "cdaoFunction.hpp"
#include "cdaoUserType.hpp"
#include "cdaoNamespace.hpp"
#include "cdaoModule.hpp"

const string daoqt_object_class =
"#include<QHash>\n\
\n\
#ifndef DAOQT_MAX_VALUE\n\
#define DAOQT_MAX_VALUE 16\n\
#endif\n\
class DaoQtMessage\n\
{\n\
  void Init( int n ){\n\
    count = n;\n\
    memset( values, 0, DAOQT_MAX_VALUE * sizeof(DaoValue*) );\n\
  }\n\
  \n\
  public:\n\
  int        count;\n\
  DaoValue  *values[DAOQT_MAX_VALUE];\n\
  \n\
  DaoQtMessage( int n=0 ){ Init( n ); }\n\
  DaoQtMessage( const DaoQtMessage & other ){\n\
    Init( other.count );\n\
    for(int i=0; i<count; i++) DaoValue_Copy( other.values[i], & values[i] );\n\
  }\n\
  ~DaoQtMessage(){ DaoValue_ClearAll( values, count ); }\n\
};\n\
Q_DECLARE_METATYPE( DaoQtMessage );\n\
\n\
class DaoQtSlot : public QObject\n\
{ Q_OBJECT\n\
\n\
public:\n\
	DaoQtSlot(){\n\
		anySender = NULL;\n\
		daoReceiver = NULL;\n\
		daoSignal = NULL;\n\
		daoSlot = NULL;\n\
	}\n\
	DaoQtSlot( void *p, DaoCdata *rev, const QString & sig, DaoRoutine *slot ){\n\
		anySender = p;\n\
		daoReceiver = rev;\n\
		daoSignal = NULL;\n\
		qtSignal = sig;\n\
		daoSlot = slot;\n\
	}\n\
	DaoQtSlot( void *p, DaoCdata *rev, void *sig, DaoRoutine *slot ){\n\
		anySender = p;\n\
		daoReceiver = rev;\n\
		daoSignal = sig;\n\
		daoSlot = slot;\n\
	}\n\
	bool Match( void *sender, const QString & signal, DaoRoutine *slot ){\n\
		return anySender == sender && qtSignal == signal && daoSlot == slot;\n\
	}\n\
	bool Match( void *sender, void *signal, DaoRoutine *slot ){\n\
		return anySender == sender && daoSignal == signal && daoSlot == slot;\n\
	}\n\
	\n\
	void      *anySender;\n\
	void      *daoSignal;\n\
	QString    qtSignal;\n\
	\n\
	DaoCdata    *daoReceiver;\n\
	DaoRoutine  *daoSlot;\n\
	\n\
public slots:\n\
	void slotDaoQt( void*, const QString&, const DaoQtMessage &m ){ slotDao( m ); }\n\
	void slotDaoDao( void *o, void *s, const DaoQtMessage &m ){\n\
		if( o == anySender && s == daoSignal ) slotDao( m );\n\
	}\n\
	void slotDao( const DaoQtMessage &m ){\n\
		DaoProcess *vmp = DaoVmSpace_AcquireProcess( __daoVmSpace );\n\
		DaoValue *obj = NULL;\n\
		if( DaoRoutine_CastRoutine( daoSlot ) == NULL ) return;\n\
		obj = DaoCdata_GetObject( daoReceiver );\n\
		DaoProcess_Call( vmp, daoSlot, obj, m.values, m.count );\n\
	}\n\
};\n\
\n\
class DaoQtObject : public QObjectUserData\n\
{\n\
	public:\n\
	~DaoQtObject(){ for(int i=0,n=daoSlots.size(); i<n; i++) delete daoSlots[i]; }\n\
	\n\
	QList<DaoQtSlot*>  daoSlots;\n\
	\n\
	DaoCdata  *_cdata;\n\
	\n\
	void Init( DaoCdata *d ){ _cdata = d; }\n\
	static unsigned int RotatingHash( const QByteArray & key ){\n\
		int i, n = key.size();\n\
		unsigned long long hash = n;\n\
		for(i=0; i<n; i++) hash = ((hash<<4)^(hash>>28)^key[i])&0x7fffffff;\n\
		return hash % 997;\n\
	}\n\
	DaoQtSlot* Find( void *sender, const QString & signal, DaoRoutine *slot ){\n\
		int i, n = daoSlots.size();\n\
		for(i=0; i<n; i++){\n\
			DaoQtSlot *daoslot = daoSlots[i];\n\
			if( daoslot->Match( sender, signal, slot ) ) return daoslot;\n\
		}\n\
		return NULL;\n\
	}\n\
	DaoQtSlot* Find( void *sender, void *signal, DaoRoutine *slot ){\n\
		int i, n = daoSlots.size();\n\
		for(i=0; i<n; i++){\n\
			DaoQtSlot *daoslot = daoSlots[i];\n\
			if( daoslot->Match( sender, signal, slot ) ) return daoslot;\n\
		}\n\
		return NULL;\n\
	}\n\
	DaoQtSlot* Add( void *sender, const QString & signal, DaoRoutine *slot ){\n\
		DaoQtSlot *daoslot = new DaoQtSlot( sender, _cdata, signal, slot );\n\
		daoSlots.append( daoslot );\n\
		return daoslot;\n\
	}\n\
	DaoQtSlot* Add( void *sender, void *signal, DaoRoutine *slot ){\n\
		DaoQtSlot *daoslot = new DaoQtSlot( sender, _cdata, signal, slot );\n\
		daoSlots.append( daoslot );\n\
		return daoslot;\n\
	}\n\
};\n";


const string daoss_class =
"\n\
class DAO_DLL2_$(module) DaoSS_$(idname) : $(ss_parents)\n\
{ Q_OBJECT\n\
public:\n\
	DaoSS_$(idname)() $(init_parents) {}\n\
\n\
$(Emit)\n\
\n\
public slots:\n\
$(slots)\n\
\n\
signals:\n\
$(signalDao)\n\
$(signals)\n\
};\n";

const string qt_Emit =
"	void Emit( void *o, void *s, const DaoQtMessage &m ){ emit signalDao( o, s, m ); }\n";

const string qt_signalDao =
"	void signalDao( void *o, void *s, const DaoQtMessage &m );\n\
	void signalDaoQt( void*, const QString&, const DaoQtMessage &m );\n";

const string qt_init = "";

const string qt_make_linker =
"   DaoSS_$(idname) *linker = new DaoSS_$(idname)();\n\
   setUserData( 0, linker );\n\
   linker->Init( _cdata );\n";

const string qt_make_linker3 =
"  DaoSS_$(idname) *linker = new DaoSS_$(idname)();\n\
  object->setUserData( 0, linker );\n\
  linker->Init( NULL );\n";

const string qt_virt_emit =
"	virtual void Emit( void *o, void *s, const DaoQtMessage &m ){}\n";

const string qt_connect_decl =
"static void dao_QObject_emit( DaoProcess *_proc, DaoValue *_p[], int _n );\n\
static void dao_QObject_connect_dao( DaoProcess *_proc, DaoValue *_p[], int _n );\n";

const string qt_connect_dao =
"  { dao_QObject_emit, \"emit( self : QObject, signal : any, ... )\" },\n\
  { dao_QObject_connect_dao, \"connect( sender : QObject, signal : any, receiver : QObject, slot : any )\" },\n";

const string qt_connect_func =
"static void dao_QObject_emit( DaoProcess *_proc, DaoValue *_p[], int _n )\n\
{\n\
	QObject* self = (QObject*) DaoValue_TryCastCdata(_p[0], dao_type_QObject );\n\
	DaoSS_QObject *linker = (DaoSS_QObject*) self->userData(0);\n\
	DaoValue *signal = _p[1];\n\
	if( self == NULL || linker == NULL ) return;\n\
  DaoQtMessage msg( _n-2 );\n\
  for(int i=0; i<_n-2; i++) DaoValue_Copy( _p[i+2], & msg.values[i] );\n\
	linker->Emit( linker, signal->v.p, msg );\n\
}\n\
static void dao_QObject_connect_dao( DaoProcess *_proc, DaoValue *_p[], int _n )\n\
{\n\
	QObject *sender = (QObject*) DaoValue_TryCastCdata(_p[0], dao_type_QObject );\n\
	QObject *receiver = (QObject*) DaoValue_TryCastCdata(_p[2], dao_type_QObject);\n\
	DaoSS_QObject *senderSS = (DaoSS_QObject*) sender->userData(0);\n\
	DaoSS_QObject *receiverSS = (DaoSS_QObject*) receiver->userData(0);\n\
	DaoValue *signal = _p[1];\n\
	DaoValue *slot = _p[3];\n\
	QByteArray s1( \"1\" );\n\
	QByteArray s2( \"2\" );\n\
	QByteArray s3;\n\
	if( sender == NULL || receiver == NULL ) return;\n\
	if( signal.t != DAO_STRING || slot.t != DAO_STRING ){\n\
		if( senderSS == NULL || receiverSS == NULL ) return;\n\
	}\n\
	if( signal.t == DAO_STRING && slot.t == DAO_STRING ){ /* Qt -> Qt */\n\
		s1 += DaoValue_TryGetMBString(slot);\n\
		s2 += DaoValue_TryGetMBString(signal);\n\
		QObject::connect( sender, s2.data(), receiver, s1.data() );\n\
	}else if( signal.t == DAO_STRING ){ /* Qt -> Dao */\n\
		QByteArray name( DString_GetData(signal.v.s) );\n\
		QByteArray size = QString::number( DaoQtObject::RotatingHash( name ) ).toLocal8Bit();\n\
		QByteArray ssname( name ); \n\
		int i = name.indexOf( \'(\' );\n\
		if( i>=0 ) ssname = name.mid( 0, i ) + size;\n\
		s2 += name;\n\
		s1 += \"slot_\" + ssname + name.mid(i);\n\
		s3 += \"2signal_\" + ssname + \"(void*,const QString&,const DaoQtMessage&)\";\n\
		/* signal -> daoqt_signal -> slotDaoQt -> dao */\n\
		QObject::connect( sender, s2.data(), senderSS, s1.data() );\n\
		DaoQtSlot *daoSlot = receiverSS->Find( senderSS, name, slot );\n\
		if( daoSlot == NULL ){\n\
			daoSlot = receiverSS->Add( senderSS, name, slot );\n\
			s3 = \"2signal_\" + ssname + \"(const DaoQtMessage&)\";\n\
			QObject::connect( senderSS, s3.data(), daoSlot, SLOT( slotDao(const DaoQtMessage&) ) );\n\
		}\n\
	}else if( slot.t == DAO_STRING ){ /* Dao -> Qt */\n\
		QByteArray name( DString_GetData(slot.v.s) );\n\
		QByteArray size = QString::number( DaoQtObject::RotatingHash( name ) ).toLocal8Bit();\n\
		QByteArray ssname( name ); \n\
		int i = name.indexOf( \'(\' );\n\
		if( i>=0 ) ssname = name.mid( 0, i ) + size;\n\
		s1 += name;\n\
		s2 += \"signal_\" + ssname + name.mid(i);\n\
		s3 += \"1slot_\" + ssname + \"(void*,void*,const DaoQtMessage&)\";\n\
		/* signalDaoQt -> daoqt_slot -> slot */\n\
		QObject::connect( senderSS, SIGNAL( signalDao(void*,void*,const DaoQtMessage&) ),\n\
				receiverSS, s3.data() );\n\
		QObject::connect( receiverSS, s2.data(), receiver, s1.data() );\n\
	}else{ /* Dao -> Dao */\n\
		DaoQtSlot *daoSlot = receiverSS->Find( senderSS, signal.v.p, slot );\n\
		if( daoSlot == NULL ){\n\
			daoSlot = receiverSS->Add( senderSS, signal.v.p, slot );\n\
			QObject::connect( senderSS, SIGNAL( signalDao(void*,void*,const DaoQtMessage&) ),\n\
					daoSlot, SLOT( slotDaoDao(void*,void*,const DaoQtMessage&) ) );\n\
		}\n\
	}\n\
}\n";

const string qt_qstringlist_decl =
"static void dao_QStringList_fromDaoList( DaoProcess *_proc, DaoValue *_p[], int _n );\n\
static void dao_QStringList_toDaoList( DaoProcess *_proc, DaoValue *_p[], int _n );\n";

const string qt_qstringlist_dao =
"  { dao_QStringList_fromDaoList, \"QStringList( dslist : list<string> )=>QStringList\" },\n\
  { dao_QStringList_toDaoList, \"toDaoList( self : QStringList )=>list<string>\" },\n";

const string qt_qstringlist_func =
"static void dao_QStringList_fromDaoList( DaoProcess *_proc, DaoValue *_p[], int _n )\n\
{\n\
	QStringList *_self = new QStringList();\n\
	DaoList *dlist = _p[0]->v.list;\n\
	int i, m = DaoList_Size( dlist );\n\
	DaoProcess_PutCdata( _proc, _self, dao_type_QStringList );\n\
	for(i=0; i<m; i++){\n\
		DaoValue *it = DaoList_GetItem( dlist, i );\n\
		if( it.t != DAO_STRING ) continue;\n\
		_self->append( QString( DString_GetData( it.v.s ) ) );\n\
	}\n\
}\n\
static void dao_QStringList_toDaoList( DaoProcess *_proc, DValue *_p[], int _n )\n\
{\n\
	QStringList* self= (QStringList*) DaoValue_TryCastCdata( _p[0], dao_type_QStringList );\n\
	DaoList *dlist = DaoProcess_PutList( _proc );\n\
	DValue it = DValue_NewMBString( \"\", 0 );\n\
	int i, m = self->size();\n\
	for(i=0; i<m; i++){\n\
		DString_SetMBS( it.v.s, (*self)[i].toLocal8Bit().data() );\n\
		DaoList_PushBack( dlist, it );\n\
	}\n\
	DValue_Clear( & it );\n\
}\n";

const string qt_qobject_cast_decl =
"static void dao_$(host)_qobject_cast( DaoProcess *_proc, DValue *_p[], int _n );\n";

const string qt_qobject_cast_dao =
"  { dao_$(host)_qobject_cast, \"qobject_cast( object : QObject )=>$(host)\" },\n";

const string qt_qobject_cast_func =
"static void dao_$(host)_qobject_cast( DaoProcess *_proc, DValue *_p[], int _n )\n\
{\n\
  QObject *from = (QObject*)DaoValue_TryCastCdata( _p[0], dao_type_QObject );\n\
  $(host) *to = qobject_cast<$(host)*>( from );\n\
  DaoProcess_WrapCdata( _proc, to, dao_type_$(host) );\n\
}\n";

const string qt_qobject_cast_func2 =
"static void dao_$(host)_qobject_cast( DaoProcess *_proc, DValue *_p[], int _n )\n\
{\n\
  QObject *from = (QObject*)DaoValue_TryCastCdata(_p[0], dao_type_QObject);\n\
  DaoValue *to = DaoQt_Get_Wrapper( from );\n\
  if( to ){\n\
    DaoProcess_PutValue( _proc, to );\n\
    return;\n\
  }\n\
  $(host) *to2 = qobject_cast<$(host)*>( from );\n\
  DaoProcess_WrapCdata( _proc, to2, dao_type_$(host) );\n\
}\n";

const string qt_application_decl =
"static void dao_QApplication_DaoApp( DaoProcess *_proc, DValue *_p[], int _n );\n";

const string qt_application_dao =
"  { dao_QApplication_DaoApp, \"QApplication( name : string )=>QApplication\" },\n";

const string qt_application_func =
"static void dao_QApplication_DaoApp( DaoProcess *_proc, DValue *_p[], int _n )\n\
{\n\
  QApplication *app = (QApplication*) QApplication::instance();\n\
  if( app ){\n\
    DaoProcess_WrapCdata( _proc, app, dao_type_QApplication );\n\
    return;\n\
  }\n\
  static int argc = 1;\n\
  static DString *str = DString_New(1);\n\
  static char* argv = (char*)DValue_GetMBString( _p[0] );\n\
  DString_SetMBS( str, argv );\n\
  argv = DString_GetData( str );\n\
  DaoCxx_QApplication *_self = DaoCxx_QApplication_New( argc, & argv, QT_VERSION );\n\
  DaoProcess_PutValue( _proc, (DaoValue*) _self->_cdata );\n\
}\n";

const string qt_qstream_decl =
"static void dao_QTextStream_Write1( DaoProcess *_proc, DValue *_p[], int _n );\n\
static void dao_QTextStream_Write2( DaoProcess *_proc, DValue *_p[], int _n );\n\
static void dao_QTextStream_Write3( DaoProcess *_proc, DValue *_p[], int _n );\n\
static void dao_QTextStream_Write4( DaoProcess *_proc, DValue *_p[], int _n );\n\
static void dao_QTextStream_Write5( DaoProcess *_proc, DValue *_p[], int _n );\n";

const string qt_qstream_dao =
"  { dao_QTextStream_Write1, \"write( self : QTextStream, data : int )=>QTextStream\" },\n\
  { dao_QTextStream_Write2, \"write( self : QTextStream, data : float )=>QTextStream\" },\n\
  { dao_QTextStream_Write3, \"write( self : QTextStream, data : float )=>QTextStream\" },\n\
  { dao_QTextStream_Write4, \"write( self : QTextStream, data : string )=>QTextStream\" },\n\
  { dao_QTextStream_Write5, \"write( self : QTextStream, data : any )=>QTextStream\" },\n";

const string qt_qstream_func =
"static void dao_QTextStream_Write1( DaoProcess *_proc, DValue *_p[], int _n )\n\
{\n\
  QTextStream *self = (QTextStream*) DaoValue_TryCastCdata( _p[0], dao_type_QTextStream );\n\
  *self << _p[1]->v.i;\n\
  DaoProcess_PutValue( _proc, (DaoValue*) _p[0]->v.cdata );\n\
}\n\
static void dao_QTextStream_Write2( DaoProcess *_proc, DValue *_p[], int _n )\n\
{\n\
  QTextStream *self = (QTextStream*) DaoValue_TryCastCdata( _p[0], dao_type_QTextStream );\n\
  *self << _p[1]->v.f;\n\
  DaoProcess_PutValue( _proc, (DaoValue*) _p[0]->v.cdata );\n\
}\n\
static void dao_QTextStream_Write3( DaoProcess *_proc, DValue *_p[], int _n )\n\
{\n\
  QTextStream *self = (QTextStream*) DaoValue_TryCastCdata( _p[0], dao_type_QTextStream );\n\
  *self << _p[1]->v.d;\n\
  DaoProcess_PutValue( _proc, (DaoValue*) _p[0]->v.cdata );\n\
}\n\
static void dao_QTextStream_Write4( DaoProcess *_proc, DValue *_p[], int _n )\n\
{\n\
  QTextStream *self = (QTextStream*) DaoValue_TryCastCdata( _p[0], dao_type_QTextStream );\n\
  *self << DValue_GetMBString( _p[1] );\n\
  DaoProcess_PutValue( _proc, (DaoValue*) _p[0]->v.cdata );\n\
}\n\
static void dao_QTextStream_Write5( DaoProcess *_proc, DValue *_p[], int _n )\n\
{\n\
  QTextStream *self = (QTextStream*) DaoValue_TryCastCdata( _p[0], dao_type_QTextStream );\n\
  *self << _p[1]->v.p;\n\
  DaoProcess_PutValue( _proc, (DaoValue*) _p[0]->v.cdata );\n\
}\n";

const string dao_add_extrc_decl =
"static void dao_$(host)_add_extrc( DaoProcess *_proc, DaoValue *_p[], int _n );\n";

const string dao_add_extrc_dao =
"  { dao_$(host)_add_extrc, \"dao_add_external_reference( self : $(host) )\" },\n";

const string dao_add_extrc_func =
"static void dao_$(host)_add_extrc( DaoProcess *_proc, DaoValue *_p[], int _n )\n\
{\n\
	DaoCxx_$(host) *self = (DaoCxx_$(host)*)DaoValue_TryGetCdata(_p[0]);\n\
	self->DaoAddExternalReference();\n\
}\n";

const string dao_proto =
"  { dao_$(host_idname)_$(cxxname)$(overload), \"$(daoname)( $(parlist) )$(retype)\" },\n";

const string cxx_wrap_proto 
= "static void dao_$(host_idname)_$(cxxname)$(overload)( DaoProcess *_proc, DaoValue *_p[], int _n )";

const string tpl_struct = "DAO_DLL_$(module) $(qname)* Dao_$(idname)_New();\n";

const string tpl_struct_alloc =
"$(qname)* Dao_$(idname)_New()\n{\n\
	$(qname) *self = ($(qname)*) calloc( 1, sizeof($(qname)) );\n\
	return self;\n\
}\n";
const string tpl_struct_alloc2 =
"$(host_qname)* Dao_$(host_idname)_New()\n{\n\
	$(host_qname) *self = new $(host_qname)();\n\
	return self;\n\
}\n";

const string tpl_struct_daoc = 
"typedef struct Dao_$(idname) Dao_$(idname);\n\
struct DAO_DLL2_$(module) Dao_$(idname)\n\
{\n\
	$(qname)  nested;\n\
	$(qname) *object;\n\
	DaoCdata *_cdata;\n\
};\n\
DAO_DLL_$(module) Dao_$(idname)* Dao_$(idname)_New();\n";

const string tpl_struct_daoc_alloc =
"Dao_$(idname)* Dao_$(idname)_New()\n\
{\n\
	Dao_$(idname) *wrap = calloc( 1, sizeof(Dao_$(idname)) );\n\
	$(qname) *self = ($(qname)*) wrap;\n\
	wrap->_cdata = DaoCdata_New( dao_type_$(idname), wrap );\n\
	wrap->object = self;\n\
$(callbacks)$(selfields)\treturn wrap;\n\
}\n";

const string tpl_set_callback = "\tself->$(callback) = Dao_$(name)_$(callback);\n";
const string tpl_set_selfield = "\tself->$(field) = wrap;\n";

const string c_nowrap_new = 
"static void dao_$(host_idname)_$(cxxname)( DaoProcess *_proc, DaoValue *_p[], int _n )\n\
{\n\
	$(qname) *self = Dao_$(idname)_New();\n\
	DaoProcess_WrapCdata( _proc, self, dao_type_$(host_idname) );\n\
}\n";
const string c_wrap_new = 
"static void dao_$(host_idname)_$(cxxname)( DaoProcess *_proc, DaoValue *_p[], int _n )\n\
{\n\
	Dao_$(host_idname) *self = Dao_$(host_idname)_New();\n\
	DaoProcess_PutValue( _proc, (DaoValue*) self->_cdata );\n\
}\n";
const string cxx_wrap_new = 
"static void dao_$(host_idname)_$(cxxname)( DaoProcess *_proc, DaoValue *_p[], int _n )\n\
{\n\
	DaoCxx_$(host_idname) *self = DaoCxx_$(host_idname)_New();\n\
	DaoProcess_PutValue( _proc, (DaoValue*) self->_cdata );\n\
}\n";
const string cxx_wrap_alloc2 = 
"static void dao_$(host_idname)_$(cxxname)( DaoProcess *_proc, DaoValue *_p[], int _n )\n\
{\n\
	$(host_qname) *self = Dao_$(host_idname)_New();\n\
	DaoProcess_PutCdata( _proc, self, dao_type_$(host_idname) );\n\
}\n";

const string tpl_class_def = 
"class DAO_DLL2_$(module) DaoCxxVirt_$(idname) $(virt_supers)\n{\n\
	public:\n\
	DaoCxxVirt_$(idname)(){ _cdata = 0; }\n\
	void DaoInitWrapper( DaoCdata *d );\n\n\
	DaoCdata *_cdata;\n\
\n$(virtuals)\n\
$(qt_virt_emit)\n\
};\n\
class DAO_DLL2_$(module) DaoCxx_$(idname) : public $(qname), public DaoCxxVirt_$(idname)\n\
{ $(Q_OBJECT)\n\n\
\tpublic:\n\
$(constructors)\n\
	~DaoCxx_$(idname)();\n\
	void DaoInitWrapper();\n\n\
$(methods)\n\
};\n";

const string tpl_class2 = 
tpl_class_def + "$(qname)* Dao_$(idname)_Copy( const $(qname) &p );\n";

const string tpl_class_init =
"void DaoCxxVirt_$(idname)::DaoInitWrapper( DaoCdata *d )\n\
{\n\
	_cdata = d;\n\
$(init_supers)\
$(qt_init)\
}\n\
DaoCxx_$(idname)::~DaoCxx_$(idname)()\n\
{\n\
	if( _cdata ){\n\
		DaoCdata_SetData( _cdata, NULL );\n\
		DaoGC_DecRC( (DaoValue*) _cdata );\n\
	} \n\
}\n\
void DaoCxx_$(idname)::DaoInitWrapper()\n\
{\n\
	_cdata = DaoCdata_New( dao_type_$(idname), this );\n\
	DaoGC_IncRC( (DaoValue*)_cdata );\n\
	DaoCxxVirt_$(idname)::DaoInitWrapper( _cdata );\n\
$(qt_make_linker)\
}\n";
const string tpl_class_init_qtss = 
"DAO_DLL_$(module) void Dao_$(idname)_InitSS( $(qname) *p )\n\
{\n\
   if( p->userData(0) == NULL ){\n\
		DaoSS_$(idname) *linker = new DaoSS_$(idname)();\n\
		p->setUserData( 0, linker );\n\
		linker->Init( NULL );\n\
	}\n\
}\n";
const string tpl_class_copy = tpl_class_init +
"DAO_DLL_$(module) $(qname)* Dao_$(idname)_Copy( const $(qname) &p )\n\
{\n\
	$(qname) *object = new $(qname)( p );\n\
$(qt_make_linker3)\n\
	return object;\n\
}\n";
const string tpl_class_decl_constru = 
"	DaoCxx_$(idname)( $(parlist) ) : $(qname)( $(parcall) ){}\n";

const string tpl_class_new =
"\nDAO_DLL_$(module) DaoCxx_$(idname)* DaoCxx_$(idname)_New( $(parlist) );\n";
const string tpl_class_new_novirt =
"\nDAO_DLL_$(module) $(qname)* Dao_$(idname)_New( $(parlist) );\n";
const string tpl_class_init_qtss_decl =
"\nDAO_DLL_$(module) void Dao_$(idname)_InitSS( $(qname) *p );\n";

const string tpl_class_noderive =
"\nDAO_DLL_$(module) $(qname)* Dao_$(idname)_New( $(parlist) )\n\
{\n\
	$(qname) *__object = new $(qname)( $(parcall) );\n\
$(qt_make_linker3)\n\
	return __object;\n\
}\n";
const string tpl_class_init2 =
"\nDAO_DLL_$(module) DaoCxx_$(idname)* DaoCxx_$(idname)_New( $(parlist) )\n\
{\n\
	DaoCxx_$(idname) *self = new DaoCxx_$(idname)( $(parcall) );\n\
	self->DaoInitWrapper();\n\
	return self;\n\
}\n";

const string tpl_init_super = "\tDaoCxxVirt_$(super)::DaoInitWrapper( d );\n";

const string tpl_meth_decl =
"\t$(retype) $(name)( int &_cs$(comma) $(parlist) )$(extra);\n";

const string tpl_meth_decl2 =
"\t$(retype) $(name)( $(parlist) )$(extra);\n";

const string tpl_meth_decl3 =
"$(retype) DaoWrap_$(name)( $(parlist) )$(extra){ $(return)$(qname)::$(cxxname)( $(parcall) ); }\n";

const string tpl_raise_call_protected =
"  if( DaoValue_CastCdata(_p[0],NULL) && DaoCdata_GetObject((DaoCdata*)_p[0]) == NULL ){\n\
    DaoProcess_RaiseError( _proc, NULL, \"call to protected method\" );\n\
    return;\n\
  }\n";

const string cxx_getter_proto = 
"static void dao_$(host_idname)_GETF_$(name)( DaoProcess *_proc, DaoValue *_p[], int _n )";

const string cxx_setter_proto = 
"static void dao_$(host_idname)_SETF_$(name)( DaoProcess *_proc, DaoValue *_p[], int _n )";

const string cxx_get_item_proto = 
"static void dao_$(host_idname)_GETI_$(name)( DaoProcess *_proc, DaoValue *_p[], int _n )";

const string cxx_set_item_proto = 
"static void dao_$(host_idname)_SETI_$(name)( DaoProcess *_proc, DaoValue *_p[], int _n )";

const string cxx_get_pixel_proto = 
"static void dao_$(host_idname)_GETI_Pixel( DaoProcess *_proc, DaoValue *_p[], int _n )";

const string cxx_set_pixel_proto = 
"static void dao_$(host_idname)_SETI_Pixel( DaoProcess *_proc, DaoValue *_p[], int _n )";

const string cxx_gs_user =
"  $(host_qname) *self = ($(host_qname)*)DaoValue_TryCastCdata(_p[0],dao_type_$(typer));\n";

const string dao_getter_proto = 
"  { dao_$(host_idname)_GETF_$(name), \".$(name)( self :$(daoname) )=>$(ftype)\" },\n";

const string dao_setter_proto = 
"  { dao_$(host_idname)_SETF_$(name), \".$(name)=( self :$(daoname), $(name) :$(ftype) )\" },\n";

const string dao_get_item_proto = 
"  { dao_$(host_idname)_GETI_$(name), \"[]( self :$(daoname), i :int )=>$(itype)\" },\n";

const string dao_set_item_proto = 
"  { dao_$(host_idname)_SETI_$(name), \"[]=( self :$(daoname), i :int, value :$(itype) )\" },\n";

const string dao_get_pixel_proto = 
"  { dao_$(host_idname)_GETI_Pixel, \"[]( self :$(daoname), i :int, j :int, pixel :enum<uint8,uint16,uint32>=$uint8 )=>int\" },\n";

const string dao_set_pixel_proto = 
"  { dao_$(host_idname)_SETI_Pixel, \"[]=( self :$(daoname), value :int, i :int, j :int, pixel :enum<uint8,uint16,uint32>=$uint8 )\" },\n";

const string numlist_code = 
"\n\nstatic DaoNumberEntry dao_$(typer)_Nums[] = \n\
{\n$(nums)  { NULL, 0, 0 }\n};\n";

const string methlist_code = numlist_code +
"\n\n$(decls)\n\
static DaoFunctionEntry dao_$(typer)_Meths[] = \n\
{\n$(meths)  { NULL, NULL }\n};\n";

const string methlist_code2 = 
"\n\n$(decls)\n";

const string delete_struct = 
"static void Dao_$(typer)_Delete( DaoValue *self )\n\
{\n\
	if( self->xCdata.data ) free( DaoValue_TryGetCdata( self ) );\n\
	DaoCstruct_Delete( (DaoCstruct*) self );\n\
}\n";

const string delete_class = 
"static void Dao_$(typer)_Delete( DaoValue *self )\n\
{\n\
	$(comment)delete ($(qname)*) DaoValue_TryGetCdata( self );\n\
	DaoCstruct_Delete( (DaoCstruct*) self );\n\
}\n";

const string get_gcfields =
"static void Dao_$(typer)_HandleGC( DaoValue *P, DList *VS, DList *AS, DList *MS, int RM )\n\
{\n\
	DaoCxx_$(typer) *self = (DaoCxx_$(typer)*) DaoValue_TryGetCdata( P );\n\
	if( self->_cdata ) DList_Append( VS, (void*) self->_cdata );\n\
	if( RM ){\n\
$(breakref)\
		self->_cdata = NULL;\n\
	}\n\
}\n";

const string cast_to_parent = 
"void* dao_cast_$(typer)_$(parent2)( void *data, int down )\n\
{\n\
	if( down ) return static_cast<$(child)*>(($(parent)*)data);\n\
	return dynamic_cast<$(parent)*>(($(child)*)data);\n\
}\n";
const string cast_to_parent_virtual_base = 
"void* dao_cast_$(typer)_$(parent2)( void *data, int down )\n\
{\n\
	if( down ) return dynamic_cast<$(child)*>(($(parent)*)data);\n\
	return dynamic_cast<$(parent)*>(($(child)*)data);\n\
}\n";


const string usertype_code =
"$(cast_funcs)\n\
static DaoTypeCore $(typer)_Core = \n\
{\n\
  \"$(daotypename)\",\n\
  sizeof($(qname)),\n\
  { $(parents)NULL },\n\
  dao_$(typer)_Nums,\n\
  dao_$(typer)_Meths,\n\
  DaoCstruct_CheckGetField,    DaoCstruct_DoGetField,\n\
  DaoCstruct_CheckSetField,    DaoCstruct_DoSetField,\n\
  DaoCstruct_CheckGetItem,     DaoCstruct_DoGetItem,\n\
  DaoCstruct_CheckSetItem,     DaoCstruct_DoSetItem,\n\
  DaoCstruct_CheckUnary,       DaoCstruct_DoUnary,\n\
  DaoCstruct_CheckBinary,      DaoCstruct_DoBinary,\n\
  DaoCstruct_CheckConversion,  DaoCstruct_DoConversion,\n\
  DaoCstruct_CheckForEach,     DaoCstruct_DoForEach,\n\
  DaoCstruct_Print,\n\
  NULL,\n\
  NULL,\n\
  NULL,\n\
  NULL,\n\
  NULL,\n\
  $(delete),\n\
  $(gcfields)\n\
};\n\
DaoTypeCore *dao_$(typer)_Core = & $(typer)_Core;\n\
DaoType *dao_type_$(typer) = NULL;\n";

const string usertype_code_none =
"static DaoTypeCore $(typer)_Core = \n\
{\n\
  \"$(daotypename)\",\n\
  0,\n\
  { NULL },\n\
  NULL,\n\
  NULL,\n\
  NULL,  NULL,\n\
  NULL,  NULL,\n\
  NULL,  NULL,\n\
  NULL,  NULL,\n\
  NULL,  NULL,\n\
  NULL,  NULL,\n\
  NULL,  NULL,\n\
  NULL,  NULL,\n\
  NULL,\n\
  NULL,\n\
  NULL,\n\
  NULL,\n\
  NULL,\n\
  NULL,\n\
  $(delete),\n\
  NULL\n\
};\n\
\n\
DaoTypeCore *dao_$(typer)_Core = & $(typer)_Core;\n\
DaoType *dao_type_$(typer) = NULL;\n";

const string cxx_get_pixel_codes =
"  daoint i = DaoValue_TryGetInteger( _p[1] );\n\
  daoint j = DaoValue_TryGetInteger( _p[2] );\n\
  daoint pixel = 0;\n\
  switch( DaoValue_TryGetEnum( _p[3] ) ){\n\
  case 0:\n\
  {\n\
    unsigned char *pixels = (unsigned char*) self->$(name);\n\
    pixel = pixels[i*self->$(pitch) + j];\n\
    break;\n\
  }\n\
  case 1:\n\
  {\n\
    unsigned short *pixels = (unsigned short*) self->$(name);\n\
    pixel = pixels[i*self->$(pitch) + j];\n\
    break;\n\
  }\n\
  case 2:\n\
  {\n\
    unsigned int *pixels = (unsigned int*) self->$(name);\n\
    pixel = pixels[i*self->$(pitch) + j];\n\
    break;\n\
  }\n\
  default: break;\n\
  }\n\
  DaoProcess_PutInteger( _proc, pixel );\n\
";

const string cxx_set_pixel_codes =
"  daoint i = DaoValue_TryGetInteger( _p[1] );\n\
  daoint j = DaoValue_TryGetInteger( _p[2] );\n\
  daoint pixel = DaoValue_TryGetInteger( _p[0] );\n\
  switch( DaoValue_TryGetEnum( _p[3] ) ){\n\
  case 0:\n\
  {\n\
    unsigned char *pixels = (unsigned char*) self->$(name);\n\
    pixels[i*self->$(pitch) + j] = pixel;\n\
    break;\n\
  }\n\
  case 1:\n\
  {\n\
    unsigned short *pixels = (unsigned short*) self->$(name);\n\
    pixels[i*self->$(pitch) + j] = pixel;\n\
    break;\n\
  }\n\
  case 2:\n\
  {\n\
    unsigned int *pixels = (unsigned int*) self->$(name);\n\
    pixels[i*self->$(pitch) + j] = pixel;\n\
    break;\n\
  }\n\
  default: break;\n\
  }\n\
  DaoProcess_PutInteger( _proc, pixel );\n\
";


//const string usertype_code_none = methlist_code + usertype_code;
const string usertype_code_struct = methlist_code + usertype_code;
const string usertype_code_struct2 = methlist_code + delete_struct + usertype_code;
const string usertype_code_class = methlist_code + delete_class + usertype_code;
const string usertype_code_class2 = methlist_code + delete_class + get_gcfields + usertype_code;

extern string cdao_string_fill( const string & tpl, const map<string,string> & subs );
extern string normalize_type_name( const string & name );
extern string cdao_make_dao_template_type_name( const string & name );
extern string cdao_remove_type_scopes( const string & qname );
extern string cdao_substitute_typenames( const string & qname );


CDaoUserType::CDaoUserType( CDaoModule *mod, const RecordDecl *decl )
{
	module = mod;
	used = false;
	useTypeTag = false;
	typedefed = false;
	unsupported = false;
	isRedundant = true;
	isRedundant2 = false;
	forceOpaque = false;
	dummyTemplate = false;
	isQObject = isQObjectBase = false;
	isMBString = isWCString = false;
	isNumber  = 0;
	wrapCount = 0;
	wrapType = CDAO_WRAP_TYPE_NONE;
	wrapTypeHint = CDAO_WRAP_TYPE_NONE;
	alloc_default = "NULL";
	this->decl = NULL;
	SetDeclaration( (RecordDecl*) decl );
}
void CDaoUserType::SetDeclaration( RecordDecl *decl )
{
	size_t pos;
	bool isC = module->compiler->getPreprocessor().getLangOpts().C99;
	this->decl = decl;
	location = decl->getLocation();
	if( decl->getDefinition() ) location = decl->getDefinition()->getLocation();

	qname = CDaoModule::GetQName( decl );
	idname = CDaoModule::GetIdName( decl );
	name = name2 = decl->getNameAsString();
	if( (pos = name.find( '<' )) != string::npos ) name.erase( pos );

	if( isC && decl->isStruct() ){
		if( qname.find( "struct " ) != 0 ) qname = "struct " + qname;
	}else if( isC && decl->isUnion() ){
		if( qname.find( "union " ) != 0 ) qname = "union " + qname;
	}
	SearchHints();
}
void CDaoUserType::SearchHints()
{
	string qname = this->qname;
	size_t i, j, pos;

	while( (pos = qname.find( "> >" )) != string::npos ) qname.replace( pos, 3, ">>" );
	map<string,vector<string> >::iterator it = module->functionHints.find( qname );
	if( it == module->functionHints.end() ){
		string qname2 = qname;
		if( (pos = qname2.find( '<' )) != string::npos ) qname2.erase( pos );
		it = module->functionHints.find( qname2 );
	}
	if( it != module->functionHints.end() ){
		CDaoVariable var;
		var.SetHints( it->second[0] );
		used = true; // maybe used due to hints such as "base", "new", "cxxtype" etc;
		if( var.unsupported ) forceOpaque = true;
		if( var.wrapNone ){
			unsupported = true;
			isRedundant2 = true;
		}
		useTypeTag = var.useTypeTag;
		isMBString = var.isMBS;
		isWCString = var.isWCS;
		isNumber = var.isNumber;
		if( isMBString or isWCString or isNumber ) toValue = var.names[0];
		if( var.hasBaseHint ) baseFromHint = var.names;
		if( var.hasDeleteHint ) hintDelete = var.hintDelete;
		if( var.wrapOpaque ){
			forceOpaque = true;
			wrapTypeHint = CDAO_WRAP_TYPE_OPAQUE;
		}else if( var.wrapDirect ){
			wrapTypeHint = CDAO_WRAP_TYPE_DIRECT;
		}
		if( var.hasDaoTypeHint ){
			this->qname = var.hintDaoType;
			idname = cdao_qname_to_idname( this->qname );
			name2 = this->qname;
			pos = name2.find( "::" );
			if( pos != string::npos ) name2.erase( 0, pos + 2 );
			name = name2;
			pos = name.find( "<" );
			if( pos != string::npos ) name.erase( pos, string::npos );
			wrapType = CDAO_WRAP_TYPE_NONE;
		}
	}
	extraMethods.clear();
	for(i=1; i<1000; ++i){
		char buf[10];
		sprintf( buf, "%i", (int)i );
		string exname = qname + "::" + buf;
		it = module->functionHints.find( exname );
		if( it == module->functionHints.end() ) break;

		string fname, proto;

		CDaoVariable var;
		var.SetHints( it->second[0] );
		fname = proto = var.name;
		proto.erase( 0, idname.size()+5 ); // dao_idname_
		proto += "( ";
		for(j=1; j<it->second.size(); ++j){
			CDaoVariable var2;
			var2.SetHints( it->second[j] );
			if( j > 1 ) proto += ", ";
			proto += var2.name + " :" + var2.hintDaoType;
		}
		proto += " )";
		if( var.hasCodeBlockHint ) proto += " [" + var.hintCodeBlock + "]";
		if( var.hintDaoType.size() ) proto += " => " + var.hintDaoType;
		extraMethods.push_back( fname );
		extraMethods.push_back( proto );
	}
}
void CDaoUserType::SetNamespace( const CDaoNamespace *ns )
{
}

bool CDaoUserType::IsFromMainModule()
{
	return module->IsFromMainModule( location );
}
bool CDaoUserType::IsFromRequiredModules()
{
	return module->IsFromRequiredModules( location );
}
string CDaoUserType::GetInputFile()const
{
	return module->GetFileName( location );
}
void CDaoUserType::Clear()
{
	type_decls.clear();
	type_codes.clear();
	meth_decls.clear();
	meth_codes.clear();
	dao_meths.clear();
	alloc_default.clear();
	cxxWrapperVirt.clear();
	typer_codes.clear();
	virtualMethods.clear();
}
string UppercaseString( const string & s )
{
	string str( s );
	std::transform(str.begin(), str.end(),str.begin(), ::toupper);
	return str;
}
int CDaoUserType::GenerateSimpleTyper()
{
	map<string,string> kvmap;
	string ss = dummyTemplate ? "<>" : "";
	char sindex[50];
	int i, n;
	set_bases = "";
	for(i=0,n=baseFromHint.size(); i<n; i++){
		sprintf( sindex, "%i", i );
		string supname2 = cdao_qname_to_idname( baseFromHint[i] );
		set_bases += "\tdao_" + idname + "_Core->supers[" + sindex + "] = dao_" + supname2 + "_Core;\n";
	}
	if( n ){
		sprintf( sindex, "%i", n );
		set_bases += "\tdao_" + idname + "_Core->supers[" + sindex + "] = NULL;\n";
	}
	kvmap[ "module" ] = UppercaseString( module->moduleInfo.name );
	kvmap[ "typer" ] = idname;
	kvmap[ "name2" ] = name2;
	kvmap[ "delete" ] = "NULL";
	kvmap[ "qname" ] = qname;
	kvmap[ "daotypename" ] = cdao_make_dao_template_type_name( qname ) + ss;
	if( hintDelete.size() ) kvmap[ "delete" ] = hintDelete;
	typer_codes = cdao_string_fill( usertype_code_none, kvmap );
	wrapType = CDAO_WRAP_TYPE_OPAQUE;
	if( dummyTemplate ) used = true;
	return 0;
}
int CDaoUserType::Generate()
{
	bool isC = module->compiler->getPreprocessor().getLangOpts().C99;
	RecordDecl *dd = decl->getDefinition();

	if( isMBString and not module->UseVariantString() ) isRedundant2 = true;
	if( isWCString and not module->UseVariantString() ) isRedundant2 = true;
	if( isNumber and not module->UseVariantNumber() ) isRedundant2 = true;

	SearchHints();
	outs() << "generating: " << qname << "\n";

#if 0
	if( dd != NULL && dd != decl ){
		outs()<<"-------------------- " << qname << " " << typedefed << " " << this << "\n";
		CDaoUserType *UT = module->GetUserType( dd );
		if( UT && UT->typedefed == false && UT->qname != qname ){
			UT->qname = qname;
			UT->wrapType = CDAO_WRAP_TYPE_NONE;
			if( typedefed ) UT->typedefed = true;
			UT->Generate();
		}
	}
#endif

	if( useTypeTag ){
		// In /usr/include/sys/stat.h, a struct and a function are both named as "stat",
		// to use the struct, the "struct" needs to be explicitly written in order to
		// use the struct. For ClangDao, a type can have hint "usetag", so that the
		// struct or union tag will be explicitly prefixed to the type name even in C++.
		// See DaoFLTK/fltk.cpp.
		if( decl->isStruct() ){
			if( qname.find( "struct " ) != 0 ){
				qname = "struct " + qname;
				wrapType = CDAO_WRAP_TYPE_NONE;
			}
		}else if( decl->isUnion() ){
			if( qname.find( "union " ) != 0 ){
				qname = "union " + qname;
				wrapType = CDAO_WRAP_TYPE_NONE;
			}
		}
	}else if( isC && (decl->isStruct() || decl->isUnion()) ){
		if( typedefed == false ){
			if( decl->isStruct() ){
				if( qname.find( "struct " ) != 0 ){
					qname = "struct " + qname;
					wrapType = CDAO_WRAP_TYPE_NONE;
				}
			}else if( decl->isUnion() ){
				if( qname.find( "union " ) != 0 ){
					qname = "union " + qname;
					wrapType = CDAO_WRAP_TYPE_NONE;
				}
			}
		}else{
			if( decl->isStruct() ){
				if( qname.find( "struct " ) == 0 ){
					qname.erase( 0, 7 );
					wrapType = CDAO_WRAP_TYPE_NONE;
				}
			}else if( decl->isUnion() ){
				if( qname.find( "union " ) == 0 ){
					qname.erase( 0, 6 );
					wrapType = CDAO_WRAP_TYPE_NONE;
				}
			}
		}
	}

	size_t pos = name.find( '(' );
	if( pos != string::npos ) name.erase( pos );
	if( qname.find( "(anonymous" ) != string::npos ){
		//outs() << qname << " " << module->GetFileName( decl->getLocation() ) << "============\n";
		//decl->getLocation().print( outs(), module->compiler->getSourceManager() );
		//outs() << "\n" << decl->isAnonymousStructOrUnion() << "\n\n";
		unsupported = true;
		return 0;
	}
	isRedundant = isRedundant2;
	if( isRedundant || unsupported ) return 0;
	if( name.find( "__" ) == 0 || qname.find( "__" ) == 0 ){
		forceOpaque = true;
	}
	if( name == "reverse_iterator" && idname.find( "std_0_reverse_iterator" ) == 0 ){
		// There is a problem to wrap std::reverse_iterator
		forceOpaque = true;
	}

	if( decl->isAnonymousStructOrUnion() ) return 0;
	if( module->finalGenerating == false && dd == NULL ) return 0;

	// ignore redundant declarations:
	isRedundant = dd != NULL && dd != decl;
	if( isRedundant ) return 0;
	if( wrapCount ){
		if( dd == NULL && wrapType == CDAO_WRAP_TYPE_OPAQUE ) return 0;
		if( dd == decl && wrapType >= CDAO_WRAP_TYPE_DIRECT ) return 0;
	}
	wrapCount = 0;
	Clear();

	// simplest wrapping for types declared or defined outsided of the modules:
	if( not module->IsFromModules( location ) ) return GenerateSimpleTyper();

	// simplest wrapping for types declared but not defined:
	if( dd == NULL || forceOpaque ) return GenerateSimpleTyper();
	if (CXXRecordDecl *record = dyn_cast<CXXRecordDecl>(decl)) return Generate( record );
	return Generate( decl );
}
void CDaoUserType::SetupDefaultMapping( map<string,string> & kvmap )
{
	kvmap[ "module" ] = UppercaseString( module->moduleInfo.name );
	kvmap[ "host_qname" ] = qname;
	kvmap[ "host_idname" ] = idname;
	kvmap[ "qname" ] = qname;
	kvmap[ "idname" ] = idname;
	kvmap[ "cxxname" ] = name;
	kvmap[ "name" ] = name;
	kvmap[ "name2" ] = name2;
	kvmap[ "daoname" ] = cdao_make_dao_template_type_name( qname );
	kvmap[ "daotypename" ] = cdao_make_dao_template_type_name( qname );

	kvmap[ "retype" ] = "=>" + name;
	kvmap[ "overload" ] = "";
	kvmap[ "virt_supers" ] = "";
	kvmap[ "supers" ] = "";
	kvmap[ "virtuals" ] = "";
	kvmap[ "methods" ] = "";
	kvmap[ "init_supers" ] = "";
	kvmap[ "parlist" ] = "";
	kvmap[ "parcall" ] = "";

	kvmap[ "Q_OBJECT" ] = "";
	kvmap[ "qt_signal_slot" ] = "";
	kvmap[ "qt_init" ] = "";
	kvmap[ "ss_parents" ] = "public QObject";
	kvmap[ "init_parents" ] = "";
	kvmap[ "qt_make_linker" ] = "";
	kvmap[ "qt_virt_emit" ] = "";
	kvmap[ "qt_make_linker3" ] = "";

	kvmap["signals"] = "";
	kvmap["slots"] = "";
	kvmap["member_wrap"] = "";
	kvmap["set_wrap"] = "";
	kvmap["Emit"] = "";
	kvmap["slotDaoQt"] = "";
	kvmap["signalDao"] = "";

	kvmap[ "typer" ] = idname;
	kvmap[ "child" ] = qname;
	kvmap[ "parent" ] = "";
	kvmap[ "parent2" ] = "";
	kvmap[ "decls" ] = "";
	kvmap[ "nums" ] = "";
	kvmap[ "meths" ] = "";
	kvmap[ "parents" ] = "";
	kvmap[ "casts" ] = "";
	kvmap[ "cast_funcs" ] = "";
	kvmap[ "alloc" ] = alloc_default;
	kvmap[ "delete" ] = "NULL";
	kvmap[ "if_parent" ] = "";
	kvmap[ "gcfields" ] = "NULL";
	kvmap[ "comment" ] = "";
	kvmap["delete"] = "Dao_" + idname + "_Delete";

}
void CDaoUserType::WrapField( CXXRecordDecl::field_iterator fit, map<string,string> kvmap )
{
	string codes;
	CDaoVariable field( module );
	field.SetQualType( fit->getTypeSourceInfo()->getType(), location );
	field.name = fit->getNameAsString();

	if( fit->isAnonymousStructOrUnion() ){
		//outs()<<field.name<<"------------------------------\n";
		const RecordType *RT = field.qualtype->getAsStructureType();
		if( RT == NULL ) RT = field.qualtype->getAsUnionType();
		RecordDecl *RD = RT->getDecl();
		// Handle injected fields:
		RecordDecl::field_iterator fit, fend;
		for(fit=RD->field_begin(),fend=RD->field_end(); fit!=fend; fit++){
			if( fit->getAccess() != AS_public ) continue;
			WrapField( fit, kvmap );
		}
	}
	if( field.name == "" || field.name[0] == '_' ) return;

	size_t pos;
	map<string,vector<string> >::iterator it = module->functionHints.find( qname + "::" + field.name );
	if( it == module->functionHints.end() ){
		string qname2 = qname;
		if( (pos = qname2.find( '<' )) != string::npos ) qname2.erase( pos );
		it = module->functionHints.find( qname2 + "::" + field.name );
	}
	if( it != module->functionHints.end() ) field.SetHints( it->second[0] );

	field.Generate( VAR_INDEX_FIELD );
	if( field.unsupported ) return;
	kvmap[ "name" ] = field.name;
	kvmap[ "ftype" ] = field.daotype;
	kvmap[ "itype" ] = field.dao_itemtype;
	string cxxproto = cdao_string_fill( cxx_getter_proto, kvmap );
	dao_meths += cdao_string_fill( dao_getter_proto, kvmap );
	meth_decls += cxxproto + ";\n";
	meth_codes += cxxproto + "\n{\n" + cdao_string_fill( cxx_gs_user, kvmap ) + field.getter + "}\n";
	if( field.ispixels && field.names.size() == 3 ){
		kvmap[ "pitch" ] = field.names[0];
		kvmap[ "width" ] = field.names[1];
		kvmap[ "height" ] = field.names[2];
		cxxproto = cdao_string_fill( cxx_get_pixel_proto, kvmap );
		dao_meths += cdao_string_fill( dao_get_pixel_proto, kvmap );
		codes = cdao_string_fill( cxx_get_pixel_codes, kvmap );
		meth_decls += cxxproto + ";\n";
		meth_codes += cxxproto + "\n{\n" + cdao_string_fill( cxx_gs_user, kvmap ) + codes + "}\n";
		if( field.readonly ) return;
		cxxproto = cdao_string_fill( cxx_set_pixel_proto, kvmap );
		dao_meths += cdao_string_fill( dao_set_pixel_proto, kvmap );
		codes = cdao_string_fill( cxx_set_pixel_codes, kvmap );
		meth_decls += cxxproto + ";\n";
		meth_codes += cxxproto + "\n{\n" + cdao_string_fill( cxx_gs_user, kvmap ) + codes + "}\n";
	}
	if( field.readonly ) return;
	if( field.isUserData ) set_fields += "\tself->" + field.name + " = self;\n";
	if( field.setter == "" ) return;
	cxxproto = cdao_string_fill( cxx_setter_proto, kvmap );
	dao_meths += cdao_string_fill( dao_setter_proto, kvmap );
	meth_decls += cxxproto + ";\n";
	meth_codes += cxxproto + "\n{\n" + cdao_string_fill( cxx_gs_user, kvmap ) + field.setter + "}\n";
}
int CDaoUserType::Generate( RecordDecl *decl )
{
	int i, j, n, m;
	vector<CDaoFunction> callbacks;
	map<string,string> kvmap;

	wrapType = CDAO_WRAP_TYPE_PROXY;
	//outs() << name << " " << qname << " CDaoUserType::Generate( RecordDecl *decl ) " << this << "\n";

	set_fields = "";
	SetupDefaultMapping( kvmap );

	RecordDecl::field_iterator fit, fend;
	for(fit=decl->field_begin(),fend=decl->field_end(); fit!=fend; fit++){
		const Type *type = fit->getTypeSourceInfo()->getType().getTypePtr();
		const PointerType *pt = dyn_cast<PointerType>( type );
		WrapField( fit, kvmap );
		if( pt == NULL ) continue;
		const Type *pt2 = pt->getPointeeType().getTypePtr();
		const ParenType *pt3 = dyn_cast<ParenType>( pt2 );
		if( pt3 == NULL ) continue;
		const Type *pt4 = pt3->getInnerType().getTypePtr();
		const FunctionProtoType *ft = dyn_cast<FunctionProtoType>( pt4 );
		if( ft == NULL ) continue;
		callbacks.push_back( CDaoFunction( module ) );
		callbacks.back().SetCallback( (FunctionProtoType*) ft, *fit );
		callbacks.back().location = location;
	}
	kvmap[ "parlist" ] = "";
	kvmap[ "retype" ] = "=>" + name;
	kvmap[ "overload" ] = "";

	dao_meths += cdao_string_fill( dao_proto, kvmap );
	meth_decls += cdao_string_fill( cxx_wrap_proto, kvmap ) + ";\n";
	if( callbacks.size() == 0 ){
		type_decls = cdao_string_fill( tpl_struct, kvmap );
		type_codes = cdao_string_fill( tpl_struct_alloc, kvmap );
		meth_codes += cdao_string_fill( c_nowrap_new, kvmap );
	}else{
		meth_codes += cdao_string_fill( c_wrap_new, kvmap );

		string set_callbacks;
		kvmap[ "callback" ] = "";
		for(i=0,n=callbacks.size(); i<n; i++){
			CDaoFunction & meth = callbacks[i];
			meth.Generate();
			if( meth.excluded or not meth.generated ) continue;
			wrapCount += 1;
			kvmap[ "callback" ] = meth.cxxName;
			set_callbacks += cdao_string_fill( tpl_set_callback, kvmap );
			type_decls += meth.cxxWrapperVirtProto;
			type_codes += meth.cxxWrapperVirt;
			CDaoProxyFunction::Use( meth.signature2 );
		}
		kvmap[ "field" ] = "";
#if 0
		for( f in selfields.keys() ){
			kvmap[ 'field' ] = f;
			setfields += tpl_set_selfield.expand( kvmap );
		}
		kvmap = { 'name'=>name, 'callbacks'=>callbacks, 'selfields'=>setfields };
#endif
		kvmap[ "callbacks" ] = set_callbacks;
		kvmap[ "selfields" ] = set_fields;
		type_decls += cdao_string_fill( tpl_struct_daoc, kvmap );
		type_codes += cdao_string_fill( tpl_struct_daoc_alloc, kvmap );
	}


	kvmap[ "decls" ] = meth_decls;
	kvmap[ "meths" ] = dao_meths;
#if 0
	if( del_tests.has( utp.condType ) ){
		kvmap["condition"] = del_tests[ utp.condType ];
		kvmap["deltest"] = "Dao_" + utp.name + "_DelTest";
		return usertype_code_class2.expand( kvmap );
	}
#endif
	if( hintDelete.size() ){
		kvmap[ "delete" ] = hintDelete;
		typer_codes = cdao_string_fill( usertype_code_struct, kvmap );
	}else{
		typer_codes = cdao_string_fill( usertype_code_struct2, kvmap );
	}
	return 0;
}

int CDaoUserType::Generate( CXXRecordDecl *decl )
{
	int i, j, n, m;
	bool hasVirtual = false; // protected or public virtual function;
	bool hasProtected = false;
	Preprocessor & pp = module->compiler->getPreprocessor();
	SourceManager & sm = module->compiler->getSourceManager();
	map<string,string> kvmap;
	map<string,int> overloads;

	wrapType = CDAO_WRAP_TYPE_NONE;

	vector<VarDecl*>      vars;
	vector<EnumDecl*>     enums;
	vector<CDaoFunction>  methods;
	vector<CDaoFunction>  constructors;

	vector<CDaoFunction*> pubslots;
	vector<CDaoFunction*> protsignals;

	string daoc_supers;
	string virt_supers;
	string init_supers;
	string ss_supers;
	string ss_init_sup;
	string class_new;
	string class_decl;
	string parents, casts, cast_funcs;

	outs() << "generating: " << qname << "\n";
	SetupDefaultMapping( kvmap );

	map<CXXMethodDecl*,CDaoUserType*>::iterator imd, emd;
	map<const RecordDecl*,CDaoUserType*>::iterator find;
	CXXRecordDecl::base_class_iterator baseit, baseend = decl->bases_end();
	for(baseit=decl->bases_begin(); baseit != baseend; baseit++){
		CXXRecordDecl *p = baseit->getType().getTypePtr()->getAsCXXRecordDecl();
		find = module->allUsertypes.find( p );
		if( find == module->allUsertypes.end() ){
			module->HandleUserType( baseit->getType(), location );
			find = module->allUsertypes.find( p );
			if( find == module->allUsertypes.end() ) continue;
		}

		CDaoUserType *sup = find->second;
		priorUserTypes.push_back( sup );

		string supname = sup->idname;
		sup->Generate();
		if( module->finalGenerating == false ) return 0;

		if( baseit->getAccessSpecifier() == AS_public and not sup->unsupported ){
			string supname = sup->qname;
			string supname2 = sup->idname;
			parents += "dao_" + supname2 + "_Core, ";
			kvmap[ "parent" ] = supname;
			kvmap[ "parent2" ] = supname2;
			if( baseit->isVirtual() ){
				casts += "dao_cast_" + idname + "_" + supname2 + ",";
				cast_funcs += cdao_string_fill( cast_to_parent_virtual_base, kvmap );
			}else{
				casts += "dao_cast_" + idname + "_" + supname2 + ",";
				cast_funcs += cdao_string_fill( cast_to_parent, kvmap );
			}
		}
		sup->used = true;

		//outs() << "parent: " << qname << "  " << sup->qname << " " << (int)sup->wrapType << "\n";
		if( sup->wrapType != CDAO_WRAP_TYPE_PROXY ) continue;

		for(imd=sup->virtualMethods.begin(),emd=sup->virtualMethods.end(); imd!=emd; imd++){
			virtualMethods[imd->first] = imd->second;
		}
		kvmap[ "super" ] = supname;
		if( virt_supers.size() ){
			daoc_supers += ',';
			virt_supers += ',';
		}
		daoc_supers += " public DaoCxx_" + supname;
		virt_supers += " public DaoCxxVirt_" + supname;
		init_supers += cdao_string_fill( tpl_init_super, kvmap );
		if( sup->isQObject ){
			if( ss_supers.size() ){
				ss_supers += ',';
				ss_init_sup += ',';
			}
			ss_supers += " public DaoSS_" + supname;
			ss_init_sup += " DaoSS_" + supname + "()";
		}
	}
	CXXRecordDecl::decl_iterator dit, dend;
	for(dit=decl->decls_begin(),dend=decl->decls_end(); dit!=dend; dit++){
		EnumDecl *edec = dyn_cast<EnumDecl>( *dit );
		if( edec == NULL || dit->getAccess() != AS_public ) continue;
		enums.push_back( edec );
	}
	for(dit=decl->decls_begin(),dend=decl->decls_end(); dit!=dend; dit++){
		TypedefDecl *TDD = dyn_cast<TypedefDecl>( *dit );
		if( TDD == NULL || dit->getAccess() != AS_public ) continue;
		//module->HandleTypeDefine( TDD );
		QualType qtype = TDD->getUnderlyingType();
		//outs() << "---" << TDD->getQualifiedNameAsString() << " " << qtype.getAsString() << "\n";
	}
	CXXRecordDecl::field_iterator fit, fend;
	for(fit=decl->field_begin(),fend=decl->field_end(); fit!=fend; fit++){
		if( fit->getAccess() != AS_public ) continue;
		WrapField( fit, kvmap );
	}
	CXXRecordDecl::method_iterator methit, methend = decl->method_end();
	for(methit=decl->method_begin(); methit!=methend; methit++){
		string name = methit->getNameAsString();
		//outs() << name << ": " << methit->getAccess() << " ......................\n";
		//outs() << methit->getType().getAsString() << "\n";
		if( methit->isImplicit() ) continue;
		if( methit->getAccess() == AS_private && ! methit->isPure() ) continue;
		if( methit->isVirtual() ) hasVirtual = true;
		if( dyn_cast<CXXDestructorDecl>( *methit ) ) continue;
		if( methit->getAccess() == AS_protected ) hasProtected = true;
		methods.push_back( CDaoFunction( module, *methit, ++overloads[name] ) );
		methods.back().location = location;
		if( methit->isVirtual() ){
			virtualMethods[ *methit ] = this;
			CXXMethodDecl::method_iterator it2, end2 = methit->end_overridden_methods();
			for(it2=methit->begin_overridden_methods(); it2!=end2; it2++)
				virtualMethods.erase( (CXXMethodDecl*)*it2 );
		}
	}

	bool has_ctor = false;
	bool has_public_ctor = false;
	bool has_protected_ctor = false;
	bool has_private_ctor = false;
	bool has_private_ctor_only = false;
	bool has_explicit_default_ctor = false;
	bool has_implicit_default_ctor = true;
	bool has_public_destructor = true;
	bool has_private_destructor = false;
	bool has_non_public_copy_ctor = false;
	CXXRecordDecl::ctor_iterator ctorit, ctorend = decl->ctor_end();
	for(ctorit=decl->ctor_begin(); ctorit!=ctorend; ctorit++){
		string name = ctorit->getNameAsString();
		//outs() << name << ": " << ctorit->getAccess() << " ......................\n";
		//outs() << ctorit->getType().getAsString() << "\n";
		if( ctorit->isImplicit() ) continue;
		//if( not ctorit->isImplicitlyDefined() ){ // not for clang 3.2:
			has_ctor = true;
			has_implicit_default_ctor = false;
			if( ctorit->getAccess() == AS_private ) has_private_ctor = true;
			if( ctorit->getAccess() == AS_protected ) has_protected_ctor = true;
			if( ctorit->getAccess() == AS_public ) has_public_ctor = true;
			if( ctorit->isCopyConstructor() && ctorit->getAccess() != AS_public )
				has_non_public_copy_ctor = true;
			if( ctorit->param_size() == 0 ){
				has_explicit_default_ctor = true;
				if( ctorit->getAccess() == AS_private ) has_private_ctor_only = true;
			}
		//}
		if( ctorit->getAccess() == AS_private ) continue;
		constructors.push_back( CDaoFunction( module, *ctorit, ++overloads[name] ) );
		constructors.back().location = location;
	}
	if( decl->hasUserDeclaredDestructor() ){
		CXXDestructorDecl *destr = decl->getDestructor();
		if( destr && destr->getAccess() != AS_public ){
			has_public_destructor = false;
			has_private_destructor = destr->getAccess() == AS_private;
		}
		if( destr ){
			const Type *type = destr->getTypeSourceInfo()->getType().getTypePtr();
			const FunctionProtoType *ft = type->getAs<FunctionProtoType>();
			if( ft->hasDynamicExceptionSpec() ) has_private_destructor = true;
		}
	}
	if( has_public_ctor || has_protected_ctor ) has_private_ctor_only = false;
	if( has_implicit_default_ctor ) has_public_ctor = true;

	if( isQObject ){
		map<const CXXMethodDecl*,CDaoFunction*> dec2meth;
		for(i=0, n = methods.size(); i<n; i++){
			CDaoFunction & meth = methods[i];
			const CXXMethodDecl *mdec = dyn_cast<CXXMethodDecl>( meth.funcDecl );
			dec2meth[mdec] = & meth;
		}
		bool is_pubslot = false;
		bool is_signal = false;
		for(dit=decl->decls_begin(),dend=decl->decls_end(); dit!=dend; dit++){
			AccessSpecDecl *asdec = dyn_cast<AccessSpecDecl>( *dit );
			if( asdec == NULL ){
				CXXMethodDecl *mdec = dyn_cast<CXXMethodDecl>( *dit );
				if( mdec == NULL ) continue;
				if( dec2meth.find( mdec ) == dec2meth.end() ) continue;
				if( is_pubslot ) pubslots.push_back( dec2meth[mdec] );
				if( is_signal ) protsignals.push_back( dec2meth[mdec] );
				continue;
			}
			is_pubslot = is_signal = false;
			string as = module->ExtractSource( asdec->getSourceRange() );
			is_pubslot = as.find( "public" ) != string::npos and as.find( "slots" ) != string::npos;
			is_signal = as.find( "signals" ) != string::npos;
			//outs() << as << " " << is_pubslot << is_signal << " access\n";
		}
	}
	if( module->skipProtected ) hasProtected = false;
	if( module->skipVirtual ){
		virtualMethods.clear();
		hasVirtual = false;
	}

	// Wrapping as derivable type when there is no private-only constructor, and:
	// 1. there is a virtual function, so that it can be re-implemented by derived Dao class;
	// 2. or, there is a protected function, so that it can be accessed by derived Dao class;
	// 3. or, there is a Qt signal/slot, so that such signal/slot can be connected with Dao.
	bool proxyWrapping = has_private_ctor_only == false && has_private_destructor == false;
	proxyWrapping &= hasVirtual || hasProtected 
		|| pubslots.size() || protsignals.size() || daoc_supers.size() != 0;

	wrapType = proxyWrapping ? CDAO_WRAP_TYPE_PROXY : CDAO_WRAP_TYPE_DIRECT;
	if( wrapTypeHint && wrapType > wrapTypeHint ) wrapType = wrapTypeHint;

	//outs()<<name<<": "<<(int)wrapType<<" "<<has_private_ctor_only<<" "<<daoc_supers.size()<<"\n";

	if( wrapType != CDAO_WRAP_TYPE_DIRECT or not decl->isAbstract() ){
		for(i=0,n=constructors.size(); i<n; i++){
			CDaoFunction & meth = constructors[i];
			const CXXConstructorDecl *ctor = dyn_cast<CXXConstructorDecl>( meth.funcDecl );
			meth.Generate();
			if( not meth.generated ) continue;
			//XXX if( ctor->getAccess() != AS_public ) continue;
			if( ctor->getAccess() == AS_private ) continue;
			wrapCount += 1;
			dao_meths += meth.daoProtoCodes;
			meth_decls += meth.cxxProtoCodes + (meth.cxxProtoCodes.size() ? ";\n" : "");
			meth_codes += meth.cxxWrapper;
		}
	}
	for(i=0,n=methods.size(); i<n; i++){
		CDaoFunction & meth = methods[i];
		const CXXConstructorDecl *ctor = dyn_cast<CXXConstructorDecl>( meth.funcDecl );
		const CXXMethodDecl *mdec = dyn_cast<CXXMethodDecl>( meth.funcDecl );
		if( ctor ) continue;
		meth.Generate();
		if( not meth.generated ) continue;
		if( mdec->getAccess() == AS_protected && not mdec->isPure() && not mdec->isOverloadedOperator() ){
			if( wrapType == CDAO_WRAP_TYPE_PROXY ){
				wrapCount += 1;
				dao_meths += meth.daoProtoCodes;
				meth_decls += meth.cxxProtoCodes + (meth.cxxProtoCodes.size() ? ";\n" : "");
			}
		}
		if( mdec->getAccess() != AS_public ) continue;
		wrapCount += 1;
		dao_meths += meth.daoProtoCodes;
		meth_decls += meth.cxxProtoCodes + (meth.cxxProtoCodes.size() ? ";\n" : "");
		meth_codes += meth.cxxWrapper;
		if( meth.cxxName == "operator[]" && meth.retype.qualtype->isReferenceType() ){
			int k, m = meth.parlist.size();
			QualType canotype = meth.retype.qualtype.getCanonicalType();
			if( meth.ConstQualified() ) continue;
			//if( canotype.getCVRQualifiers() & Qualifiers::Const ) continue;
			//if( canotype.isConstQualified() ) continue;

			CDaoVariable value( module );
			value.SetQualType( canotype, location );
			value.name = "_value";
			value.Generate( m, m-1 );
			string dao2cxxcodes;
			for(k=0; k<m; k++){
				CDaoVariable & vo = meth.parlist[k];
				dao2cxxcodes += vo.dao2cxx;
			}
			size_t start = meth.cxxWrapper.find( dao2cxxcodes );
			if( start == string::npos ) continue;
			start = meth.cxxWrapper.find( '=', start + dao2cxxcodes.size() );
			size_t end = meth.cxxWrapper.find( ';', start );
			string call = meth.cxxWrapper.substr( start + 1, end - start - 1 );
			dao2cxxcodes += value.dao2cxx;

			string daoproto = meth.daoProtoCodes;
			start = daoproto.find( "operator_43" );
			daoproto.replace( start, 11, "SETI" );
			start = daoproto.find( "=>" );
			daoproto.erase( start-1 );
			daoproto += ", _value :" + value.daotype + " )\" },\n";
			start = daoproto.find( '(' );
			daoproto.insert( start, "=" );

			string cxxproto = meth.cxxProtoCodes;
			start = cxxproto.find( "operator_43" );
			cxxproto.replace( start, 11, "SETI" );
			string codes = cxxproto + "\n{\n" + dao2cxxcodes;
			codes += " " + call + " = " + value.cxxcall + ";\n}\n";

			dao_meths += daoproto;
			meth_decls += cxxproto + (cxxproto.size() ? ";\n" : "");
			meth_codes += codes;
		}
	}
	for(i=0,n=extraMethods.size(); i<n; i+=2){
		meth_decls += "extern void " + extraMethods[i] + "( DaoProcess *_proc, DaoValue *_p[], int _n );";
		dao_meths += "  { " + extraMethods[i] + ", \"" + extraMethods[i+1] + "\" },\n";
	}
	if( isQObject ){
		string qt_signals;
		string qt_slots;
		for(i=0, n = pubslots.size(); i<n; i++){
			CDaoFunction *meth = pubslots[i];
			qt_signals += meth->qtSlotSignalDecl;
			qt_slots += meth->qtSlotSlotDecl;
			type_codes += meth->qtSlotSlotCode;
		}
		for(i=0, n = protsignals.size(); i<n; i++){
			CDaoFunction *meth = pubslots[i];
			qt_signals += meth->qtSignalSignalDecl;
			qt_slots += meth->qtSignalSlotDecl;
			type_codes += meth->qtSignalSlotCode;
		}
		kvmap["signals"] = qt_signals;
		kvmap["slots"] = qt_slots;
		kvmap["ss_parents"] = ss_supers;
		if( ss_init_sup.size() ) kvmap["init_parents"] = ":"+ ss_init_sup;
		if( isQObjectBase ){
			kvmap["member_wrap"] = "   " + name + " *wrap;\n";
			kvmap["set_wrap"] = "wrap = w;";
			kvmap["Emit"] = qt_Emit;
			kvmap["signalDao"] = qt_signalDao;
		}
		type_decls += cdao_string_fill( daoss_class, kvmap );
	} // isQObject

	if( has_implicit_default_ctor ){
		kvmap[ "daoname" ] = name;
		dao_meths = cdao_string_fill( dao_proto, kvmap ) + dao_meths;
		meth_decls = cdao_string_fill( cxx_wrap_proto, kvmap ) + ";\n" + meth_decls;
		if( wrapType == CDAO_WRAP_TYPE_DIRECT ){
			meth_codes = cdao_string_fill( cxx_wrap_alloc2, kvmap ) + meth_codes;
			type_decls += cdao_string_fill( tpl_struct, kvmap );
			type_codes += cdao_string_fill( tpl_struct_alloc2, kvmap );
		}else{
			meth_codes = cdao_string_fill( cxx_wrap_new, kvmap ) + meth_codes;
			class_new += cdao_string_fill( tpl_class_new, kvmap );
			type_codes += cdao_string_fill( tpl_class_init2, kvmap );
		}
		kvmap[ "daoname" ] = cdao_make_dao_template_type_name( qname );
	}
	kvmap[ "nums" ] = module->MakeConstNumItems( enums, vars, qname, true );
	kvmap[ "decls" ] = meth_decls;
	kvmap[ "meths" ] = dao_meths;
	kvmap["constructors"] = "";
	kvmap[ "comment" ] = has_public_destructor ? "" : "//";

	for(i=0,n=baseFromHint.size(); i<n; i++){
		string supname2 = cdao_qname_to_idname( baseFromHint[i] );
		parents += "dao_" + supname2 + "_Core, ";
	}
	kvmap[ "parents" ] = parents;

	//outs()<<qname<<": "<<wrapType<<" "<<has_private_ctor_only<<" "<<hasVirtual<<"\n";
	if( wrapType == CDAO_WRAP_TYPE_DIRECT ){
		kvmap[ "class" ] = name;
		kvmap[ "name" ] = name;
		kvmap[ "qt_make_linker3" ] = "";
		if( isQObject ) kvmap["qt_make_linker3"] = cdao_string_fill( qt_make_linker3, kvmap );
		if( decl->isAbstract() ){
			for(i=0, n = constructors.size(); i<n; i++){
				CDaoFunction & func = constructors[i];
				const CXXConstructorDecl *ctor = dyn_cast<CXXConstructorDecl>( func.funcDecl );
				if( not func.generated ) continue;
				if( ctor->getAccess() == AS_protected ) continue;
				kvmap["parlist"] = func.cxxProtoParam;
				kvmap["parcall"] = func.cxxCallParamV;
				type_codes += cdao_string_fill( tpl_class_noderive, kvmap );
				type_decls += cdao_string_fill( tpl_class_new_novirt, kvmap );
			}
		}
		typer_codes = cdao_string_fill( usertype_code_class, kvmap );
		return 0;
	}

	string kmethods;
	string kvirtuals;
	for(imd=virtualMethods.begin(),emd=virtualMethods.end(); imd!=emd; imd++){
		CXXMethodDecl *mdec = imd->first;
		string name = mdec->getNameAsString();
		bool isconst = mdec->getTypeQualifiers() & DeclSpec::TQ_const;
		if( mdec->getParent() == decl ) continue;

		CDaoFunction func( module, mdec, ++overloads[name] );
		func.Generate();
		if( func.excluded or func.cxxWrapperVirt2 == "" or not func.generated ){
			TypeLoc typeloc = mdec->getTypeSourceInfo()->getTypeLoc();
#if 0
			FunctionTypeLoc ftypeloc = cast<FunctionTypeLoc>(typeloc);
			//string source = module->ExtractSource( ftypeloc.getLocalSourceRange(), true );
			string source = module->ExtractSource( ftypeloc.getLocalSourceRange(), true );
			string source2 = module->ExtractSource( ftypeloc.getReturnLoc().getLocalSourceRange(), true );
#endif

			string proto = cdao_substitute_typenames( mdec->getReturnType().getAsString() );
			proto += " " + func.signature;
			//module->ExtractSource( mdec->getSourceRange(), true ); //with no return type
			kmethods += "\t" + proto + "{/*XXX 1*/}\n";
			outs()<<proto<<"\n";
			continue;
		}
		kvmap[ "name" ] = mdec->getNameAsString();
		kvmap[ "retype" ] = func.retype.cxxtype;
		kvmap[ "parlist" ] = func.cxxProtoParamDecl;
		kvmap[ "extra" ] = isconst ? "const" : "";
		kmethods += cdao_string_fill( tpl_meth_decl2, kvmap );
		string wrapper = func.cxxWrapperVirt2;
		string from = "DaoCxx_" + imd->second->idname + "::";
		string to = "DaoCxx_" + idname + "::";
		size_t pos = wrapper.find( from );
		if( pos != string::npos ) wrapper.replace( pos, from.size(), to );
		cxxWrapperVirt += wrapper;
	}
	for(i=0, n=methods.size(); i<n; i++){
		CDaoFunction & meth = methods[i];
		const CXXMethodDecl *mdec = dyn_cast<CXXMethodDecl>( meth.funcDecl );
		bool isconst = mdec->getTypeQualifiers() & DeclSpec::TQ_const;
		if( meth.excluded or not meth.generated ){
			if( mdec->isPure() ){
				TypeLoc typeloc = mdec->getTypeSourceInfo()->getTypeLoc();
				string source = module->ExtractSource( typeloc.getSourceRange(), true );
				//module->ExtractSource( mdec->getSourceRange(), true ); //with no return type
				kmethods += "\t" + source + "{/*XXX 1*/}\n";
			}
			continue;
		}
		if( dyn_cast<CXXConstructorDecl>( mdec ) ) continue;
		kvmap[ "name" ] = mdec->getNameAsString();
		kvmap[ "retype" ] = meth.retype.cxxtype;
		kvmap[ "parlist" ] = meth.cxxProtoParamDecl;
		kvmap[ "extra" ] = isconst ? "const" : "";
		if( mdec->isPure() && meth.cxxWrapperVirt2.size() == 0 ){
			TypeLoc typeloc = mdec->getTypeSourceInfo()->getTypeLoc();
			string source = module->ExtractSource( typeloc.getSourceRange(), true );
			//module->ExtractSource( mdec->getSourceRange(), true ); //with no return type
			kmethods += "\t" + source + "{/*XXX 1*/}\n";
		}
		if( mdec->isVirtual() && meth.cxxWrapperVirt2.size() ){
			kmethods += cdao_string_fill( tpl_meth_decl2, kvmap );
			kvmap[ "parlist" ] = meth.cxxProtoParamVirt;
			kvmap[ "comma" ] = meth.cxxProtoParamVirt.size() ? "," : "";
			kvirtuals += cdao_string_fill( tpl_meth_decl, kvmap );
			cxxWrapperVirt += meth.cxxWrapperVirt2;
		}
		if( mdec->getAccess() == AS_protected && not mdec->isPure() && not mdec->isOverloadedOperator() ){
			kvmap[ "type" ] = name;
			kvmap[ "cxxname" ] = mdec->getNameAsString();
			kvmap[ "parlist" ] = meth.cxxProtoParamDecl;
			kvmap[ "parcall" ] = meth.cxxCallParamV;
			kvmap[ "return" ] = kvmap[ "retype" ] == "void" ? "" : "return ";
			string mcode = cdao_string_fill( tpl_meth_decl3, kvmap );
			if( mdec->isStatic() ) mcode = "static " + mcode;
			kmethods += "\t" + mcode;
		}
	}

	if( isQObjectBase ){
		ss_supers += "public QObject, public DaoQtObject";
	}
	if( daoc_supers.size() ) daoc_supers = " :" + daoc_supers;
	if( virt_supers.size() ) virt_supers = " :" + virt_supers;
	if( init_supers.size() ) init_supers += "\n";

	kvmap[ "virt_supers" ] = virt_supers;
	kvmap[ "supers" ] = daoc_supers;
	kvmap[ "virtuals" ] = kvirtuals;
	kvmap[ "methods" ] = kmethods;
	kvmap[ "init_supers" ] = init_supers;
	if( isQObject ){
		kvmap["Q_OBJECT"] = "Q_OBJECT";
		kvmap["qt_init"] = qt_init;
		kvmap["qt_make_linker"] = cdao_string_fill( qt_make_linker, kvmap );
		kvmap["qt_make_linker3"] = cdao_string_fill( qt_make_linker3, kvmap );
		kvmap["qt_virt_emit"] = qt_virt_emit;
	}
	//if( has_ctor ){
		for(i=0, n = constructors.size(); i<n; i++){
			CDaoFunction & ctor = constructors[i];
			const CXXConstructorDecl *cdec = dyn_cast<CXXConstructorDecl>( ctor.funcDecl );
			if( not ctor.generated ) continue;
			//if( cdec->getAccess() == AS_protected ) continue;
			kvmap["parlist"] = ctor.cxxProtoParamDecl;
			kvmap["parcall"] = ctor.cxxCallParamV;
			class_decl += cdao_string_fill( tpl_class_decl_constru, kvmap );
			kvmap["parlist"] = ctor.cxxProtoParam;
			class_new += cdao_string_fill( tpl_class_new, kvmap );
			type_codes += cdao_string_fill( tpl_class_init2, kvmap );
		}
#if 0
	}else if( not has_private_ctor_only and not has_ctor ){
		// class has no virtual methods but has protected constructor
		// should also be wrapped, so that it can be derived by Dao class:
		class_new += cdao_string_fill( tpl_class_new, kvmap );
		type_codes += cdao_string_fill( tpl_class_init2, kvmap );
	}else if( not has_private_ctor_only ){
		type_codes += cdao_string_fill( tpl_class_noderive, kvmap );
	}
#endif
	kvmap["constructors"] = class_decl;
	for(i=0, n = methods.size(); i<n; i++){
		CDaoFunction & meth = methods[i];
		const CXXMethodDecl *mdec = dyn_cast<CXXMethodDecl>( meth.funcDecl );
		if( meth.excluded or not meth.generated ) continue;
		if( mdec->getAccess() == AS_protected && not mdec->isPure() && not mdec->isOverloadedOperator() ){
			dao_meths += meth.daoProtoCodes;
			meth_decls += meth.cxxProtoCodes + ";\n";
			string wrapper = meth.cxxWrapper;
			string name22 = "DaoCxx_" + idname;
			string from = qname + "* self = (" + qname + "*)";
			string to = name22 + "* self = (" + name22 + "*)";
			string from2 = "self->" + mdec->getNameAsString() + "(";
			string to2 = "self->DaoWrap_" + mdec->getNameAsString() + "(";
			string from3 = qname + "::" + mdec->getNameAsString() + "(";
			string to3 = name22 + "::DaoWrap_" + mdec->getNameAsString() + "(";
			size_t pos;
			if( (pos = wrapper.find( from )) != string::npos )
				wrapper.replace( pos, from.size(), to );
			if( (pos = wrapper.find( from2 )) != string::npos )
				wrapper.replace( pos, from2.size(), to2 );
			if( (pos = wrapper.find( from3 )) != string::npos ){
				wrapper.replace( pos, from3.size(), to3 );
			}
			meth_codes += wrapper;
		}
		if( mdec->isVirtual() && meth.cxxWrapperVirt.size() ){
			type_decls += meth.cxxWrapperVirtProto;
			type_codes += meth.cxxWrapperVirt;
			CDaoProxyFunction::Use( meth.signature2 );
		}
	}
	type_decls += cdao_string_fill( tpl_class_def, kvmap ) + class_new;
	type_codes += cdao_string_fill( tpl_class_init, kvmap );
	type_codes += cxxWrapperVirt;

	kvmap[ "casts" ] = casts;
	kvmap[ "cast_funcs" ] = cast_funcs;
	kvmap[ "alloc" ] = alloc_default;
	//if( utp.nested != 2 or utp.noDestructor ) return usertype_code_none.expand( kvmap );
	kvmap["delete"] = "Dao_" + idname + "_Delete";
	kvmap["gcfields"] = "Dao_" + idname + "_HandleGC";
	if( name == "QApplication" or name == "QCoreApplication" ) kvmap[ "comment" ] = "//";
	//if( isQWidget ) kvmap["if_parent"] = "if( ((" + utp.name2 + "*)self)->parentWidget() == NULL )";
#if 0
	if( del_tests.has( utp.condType ) ){
		kvmap["condition"] = del_tests[ utp.condType ];
		kvmap["deltest"] = "Dao_" + utp.name + "_DelTest";
		return usertype_code_class2.expand( kvmap );
	}
#endif
	kvmap["breakref"] = gcfields == "" ? "" : "\t\t" + gcfields + "\n";
	typer_codes = cdao_string_fill( usertype_code_class2, kvmap );
	return 0;
}
