/***************************************************************
 * Name:      qtpre.cpp
 * Purpose:   Code::Blocks plugin
 * Author:    LETARTARE
 * Created:   2015-02-27
 * Modified:  2019-03-30
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
#include "defines.h"
///-----------------------------------------------------------------------------
///	Constructor
///
/// Called by :
///		1. qtPrebuild::qtPrebuild(cbProject * _pProject, int _logindex, wxString & _nameplugin):1,
///
qtPre::qtPre(const wxString & _nameplugin, int _logindex)
	: m_namePlugin(_nameplugin),
	//  m_Mexe(_T("")), m_Uexe(_T("")), m_Rexe(_T("")), m_Lexe(_T("")),
	  m_pm(Manager::Get()),
	  m_pMam(Manager::Get()->GetMacrosManager() ),
	  m_LogPageIndex(_logindex)
{
#if   defined(__WXMSW__)
	m_Win = true; m_Linux = m_Mac = false;
#elif defined(__WXGTK__)
	m_Linux = true ; m_Win = m_Mac = false;
#elif defined(__WXMAC__)
	m_Mac = true; m_Win = m_Linux = false;
#endif
	platForm();
	//
	//m_Moc = _T("moc"); m_UI = _T("ui"); m_Qrc = _T("qrc"); m_Lm = _T("qm") ;
}
///-----------------------------------------------------------------------------
///	Destructor
///
/// Called by
///		1. qtPrebuild::~qtPrebuild():1,
///
qtPre::~qtPre()
{
}
///-----------------------------------------------------------------------------
/// Get date
///
/// Called by  :
///		1. beginMesBuildCreate():1,
///		2. endMesBuildCreate():1
///
wxString qtPre::date()
{
	wxDateTime now = wxDateTime::Now();
	//wxString str = now.FormatISODate() + _T("-") + now.FormatISOTime();
	wxString str = now.FormatDate() + _T("-") + now.FormatTime();

    return str ;
}
///-----------------------------------------------------------------------------
/// Execution time
///
/// Called by  :
///		1. endMesBuildCreate():1
///		2. endMesFileCreate():1,
///		3. endMesClean():1,
///
wxString qtPre::duration()
{
    clock_t dure = clock() - m_start;
    if (m_Linux)
        dure /= 1000;

    return wxString::Format(_T("%ld ms"), dure);
}
///-----------------------------------------------------------------------------
///  Begin duration for Debug
///
void qtPre::beginDuration(const wxString & _namefunction)
{
// date
	Mes = _T("start of ") + _namefunction  + _T(" : ") + date()  ;
	printError(Mes);
	m_start = clock();
}

///-----------------------------------------------------------------------------
///  End duration for Debug
///
void qtPre::endDuration(const wxString & _namefunction)
{
	Mes = _T("end of ") + _namefunction  + _T(" : ") + date() + _T(" -> ") + duration();
	printError(Mes);
}
///-----------------------------------------------------------------------------
/// Give internal plugin name
///
/// Called by :
///		1. QtPregen::OnPrebuild(CodeBlocksEvent& event):1,
///
wxString qtPre::namePlugin()
{
	return m_namePlugin;
}
///-----------------------------------------------------------------------------
/// Determines the system platform
///
/// Called by :
///		1. qtPre(cbProject * _pProject, int _logindex, wxString & _nameplugin):1,
///
void qtPre::platForm()
{
// choice platform
	if (! m_Win)
	{
		#undef SepD
		#define SepD 13
		#undef SizeSep
		#define SizeSep 1
	// tables  ( >= Qt-5.9 )
       //   TODO ????
		m_TablibQt.Add(_T("qtmain"),1) ;
		m_TablibQt.Add(_T("qtmaind"),1) ;
		// qt5
		m_TablibQt.Add(_T("qt5gui"),1) ;
		m_TablibQt.Add(_T("qt5core"),1) ;
		m_TablibQt.Add(_T("qt5widgets"),1) ;
		m_TablibQt.Add(_T("qt5printsupport"),1);
		m_TablibQt.Add(_T("qt5xml"),1) ;
		// qt5
		m_TablibQt.Add(_T("qt5guid"),1) ;
		m_TablibQt.Add(_T("qt5cored"),1) ;
		m_TablibQt.Add(_T("qt5widgetsd"),1);
		m_TablibQt.Add(_T("qt5printsupportd"),1);
		m_TablibQt.Add(_T("qt5xmld"),1) ;
	}

	if (m_Win)
	{
Mes = _T("Platform is 'Win'");

		#undef Eol
		#define Eol CrLf
	// tables
		m_TablibQt.Add(_T("qtmain"),1) ;
		m_TablibQt.Add(_T("qtmaind"),1) ;
		// qt5
		m_TablibQt.Add(_T("qt5gui"),1) ;
		m_TablibQt.Add(_T("qt5core"),1) ;
		m_TablibQt.Add(_T("qt5widgets"),1) ;
		m_TablibQt.Add(_T("qt5printsupport"),1);
		m_TablibQt.Add(_T("qt5xml"),1) ;
			// <= qt5.9
		m_TablibQt.Add(_T("qt5guid"),1) ;
		m_TablibQt.Add(_T("qt5cored"),1) ;
		m_TablibQt.Add(_T("qt5widgetsd"),1);
		m_TablibQt.Add(_T("qt5printsupportd"),1);
		m_TablibQt.Add(_T("qt5xmld"),1) ;
	}
	else
	if (m_Mac)
	{
Mes = _T("Platform is 'Mac'");
		#undef Eol
		#define Eol Cr
	}
	else
	if (m_Linux)
	{
Mes = _T("Platform is 'Linux'");
		#undef Eol
		#define Eol Lf
	}

//print(Mes);

}
///-----------------------------------------------------------------------------
///  Get version SDK
///
///  Called by :
/// 	1. QtPregen::OnAttach():1,
///
wxString qtPre::GetVersionSDK()
{
	// from 'cbplugin.h'
	uint16_t major 	= PLUGIN_SDK_VERSION_MAJOR
			,minor 	= PLUGIN_SDK_VERSION_MINOR
			,release= PLUGIN_SDK_VERSION_RELEASE;
	Mes =  strInt(major) + _T(".") + strInt(minor) + _T(".") + strInt(release);

    return  Mes ;
}

///-----------------------------------------------------------------------------
/// test clean
///
///  Called by :
///		none

bool qtPre::isClean()
{
	if (m_clean)
	{
		m_Filecreator.Clear();
		m_Registered.Clear();
		m_Createdfile.Clear();
		m_Filestocreate.Clear();
		m_Filewascreated.Clear();
	}

	return m_clean;
}

///-----------------------------------------------------------------------------
/// Set abort
///
/// Called by :
///		1. QtPregen::OnAbortAdding(CodeBlocksEvent& event):1,
///
void qtPre::setAbort()
{
	m_abort = true;
}

///-----------------------------------------------------------------------------
/// Set Set 'm_pProject'
///
/// Called by :
///		1. QtPregen::OnRenameProjectOrTarget(CodeBlocksEvent& _event):1,
///		2. QtPregen::onProjectFileRemoved(CodeBlocksEvent& _event):1,
///
/// Calls to :
/// 	1. qtPre::isVirtualTarget(const wxString& _nametarget, bool _warning=false):1,
///		2. qtpre::listRealsTargets(const wxString& _nametarget):1,

void qtPre::setProject(cbProject * _pProject)
{
	bool good = _pProject != nullptr ;
	if (good)
	{
		m_pProject = _pProject;
		m_nameProject = m_pProject->GetTitle();
		/// active target is perhaps virtual !!
		m_nameActiveTarget = m_pProject->GetActiveBuildTarget();
		if (isVirtualTarget(m_nameActiveTarget))
			m_nameActiveTarget = listRealsTargets(m_nameActiveTarget).Item(0);
//Mes = _T("Project name = ") + m_nameProject ;
//Mes += _T(", first real m_nameActiveTarget = ") + Quote + m_nameActiveTarget + Quote;
//print(Mes);
		ProjectBuildTarget * pBuildTarget = m_pProject->GetBuildTarget(m_nameActiveTarget);
		if (pBuildTarget)   setBuildTarget(pBuildTarget);
	}
	else
		printError(_T(" Error in 'qtPre::setProject(_pProject == null)'") );
}

///-----------------------------------------------------------------------------
/// Set Set 'm_pBuildTarget'
///
/// Called by :
///		1. qtPre::setProject(cbProject * _pProject):1,
///
void qtPre::setBuildTarget(ProjectBuildTarget * _pBuildTarget)
{
	if (_pBuildTarget)
	{
//Mes = _T(" setBuildTarget ... ") + _pBuildTarget->GetFullTitle() ;
//print(Mes);
		m_pBuildTarget = _pBuildTarget;
		m_dirObjects =  m_pBuildTarget->GetObjectOutput();
//Mes = _T("Before From target :: m_dirObjects = ") + Quote + m_dirObjects + Quote ;
//printWarn(Mes);
        m_pMam->ReplaceMacros(m_dirObjects);
//Mes = _T("From target :: m_dirObjects = ") + Quote + m_dirObjects + Quote ;
//printWarn(Mes);
	}
	else
		printError(_T(" Error in 'qtPre::setBuildTarget(_pbuildtarget == null)'") );
}

///-----------------------------------------------------------------------------
///	Give the full complement filename  : target + "\" + filename
///
///  Called by :
///		1. complementDirectory():1,

wxString qtPre::fullFilename(const wxString & _file)
{
	wxString filename = _file;
	wxString nameactivetarget = m_pProject->GetActiveBuildTarget() ;
	filename = nameactivetarget + wxString(Slash) + filename;

	return filename;
}

///-----------------------------------------------------------------------------
///	Give the complement directory
///
///  Called by :
///		1. qtPrebuild::buildOneFile(cbProject * _pProject, const wxString& _fcreator):1,
///		2. qtPrebuild::addAllFiles():1,

wxString qtPre::complementDirectory() const
{
	wxString nameactivetarget = m_pProject->GetActiveBuildTarget() ;
	wxString dircomplement = m_dirPreBuild + nameactivetarget + wxString(Slash) ;

	return dircomplement;
}

///-----------------------------------------------------------------------------
///  Verify complement file
///		ex : 'moc_xxx.cpp', 'ui_xxx.h', 'qrc_xxx.cpp',
///  	file format  = '"m_dirPreBuild"\..._xxxx.ext',
///  	... = ["moc", "ui", "qrc"]
///
///  Called by :
///		1. QtPregen::onProjectFileRemoved(CodeBlocksEvent& _event)
///
bool qtPre::isComplementFile(const wxString & _file)
{
//Mes = _T("isComplementFile -> ") + _file ;
//printWarn(Mes);
//print(allStrTable(_T("m_Registered"), m_Registered));

// target name
	m_nameActiveTarget = m_pProject->GetActiveBuildTarget() ;
	wxString filename = m_dirPreBuild + fullFilename(_file) ;
//Mes = filename;
//printWarn(Mes);
	int16_t  index = m_Registered.Index (filename);
	bool ok = index != wxNOT_FOUND;

	return ok;
}

///-----------------------------------------------------------------------------
///  Search creator file  ex : 'xxx.h'
///		format 'm_Filecreator' ->  'dircreator\xxx.h'
///
///  Called by :
///		1. QtPregen::onProjectFileRemoved(CodeBlocksEvent& _event)
///
bool qtPre::isCreatorFile(const wxString & _file)
{
//Mes = _T("isCreatorFile -> ") + file ;
//printWarn(Mes);
//print(allStrTable(_T("m_Filecreator"), m_Filecreator));
	wxString dircreator = m_Filecreator.Item(0).BeforeLast(Slash)  ;
	wxString filename =  dircreator + Slash + _file;

	int16_t  index = m_Filecreator.Index (filename);
	bool ok = index != wxNOT_FOUND;
//Mes = _T(" -> file = ") + file;
//Mes += _T(", filename = ") + filename;
//Mes += _T(", index = ") + strInt(index);
//print(Mes);

	return ok;
}

///-----------------------------------------------------------------------------
/// Gives the name of the file to create on
///		file = xxx.h(pp) 	->	moc_xxx.cpp
///		file = xxx.ui	->	ui_xxx.h
///		file = xxx.rc	->  rc_xxx.cpp
///		file = xxx.cpp	->  xxx.moc
///
/// Called by  :
///		1. qtPrebuild::buildOneFile(cbProject * _pProject, const wxString& _fcreator):1,
///		2. qtPrebuild::unregisterFileComplement(const wxString & file, bool _creator, bool _first):1,
///		3. qtPrebuild::addAllFiles():1,
///
wxString qtPre::nameCreated(const wxString& _file)
{
	wxString name  = _file.BeforeLast('.') ;
	wxString fout ;
	if (name.Find(Slash) > 0)
	{
	// short name
		name = name.AfterLast(Slash) ;
	// before path
		fout += _file.BeforeLast(Slash) + wxString(Slash) ;
	}
	wxString ext  = _file.AfterLast('.')  ;
//1- file *.ui  (forms)
	if ( ext.Matches(m_UI) )
		fout += m_UI + _T("_") + name + DOT_EXT_H  ;
	else
//2- file *.qrc  (resource)
	if (ext.Matches(m_Qrc) )
		fout += m_Qrc + _T("_") + name + DOT_EXT_CPP  ;
	else
//3- file *.h or *hpp with 'Q_OBJECT' and/or 'Q_GADGET'
	if (ext.Matches(EXT_H) || ext.Matches(EXT_HPP)  )
		fout +=  m_Moc + _T("_") + name + DOT_EXT_CPP ;
	else
//4- file *.cpp with 'Q_OBJECT' and/or 'Q_GADGET'
	if (ext.Matches(EXT_CPP) )
		fout +=  name + DOT_EXT_MOC ;

	fout = fout.AfterLast(Slash) ;

	return fout  ;
}

///-----------------------------------------------------------------------------
///  Set index page to log
///
///  Called by : none
///
void qtPre::SetPageIndex(int _logindex)
{
	m_LogPageIndex = _logindex;
}

///-----------------------------------------------------------------------------
/// Search virtual target
///
/// Called by  :
///		1. listRealsTargets(const wxString& _nametarget):1,
///		2. detectQtProject(cbProject * _pProject, bool _report):1,
///		3. detectQtTarget(const wxString& _nametarget, cbProject * _pProject)
///		4. qtPrebuild::findGoodfilesTarget():1,
///		5. qtPrebuild::findGoodfiles():1,
///
bool qtPre::isVirtualTarget(const wxString& _nametarget, bool _warning)
{
	bool ok = m_pProject->HasVirtualBuildTarget(_nametarget);
/*  NOT USED ...
	if (_warning) {
			Mes = _T("'") + namevirtualtarget + _T("'") ;
			Mes += _(" is a virtual target !!") ;
			Mes += Lf + _("NOT YET IMPLEMENTED...") ;
			Mes += Lf + _("... you must use the real targets instead.")  ;
			printError(Mes);
			cbMessageBox(Mes, m_namePlugin + Space + _("plugin"), wxICON_ERROR)  ;
	}
*/

	return ok  ;
}

