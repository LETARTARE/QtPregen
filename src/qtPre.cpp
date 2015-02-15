/***************************************************************
 * Name:      qtPre.cpp
 * Purpose:   Code::Blocks plugin	'qtPregenForCB.cbp'   0.2.4
 * Author:    LETARTARE
 * Created:   2015-02-15
 * Copyright: LETARTARE
 * License:   GPL
 **************************************************************/
#include "qtPre.h"

#include <sdk.h>
#include <manager.h>
#include <cbproject.h>
#include <compiletargetbase.h>
#include <projectmanager.h>
#include <macrosmanager.h>
#include <editormanager.h>
#include <cbeditor.h>
#include <wx/datetime.h>
#include <projectfile.h>
// not change !
#include "print.h"
///-----------------------------------------------------------------------------
/// called by :
///	1. qtprebuild::qtprebuild():1,
///
qtPre::qtPre(cbProject * prj )
	: m_Thename(_T("QtPregenForCB_plugin")),
	  m_Win(false), m_Linux(false), m_Mac(false),
	  m_Mexe(_T("")), m_Uexe(_T("")), m_Rexe(_T("")), m_Lexe(_T("")),
	  Mes(_T("")), m_Nameproject(_T("")),
	  m_Mam(Manager::Get()->GetMacrosManager() ),
	  m_Project(prj)
{
	if (m_Project)
		m_Nameproject = m_Project->GetTitle();
	else
		m_Nameproject = wxEmptyString;

#if   defined(__WXMSW__)
	m_Win = true; m_Linux = m_Mac = false;
#elif defined(__WXGTK__)
	m_Linux = true ; m_Win = m_Mac = false;
#elif defined(__WXMAC__)
	m_Mac = true; m_Win = m_Linux = false;
#endif
	platForm();
	//
	m_Moc = _T("moc"); m_Ui = _T("ui"); m_Qrc = _T("qrc"); m_Lm = _T("qm") ;
}
///-----------------------------------------------------------------------------
/// called by
///  1.
///
qtPre::~qtPre()
{
}

///-----------------------------------------------------------------------------
///  called by :
///		QtPregen::OnPrebuild(CodeBlocksEvent& event):1,
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
	if (! m_Win) {
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

	if (m_Win)  {
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
	if (m_Mac)  {
		#undef Eol
		#define Eol Cr
	}
	else
	if (m_Linux) {
		#undef Eol
		#define Eol Lf
	}
}
///-----------------------------------------------------------------------------
///  called by :
/// 		none
///
void qtPre::setVersion(const wxString& ver)
{   // TODO
}
///-----------------------------------------------------------------------------
/// Detection of a 'Qt' m_Project : it contains at least one target Qt
///
/// called by  :
///	1. QtPregen::OnPrebuild(CodeBlocksEvent& event):1,
///
/// calls to :
/// 1. hasLibQt(m_Project):1+,
///
bool qtPre::detectQt(cbProject * prj) {
	m_Project = prj;
	bool ok = m_Project != nullptr;
	if (!ok)
		return ok;

// project name
	m_Nameproject = m_Project->GetTitle() ;
// search project
	ok = hasLibQt(m_Project) ;
	if (! ok) {
		ProjectBuildTarget* buildtarget;
		uint16_t nt = 0 , ntargets = m_Project->GetBuildTargetsCount() ;
		while (nt < ntargets && ! ok ) {
		// retrieve the target libraries  'ProjectBuildTarget* builstarget'
			buildtarget = m_Project->GetBuildTarget(nt++) ;
			if (!buildtarget)
				continue ;
// search target
			ok = hasLibQt(buildtarget) ;
			if (ok)
				break;
		}
	}

	bool valid = ok ;
	if (valid)  {
		wxString  title = m_Nameproject + _T(" uses Qt libraries !") ;
		Mes = _("It will generate (remove) the complements files...") ;
		InfoWindow::Display(title, Mes, 3000);

		bool usemake = prj->IsMakefileCustom() ;
		if(usemake)   {
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
/// Return true if good       'CompileTargetBase* container'
///
/// called by  :
///	1. detectProjectQt():2,
///
/// calls to :
///  none
///
bool qtPre::hasLibQt(CompileTargetBase * container) {
	bool ok = false;
	if (!container)
		return ok;

	wxArrayString tablibs = container->GetLinkLibs() ;
	uint16_t nlib = tablibs.GetCount() ;
	if (nlib > 0 ) {
		wxString namelib ;
		uint16_t u=0;
		int16_t index= -1, pos ;
		while (u < nlib && !ok ) {
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
