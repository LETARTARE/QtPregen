/***************************************************************
 * Name:      qtpre.cpp
 * Purpose:   Code::Blocks plugin	'qtPregen.cbp'   1.1
 * Author:    LETARTARE
 * Created:   2015-02-27
 * Modified:  2017-12-15
 * Copyright: LETARTARE
 * License:   GPL
 **************************************************************/
#include "qtpre.h"

#include <sdk.h>
#include <cbplugin.h>      // sdk version
#include <manager.h>
#include <cbproject.h>
#include <compiletargetbase.h>
#include <projectmanager.h>
#include <macrosmanager.h>
#include <editormanager.h>
#include <cbeditor.h>
#include <wx/datetime.h>
// for linux
#include <wx/dir.h>
#include <projectfile.h>
#include "infowindow.h"     // InfoWindow::Display(
// not place change !
#include "print.h"
///-----------------------------------------------------------------------------
/// called by :
///		1. qtPrebuild::qtPrebuild(cbProject * prj, int logindex):1,
///
qtPre::qtPre(cbProject * prj, int logindex, wxString & nameplugin)
	: m_namePlugin(nameplugin),
	  m_Win(false), m_Linux(false), m_Mac(false),
	  m_Mexe(_T("")), m_Uexe(_T("")), m_Rexe(_T("")), m_Lexe(_T("")),
	  Mes(_T("")), m_nameproject(_T("")),
	  m_mam(Manager::Get()->GetMacrosManager() ),
	  m_project(prj), m_LogPageIndex(logindex)
{
	if (m_project)
		m_nameproject = m_project->GetTitle();
	else
		m_nameproject = wxEmptyString;

#if   defined(__WXMSW__)
	m_Win = true; m_Linux = m_Mac = false;
#elif defined(__WXGTK__)
	m_Linux = true ; m_Win = m_Mac = false;
#elif defined(__WXMAC__)
	m_Mac = true; m_Win = m_Linux = false;
#endif
	platForm();
	//
	m_Moc = _T("moc"); m_UI = _T("ui"); m_Qrc = _T("qrc"); m_Lm = _T("qm") ;
}
///-----------------------------------------------------------------------------
/// called by
///  1. qtPrebuild::~qtPrebuild():1,
///
qtPre::~qtPre()
{
}
///-----------------------------------------------------------------------------
/// Give internal name
///
/// called by :
///		1. QtPregen::OnPrebuild(CodeBlocksEvent& event):1,
///
wxString qtPre::namePlugin()
{
	return m_namePlugin;
}
///-----------------------------------------------------------------------------
/// Determines the system platform
///
/// called by :
///		1. qtPre(cbProject * prj, int logindex):1,
///
void qtPre::platForm() {
// choice platform
	if (! m_Win)
	{
		#undef SepD
		#define SepD 13
		#undef Sizesep
		#define Sizesep 1
	// tables  ( >= Qt-5.9 )
       //   TODO ????
		m_TablibQt.Add(_T("qtmain"),1) ;
	//	m_TablibQt.Add(_T("qtmaind"),1) ;
		// qt4   ???
	//	m_TablibQt.Add(_T("qtgui"),1) ;
	//	m_TablibQt.Add(_T("qtcore"),1) ;
	//	m_TablibQt.Add(_T("qtguid"),1) ;
	//	m_TablibQt.Add(_T("qtcored"),1) ;
		// qt5   ???
		m_TablibQt.Add(_T("qt5gui"),1) ;
		m_TablibQt.Add(_T("qt5core"),1) ;
		m_TablibQt.Add(_T("qt5widgets"),1) ;
		/// qt < 5.9.0
		//m_TablibQt.Add(_T("Qt5Guid"),1) ;
		//m_TablibQt.Add(_T("Qt5Cored"),1) ;
		//m_TablibQt.Add(_T("Qt5Widgetsd"),1);
	}

	if (m_Win)
	{
		#undef Eol
		#define Eol CrLf
	// tables
		m_TablibQt.Add(_T("qtmain"),1) ;
		m_TablibQt.Add(_T("qtmaind"),1) ;
		// qt4
		m_TablibQt.Add(_T("qtgui4"),1) ;
		m_TablibQt.Add(_T("qtcore4"),1) ;
		m_TablibQt.Add(_T("qtguid4"),1) ;
		m_TablibQt.Add(_T("qtcored4"),1) ;
		// qt5
		m_TablibQt.Add(_T("qt5gui"),1) ;
		m_TablibQt.Add(_T("qt5core"),1) ;
		m_TablibQt.Add(_T("qt5widgets"),1) ;
			// <= qt5.9
		m_TablibQt.Add(_T("qt5guid"),1) ;
		m_TablibQt.Add(_T("qt5cored"),1) ;
		m_TablibQt.Add(_T("qt5widgetsd"),1);
	}
	else
	if (m_Mac)
	{
		#undef Eol
		#define Eol Cr
	}
	else
	if (m_Linux)
	{
		#undef Eol
		#define Eol Lf
	}
}
///-----------------------------------------------------------------------------
///  Get version SDK
///
///  called by :
/// 	1. QtPregen::OnAttach():1,
///
wxString qtPre::GetVersionSDK()
{
	uint16_t major 	= PLUGIN_SDK_VERSION_MAJOR ;
	uint16_t minor 	= PLUGIN_SDK_VERSION_MINOR ;
	uint16_t release= PLUGIN_SDK_VERSION_RELEASE;
	Mes = (wxString() << major) + _T(".") + (wxString() << minor) + _T(".") + (wxString() << release);
	//print(Mes);

    return  Mes ;
}
///-----------------------------------------------------------------------------
///  Set version SDK
///
///  called by :
/// 		none
///
void qtPre::SetVersionSDK(const wxString & SetVersionSDK)
{   // TODO ?
}

