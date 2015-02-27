/*************************************************************
 * Name:      QtPregen.cpp
 * Purpose:   Code::Blocks plugin  'qtPregenForCB.cbp'   0.8.3
 * Author:    LETARTARE
 * Created:   2015-02-27
 * Copyright: LETARTARE
 * License:   GPL
 *************************************************************
 */
#include <sdk.h> 	// Code::Blocks SDK
#include <loggers.h>
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
	: m_project(nullptr), m_prebuild(nullptr) ,
	  m_PregenLog(nullptr), m_LogPageIndex(0), m_LogMan(nullptr)
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
	Manager::Get()->RegisterEventSink(cbEVT_COMPILER_STARTED, functorGen);

//3- construct the log
	m_LogMan = Manager::Get()->GetLogManager();
	if(m_LogMan)
    {
        m_PregenLog = new TextCtrlLogger(true);
        m_LogPageIndex = m_LogMan->SetLog(m_PregenLog);
 //   Mes = wxString()<<m_LogPageIndex;
 //   Print(Mes);
        m_LogMan->Slot(m_LogPageIndex).title = _("PreBuild log");
        CodeBlocksLogEvent evtAdd1(cbEVT_ADD_LOG_WINDOW, m_PregenLog, m_LogMan->Slot(m_LogPageIndex).title);
        Manager::Get()->ProcessEvent(evtAdd1);
        // last log
        CodeBlocksLogEvent evtGetActive(cbEVT_GET_ACTIVE_LOG_WINDOW);
        Manager::Get()->ProcessEvent(evtGetActive);
        m_Lastlog   =  evtGetActive.logger;
        m_LastIndex = evtGetActive.logIndex;
        // display m_PregenLog
        CodeBlocksLogEvent evtSwitch(cbEVT_SWITCH_TO_LOG_WINDOW, m_PregenLog);
        Manager::Get()->ProcessEvent(evtSwitch);
    }