///-----------------------------------------------------------------------------
/// Search all not virtual target
///
/// Called by  :
///		1. detectQtProject(cbProject * _pProject, bool _report):1,
///		2. qtPrebuild::buildOneFile(cbProject * _pProject, const wxString& _fcreator):1,
///
wxArrayString qtPre::listRealsTargets(const wxString& _nametarget)
{
	wxArrayString realsTargets;
	bool ok = m_pProject->HasVirtualBuildTarget(_nametarget);
	if (ok)
		realsTargets = m_pProject->GetExpandedVirtualBuildTargetGroup(_nametarget);
	else
	    realsTargets.Add(_nametarget);

	return realsTargets;
}

///-----------------------------------------------------------------------------
/// Detection of a 'Qt' Project : it contains at least one target Qt
///
/// Called by  :
///		1. QtPregen::OnActivateProject(CodeBlocksEvent& _event):1,
///		2. QtPregen::OnNewProject(CodeBlocksEvent& _event):1,
///		3. QtPregen::OnRenameProjectOrTarget(CodeBlocksEvent& _event):1,
///		4. QtPregen::OnAddComplements(CodeBlocksEvent& _event):1,
///
/// Calls to :
///		1. isVirtualTarget(const wxString& _nametarget, bool _warning=false):1,
///		2. listRealsTargets(const wxString& _nametarget):1,
///		3. findLibQtToTargets():2,
///		4. hasLibQt(CompileTargetBase * _pContainer):1,
///
bool qtPre::detectQtProject(cbProject * _pProject, bool _report)
{
//Mes = _T("=> 'qtPre::detectQtProject(...)'") ;
//printWarn(Mes);
	if (! _pProject)
		return false;

// project name
	m_pProject = _pProject;
	m_nameProject = m_pProject->GetTitle() ;
//Mes = _T("m_nameProject = ") + m_nameProject;
//printWarn(Mes);
    m_nameActiveTarget = m_pProject->GetActiveBuildTarget();
/// TO REVIEW ...
// attention : might be virtual target !
	if (isVirtualTarget(m_nameActiveTarget))
    // peek the first real target
		m_nameActiveTarget = listRealsTargets(m_nameActiveTarget).Item(0);
//Mes = _T("first real target = ") + m_nameActiveTarget;
//printWarn(Mes);
    ProjectBuildTarget* pBuildTarget =  m_pProject->GetBuildTarget(m_nameActiveTarget) ;
    if (!pBuildTarget)
		return false;

//Mes = _T("pBuiltarget-> Title = ") + pBuildtarget->GetFullTitle();
//printWarn(Mes);
// option relation target <-> project for compiler
    OptionsRelation rel = pBuildTarget->GetOptionRelation(ortCompilerOptions);
//Mes = _T("rel = ") + strInt((int)rel);
//printWarn(Mes);
/** rel =
 *   orUseParentOptionsOnly = 0,
 *   orUseTargetOptionsOnly = 1,   : independent target
 *   orPrependToParentOptions = 2,
 *   orAppendToParentOptions = 3,
 */
    bool ok03 = false, ok12 = false;
// use parent only or append target to parent ?
    // 0 or 3
    if (rel == orUseParentOptionsOnly || rel == orAppendToParentOptions)
    {
        // search libraries to project
        ok03 = hasLibQt(m_pProject) ;
        if (!ok03 && rel == orAppendToParentOptions)
         // search libraries to all targets
            ok03 = findLibQtToTargets();
    }
//Mes = _T("ok03 = ") + strInt((int)ok03);
//printWarn(Mes);

// use target only or prepend target to parent ?
    // 1 or 2
    if (rel == orUseTargetOptionsOnly || rel == orPrependToParentOptions)
    {
         // search libraries to targets
        ok12 = findLibQtToTargets();
//Mes = _T("ok12 = ") + strInt((int)ok12);
//printWarn(Mes);
        if (!ok12 && rel == orPrependToParentOptions)
        // search libraries to project
            ok12 = hasLibQt(m_pProject) ;
    }
//Mes = _T("ok12 = ") + strInt((int)ok12);
//printWarn(Mes);
	bool valid = ok03 || ok12;
	if (valid)
	{
		if (_report)
		{
			wxString  title = _("The project") + Space + m_nameProject + Space + _("uses Qt libraries !") ;
			Mes = Quote + m_namePlugin + Quote + Space + _("will generate the complements files...") ;
			InfoWindow::Display(title, Mes, 5000);
			// to Code::Blocks log
			//Print(title + Space + Mes) ;
		}

		bool usemake = m_pProject->IsMakefileCustom() ;
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
/// Detection of a 'Qt' Target
///
/// Called by  :
///		1. QtPregen::OnActivateProject(CodeBlocksEvent& _event):1,
///		2. QtPregen::OnNewProject(CodeBlocksEvent& _event):1,
///		3. QtPregen::OnRenameProjectOrTarget(CodeBlocksEvent& _event):1,
///		4. QtPregen::OnActivateTarget(CodeBlocksEvent& _event):1,
///
/// Calls to :
///		1. isVirtualTarget(const wxString& _nametarget, bool _warning):1,
///
bool qtPre::detectQtTarget(const wxString& _nametarget, cbProject * _pProject)
{
	bool ok = false;
	if(!_pProject || _nametarget.IsEmpty())
		return ok;
// is virtual target ?
	if (isVirtualTarget(_nametarget, WITH_WARNING))
		return ok;
//Mes =  Tab + Quote + nametarget + Quote + _T(" is NOT virtual") ;
//printWarn(Mes);

	m_pProject = _pProject;
// is a real target
	ProjectBuildTarget * pBuildTarget = m_pProject->GetBuildTarget(_nametarget);
	if (! pBuildTarget)
		return ok;
//Mes =  Tab + Quote + nametarget + Quote + _T(" exists") ;
//printWarn(Mes);
	ok = hasLibQt(pBuildTarget);
	if (!ok)
	{
		// search into project libraries only, not others targets

	}
//Mes = _T("detectQtTarget(...) = ") + strBool(ok);
//printWarn(Mes);

	return ok;
}

///-----------------------------------------------------------------------------
/// Return true if find library QT in targets of 'm_pProject'
///
/// Called by  :
///		1. detectQtProject(cbProject * _pProject, bool _report = false):2,
///
///	Calls to :
///		1. hasLibQt(CompileTargetBase * _pContainer):1,
///
bool qtPre::findLibQtToTargets()
{
    bool ok = false ;
// search library to targets
    ProjectBuildTarget* pBuildTarget;
    uint16_t nt = 0 , ntargets = m_pProject->GetBuildTargetsCount() ;
    while (nt < ntargets && ! ok )
    {
    // retrieve the target libraries  'ProjectBuildTarget* pBuildTarget'
        pBuildTarget = m_pProject->GetBuildTarget(nt++) ;
        if (!pBuildTarget)
            continue ;
//Mes = strInt(nt) + _T(" -> ") + Quote + buildtarget->GetTitle() + Quote;
//printWarn(Mes);
        ok = hasLibQt(pBuildTarget) ;
        if (ok)
            break;
    }
//Mes = _T("findLibQtToTargets() = ") + strBool(ok);
//printWarn(Mes);

    return ok;
}

///-----------------------------------------------------------------------------
/// Return true if good  'CompileTargetBase* container'
///
/// Called by  :
///		1. detectQtProject(cbProject * _pProject, bool _report = false):2,
///		2. findLibQtToTargets():1,
///
bool qtPre::hasLibQt(CompileTargetBase * _pContainer)
{
//Mes = _T("into 'qtPre::hasLibQt(...)'") ;
//printWarn(Mes);
	bool ok = false;
	if (!_pContainer)
		return ok;

	wxArrayString tablibs = _pContainer->GetLinkLibs() ;
	uint16_t nlib = tablibs.GetCount() ;
//Mes = _T("nlib = ") + strInt(nlib);
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
//Mes = strInt(u) + _T(" -> ") + Quote + namelib + Quote;
//printWarn(Mes);
			// no prefix "lib"
			pos = namelib.Find(_T("lib")) ;
			if (pos == 0)
				namelib.Remove(0, 3) ;
			// begin == "qt"
			pos = namelib.Find(_T("qt")) ;
//Mes = _T("pos 'qt' = ") + strInt(pos);
//printWarn(Mes);
			if (pos != 0)
				continue ;
			// find
/// ATTENTION : the table should then contain all Qt libraries !!
			index = m_TablibQt.Index(namelib);
//Mes = _T("index = ") + strInt(index);
//printWarn(Mes);
			ok = index != -1 ;
			// first finded
			if (ok)
				break;
		}
	}
//Mes = _T("haslibQt(...) = ") + strBool(ok);
//printWarn(Mes);

	return ok;
}