///-----------------------------------------------------------------------------
///
///  called by :
///		1. QtPregen::OnCleanPregen(CodeBlocksEvent& event)

bool qtPre::isClean()
{
	return m_clean;
}

///-----------------------------------------------------------------------------
///
///  called by :
///		1. QtPregen::OnFileRemovedPregen(CodeBlocksEvent& event)

wxString qtPre::complementDirectory() const
{
	wxString nameactivetarget = m_project->GetActiveBuildTarget() ;
//	wxString dircomplement = m_dirgen + nameactivetarget + wxString(Slash) ;
	wxString dircomplement = m_dirgen ; // + nameactivetarget + wxString(Slash) ;

	return dircomplement;
}

///-----------------------------------------------------------------------------
///  Verify supplement file  ex : 'moc_xxx.cpp', 'ui_xxx.h', 'qrc_xxx.cpp'
///  file format  = "qtprebuild\NameTarget\..._xxxx.ext"
///  ... = ["moc", "ui", "qrc"]
///
///  called by :
///		1. qtPrebuild::unregisterProjectFile(const wxString & complement)
///
bool qtPre::isComplementFile(wxString & file)
{
	// target name
	m_nameactivetarget = m_project->GetActiveBuildTarget() ;
//	wxString filename = m_dirgen + m_nameactivetarget + wxString(Slash) + file;
	wxString filename = m_dirgen + file ; //+ m_nameactivetarget + wxString(Slash);
//Mes = filename;
//printWarn(Mes);
	int16_t  index = m_Registered.Index (filename);
	bool ok = index != wxNOT_FOUND;

	return ok;
}

///-----------------------------------------------------------------------------
///  Search creator file  ex : 'xxx.h'
///
///  called by :
///		1. qtPrebuild::unregisterProjectFile(const wxString & complement)
///
bool qtPre::isCreatorFile(wxString & file)
{
//Mes = _T("isCreatorFile -> ") + file;
//printWarn(Mes);
	int16_t  index = m_Filecreator.Index (file);
	bool ok = index != wxNOT_FOUND;

	return ok;
}

///-----------------------------------------------------------------------------
///  Set index page to log
///  called by :
/// 		none
///
void qtPre::SetPageIndex(int logindex)
{
	m_LogPageIndex = logindex;
}

