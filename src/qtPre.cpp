/***************************************************************
 * Name:      qtPre.cpp
 * Purpose:   Code::Blocks plugin	'qtPregenForCB.cbp'   0.8.3
 * Author:    LETARTARE
 * Created:   2015-02-27
 * Copyright: LETARTARE
 * License:   GPL
 **************************************************************/
#include "qtPre.h"

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
#include <projectfile.h>
// not place change !
#include "print.h"
///-----------------------------------------------------------------------------
/// called by :
///	1. qtprebuild::qtprebuild():1,
///
qtPre::qtPre(cbProject * prj, int logindex)
	: m_Thename(_T("QtPregenForCB_plugin")),
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
///  1.
///
qtPre::~qtPre()
{
}
///-----------------------------------------------------------------------------
/// Give internal name
///
/// called by :
///		QtPregen::OnPrebuild(CodeBlocksEvent& event):1,
///
wxString qtPre::namePlugin()
{
	return m_Thename;
}
///-----------------------------------------------------------------------------
/// Determines the system platform
///
/// called by :
///	1. qtpre():1,
///
void qtPre::platForm() {
// choice platform
	if (! m_Win)
	{
		#undef Sepd
		#define Sepd 13
		#undef Sizesep
		#define Sizesep 1
	// tables
       /*   TODO ????
		m_TablibQt.Add(_T("qtmain"),1) ;
		m_TablibQt.Add(_T("qtmaind"),1) ;
		// qt4   ???
		m_TablibQt.Add(_T("qtgui"),1) ;
		m_TablibQt.Add(_T("qtcore"),1) ;
		m_TablibQt.Add(_T("qtguid"),1) ;
		m_TablibQt.Add(_T("qtcored"),1) ;
		// qt5   ???
		m_TablibQt.Add(_T("qt5gui"),1) ;
		m_TablibQt.Add(_T("qt5core"),1) ;
		m_TablibQt.Add(_T("qt5widgets"),1) ;
		m_TablibQt.Add(_T("qt5guid"),1) ;
		m_TablibQt.Add(_T("qt5cored"),1) ;
		m_TablibQt.Add(_T("qt5widgetsd"),1);
        */
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
	Mes = (wxString()<<major) + _T(".") + (wxString()<<minor) + _T(".") + (wxString()<<release);
	//print(Mes);

    return  Mes ;
}
///-----------------------------------------------------------------------------
///  Set version SDK
///
///  called by :
/// 		none
///
void qtPre::SetVersionSDK(const wxString& ver)
{   // TODO
}
///-----------------------------------------------------------------------------
///  Set index page to log
///
///  called by :
/// 		none
///
void qtPre::SetPageIndex(int logindex)
{
	m_LogPageIndex = logindex;
}

///-----------------------------------------------------------------------------
/// Detection of a 'Qt' Project : it contains at least one target Qt
///
/// called by  :
///	1. QtPregen::OnActivate(CodeBlocksEvent& event):1,
///	2. QtPregen::OnPregen(CodeBlocksEvent& event):1,
///
/// calls to :
/// 1. hasLibQt(Project):1+,
///
bool qtPre::detectQt(cbProject * prj, bool report)
{
	m_project = prj;
	bool ok = m_project != nullptr;
	if (!ok)
		return ok;

// project name
	m_nameproject = m_project->GetTitle() ;
// search to project
	ok = hasLibQt(m_project) ;
	if (! ok)
	{
		ProjectBuildTarget* buildtarget;
		uint16_t nt = 0 , ntargets = m_project->GetBuildTargetsCount() ;
		while (nt < ntargets && ! ok )
		{
		// retrieve the target libraries  'ProjectBuildTarget* builstarget'
			buildtarget = m_project->GetBuildTarget(nt++) ;
			if (!buildtarget)
				continue ;
// search to target
			ok = hasLibQt(buildtarget) ;
			if (ok)
				break;
		}
	}

	bool valid = ok ;
	if (valid)
	{
		if (report)
		{
			wxString  title = m_nameproject + _T(" uses Qt libraries !") ;
			Mes = _("It will generate the complements files...") ;
			InfoWindow::Display(title, Mes, 2000);
		}

		bool usemake = prj->IsMakefileCustom() ;
		if(usemake)
		{
			Mes = _T("... but please, DISABLE using of custom makefile");
			Mes += Lf + Quote + m_Thename + Quote + _T(" not use makefile.");
			print(Mes);
			Mes += Lf + _("CAN NOT CONTINUE !") ;
			cbMessageBox(Mes, _("Used makefile !!"), wxICON_WARNING ) ;
			valid = false;
			Mes = _T("QtPregen -> end ...");
			printWarn(Mes);
			return valid ;
		}
	}

	return valid;
}
///-----------------------------------------------------------------------------
/// Return true if good  'CompileTargetBase* container'
///
/// called by  :
///	1. detectProjectQt():2,
///
/// calls to :
///  none
///
bool qtPre::hasLibQt(CompileTargetBase * container)
{
	bool ok = false;
	if (!container)
		return ok;

	wxArrayString tablibs = container->GetLinkLibs() ;
	uint16_t nlib = tablibs.GetCount() ;
	if (nlib > 0 )
	{
		wxString namelib ;
		uint16_t u=0;
		int16_t index= -1, pos ;
		while (u < nlib && !ok )
		{
			// lower, no extension
			namelib = tablibs.Item(u++).Lower().BeforeFirst('.') ;
			// no prefix "lib"
			pos = namelib.Find(_T("lib")) ;
			if (pos == 0)
				namelib.Remove(0, 3) ;
			// begin == "qt"
			pos = namelib.Find(_T("qt")) ;
			if (pos != 0)
				continue ;
			// compare
			index = m_TablibQt.Index(namelib);
			ok = index != -1 ;
			// first finded
			if (ok)
				break;
		}
	}

	return ok;
}
///-----------------------------------------------------------------------------
/// For replace 'Targetsfile = prjfile.buildTargets'
/// copy a 'wxArrayString' to an another
///
/// called by  :
///	1. findwasCreated():1,
///	2. filesTocreate(bool):1,
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