///-----------------------------------------------------------------------------
/// Create a new directory,transfer all files to the new directory, delete old
///	directory
///
/// calle by :
///		1. QtPregen::OnRenameProjectOrTarget(CodeBlocksEvent& _event):1,
/*
bool qtPre::newNameComplementDirTarget(wxString & _newname, wxString & _oldname)
{
/// _oldname NOT USED ??
	m_dirProject = m_pProject->GetBasePath();
	wxFileName::SetCwd (m_dirProject);
//Mes = _T("dirproject = ") + Quote + m_dirProject + Quote;
//printWarn(Mes);
	wxString complementDir = m_dirPreBuild + _oldname ;
	bool ok = wxFileName::DirExists(complementDir);
	if (ok)
	{
Mes = _T("this old repertory exists :") + Quote +  complementDir  + Quote;
printWarn(Mes);
      //  wxFileName filesystem;
		//filesystem.AssignDir(m_dirPreBuild);
		ok = ::wxRenameFile( complementDir, m_dirPreBuild + _newname);
		if (ok)
		{
			complementDir = m_dirPreBuild + _newname ;
			ok = wxFileName::DirExists(complementDir);
			if (ok)
			{
				Mes = _T("this new repertory exists :") + Quote +  complementDir  + Quote;
				printWarn(Mes);
			}
		}
		else
		{
			Mes = _T("this repertory NOT exists : ") + Quote + complementDir + Quote;
			printError(Mes);
		}

	}

	return ok;
}
*/
///-----------------------------------------------------------------------------
/// Change old to new directory of complements
///
/// calle by :
///		1. QtPregen::OnRenameProjectOrTarget(CodeBlocksEvent& _event):1,
/*
bool qtPre::renameDirTargetComplements(wxString & _oldname, wxString & _newname)
{
/// _oldname NOT USED ??
	m_dirProject = m_pProject->GetBasePath();
	wxFileName::SetCwd (m_dirProject);
Mes = _T("m_dirproject = ") + Quote + m_dirProject + Quote;
Mes += _T(", m_dirPreBuild = ") + Quote + m_dirPreBuild + Quote;
printWarn(Mes);
	bool ok = wxFileName::DirExists(m_dirPreBuild);
	if (ok)
	{
        wxFileName filesystem;
		filesystem.AssignDir(m_dirPreBuild);
	// create a new directory
		ok = filesystem.Mkdir(_newname);
	// copy old directory
		if (ok)
		{
			//
		}

	}
	return ok;
}
*/
///-----------------------------------------------------------------------------
/// Detects already existing complement files in the project and
///	populates complement tables with existing files
///
/// Called by :
///		1. QtPregen::OnActivateProject(CodeBlocksEvent& _event):1,
///		2. QtPregen::OnRenameProjectOrTarget(CodeBlocksEvent& _event):2,
///		3. QtPregen::OnAddComplements(CodeBlocksEvent& _event):1,
///
/// Calls to :
///		1. refreshTables():1,