///-----------------------------------------------------------------------------
/// Search virtual target
///
/// called by  :
///		1. findGoodfiles():1,
///
bool qtPre::isVirtualTarget(const wxString& nametarget, bool warning)
{
// search virtual target : vtargetsTable <- table::wxArrayString
	wxString namevirtualtarget = _T("") ;
	wxArrayString vtargetsTable = m_project->GetVirtualBuildTargets () ;
	int nvirtual = vtargetsTable.GetCount() ;
	bool ok = nvirtual > 0 ;
	if (ok)
	{
		ok = false  ;
	// virtual name
		for (int nv = 0; nv < nvirtual ; nv++ )
		{
			namevirtualtarget = vtargetsTable.Item(nv) ;
			ok = nametarget.Matches(namevirtualtarget) ;
			if (!ok)
				continue;

			if (warning)			{
				Mes = _T("'") + namevirtualtarget + _T("'") ;
				Mes += _(" is a virtual target !!") ;
				Mes += Lf + _("NOT YET IMPLEMENTED...") ;
				Mes += Lf + _("... you must use the real targets.")  ;
				Mes += Lf + _("Cannot continue.") ;
				printErr(Mes);
				cbMessageBox(Mes, _T(""), wxICON_ERROR)  ;
			}
			break;
		}
	}

	return ok  ;
}

///-----------------------------------------------------------------------------
/// Search all not virtual target
///
/// called by  :
///		1. buildOneFile(cbProject * prj, const wxString& fcreator):1;
///
wxArrayString qtPre::listRealsTargets(const wxString& nametarget)
{
	wxArrayString realsTargets;
	bool ok = isVirtualTarget(nametarget);
	if(ok)
	{
		// list virtual targets
		wxArrayString vtargetsTable = m_project->GetVirtualBuildTargets () ;
		int nvirtual = vtargetsTable.GetCount() ;
		ok = nvirtual > 0 ;
		if (ok)
		{
			wxString namevirtualtarget = _T("") ;
			ok = false  ;
		// virtual name
			for (int nv = 0; nv < nvirtual ; nv++ )
			{
				namevirtualtarget = vtargetsTable.Item(nv) ;
				ok = nametarget.Matches(namevirtualtarget) ;
				if (!ok)
					continue;
            // targets no virtuals
				realsTargets = m_project->GetVirtualBuildTargetGroup(nametarget) ;
				break;
			}
		}
	}
	else
	{
	    realsTargets.Add(nametarget);
    }

	return realsTargets;
}

