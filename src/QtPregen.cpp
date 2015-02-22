/*************************************************************
 * Name:      QtPregen.cpp
 * Purpose:   Code::Blocks plugin  'qtPregenForCB.cbp'   0.6.0
 * Author:    LETARTARE
 * Created:   2015-02-22
 * Copyright: LETARTARE
 * License:   GPL
 *************************************************************
 */
#include <sdk.h> 	// Code::Blocks SDK
#include "QtPregen.h"
#include "qtprebuild.h"
#include "print.h"
//------------------------------------------------------------------------------
// Register the plugin with Code::Blocks.
// We are using an anonymous namespace so we don't litter the global one.
namespace
{
    PluginRegistrant<QtPregen> reg(_T("QtPregen"));
}
///-----------------------------------------------------------------------------
///	Load ressource 'QtPregen.zip'
///
QtPregen::QtPregen()
	: m_project(nullptr), m_prebuild(nullptr)
{
	if(!Manager::LoadResource(_T("QtPregen.zip")))
		NotifyMissingFile(_T("QtPregen.zip"));
}
///-----------------------------------------------------------------------------
/// Create handlers event and creating the pre-builders
///
void QtPregen::OnAttach()
{
//1- handle project activate
	cbEventFunctor<QtPregen, CodeBlocksEvent>* functorActivate =
		new cbEventFunctor<QtPregen, CodeBlocksEvent>(this, &QtPregen::OnActivate);
	Manager::Get()->RegisterEventSink(cbEVT_PROJECT_ACTIVATE, functorActivate);
//2- handle build start
	cbEventFunctor<QtPregen, CodeBlocksEvent>* functorGen =
		new cbEventFunctor<QtPregen, CodeBlocksEvent>(this, &QtPregen::OnPregen);
	Manager::Get()->RegisterEventSink(cbEVT_PREGEN_ALL, functorGen);
//3- handle compile file start
	cbEventFunctor<QtPregen, CodeBlocksEvent>* functorCompile =
		new cbEventFunctor<QtPregen, CodeBlocksEvent>(this, &QtPregen::OnPrecompile);
	Manager::Get()->RegisterEventSink(cbEVT_PREGEN_FILE, functorCompile);

//4- construct the builder
	// construct new 'm_prebuild'
	m_project = Manager::Get()->GetProjectManager()->GetActiveProject();
	m_prebuild = new qtPrebuild(m_project);
}
///-----------------------------------------------------------------------------
///	Delete the pre-builders and do de-initialization for plugin
///
void QtPregen::OnRelease(bool appShutDown)
{
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
/// Activate new m_project
///
/// called by :
///
/// calls to :
///		1. detectQt(prj):1,
///
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
	bool valid = m_prebuild->detectQt(prj, true);
	if (valid) {
		Mes = _("it's a Qt project...");
	//	print(Mes);
	}
}
///-----------------------------------------------------------------------------
/// Build all complement files for Qt
///
/// called by :
///	- menu
///		1. 'Build->Build'
///		2. 'Build->Run'
///		3. 'Build and Run'
///		4. 'Build->Rebuild'
///		5. 'Build->Clean'
///	- project popup
///		1. 'Build'
///		2. 'ReBuild'
///		3. 'Clean'
///
/// calls to :
///      -# m_prebuild->detectQt(prj, report):1,
///      -# m_prebuild->buildQt(prj, Ws, Rebuild):1
///
void QtPregen::OnPregen(CodeBlocksEvent& event)
{
// missing m_builder  'm_prebuild'
	if (!m_prebuild) return;

// the active project
	cbProject *prj = event.GetProject();
	// no project !!
	if(!prj)  {
		Mes = _T("QtPregen -> no project supplied");
		printErr(Mes);
		return;
	}

// detect Qt project
	bool valid = m_prebuild->detectQt(prj);
	// not Qt project
	if (! valid) return;

// realtarget
	wxString targetname = event.GetBuildTargetName() ;
	prj->SetActiveBuildTarget(targetname);

// display log
	printLn;
	Mes = Quote + prj->GetTitle() + Quote;
	Mes += _T("->")+ Quote + targetname + Quote;  // realtarget
	printWarn(Mes);

// booleans from 'CompilerGCC'
	void* p = event.GetClientData();
	bool Ws = ((s_rebuild*)p)->workspace;
	bool Clean = ((s_rebuild*)p)->clean;
	bool Build = ((s_rebuild*)p)->build;
	bool Rebuild = Clean && Build;

///********************************
/// Build all complement files
///********************************
	if (Build) {
	// begin pre-Build
		Mes = Tab + _T("-> begin pre-Build...");
		printWarn(Mes);
    // preBuild active target !!!
		bool ok = m_prebuild->buildQt(prj, Ws, Rebuild);
		if (! ok) {
			Mes = _T("Error pre-Build !!!");
			printErr(Mes);
		}
	// end preBuild
		Mes = Tab + _T("<- ... end pre-Build");
		printWarn(Mes);
	}

// memorize last m_project
	m_project = prj;
}
///-----------------------------------------------------------------------------
/// Compile one complement file for Qt
///
///called by :
///	- menu
///		1. 'Build->Compile current file'
///	- file popup
///		1. 'Build file'
///		2. 'Clean file'
///
void QtPregen::OnPrecompile(CodeBlocksEvent& event)
{
// missing builder	'm_prebuild' !
	if (!m_prebuild)  return;

// the active project
	cbProject *prj = event.GetProject();
	// no project !!
	if(!prj)  {
		Mes = _T("QtPregen -> no project supplied");
		printErr(Mes);
		return;
	}
// detect Qt project
	bool valid = m_prebuild->detectQt(prj, true);
	// not Qt m_project
	if (! valid) return;

// display log
	wxString file = event.GetString();
	printLn;
	Mes = Quote + prj->GetTitle() + Quote;
	Mes += _T("->")+ Quote + prj->GetActiveBuildTarget () + Quote;  // realtarget
	Mes += _T(" : ") + Quote + file + Quote;  // filename
	printWarn(Mes);

///********************************
/// Build one file
///********************************
// begin pre-Compile
	Mes = Tab + _T("-> begin pre-Compile...");
	printWarn(Mes);
// preCompile active file
	bool elegible = m_prebuild->buildFileQt(prj, file);

// end preCompile
	Mes = Tab + _T("<- ... end pre-Compile");
	printWarn(Mes);
}
///-----------------------------------------------------------------------------