bool qtPre::detectComplementsOnDisk(cbProject * _pProject, const wxString & _nametarget,  bool _report)
{
/// DEBUG
//* **********************************
//	beginDuration(_T("qtPre::detectComplementsOnDisk(...)"));
//* *********************************
	if (!_nametarget.IsEmpty())
		m_nameActiveTarget = _nametarget;
	m_dirProject = _pProject->GetBasePath();
	wxFileName::SetCwd (m_dirProject);
	wxString diradding = m_dirPreBuild + m_nameActiveTarget ;
//Mes = _T("diradding  = ") + Quote + diradding  + Quote;
//printWarn(Mes);
	bool ok = wxDirExists(diradding);
	if (ok)
	{
		m_Filewascreated.Clear();
	// register all complement files in 'm_Filewascreated'
		size_t n = wxDir::GetAllFiles(m_dirPreBuild, &m_Filewascreated );
	// complement files in target
		wxArrayString  filewascreated;
		n = wxDir::GetAllFiles(diradding, &filewascreated );
		Mes = Tab + _(", with") + Space;
		if (_report)
		{
			if (n)
			{
				Mes += strInt(n) ;
				Mes += Space + _("complement files already created on disk in") ;
				Mes += Space + Quote + diradding + Quote;
			}
			else
			{
				Mes += _("no complement file to disk.") ;
			}
			print(Mes);
		}
		Mes = wxEmptyString;
		ok = ! m_Filewascreated.IsEmpty();
	// rebuild all tables
		if (ok)
		{
			refreshTables();
		}
	// error complements ?
		else
		{
			// advice ?
		//	Mes =  _("Complements detected error !!");
		//	printError(Mes) ;
		}
	}

/// DEBUG
//* **********************************
//	endDuration(_T("qtPre::detectComplementsOnDisk(...)"));
//* *********************************

	return ok;
}

