/*************************************************************
 * Name:      QtPregen.cpp
 * Purpose:   Code::Blocks plugin  'qtPregenForCB.cbp'   0.2.2
 * Author:    LETARTARE
 * Created:   2015-02-10
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
	: project(nullptr), prebuild(nullptr),
	buildAllOk(false) , cleanAllOk(false),
	IdBuild(0), IdCompile(0), IdRun(0), IdBuildRun(0), IdRebuild(0), IdClean(0),
	IdpBuild(0), IdpRebuild(0), IdpClean(0),
	IdfBuild(0), IdfClean(0)
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
	// handle build start
	cbEventFunctor<QtPregen, CodeBlocksEvent>* functorBuild =
		new cbEventFunctor<QtPregen, CodeBlocksEvent>(this, &QtPregen::OnPrebuild);
	Manager::Get()->RegisterEventSink(cbEVT_COMPILER_STARTED, functorBuild);

// construct the builder
	// construct new 'prebuild'
	project = Manager::Get()->GetProjectManager()->GetActiveProject();
	prebuild = new qtPrebuild(project);
}
///-----------------------------------------------------------------------------
///	Delete the pre-builder and do de-initialization for plugin
///
void QtPregen::OnRelease(bool appShutDown)
{
// delete builder
	if (prebuild)  {
		delete prebuild;
		prebuild = nullptr;
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
		IdBuild 	= builder->FindItem(_("Build"));
		IdCompile 	= builder->FindItem(_("Compile current file"));
		IdRun 		= builder->FindItem(_("Run"));
		IdBuildRun	= builder->FindItem(_("Build and run"));
		IdRebuild 	= builder->FindItem(_("Rebuild"));
		IdClean 	= builder->FindItem(_("Clean"));
	}
}
///-----------------------------------------------------------------------------
///  Retrieves the IDs of the popup menu items
///
void QtPregen::BuildModuleMenu(const ModuleType type, wxMenu* menu, const FileTreeData* data)
{
	if (!IsAttached())
        return;

// we 're only interested in project manager's menus
    if (!menu || type != mtProjectManager)
        return;

// right click on item menu ...else -> 0
    if (data) {
    	FileTreeData::FileTreeDataKind typedata  = data->GetKind();
    	// ... click project
		bool preproject = typedata == FileTreeData::ftdkProject  ;
		// ... click file
		bool prefile 	= typedata == FileTreeData::ftdkFile ;

		if (preproject)  {
			// popup menu on a project
			IdpClean  	= menu->FindItem(_("Clean"));
			IdpBuild  	= menu->FindItem(_("Build"));
			IdpRebuild  = menu->FindItem(_("Rebuild"));
		}
		else
		if (prefile) {
			// popup menu on a file
			IdfBuild  	= menu->FindItem(_("Build file"));
			IdfClean  	= menu->FindItem(_("Clean file"));
		}

// it's active project
		wxString nameprjact = Manager::Get()->GetProjectManager()->GetActiveProject()->GetTitle() ;
		wxString nameprj = data->GetProject()->GetTitle() ;
		if ( nameprj.Matches(nameprjact)  )  {
			if (preproject) {
			// TODO
			// test if qt project ??
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
///	- project popup
///		1. 'Build'
///		2. 'ReBuild'
///		3. 'Clean'
///	- file popup
///		1. 'Build file'
///		2. 'Clean file'
///
///calls to :
///	- prebuild->
///		1. detectQt(prj):1,
///		2. cleanCreated(prj):1,
///
void QtPregen::OnPrebuild(CodeBlocksEvent& event)
{
// missing builder	'prebuild' !
	if (!prebuild)  return;
// the active project
	cbProject *prj = event.GetProject();
	// no project !!
	if(!prj)  {
		Mes = _T("QtPregen -> no project supplied");
		printErr(Mes);
		return;
	}
// detect Qt project
	bool valid = prebuild->detectQt(prj);
	// not Qt project
	if (! valid) return;

// test patch C::B
	int eventId = event.GetId();
    if (eventId <= 0) {
		Mes = _T("The fix for 'Code::Blocks' are not applied !!");
		Mes += Lf + Quote + prebuild->namePlugin() + Quote + _(" CAN NOT CONTINUE !") ;
		cbMessageBox(Mes, _("Used fix for 'Code::Blocks' !!"), wxICON_WARNING ) ;
		return;
	}

// new project 	?
	if (prj != project) {
		buildAllOk = false;
		cleanAllOk = false;
	}
/*
Mes =  _T("buildAllOk = ") ;
Mes += buildAllOk ? _T("true") : _T("false") ;
Mes += Lf ;
Mes += _T("cleanAllOk = ") ;
Mes += cleanAllOk ? _T("true") : _T("false")   ;
printErr(Mes);
*/

// choice 'clean' or 'compilefile' or 'build'
    bool clean = eventId == IdClean || eventId == IdpClean || eventId == IdfClean;
    bool compilefile = eventId == IdCompile || eventId == IdfBuild ;
    bool build = eventId == IdBuild || eventId == IdRebuild ||
				 eventId == IdRun || eventId == IdBuildRun ||
				 eventId == IdpBuild || eventId == IdpRebuild ||
				 eventId == IdfBuild ;


	wxString mesid ;
	printLn;
///********************************
///1- Delete all files complements
///********************************
	if (clean) {
		//if (cleanAllOk) return;

	// begin preClean
		Mes = _T("QtPregen -> begin pre-Clean...");
		printWarn(Mes);
    // preCLean active target
		cleanAllOk = prebuild->cleanCreated(prj);
		if (!cleanAllOk) {
			Mes = _T("Error pre-Clean !!!");
			printErr(Mes);
		}
		buildAllOk = false;
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
		bool allBuild = eventId == IdRebuild || eventId == IdpRebuild ;
		//if (! allBuild && buildAllOk) return;
	// begin preBuild
		Mes = _T("QtPregen -> begin pre-Build...");
		printWarn(Mes);
    // preBuild
		buildAllOk = prebuild->buildQt(prj, allBuild);
		if (!buildAllOk) {
			Mes = _T("Error pre-Rebuild !!!");
			printErr(Mes);
		}
		cleanAllOk = false;
	// end preBuild
		Mes = _T("QtPregen -> ... end pre-Build");
		printWarn(Mes);
	}
    printLn;
    /// memorize last project
	project = prj;
/*
	Mes =  _T("buildAllOk = ") ;
	Mes += buildAllOk ? _T("true") : _T("false") ;
	Mes += Lf ;
	Mes +=_T("cleanAllOk = ") ;
	Mes += cleanAllOk ? _T("true") : _T("false")   ;
	printErr(Mes);
*/
}
///-----------------------------------------------------------------------------
