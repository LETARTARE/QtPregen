/*************************************************************
 * Name:      QtPregen.cpp
 * Purpose:   Code::Blocks plugin  'qtPregenForCB.cbp'   0.2.4
 * Author:    LETARTARE
 * Created:   2015-02-15
 * Copyright: LETARTARE
 * License:   GPL
 *************************************************************
 */

#include <sdk.h> 	// Code::Blocks SDK
#include <compilergcc/compilergcc.h>
#include "QtPregen.h"
#include "qtprebuild.h"
#include "print.h"
//------------------------------------------------------------------------------
// extern int idMenuCompileFromProjectManager ;
//------------------------------------------------------------------------------
// Register the plugin with Code::Blocks.
// We are using an anonymous namespace so we don't litter the global one.
namespace
{
    PluginRegistrant<QtPregen> reg(_T("QtPregen"));
}
///-----------------------------------------------------------------------------
///	Load ressource QtPregen.zip
///
QtPregen::QtPregen()
	: m_project(nullptr), m_prebuild(nullptr),
	m_buildAllOk(false) , m_cleanAllOk(false),
	m_IdBuild(0), m_IdCompile(0), m_IdRun(0), m_IdBuildRun(0), m_IdRebuild(0), m_IdClean(0),
	m_IdpBuild(0), m_IdpRebuild(0), m_IdpClean(0),
	m_IdfBuild(0), m_IdfClean(0)
{
	if(!Manager::LoadResource(_T("QtPregen.zip")))
		NotifyMissingFile(_T("QtPregen.zip"));
}
///-----------------------------------------------------------------------------
/// Create handlers event and Creating the pre-builder
///
void QtPregen::OnAttach()
{
// handlers event
	//1- handle project activate
	cbEventFunctor<QtPregen, CodeBlocksEvent>* functorActivate =
		new cbEventFunctor<QtPregen, CodeBlocksEvent>(this, &QtPregen::OnActivate);
	Manager::Get()->RegisterEventSink(cbEVT_PROJECT_ACTIVATE, functorActivate);
	//2- handle build start
	cbEventFunctor<QtPregen, CodeBlocksEvent>* functorBuild =
		new cbEventFunctor<QtPregen, CodeBlocksEvent>(this, &QtPregen::OnPrebuild);
	Manager::Get()->RegisterEventSink(cbEVT_COMPILER_STARTED, functorBuild);

// construct the builder
	// construct new 'm_prebuild'
	m_project = Manager::Get()->GetProjectManager()->GetActiveProject();
	m_prebuild = new qtPrebuild(m_project);
}
///-----------------------------------------------------------------------------
///	Delete the pre-builder and do de-initialization for plugin
///
void QtPregen::OnRelease(bool appShutDown)
{
// delete builder
	if (m_prebuild)  {
		delete m_prebuild;
		m_prebuild = nullptr;
	}
// do de-initialization for your plugin
    // if appShutDown is true, the plugin is unloaded because Code::Blocks is being shut down,
    // which means you must not use any of the SDK Managers
    // NOTE: after this function, the inherited member variable
    // m_IsAttached will be FALSE...
    Manager::Get()->RemoveAllEventSinksFor(this);
}
///-----------------------------------------------------------------------------
///  Retrieves the IDs of the menu items 'Build'
///
void QtPregen::BuildMenu(wxMenuBar* menuBar)
{
	if (!IsAttached())
        return;

	int pos = menuBar->FindMenu(_("Build"));
	if (pos !=-1) {
		wxMenu * builder = menuBar->GetMenu(pos);
		m_IdBuild 		= builder->FindItem(_("Build"));
		m_IdCompile 	= builder->FindItem(_("Compile current file"));
		m_IdRun 		= builder->FindItem(_("Run"));
		m_IdBuildRun	= builder->FindItem(_("Build and run"));
		m_IdRebuild 	= builder->FindItem(_("Rebuild"));
		m_IdClean 		= builder->FindItem(_("Clean"));
	}
}
///-----------------------------------------------------------------------------
///  Retrieves the IDs of the popup menu items
///
void QtPregen::BuildModuleMenu(const ModuleType type, wxMenu* menu, const FileTreeData* data)
{
	if (!IsAttached())
        return;

// we 're only interested in m_project manager's menus
    if (!menu || type != mtProjectManager)
        return;

// right click on item menu ...else -> 0
    if (data) {
    	FileTreeData::FileTreeDataKind typedata  = data->GetKind();
    	// ... click m_project
		bool preproject = typedata == FileTreeData::ftdkProject  ;
		// ... click file
		bool prefile 	= typedata == FileTreeData::ftdkFile ;

		if (preproject)  {
			// popup menu on a m_project
			m_IdpClean  	= menu->FindItem(_("Clean"));
			m_IdpBuild  	= menu->FindItem(_("Build"));
			m_IdpRebuild  = menu->FindItem(_("Rebuild"));
		}
		else
		if (prefile) {
			// popup menu on a file
			m_IdfBuild  	= menu->FindItem(_("Build file"));
			m_IdfClean  	= menu->FindItem(_("Clean file"));
		}

// it's active m_project
		wxString nameprjact = Manager::Get()->GetProjectManager()->GetActiveProject()->GetTitle() ;
		wxString nameprj = data->GetProject()->GetTitle() ;
		if ( nameprj.Matches(nameprjact)  )  {
			if (preproject) {
			// TODO
			// test if qt m_project ??
			}
			else
			if (prefile) {
			// TODO
			// tests if the file requires complements files
			}
		}
    }
}
///-----------------------------------------------------------------------------
/// Activate new m_project
///
/// called by :
///
/// calls to :
///		1. detectQt(prj):1,
void QtPregen::OnActivate(CodeBlocksEvent& event)
{
// missing builder 'm_prebuild'!
	if (!m_prebuild)  return;

// the active project
	cbProject *prj = event.GetProject();
	// no m_project !!
	if(!prj)  {
		Mes = _T("QtPregen -> no project supplied");
		printErr(Mes);
		return;
	}
// detect Qt project
	bool valid = m_prebuild->detectQt(prj);
	if (valid) {
		Mes = Quote + _("");
		printWarn(Mes);
	}
}
///-----------------------------------------------------------------------------
/// Build (or Clean) all complement files for Qt
///
///called by :
///	- menu
///		-# 'Build->Build'
///		-# 'Build->Compile current file'
///		-# 'Build->Run'
///		-# 'Build and Run'
///		-# 'Build->Rebuild'
///		-# 'Build->Clean'
///	- m_project popup
///		1. 'Build'
///		2. 'ReBuild'
///		3. 'Clean'
///	- file popup
///		1. 'Build file'
///		2. 'Clean file'
///
///calls to :
///	- m_prebuild->
///		1. detectQt(prj):1,
///		2. cleanCreated(prj):1,
///
void QtPregen::OnPrebuild(CodeBlocksEvent& event)
{
// missing builder	'm_prebuild' !
	if (!m_prebuild)  return;
// the active m_project
	cbProject *prj = event.GetProject();
	// no m_project !!
	if(!prj)  {
		Mes = _T("QtPregen -> no m_project supplied");
		printErr(Mes);
		return;
	}
// detect Qt m_project
	bool valid = m_prebuild->detectQt(prj);
	// not Qt m_project
	if (! valid) return;

// test patch C::B
	int eventId = event.GetId();
    if (eventId <= 0) {
		Mes = _T("The fix for 'Code::Blocks' are not applied !!");
		Mes += Lf + Quote + m_prebuild->namePlugin() + Quote + _(" CAN NOT CONTINUE !") ;
		cbMessageBox(Mes, _("Used fix for 'Code::Blocks' !!"), wxICON_WARNING ) ;
		return;
	}

// new m_project 	?
	if (prj != m_project) {
		m_buildAllOk = false;
		m_cleanAllOk = false;
	}
/*
Mes =  _T("m_buildAllOk = ") ;
Mes += m_buildAllOk ? _T("true") : _T("false") ;
Mes += Lf ;
Mes += _T("m_cleanAllOk = ") ;
Mes += m_cleanAllOk ? _T("true") : _T("false")   ;
printErr(Mes);
*/

// choice 'clean' or 'compilefile' or 'build'
    bool clean = eventId == m_IdClean || eventId == m_IdpClean || eventId == m_IdfClean;
    bool compilefile = eventId == m_IdCompile || eventId == m_IdfBuild ;
    bool build = eventId == m_IdBuild || eventId == m_IdRebuild ||
				 eventId == m_IdRun || eventId == m_IdBuildRun ||
				 eventId == m_IdpBuild || eventId == m_IdpRebuild ||
				 eventId == m_IdfBuild ;


	wxString mesid ;
	printLn;
///********************************
///1- Delete all files complements
///********************************
	if (clean) {
		//if (m_cleanAllOk) return;

	// begin preClean
		Mes = _T("QtPregen -> begin pre-Clean...");
		printWarn(Mes);
    // preCLean active target
		m_cleanAllOk = m_prebuild->cleanCreated(prj);
		if (!m_cleanAllOk) {
			Mes = _T("Error pre-Clean !!!");
			printErr(Mes);
		}
		m_buildAllOk = false;
	// end preClean
		Mes = _T("QtPregen -> ... end pre-Clean");
		printWarn(Mes);
	}
	else
///********************************
///2- Build one file  // TODO ...
///********************************
	if (compilefile) {
/*
		if (eventId == IdCompile)
			mesid = _T("Build->Compile current file = ") ;
		else
		if (eventId == IdfBuild )
			mesid = _T("F-Build = ") ;
		// display
		printWarn(mesid + wxString()<<eventId);
*/
/*
		cbPlugin * plug =  event.GetPlugin();
		if (plug) {
			// -> 4 = ptCompiler soit cbCompilerplugin
			PluginType type = plug->GetType() ;
			if (type == ptCompiler)  {
				print(_T("plugin type = ") + (wxString()<<type) )  ;
				int id = ((CompilerGCC*)plug)-> GetIdwxEvent();
				print(_T("id = ") + wxString()<<id);
			}
		}
*/
/*
	// editor
		EditorBase* edit = event.GetEditor();
		if (!edit) {
			print(_T("empty editor !"));
			return ;
		}
		wxString file = edit->GetFilename();
		wxString title = edit->GetTitle() ;
		print(_T("file = ") + file);
		print(_T("title = ")+ title);
*/
	}
	else
	if (build) {
///********************************
///3- Build all complement files
///4- Run
///5- Build all complement files and Run
///6- Rebuild all complement files
///********************************
		bool allBuild = eventId == m_IdRebuild || eventId == m_IdpRebuild ;
		//if (! allBuild && m_buildAllOk) return;
	// begin preBuild
		Mes = _T("QtPregen -> begin pre-Build...");
		printWarn(Mes);
    // preBuild
		m_buildAllOk = m_prebuild->buildQt(prj, allBuild);
		if (!m_buildAllOk) {
			Mes = _T("Error pre-Rebuild !!!");
			printErr(Mes);
		}
		m_cleanAllOk = false;
	// end preBuild
		Mes = _T("QtPregen -> ... end pre-Build");
		printWarn(Mes);
	}
    printLn;
    /// memorize last m_project
	m_project = prj;
/*
	Mes =  _T("m_buildAllOk = ") ;
	Mes += m_buildAllOk ? _T("true") : _T("false") ;
	Mes += Lf ;
	Mes +=_T("m_cleanAllOk = ") ;
	Mes += m_cleanAllOk ? _T("true") : _T("false")   ;
	printErr(Mes);
*/
}
///-----------------------------------------------------------------------------