///-----------------------------------------------------------------------------
///	Refresh all the tables
///
///	Called by :
///		-# detectComplementsOnDisk(cbProject * _pProject, const wxString & çnametarget,  bool çreport):1,
///	Calls to :
///		1. copyArray (const wxArrayString& _strrarray, int _nlcopy = 0):2,
///		2. wasCreatedToCreator(const wxArrayString& _strarray):1,
///
bool qtPre::refreshTables(bool _debug)
{
//Mes = _T("qtPre::refreshTables(...)");
//print(Mes);
/// DEBUG
//* **********************************
//	beginDuration(_T("qtPre::refreshTables(...)"));
//* *********************************
	bool ok = true;

	m_Registered.Clear();
	m_Registered = copyArray(m_Filewascreated ) ;
///
	m_Createdfile.Clear();
	m_Createdfile = copyArray(m_Filewascreated ) ;
///
	m_Filecreator.Clear();
	m_Filecreator = wasCreatedToCreator() ;
///
	m_Filestocreate.Clear();
	for (wxString item :  m_Filewascreated)
	{
		m_Filestocreate.Add(m_Devoid);
	}
	if (_debug)
	{
		// DEBUG
		Mes = allStrTable(_T("m_Filewascreated"), m_Filewascreated);
		print(Mes);

		Mes = allStrTable(_T("m_Registered"), m_Registered);
		print(Mes);

		Mes = allStrTable(_T("m_Createdfile"), m_Createdfile);
		print(Mes);

		Mes = allStrTable(_T("m_Filecreator"), m_Filecreator);
		print(Mes);

		Mes = allStrTable(_T("m_Filestocreate"), m_Filestocreate);
		print(Mes);
	}

/// DEBUG
//* **********************************
//	endDuration(_T("qtPre::refreshTables(...)"));
//* *********************************
	return ok;
}