///-----------------------------------------------------------------------------
/// Detection of a 'Qt' Project : it contains at least one target Qt
///
/// called by  :
///		1. QtPregen::OnActivate(CodeBlocksEvent& event):1,
///		2. QtPregen::OnPregen(CodeBlocksEvent& event):1,
///
/// calls to :
/// 	1. hasLibQt(CompileTargetBase * container):1+,
///
bool qtPre::detectQt(cbProject * prj, bool report)
{
//Mes = _T("=> 'qtPre::detectQt(...)'") ;
//printWarn(Mes);
	m_project = prj;
	if (!m_project)
		return false;

// project name
	m_nameproject = m_project->GetTitle() ;
//Mes = _T("m_nameproject = ") + m_nameproject;
//printWarn(Mes);
    m_nameactivetarget = m_project->GetActiveBuildTarget();
//Mes = _T("m_nameactivetarget = ") + m_nameactivetarget;
//printWarn(Mes);
//	if (m_nameactivetarget.IsEmpty())
//		return false;

/// attention : might be virtual target !
    // peek the first real target
    m_nameactivetarget = listRealsTargets(m_nameactivetarget).Item(0);
//Mes = _T("first real target = ") + m_nameactivetarget;
//printWarn(Mes);
    ProjectBuildTarget* buildtarget =  m_project->GetBuildTarget(m_nameactivetarget) ;
    if (!buildtarget)
		return false;

//Mes = _T("builtarget-> Title = ") + buildtarget->GetFullTitle();
//printWarn(Mes);

/// option relation target <-> project for compiler
    OptionsRelation rel = buildtarget->GetOptionRelation(ortCompilerOptions);
//Mes = _T("rel = ") + (wxString() << (int)rel);
//printWarn(Mes);
/** rel =
    orUseParentOptionsOnly = 0orUseTargetOptionsOnly
    orUseTargetOptionsOnly = 1
    orPrependToParentOptions = 2
    orAppendToParentOptions = 3//
*/
    /*
    bool ok = false ;
  //  if (rel == orUseTargetOptionsOnly)
        ok =  hasLibQt(buildtarget) ;
    if (!ok)
        ok = hasLibQt(m_project) ;
        */
    bool ok03 = false, ok12 = false;
/// use parent only or append target to parent ?
    // 0 or 3
    if (rel == orUseParentOptionsOnly || rel == orAppendToParentOptions)
    {
        // search libraries to project
        ok03 = hasLibQt(m_project) ;
        if (!ok03 && rel == orAppendToParentOptions)
         // search libraries to all targets
            ok03 = findLibQtToTargets();
    }
//Mes = _T("ok03 = ") + (wxString() << (int)ok03);
//printWarn(Mes);

/// use target only or prepend target to parent ?
    // 1 or 2
    if (rel == orUseTargetOptionsOnly || rel == orPrependToParentOptions)
    {
         // search libraries to targets
        ok12 = findLibQtToTargets();
        if (!ok12 && rel == orPrependToParentOptions)
        // search libraries to project
            ok12 = hasLibQt(m_project) ;
    }
//Mes = _T("ok12 = ") + (wxString() << (int)ok12);
//printWarn(Mes);

   // bool valid = ok;
	bool valid = ok03 || ok12;
	if (valid)
	{
		if (report)
		{
			wxString  title = _("The project") + Space + + m_nameproject + Space + _("uses Qt libraries !") ;
			Mes = _("It will generate the complements files...") ;
			InfoWindow::Display(title, Mes, 5000);
		}

		bool usemake = m_project->IsMakefileCustom() ;
		if(usemake)
		{
			Mes = _T("... but please, DISABLE using of custom makefile");
			Mes += Lf + Quote + m_namePlugin + Quote + _T(" not use makefile.");
			print(Mes);

			Mes += Lf + _("CAN NOT CONTINUE !") ;
			cbMessageBox(Mes, _("Used makefile !!"), wxICON_WARNING ) ;
			valid = false;
			Mes = m_namePlugin + _T(" -> ") + _("end") + _T(" ...");
			printWarn(Mes);
			return valid ;
		}
	}

	return valid;
}
///-----------------------------------------------------------------------------
/// Return true if find library QT in targets of 'm_project'
///
/// called by  :
///		1. detectQt(cbProject * prj, bool report= false):2,
///
///
bool qtPre::findLibQtToTargets()
{
    bool ok = false ;
// search library to targets
    ProjectBuildTarget* buildtarget;
    uint16_t nt = 0 , ntargets = m_project->GetBuildTargetsCount() ;
    while (nt < ntargets && ! ok )
    {
    // retrieve the target libraries  'ProjectBuildTarget* builstarget'
        buildtarget = m_project->GetBuildTarget(nt++) ;
        if (!buildtarget)
            continue ;
        ok = hasLibQt(buildtarget) ;
        if (ok)
            break;
    }
//Mes = _T("ok = ") + (wxString() << (int)ok);
//printWarn(Mes);

    return ok;
}

