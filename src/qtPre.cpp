/***************************************************************
 * Name:      qtpre.cpp
 * Purpose:   Code::Blocks plugin
 * Author:    LETARTARE
 * Created:   2015-02-27
 * Modified:  2019-11-10
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
	Mes = wxEmptyString;
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
// target name
	Mes = _T("--------------> ") ;
	Mes += _("PreBuild");
	Mes += _T(" :") ;
	Mes += quote( m_nameActiveTarget );
	printWarn(Mes);
// date
	Mes = Lf + _T("==> ");
	Mes += _("Start of") + quote(_namefunction) ;
	Mes += _T(": ") + date();
	printWarn(Mes);
	m_start = clock();
	Mes.Clear();
}

///-----------------------------------------------------------------------------
///  End duration for Debug
///
void qtPre::endDuration(const wxString & _namefunction)
{
	Mes = _T("<== ") ;
	Mes += _("End of") + quote( _namefunction  ) ;
	Mes += _T(" : ") + date();
	Mes += _T(" -> ") + duration();
	//printError(Mes);
	printWarn(Mes);
	Mes.Clear();
}
///-----------------------------------------------------------------------------
/// Give internal plugin name
///
/// Called by : none
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
		m_TablibQt.Add(_T("qt5opengl"),1) ;
		m_TablibQt.Add(_T("qt5printsupport"),1);
		m_TablibQt.Add(_T("qt5serialport"),1) ;
		m_TablibQt.Add(_T("qt5xml"),1) ;
		// qt5
		m_TablibQt.Add(_T("qt5guid"),1) ;
		m_TablibQt.Add(_T("qt5cored"),1) ;
		m_TablibQt.Add(_T("qt5widgetsd"),1);
		m_TablibQt.Add(_T("qt5opengld"),1) ;
		m_TablibQt.Add(_T("qt5printsupportd"),1);
		m_TablibQt.Add(_T("qt5serialportd"),1) ;
		m_TablibQt.Add(_T("qt5xmld"),1) ;
	}

	if (m_Win)
	{
Mes = _("Platform is 'Win'");

		#undef Eol
		#define Eol CrLf
	// tables
		m_TablibQt.Add(_T("qtmain"),1) ;
		m_TablibQt.Add(_T("qtmaind"),1) ;
		// qt5
		m_TablibQt.Add(_T("qt5gui"),1) ;
		m_TablibQt.Add(_T("qt5core"),1) ;
		m_TablibQt.Add(_T("qt5widgets"),1) ;
		m_TablibQt.Add(_T("qt5opengl"),1) ;
		m_TablibQt.Add(_T("qt5printsupport"),1);
		m_TablibQt.Add(_T("qt5serialport"),1) ;
		m_TablibQt.Add(_T("qt5xml"),1) ;
			// <= qt5.9
		m_TablibQt.Add(_T("qt5guid"),1) ;
		m_TablibQt.Add(_T("qt5cored"),1) ;
		m_TablibQt.Add(_T("qt5widgetsd"),1);
		m_TablibQt.Add(_T("qt5opengld"),1) ;
		m_TablibQt.Add(_T("qt5printsupportd"),1);
		m_TablibQt.Add(_T("qt5serialportd"),1) ;
		m_TablibQt.Add(_T("qt5xmld"),1) ;
	}
	else
	if (m_Mac)
	{
Mes = _("Platform is 'Mac'");
		#undef Eol
		#define Eol Cr
	}
	else
	if (m_Linux)
	{
Mes = _("Platform is 'Linux'");
		#undef Eol
		#define Eol Lf
	}

print(Mes);

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

    return  strInt(major) + _T(".") + strInt(minor) + _T(".") + strInt(release);
}

///-----------------------------------------------------------------------------
/// test clean
///
///  Called by :none
///

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
///
void qtPre::setProject(cbProject * _pProject)
{
	bool good = _pProject != nullptr ;
	if (good)
	{
		m_pProject = _pProject;
		m_nameProject = m_pProject->GetTitle();

		m_nameActiveTarget = m_pProject->GetActiveBuildTarget();
		ProjectBuildTarget * pBuildTarget = m_pProject->GetBuildTarget(m_nameActiveTarget);
		if (pBuildTarget)   setBuildTarget(pBuildTarget);
		/// active target is virtual !!
		else
		//...
			;
	}
	else
		printError(_T(" Error in 'qtPre::setProject(_pProject == null)'") );
}

///-----------------------------------------------------------------------------
/// Set 'm_pBuildTarget'
///
/// Called by :
///		1. qtPre::setProject(cbProject * _pProject):1,
///
void qtPre::setBuildTarget(ProjectBuildTarget * _pBuildTarget)
{
	if (_pBuildTarget)
	{
		m_pBuildTarget = _pBuildTarget;
	/// NO for a virtaual target
		m_dirObjects =  m_pBuildTarget->GetObjectOutput();
        m_pMam->ReplaceMacros(m_dirObjects);
        if (!m_dirObjects.Last() != Slash )
				m_dirObjects += Slash;
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
//Mes = _T("isComplementFile ? -> ") + _file ;
//printWarn(Mes);
//print(allStrTable(_T("m_Registered"), m_Registered));

// target name
	m_nameActiveTarget = m_pProject->GetActiveBuildTarget() ;
// if it is a virtual target, replace it with its first real target
	virtualToFirstRealTarget(m_nameActiveTarget );

	wxString filename = m_dirPreBuild + fullFilename(_file) ;
//Mes = _T(" qtPre::isComplementFile : filename = ") + quote(filename);
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
///		2. qtPrebuild::unregisterComplementFile(const wxString & file, bool _creator, bool _first):1,
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
/// Gives if a target is a command target
///
///	Called by :
///
///		1. qtPre::isCommandTarget(const wxString& _nametarget):1,
///		2. qtPre::compileableProjectTargets():1,
///		3. qtPre::compileableVirtualTargets(const wxString& _virtualtarget):1,
///
bool qtPre::isCommandTarget(const ProjectBuildTarget * _pBuildTarget)
{
	// virtual target ?
	if (!_pBuildTarget)		return false;

	return _pBuildTarget->GetTargetType() == ::ttCommandsOnly;
}
///-----------------------------------------------------------------------------
/// Gives if a target is a command target
///
///	Called by :
///
///  	1. qtPre::detectQtTarget(const wxString& _nametarget, bool _report):1,
///		2. qtPre::fullFileCreator(const wxString&  _fcreator, wxString _creatortarget):1,
///		3. QtPregen::OnRenameProjectOrTarget(CodeBlocksEvent& _event):1,
///
bool qtPre::isCommandTarget(const wxString& _nametarget)
{
	return isCommandTarget(m_pProject->GetBuildTarget(_nametarget) );
}

///-----------------------------------------------------------------------------
/// Search virtual Qt target
///
/// Called by  :
///		1. qtPre::detectQtTarget(const wxString& _nametarget, bool _report):1
///
bool qtPre::isVirtualQtTarget(const wxString& _namevirtualtarget, bool _warning)
{
	bool ok = m_pProject->HasVirtualBuildTarget(_namevirtualtarget);
	if (ok)
	{
		//ProjectBuildTarget * pBuildTarget ;
		wxArrayString realtargets = compileableVirtualTargets(_namevirtualtarget);
		// search the first Qt targets
		for (wxString target : realtargets)
		{
			ok = hasLibQt(m_pProject->GetBuildTarget(target));
			if (ok) break;
		}
		if (ok && _warning)
		{
			Mes = quote(_namevirtualtarget) + _("is a virtual 'Qt' target") + _T(" !!") ;
			print(Mes);
		}
	}

	return ok  ;
}

///-----------------------------------------------------------------------------
/// Search all not compileable target for project
///	or Search all not compileable target for virtual project
///
/// Called by  :
///		1. detectQtProject(cbProject * _pProject, bool _report):1,
///
wxArrayString qtPre::compileableProjectTargets()
{
	wxArrayString compilTargets;
	ProjectBuildTarget * pBuildTarget ;
	uint16_t ntarget = m_pProject->GetBuildTargetsCount();
	while (ntarget)
	{
		ntarget--;
//Mes = _T("indice ntarget : ") + strInt(ntarget); printWarn(Mes);
		pBuildTarget = m_pProject->GetBuildTarget(ntarget);
		// virtual target ?
		if(!pBuildTarget) continue;
		// real command target ?
		if (isCommandTarget(pBuildTarget) ) continue;
		// a compileable target
		compilTargets.Add(pBuildTarget->GetTitle());
//Mes = _T("valid target : ") + strInt(ntarget); printWarn(Mes);
	}

	return compilTargets;
}

///-----------------------------------------------------------------------------
/// Search all not compileable target for project
///	or Search all not compileable target for virtual project
///
/// Called by  :
///		1. qtPre::detectQtProject(cbProject * _pProject, bool _report):1,
///		2. qtPre::detectComplementsOnDisk(cbProject * _pProject, const wxString & _nametarget,  bool _report):1,
///		3. qtPre::detectComplementsOnDisk(cbProject * _pProject, const wxString & _nametarget,  bool _report):1,
///		4. qtPrebuild::buildOneFile(cbProject * _pProject, const wxString& _fcreator):1,
///
wxArrayString qtPre::compileableVirtualTargets(const wxString& _virtualtarget)
{
	wxArrayString compilTargets, realTargets;
	ProjectBuildTarget * pBuildTarget ;
	// search real targets : if is not a virtual target 'compilTargets' is empty
	realTargets = m_pProject->GetExpandedVirtualBuildTargetGroup(_virtualtarget);
	// the table may contain non-compileable targets
	for (wxString target : realTargets)
	{
		// search 'ttCommandOnly' type
		pBuildTarget = m_pProject->GetBuildTarget(target);
		if (!pBuildTarget) 	continue;
		// real command target ?
		if (isCommandTarget(pBuildTarget) ) continue ;

		// good target
		compilTargets.Add(pBuildTarget->GetTitle());
	}

	return compilTargets;
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
///		1. hasLibQt(CompileTargetBase * _pContainer):1,
///		// 2. qtPre::detectQtTarget(const wxString& _nametarget, bool _report)
///
bool qtPre::detectQtProject(cbProject * _pProject, bool _report)
{
//Mes = _T("qtPre::detectQtProject (...)"); print(Mes);
	if (! _pProject)	return false;

// libraries Qt in project and targets
	m_pProject = _pProject;
	bool isQtProject =  hasLibQt(m_pProject);
//Mes = _T("isQtProject = ") + strBool(isQtProject); printWarn(Mes);
	// search in compileable targets
	if (!isQtProject)
	{
		wxArrayString compiletable = compileableProjectTargets();
//Mes = allStrTable(_T("compiltable"), compiletable); printWarn(Mes);
		for( wxString target: compileableProjectTargets())
		{
			isQtProject = hasLibQt(m_pProject->GetBuildTarget(target));
			if (isQtProject)
				break;
		}
	}

	if (isQtProject)
	{
		if (_report)
		{
			wxString nametarget = m_pProject->GetTitle() ;
			wxString title = _("The project") +  quote(m_nameProject) + _("uses Qt libraries !") ;
			wxString txt =  quote( m_namePlugin ) + _("will generate the complements files") + _T("...") ;
			// also to Code::Blocks log
			InfoWindow::Display(title, txt, 5000);
		}
		// usemake ?
		if( m_pProject->IsMakefileCustom())
		{
			Mes = _T("... ") ;
			Mes += _("but please, DISABLE using of custom makefile");
			Mes += Lf + quote(m_namePlugin) + _("not use makefile.");
			print(Mes);
			Mes += Lf + _("CAN NOT CONTINUE") ;
			Mes += _T(" !") ;
			wxString title = _("Used makefile");
			title += _T(" !!");
			cbMessageBox(Mes, title, wxICON_WARNING ) ;

			Mes = m_namePlugin + _T(" -> ") + _("end") + _T(" ...");
			printWarn(Mes);
			isQtProject = false;
		}
	}

	return isQtProject;
}

///-----------------------------------------------------------------------------
/// If virtual target give the real fist target
///
/// Called by  :
///		1. qtPrebuild::findGoodfiles():1,
///		2. qtPre::isComplementFile(const wxString & _file):1,
///
bool qtPre::virtualToFirstRealTarget(wxString& _virtualtarget, bool _warning)
{
	// verify if it'a a virtual target
	if (!m_pProject->HasVirtualBuildTarget(_virtualtarget)) return false;
	// the first real target not command target
	_virtualtarget = compileableVirtualTargets(_virtualtarget).Item(0);
	if (_warning) {
			Mes = quote( _virtualtarget );
			Mes += _("it is a virtual target");
			Mes += _T(", ");
			Mes += _("that is replaced by his first real target") ;
			Mes += quote( _virtualtarget);
			printWarn(Mes);
	}
//Mes = _T(" _virtualtarget =  ") + quote(_virtualtarget);
//print(Mes);

	return !_virtualtarget.IsEmpty();
}

///-----------------------------------------------------------------------------
/// Detection of a 'Qt' target
///
/// Called by  :
///		1. qtPre::detectComplementsOnDisk(cbProject * _pProject, const wxString & _nametarget,  bool _report):1,
///		2. QtPregen::OnActivateTarget(CodeBlocksEvent& _event):1,
///		3. QtPregen::OnRenameProjectOrTarget(CodeBlocksEvent& _event):1,
///		4. QtPregen::OnNewProject(CodeBlocksEvent& _event):1,
///
/// Calls to :
///		1. qtPre::hasLibQt(CompileTargetBase * _pContainer):1,
///
bool qtPre::detectQtTarget(const wxString& _nametarget, bool _report)
{
	bool ok = false ;
	if(!m_pProject)  return ok;
	if (isCommandTarget(_nametarget)) 	return ok;

//Mes = _T("qtpre::detectQtTarget -> ") + quote( _nametarget ) ;
//printWarn(Mes);
	// is one virtual target
	bool virtQt = isVirtualQtTarget(_nametarget);
	wxArrayString compilTargets;	//
	if (virtQt)
	{
		compilTargets =  compileableVirtualTargets(_nametarget);
		if (_report)
		{
			Mes = Tab + quote(_T("::") + _nametarget);
			Mes += _("is a virtual 'Qt' target that drives") ;
			Mes += _T(" ..."); printWarn(Mes);
		}
	}
	else
		compilTargets.Add(_nametarget) ;

/// all compileable targets
	bool goodQt = false;
	for (wxString target : compilTargets)
	{
//Mes = _T("compare : ") + quote( target) +  _T(", ") + quote( _nametarget);
//print(Mes);
		ok = hasLibQt(m_pProject->GetBuildTarget(target));
//Mes = strBool(ok) ; printError(Mes);
		if (_report)
		{
			if (virtQt) 	Mes = Tab ;
			else			Mes = wxEmptyString;
		//Mes += _T("qtpre::detectQtTarget ->");
			Mes += Tab + quote( _T("::") + target);
		// advices
			Mes += Tab + Tab + _("is") + Space;
			if(!ok)		Mes += _("NOT") + Space;
			Mes += _("a Qt target.");
			print(Mes);
		}
		goodQt = goodQt || ok ;
	}
//Mes = strBool(goodQt) ; printError(Mes);

	return goodQt;
}

///-----------------------------------------------------------------------------
/// Return true if good  'CompileTargetBase* container'
///
/// Called by  :
///		1. qtPre::detectQtProject(cbProject * _pProject, bool _report = false):1,
///		2. qtPre::detectQtTarget(const wxString& _nametarget, bool _report):1,
///		2. findLibQtToTargets():1,
///
bool qtPre::hasLibQt(CompileTargetBase * _pContainer)
{
//Mes = _T("into 'qtPre::hasLibQt(...)'") ; printWarn(Mes);
	bool ok = false;
	if (!_pContainer) 	return ok;

	wxArrayString tablibs = _pContainer->GetLinkLibs() ;
	uint16_t nlib = tablibs.GetCount() ;
//Mes = _T("nlib = ") + strInt(nlib); printWarn(Mes);
	if (nlib > 0 )
	{
		wxString namelib ;
		uint16_t u=0;
		int16_t index= -1, pos ;
		while (u < nlib && !ok )
		{
			// lower, no extension
			namelib = tablibs.Item(u++).Lower().BeforeFirst('.') ;
//Mes = strInt(u) + _T(" -> ") + quote( namelib ); printWarn(Mes);
			// no prefix "lib"
			pos = namelib.Find(_T("lib")) ;
			if (pos == 0)
				namelib.Remove(0, 3) ;
			// begin == "qt"
			pos = namelib.Find(_T("qt")) ;
//Mes = _T("pos 'qt' = ") + strInt(pos); printWarn(Mes);
			if (pos != 0) 		continue ;
			// find
/// ATTENTION : the table should then contain all Qt libraries !!
			index = m_TablibQt.Index(namelib);
//Mes = _T("index = ") + strInt(index); printWarn(Mes);
			ok = index != -1 ;
			// first finded
			if (ok)
				break;
		}
	}
//Mes = _T("haslibQt(...) = ") + strBool(ok); printWarn(Mes);

	return ok;
}

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
///		1. qtPre::refreshTables():1,

bool qtPre::detectComplementsOnDisk(cbProject * _pProject, const wxString & _nametarget,  bool _report)
{
/// DEBUG
//* *********************************************************
//	beginDuration(_T("qtPre::detectComplementsOnDisk(...)"));
//* *********************************************************
	m_dirProject = _pProject->GetBasePath();
	wxFileName::SetCwd (m_dirProject);

	bool ok = wxDirExists(m_dirPreBuild);
//Mes = _T("m_dirPreBuild = ") + quote( m_dirPreBuild ) + _T(",  ok = ") +  strBool(ok);
//print(Mes);
	// it's an old project already compiled once
	if (ok)
	{
		wxArrayString filesdisk; wxArrayString compilTargets;
		wxString nametarget; bool strangers = false;
	// deregister all old complement
		m_Filewascreated.Clear();
	// register all complement files on the disk in 'filesdisk'
		size_t n = wxDir::GetAllFiles(m_dirPreBuild, &filesdisk);
		if (!n) return false;

	// read directory names = target name of project
		for (wxString filepath : filesdisk)
		{
//Mes = quote(filepath); print(Mes);
			// extract target name
			nametarget = filepath.AfterFirst(Slash).BeforeFirst(Slash);
//Mes = quote(nametarget); print(Mes);
			// is a target of project ?
			if (_pProject->GetBuildTarget(nametarget) )
			{
				m_Filewascreated.Add(filepath) ;
			}

			else	strangers = true;
		}
		ok = m_Filewascreated.GetCount() > 0;
//Mes = _T(" n = ") + strInt(n) ; print(Mes);
		if (ok)
		{
		// initializes all other tables
			refreshTables();
			// for debug
			//refreshTables(WITH_DEBUG);
		// we have to do it for compileable targets Qt
			bool virt = m_pProject->HasVirtualBuildTarget(_nametarget);
			if (virt)
				compilTargets = compileableVirtualTargets(_nametarget);
			else
				compilTargets.Add(_nametarget);

			bool isQtTarget;
			wxString diradding; wxArrayString  filewascreated;
			for (wxString target : compilTargets)
			{
				isQtTarget = detectQtTarget(target, NO_REPORT) ;
	//	Mes = _T("target = ") + quote(target) + _T(", isQtTarget:") + strBool(isQtTarget);
	//	print(Mes);
				if (!isQtTarget) 	continue;

				diradding = m_dirPreBuild + target ;
	//	Mes = _T("diradding  = ") + quote( diradding  );
	//	printWarn(Mes);
				ok = wxDirExists(diradding);
				if (ok)
				{
					n = wxDir::GetAllFiles(diradding, &filewascreated );
					if (_report)
					{
						if (virt)	Mes = Tab;
						else 		Mes = wxEmptyString;
						Mes +=  Tab + quoteNS(_T("::") + target);
						// tabulation to SizeLe (=16)
						size_t le = Mes.Len();
						if (le <= SizeLe)	Mes.Append(' ', SizeLe-le);
						// advices
						Mes += Tab +  _("is") + Space;
						if(!ok)		Mes += _("NOT") + Space;
						Mes += _("a Qt target") ;
						Mes += _T(", ");
						Mes +=  _("with") + Space;
						if (n)
						{
							Mes += _("has") + Space + strInt(n) + Space ;
							Mes += _("complement file(s) already created on disk in") ;
							Mes += quote( diradding );
						}
						else
						{
							Mes += _("no complement file to disk.") ;
						}
						print(Mes);
					}
				}
				else
				if (_report)
				{
					Mes =  _("The directory") + quote(diradding);
					Mes += _(" no exists") ;
					Mes += _T(".") + Space ;
					Mes += _("You need to 'Rebuild' the target") ;
					Mes += _T(" !!"); printWarn(Mes) ;
				}
			} // end for
		}
		else{
			Mes =  Tab+ _T("=> ") + _("The directory") + quote(m_dirPreBuild);
			Mes += _("is empty") ;
			Mes += _T(".") + Space ;
			Mes += _("You need to 'Rebuild' the project") ;
			Mes += _T(" !!");
			printWarn(Mes) ;
		}
		if (strangers)
		{
			Mes = _T("	==>");
			Mes += quote( _pProject->GetTitle()) + _("has stranger complement files in");
			Mes += quote(m_dirPreBuild);
			Mes += _T("==> ") ;
			Mes += _("you can delete it from the disk");
			printError(Mes);
		}
	}
	else
	// it's a new project or people has deleted 'm_dirPreBuild' !
	if (_report)
	{
		//Mes = _T("qtPre::detectComplementsOnDisk -> ");
		Mes =  Tab+ _T("=> ") + _("The directory of complements does not exist") ;
		Mes += _T(" ! ,");
		Mes += _("you need to 'Rebuild' the project") ;
		Mes += _T(" !!");
		printWarn(Mes);
	}

/// DEBUG
//* *******************************************************
//	endDuration(_T("qtPre::detectComplementsOnDisk(...)"));
//* *******************************************************
	Mes.Clear();

	return ok;
}

///-----------------------------------------------------------------------------
///	Refresh all the tables
///
///	Called by :
///		1. detectComplementsOnDisk(cbProject * _pProject, const wxString & çnametarget,  bool çreport):1,
///
///	Calls to :
///		1. copyArray (const wxArrayString& _strrarray, int _nlcopy = 0):2,
///		2. wasCreatedToCreator(const wxArrayString& _strarray):1,
///
bool qtPre::refreshTables(bool _debug)
{
/// DEBUG
//* ***********************************************
//	beginDuration(_T("qtPre::refreshTables(...)"));
//* ***********************************************
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
//* *********************************************
//	endDuration(_T("qtPre::refreshTables(...)"));
//* *********************************************
	Mes.Clear();

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
	mes += Lf + quote( m_dirProject ) ;
	mes += Lf +_T("=>") + quote( _title ) ;
	uint16_t nl = _strarray.GetCount();
	mes += _T(" : ") + strInt(nl) + Space + _("files") ;
	for (uint16_t u = 0; u < nl; u++)
	{
	// read strarray line from  '1' ... 'nl'
		mes += Lf + strInt(u+1) + _T("- ") + _strarray.Item(u) ;
	}
	mes +=  Lf + _T("--------- end  debug ------------") ;

	return mes;
}

///-----------------------------------------------------------------------------
/// For create 'm_FilesCreator' from 'm_Filewascreated'
/// reverse a 'wxArrayString' to an another
///
/// Called by  :
///		1. qtPre::refreshTables():1,
///
///	Calls to :
///		1. qtPre::toFileCreator(const wxString& _fcreated):n,
///
wxArrayString qtPre::wasCreatedToCreator()
{
// read file list to 'm_Filewascreated' with (moc_....cpp, ui_....h, qrc_....cpp, __Devoid__)
// write file list to 'tmp' with  (*.h, *.cpp, *.qrc, *.ui)
	wxArrayString tmp;
	wxString fullnameCreator;
	for (wxString fcreated: m_Filewascreated)
	{
		fullnameCreator = toFileCreator(fcreated)  ;
//Mes = _T("	fcreated = ") + quote(fcreated) + _T(" => ") +  quote(fullnameCreator);
//printWarn(Mes);
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
///		1. qtPre::wasCreatedToCreator():1,
///
///	Calls to :
///		1. qtPre::fullFileCreator(const wxString& _fcreator):1,
///
wxString qtPre::toFileCreator(const wxString &_fcreated)
{
	wxString target, fcreated, fcreator , prepend, ext, name, fullname;
//Mes = _T(" ::toFileCreator :: _fcreated = ") + quote(_fcreated ) ; printWarn(Mes);
	target = _fcreated.AfterFirst(Slash).BeforeFirst(Slash);
//Mes = _T(" target = ") + quote(target) ; printWarn(Mes);
	fcreated = _fcreated.AfterLast(Slash);
//Mes = _T(" fcreated = ") + quote(fcreated ) ; printWarn(Mes);
	if (fcreated.Contains(_T("_")) )
	{
		prepend = fcreated.BeforeFirst('_') ;
		name = fcreated.AfterFirst('_') ;
	}
	/// 'xxx.moc'
	else
	{
		name = fcreated;
	}
//Mes = _T(" 		prepend = ") + quote(prepend) ; printWarn(Mes);
//Mes = _T(" 		name = ") + quote(name) ; printWarn(Mes);
	name = name.BeforeFirst('.');
	ext = fcreated.AfterLast('.');
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
	else
//5- file 'xxx.moc'  -> 'xxx.cpp'
	if (prepend.IsEmpty() && ext.Matches(m_Moc) )
		fcreator =  name + DOT_EXT_CPP ;
//6- full name
	fullname = fullFileCreator(fcreator, target);
//Mes = _T(" fcreator = ") + quote( fcreator ) ;
//Mes += _T(" ->  fullname = ") + quote( fullname ) ;
//printWarn(Mes);

	return fullname;
}

///-----------------------------------------------------------------------------
/// Gives a relative to common top level path of 'fcreator';
///		example 'parserfile.h' => 'src/parser/parserfile.h'
///
/// Called by  :
///		1. qtPre::toFileCreator(const wxString& _fcreated):1,
///
wxString qtPre::fullFileCreator(const wxString&  _fcreator, wxString _creatortarget)
{
	wxString fullname = wxEmptyString ;
	if (!m_pProject)
		return fullname;
//Mes = Tab + _T(" ==> search _fcreator = ") + quote( _fcreator ) ;
//Mes += _T(" of target :") + quote(_creatortarget);
//print(Mes);
	ProjectFile * prjfile ;
	bool good = false;
	// all files of the project
	for (uint16_t  nf = 0; nf < m_pProject->GetFilesCount() ; nf++)
	{
		// get a file of the project
		prjfile = m_pProject->GetFile(nf);
        if (!prjfile) 	continue;
        // search in all the real targets of the file
		if (prjfile->GetBuildTargets().Index(_creatortarget) == wxNOT_FOUND)	continue;
		// is a command target : it's possible ?
		if (isCommandTarget(_creatortarget) )	continue;

		// read the relative file name
		fullname = prjfile->relativeToCommonTopLevelPath ;
		// we found the file ?
		good = fullname.AfterLast(Slash).Matches(_fcreator);
		if (good)
			break;
	}

	if (!good)
	{
		Mes = _T("**") + quote(_creatortarget) + _T(":") + quote( _fcreator ) ;
		Mes += Space + _("file is missing in the CB project") + _T(" !");
		printError(Mes);
		fullname = wxEmptyString ;
	}
//Mes = _(" => fullname = ") + quote( fullname);print(Mes);

	return fullname;
}

///-----------------------------------------------------------------------------