///-----------------------------------------------------------------------------
/// Copy a 'wxArrayString' to an another
///
/// Called by  :
///		-# qtPrebuild::filesTocreate(bool _allrebuild):1,
///		-# qtPrebuild::createFiles():4,
///		-# refreshTables(bool debug):2,
///
wxArrayString qtPre::copyArray (const wxArrayString& _strarray, int _nlcopy)
{
	wxArrayString tmp ;
	int nl = _strarray.GetCount()  ;
	if (!nl)
		return  tmp ;
    // adjust number of lines to copy
	if (_nlcopy > 0 && _nlcopy <= nl)
		nl = _nlcopy;
	// a line
	wxString line;
	for (int u = 0; u < nl; u++)
	{
	// read strarray line
		line = _strarray.Item(u) ;
	// write line to tmp
		tmp.Add(line, 1) ;
	}

	return tmp ;
}

///-----------------------------------------------------------------------------
/// Give a string from a 'wxArrayString' for debug
///
/// Called by  :  for debug
///
wxString qtPre::allStrTable(const wxString& _title, const wxArrayString& _strarray)
{
//Mes = _T("qtPre::allStrTable ...");
//printError(Mes);
	wxString mes = _T("--------- debug --------------");
	mes += Lf + Quote + m_dirProject + Quote ;
	mes += Lf +_T("=> ") + Quote + _title + Quote ;
	uint16_t nl = _strarray.GetCount();
	mes += _T(" : ") + strInt(nl) + Space + _("files") ;
	for (uint16_t u = 0; u < nl; u++)
	{
	// read strarray line from  '1' ... 'nl'
		mes += Lf + strInt(u+1) + _T("- ") + _strarray.Item(u) ;
	}
	mes +=  Lf + _T("---------end  debug ------------") ;

	return mes;
}