//4- construct the builder
	// construct new 'm_prebuild'
	m_project = Manager::Get()->GetProjectManager()->GetActiveProject();
	m_prebuild = new qtPrebuild(m_project, m_LogPageIndex);
	if (m_prebuild)
	{
		Mes = _T("sdk : ") + Quote + m_prebuild->GetVersionSDK() + Quote;
		print(Mes);
	}
}
///-----------------------------------------------------------------------------
///	Delete the pre-builders and do de-initialization for plugin
///
void QtPregen::OnRelease(bool appShutDown)
{
//1- delete builder
	if (m_prebuild)
	{
		delete m_prebuild;
		m_prebuild = nullptr;
	}

//2-  delete log
	if(m_LogMan)
    {
        if(m_PregenLog)
        {
            CodeBlocksLogEvent evt(cbEVT_REMOVE_LOG_WINDOW, m_PregenLog);
            Manager::Get()->ProcessEvent(evt);
        }
    }
    m_PregenLog = nullptr;

//3- do de-initialization for your plugin
    // if appShutDown is true, the plugin is unloaded because Code::Blocks is being shut down,
    // which means you must not use any of the SDK Managers
    // NOTE: after this function, the inherited member variable
    // m_IsAttached will be FALSE...
    Manager::Get()->RemoveAllEventSinksFor(this);
}
///-----------------------------------------------------------------------------
/// Append text to log
///
/// called by :
///
///
void QtPregen::AppendToLog(const wxString& Text, Logger::level lv)
{
    if(m_PregenLog)
    {
        CodeBlocksLogEvent evtSwitch(cbEVT_SWITCH_TO_LOG_WINDOW, m_PregenLog);
        Manager::Get()->ProcessEvent(evtSwitch);
        m_PregenLog->Append(Text, lv);
    }
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
// the active project
	cbProject *prj = event.GetProject();
	if(!prj)
	{
		Mes = _T("QtPregen -> no project supplied");
		printErr(Mes);
		return;
	}

// missing builder 'm_prebuild'!
	if (!m_prebuild)
		return;

// detect Qt project
	// with report
	bool valid = m_prebuild->detectQt(prj, true);
	// no report
	//bool valid = m_prebuild->detectQt(prj, false);
	if (valid)
	{
		if (m_PregenLog)
			m_PregenLog->Clear();
		Mes = Quote + prj->GetTitle() + Quote;
		Mes += Tab + _("it's a Qt project...");
		AppendToLog(Mes, Logger::warning);
	}
}
///-----------------------------------------------------------------------------
/// Build all complement files for Qt
///
/// called by :
///	- project menu
///		1. 'Build->Build'
///		2. 'Build->Run'
///		3. 'Build and Run'
///		4. 'Build->Rebuild'
///		5. 'Build->Clean'
///	- project popup
///		1. 'Build'
///		2. 'ReBuild'
///		3. 'Clean'
///	- file menu
///		1. 'Build->Compile current file'
///	- file popup
///		1. 'Build file'
///		2. 'Clean file'
///
/// calls to :
///     -# m_prebuild->detectQt(prj, withreport):1,
///     -# m_prebuild->buildQt(prj, Ws, Rebuild):1
///		-# m_prebuild->buildFileQt(prj, file):1,
///
void QtPregen::OnPregen(CodeBlocksEvent& event)
{
// the active project
	cbProject *prj = event.GetProject();
	// no project !!
	if(!prj)
	{
		Mes = _T("QtPregen -> no project supplied");
		AppendToLog(Mes, Logger::error);
		return;
	}

// missing m_builder  'm_prebuild'
	if (!m_prebuild)
		return;

// detect Qt project, no report
	bool valid = m_prebuild->detectQt(prj, false);
	// not Qt project
	if (! valid)
		return;

// test event.GetString()
	wxString file = event.GetString();
	bool CompileFile = !file.IsEmpty();
// test event.GetInt()
	int eventInt = event.GetInt();
	bool CompileAll = eventInt > 0;

	if (!CompileFile && !CompileAll)
		return;

// last build log
	CodeBlocksLogEvent evtGetActive(cbEVT_GET_ACTIVE_LOG_WINDOW);
	Manager::Get()->ProcessEvent(evtGetActive);
	m_Lastlog   =  evtGetActive.logger;
	m_LastIndex = evtGetActive.logIndex;

	Mes = _T("");
	AppendToLog(Mes);

///********************************
/// Build all complement files
///********************************
	if (CompileAll)
	{
		/*  // enum from 'sdk_events.h'
		enum cbFutureBuild
		{
			fbNone = 0,	fbBuild, fbClean, fbRebuild, fbWorkspaceBuild,
			fbWorkspaceClean, fbWorkspaceReBuild
		};
		*/
	// calculate future build
		cbFutureBuild FBuild = static_cast<cbFutureBuild>(eventInt);
		bool Rebuild = FBuild == fbRebuild || FBuild == fbWorkspaceReBuild;
		bool Build	=  FBuild == fbBuild || FBuild == fbWorkspaceBuild || Rebuild ;
		bool Ws = FBuild == fbWorkspaceBuild || FBuild == fbWorkspaceReBuild;

	// log clear
		if (m_PregenLog && prj != m_project&& ! Ws)
			m_PregenLog->Clear();

		if (Build)
		{
			// realtarget
			wxString targetname = event.GetBuildTargetName() ;
			prj->SetActiveBuildTarget(targetname);
			// preBuild active target !!!
			bool ok = m_prebuild->buildQt(prj, Ws, Rebuild);
			if (! ok)
			{
				Mes = _T("Error PreBuild !!!");
				AppendToLog(Mes, Logger::error);
			}
		}
	}
    else
///********************************
/// Build one file
///********************************
	if (CompileFile)
	{
	// log clear
		if (m_PregenLog && prj != m_project)
			m_PregenLog->Clear();

	// preCompile active file
		bool elegible = m_prebuild->buildFileQt(prj, file);
	}

// memorize last m_project
	m_project = prj;

// switch last build log
	CodeBlocksLogEvent evtSwitch(cbEVT_SWITCH_TO_LOG_WINDOW, m_Lastlog);
    Manager::Get()->ProcessEvent(evtSwitch);
}
///-----------------------------------------------------------------------------