///-----------------------------------------------------------------------------
/// Return true if good  'CompileTargetBase* container'
///
/// called by  :
///		1. detectQt(cbProject * prj, bool report= false):2,
///
///
bool qtPre::hasLibQt(CompileTargetBase * container)
{
//Mes = _T("into 'qtPre::hasLibQt(...)'") ;
//printWarn(Mes);
	bool ok = false;
	if (!container)
		return ok;

	wxArrayString tablibs = container->GetLinkLibs() ;
	uint16_t nlib = tablibs.GetCount() ;
//Mes = _T("nlib = ") + (wxString() << nlib);
//printWarn(Mes);
	if (nlib > 0 )
	{
		wxString namelib ;
		uint16_t u=0;
		int16_t index= -1, pos ;
		while (u < nlib && !ok )
		{
			// lower, no extension
			namelib = tablibs.Item(u++).Lower().BeforeFirst('.') ;
//Mes = Quote + namelib + Quote;
//printWarn(Mes);
			// no prefix "lib"wxString() << n
			pos = namelib.Find(_T("lib")) ;
			if (pos == 0)
				namelib.Remove(0, 3) ;
			// begin == "qt"
			pos = namelib.Find(_T("qt")) ;
			if (pos != 0)
				continue ;
			// compare
			index = m_TablibQt.Index(namelib);
//Mes = _T("index = ") + (wxString() << index);
//printWarn(Mes);m_TablibQt
			ok = index != -1 ;
			// first finded
			if (ok)
				break;
		}
	}

	return ok;
}

///-----------------------------------------------------------------------------
///
///
/// called by :
/// 	1. QtPregen::OnActivate(CodeBlocksEvent& event):1,
/// calls to :
///		1. copyArray (const wxArrayString& strarray):2,
///		2. reverseFileCreator(const wxArrayString& strarray):1,

bool qtPre::detectComplements(cbProject * prj)
{
	bool ok = false;
	//
	m_nameactivetarget = prj->GetActiveBuildTarget() ;
	// wxString dirgen = m_dirgen + m_nameactivetarget + wxString(Slash);
	wxString dirgen = m_dirgen  ; // + m_nameactivetarget + wxString(Slash);
//Mes = _T("dirgen = ") + Quote + dirgen + Quote;
//printWarn(Mes);
	m_dirproject = prj->GetBasePath();
	wxFileName::SetCwd (m_dirproject);
//Mes = _T("dirproject = ") + Quote + m_dirproject + Quote;
//printWarn(Mes);
	wxDir dir(dirgen);
	ok = dir.Exists(dirgen);
//Mes = _T("dirgen exists ? = ") + (wxString() << ok);
//printWarn(Mes);
	// read all files
	if (ok)
	{
//Mes = _T("Call to 'dir.GetAllFiles(dirgen, &m_Filewascreated )'");
//printWarn(Mes);
		m_Filewascreated.Clear();
		size_t n = dir.GetAllFiles(dirgen, &m_Filewascreated );
        if (n)
		{
			Mes = Tab + _("with") + Space + (wxString() << n) ;
			Mes += Space + _("complement files already created on disk in") ;
			Mes += Space + Quote + dirgen + Quote;
		}
		else
		{
			Mes = Tab + _("No complement file to disk.") ;
		}
		printWarn(Mes);

		ok = ! m_Filewascreated.IsEmpty();
		if (ok)
		{
/*
Mes = _T("m_Filewascreated -> ") + (wxString() << m_Filewascreated.GetCount()) ;
printWarn(Mes);
uint16_t  nfiles = m_Filewascreated.GetCount() ;
for (uint16_t   i=0; i < nfiles ; i++ )
{
	Mes = (wxString() << i) + _T("- ");
	Mes += Quote + m_Filewascreated.Item(i) + Quote;
	printWarn(Mes)  ;
}
*/

///
			m_Registered.Clear();
			m_Registered = copyArray(m_Filewascreated ) ;
///
/*
Mes = _T("m_Registered -> ") + (wxString() << m_Registered.GetCount()) ;
printWarn(Mes);
nfiles = m_Registered.GetCount() ;
for (uint16_t   i=0; i < nfiles ; i++ )
{
	Mes = (wxString() << i) + _T("- ");
	Mes += Quote + m_Registered.Item(i) + Quote;
	printWarn(Mes)  ;
}
*/

///
			m_Createdfile.Clear();
			m_Createdfile = copyArray(m_Filewascreated ) ;
///
/*
Mes = _T("m_Createdfile -> ") + (wxString() << m_Createdfile.GetCount()) ;
printWarn(Mes);
nfiles = m_Createdfile.GetCount() ;
for (uint16_t   i=0; i < nfiles ; i++ )
{
	Mes = (wxString() << i) + _T("- ");
	Mes += Quote + m_Createdfile.Item(i) + Quote;
	printWarn(Mes)  ;
}
*/
///
			m_Filecreator.Clear();
			m_Filecreator = reverseFileCreator(m_Filewascreated) ;
///
/*
Mes = _T("m_Filecreator -> ") + (wxString() << m_Filecreator.GetCount()) ;
printWarn(Mes);
uint16_t nfiles = m_Filecreator.GetCount() ;
for (uint16_t   i=0; i < nfiles ; i++ )
{
	Mes = (wxString() << i) + _T("- ");
	Mes += Quote + m_Filecreator.Item(i) + Quote;
	printWarn(Mes)  ;
}
*/
///
			m_Filestocreate.Clear();
			uint16_t nf = m_Filewascreated.GetCount() ;
			for (uint16_t   i=0; i < nf ; i++ )
			{
				m_Filestocreate.Add(m_Devoid);
			}
		}
		else
		{
			// advice
			Mes =  _("Complements detected error !!");
		//	printErr(Mes) ;
		}
	}


	return ok;
}