///-----------------------------------------------------------------------------
/// For create 'm_FilesCreator' from 'm_Filewascreated'
/// reverse a 'wxArrayString' to an another
///
/// Called by  :
///		1. refreshTables():1,
///	Calls to :
///		1. toFileCreator(const wxString& _fcreated):n,
///
wxArrayString qtPre::wasCreatedToCreator()
{
// read file list to 'm_Filewascreated' with (moc_....cpp, ui_....h, qrc_....cpp, __Devoid__)
// write file list to 'tmp' with  (*.h, *.cpp, *.qrc, *.ui)
	wxArrayString tmp;
	wxString fullnameCreator;
	for (wxString fcreated: m_Filewascreated)
	{
		fullnameCreator = toFileCreator(fcreated.AfterLast(Slash))  ;
	//add to array
		tmp.Add(fullnameCreator);
	}

	return tmp;
}

///-----------------------------------------------------------------------------
/// For create  'creator files' from  'created files'
/// reverse a (string' to an another
///
/// Called by  :
///		1. wasCreatedToCreator():1,
///	Calls to :
///		1. fullFileCreator(const wxString& _fcreator):1,
///
wxString qtPre::toFileCreator(const wxString& _fcreated)
{
	wxString fcreator , prepend, ext, name, fullname;
//Mes = _T(" fcreated = ") + Quote + fcreated + Quote ;
//printWarn(Mes);
	prepend = _fcreated.BeforeFirst('_') ;
	name = _fcreated.AfterFirst('_') ;
	name = name.BeforeFirst('.');
	ext = _fcreated.AfterLast('.');
//1- file 'ui_uuu.h'  (forms)  -> 'uuu.ui'
	if ( prepend.Matches(m_UI) && ext.Matches(EXT_H))
		fcreator =  name + DOT_EXT_UI ;
	else
//2- file 'qrc_rrr.cpp'  (resource) -> 'rrr.qrc'
	if (prepend.Matches(m_Qrc) )
		fcreator = name + DOT_EXT_QRC  ;
	else
//4- file 'moc_mmm.cpp'  -> 'mmm.h'
	if (prepend.Matches(m_Moc) )
		fcreator =  name + DOT_EXT_H ;
//5- file 'xxx.moc'  -> 'xxx.cpp'
	if (ext.Matches(m_Moc) )
		fcreator =  name + DOT_EXT_CPP ;
//6- full name
	fullname = fullFileCreator(fcreator);
//Mes = _T(" fcreator = ") + Quote + fcreator + Quote ;
//Mes += _T(" ->  fullname = ") + Quote + fullname + Quote ;
//printWarn(Mes);

	return fullname;
}

///-----------------------------------------------------------------------------
/// Gives a relative to common top level path of 'fcreator';
///		example 'parserfile.h' => 'src/parser/parserfile.h'
///
/// Called by  :
///		1. toFileCreator(const wxString& _fcreated):1,
///

wxString qtPre::fullFileCreator(const wxString&  _fcreator)
{
	wxString fullname = wxEmptyString ;
	if (!m_pProject || !m_pBuildTarget)
		return fullname;
//Mes = Tab + _T(" ==> search fcreator = ") + Quote + fcreator + Quote ;
// all files project
	uint16_t nfproject = m_pProject->GetFilesCount();
	if (!nfproject)
		return fullname;

	ProjectFile * prjfile ;
	ProjectBuildTarget* pBuildTarget;
	wxArrayString tabtargets ;
	bool good = false;
	for (uint16_t  nf = 0; nf < nfproject ; nf++)
	{
		prjfile = m_pProject->GetFile(nf);
        if (!prjfile)
			continue;
        // all real build targets
		tabtargets = prjfile->GetBuildTargets() ;
		for ( wxString nametarget : tabtargets)
		{
			if(nametarget.Matches(m_nameActiveTarget) )
			{
				pBuildTarget = m_pProject->GetBuildTarget(nametarget);
				if (pBuildTarget->GetTargetType() == ttCommandsOnly)
				   continue;
//wxString mes = _("nametarget = ") + Quote + nametarget + Quote;
				fullname  = prjfile->relativeToCommonTopLevelPath ;
				if (fullname.IsEmpty() || fullname.StartsWith(m_dirPreBuild) )
					continue;
//mes += _(" => fullname = ") + Quote + fullname+ Quote;
//printWarn(mes);
				good = fullname.Contains(_fcreator);
				if (good)
					break;
			}
		}
		if (good)
			break;
	}
	if (!good)
	{
		Mes = _T("The") + Space + Quote + _fcreator + Quote ;
		Mes += Space + _T("file is missing in the CB project !");
		printError(Mes);
		fullname = wxEmptyString ;
	}

	return fullname;
}

///-----------------------------------------------------------------------------