///-----------------------------------------------------------------------------
/// For replace 'Targetsfile = prjfile.buildTargets'
/// copy a 'wxArrayString' to an another
///
/// called by  :
///		-# detectComplements(cbProject * prj):1,
///		-# filesTocreate(bool allrebuild):1,
///
wxArrayString qtPre::copyArray (const wxArrayString& strarray)
{
	wxArrayString tmp ;
	int nl = strarray.GetCount()  ;
	if (nl == 0)
		return  tmp ;

	// a line
	wxString line;
	for (int u = 0; u < nl; u++)
	{
	// read strarray line
		line = strarray.Item(u) ;
	// write line to tmp
		tmp.Add(line, 1) ;
	}

	return tmp ;
}

///-----------------------------------------------------------------------------
/// For create 'm_FilesCreator' from 'm_Filewascreated
/// reverse a 'wxArrayString' to an another
///
/// called by  :
///		1. detectComplements(cbProject * prj)
///

wxArrayString qtPre::reverseFileCreator(const wxArrayString& strarray)
{
	wxArrayString tmp ;
// read file list to 'strarray' with (moc_....cpp, ui_....cpp, qrc_....cpp, __Devoid__)
	uint16_t nfiles = strarray.GetCount() ;
// write file list to 'tmp' with  (*.h, *.cpp, *.qrc, *.ui)
	wxString fcreated, fcreator, prepend, ext, name ;
	for (uint16_t u=0; u < nfiles ; u++ )
	{
//Mes = Tab + (wxString()<<u) + _T("- ") ;
	// created
		fcreated = strarray.Item(u);
		fcreated = fcreated.AfterLast(Slash);
//Mes +=  Quote + fcreated + Quote;
		prepend = fcreated.BeforeFirst('_') ;
//Mes += Tab + Quote + prepend + Quote;
		name = fcreated.AfterFirst('_') ;
//Mes += Tab + Quote + name + Quote;
		name = name.BeforeFirst('.');
//Mes += Tab + Quote + name + Quote;
		ext = fcreated.AfterLast('.');
//Mes += Tab + Quote + ext + Quote;
	//1- file 'ui_uuu.h'  (forms)  -> 'uuu.ui'
		if ( prepend.Matches(m_UI) && ext.Matches(EXT_H))
			fcreator =  name + DOT_EXT_UI ;
		else
	//2- file 'qrc_rrr.cpp'  (resource) -> 'rrr.cpp'
		if (prepend.Matches(m_Qrc) )
			fcreator = name + DOT_EXT_CPP  ;
		else
	//4- file 'moc_mmm.cpp'  -> 'mmm.h'
		if (prepend.Matches(m_Moc) )
			fcreator =  name + DOT_EXT_H ;
	//5- file 'xxx.moc'  -> 'xxx.cpp'
		if (ext.Matches(m_Moc) )
			fcreator =  name + DOT_EXT_CPP ;
//Mes += _T(" => ") + Quote + fcreator + Quote ;
//printWarn(Mes);
		tmp.Add(fcreator);
	}

	return tmp;
}
///-----------------------------------------------------------------------------
