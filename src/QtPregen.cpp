/*************************************************************
 * Name:      QtPregen.cpp
 * Purpose:   Code::Blocks plugin 'qtPregenForCB.cbp' 0.9
 * Author:    LETARTARE
 * Created:   2015-10-17
 * Modified:  2017-07-26
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
	: m_project(nullptr), m_prebuild(nullptr), m_qtproject(false),
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
// register event sinks
    Manager* pm = Manager::Get();
//1- handle project activate
	cbEventFunctor<QtPregen, CodeBlocksEvent>* functorActivate =
		new cbEventFunctor<QtPregen, CodeBlocksEvent>(this, &QtPregen::OnActivate);
	pm->RegisterEventSink(cbEVT_PROJECT_ACTIVATE, functorActivate);
//2- handle build start
	cbEventFunctor<QtPregen, CodeBlocksEvent>* functorGen =
		new cbEventFunctor<QtPregen, CodeBlocksEvent>(this, &QtPregen::OnPregen);
	pm->RegisterEventSink(cbEVT_COMPILER_STARTED, functorGen);
//3- handle clean start
	cbEventFunctor<QtPregen, CodeBlocksEvent>* functorClean =
		new cbEventFunctor<QtPregen, CodeBlocksEvent>(this, &QtPregen::OnCleanPregen);
	pm->RegisterEventSink(cbEVT_CLEAN_PROJECT_STARTED, functorClean);
//4- a file was removed !
	cbEventFunctor<QtPregen, CodeBlocksEvent>* functorFileRemoved =
		new cbEventFunctor<QtPregen, CodeBlocksEvent>(this, &QtPregen::OnFileRemovedPregen);
	pm->RegisterEventSink(cbEVT_PROJECT_FILE_REMOVED, functorFileRemoved);

//5- construct the log
	m_LogMan = pm->GetLogManager();
	if(m_LogMan)
    {
        m_PregenLog = new TextCtrlLogger(true);
        m_LogPageIndex = m_LogMan->SetLog(m_PregenLog);
        m_LogMan->Slot(m_LogPageIndex).title = _("PreBuild log");
        CodeBlocksLogEvent evtAdd1(cbEVT_ADD_LOG_WINDOW, m_PregenLog, m_LogMan->Slot(m_LogPageIndex).title);
        pm->ProcessEvent(evtAdd1);
        // last log
        CodeBlocksLogEvent evtGetActive(cbEVT_GET_ACTIVE_LOG_WINDOW);
        pm->ProcessEvent(evtGetActive);
        m_Lastlog   = evtGetActive.logger;
        m_LastIndex = evtGetActive.logIndex;
        // display m_PregenLog
        CodeBlocksLogEvent evtSwitch(cbEVT_SWITCH_TO_LOG_WINDOW, m_PregenLog);
        pm->ProcessEvent(evtSwitch);
    }

//4- construct the builder
	// construct new 'm_prebuild'
	m_project = pm->GetProjectManager()->GetActiveProject();
	m_prebuild = new qtPrebuild(m_project, m_LogPageIndex);
	if (m_prebuild)
	{
		Mes = _T("sdk => ") + Quote + m_prebuild->GetVersionSDK() + Quote;
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
///       ...
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
/// called by : 'cbEVT_PROJECT_ACTIVATE'
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
		Mes = _T("QtPregen -> no project supplied !!");
		AppendToLog(Mes, Logger::error);
		event.Skip();
		return;
	}

// missing builder 'm_prebuild'!
	if (!m_prebuild)
	{
		Mes = _T("QtPregen -> no 'm_prebuild' !!!");
		AppendToLog(Mes, Logger::error);
		event.Skip();
		return;
	}
// detect Qt project
	// with report
	m_qtproject = m_prebuild->detectQt(prj, true);
	// no report
		//bool valid = m_prebuild->detectQt(prj, false);
	// clear log
	if (m_PregenLog)
		m_PregenLog->Clear();
	// advice
	Mes = Quote + prj->GetTitle() + Quote;
	if (m_qtproject)
	{
		// advice
		Mes += Space + _("is a Qt project...");
		AppendToLog(Mes, Logger::warning);
		// complements exists already ?
		bool ok = m_prebuild->detectComplements(prj);
		// init
		m_removingfirst = true;
	}
	else
	{
		// advice
		Mes += Space + _("is NOT a Qt project !!");
		AppendToLog(Mes, Logger::warning);
	}
/// The event processing system continues searching
	event.Skip();
}
///-----------------------------------------------------------------------------
/// Build all complement files for Qt
///
/// Called by : 'cbEVT_COMPILER_STARTED'
///		'compilergcc::DoPreGen()' with
///	- project menu
///		1. 'Build->Build'
///		2. 'Build->Compile current file'
///			3. 'Build->Run'
///		4. 'Build and Run'
///		5. 'Build->Rebuild'
///		6. 'Build->Clean'
///		7. 'Build->Build workspace'
///		8. 'Build->Rebuild workspace'
///		9. 'Build->Clean workspace'

///	- project popup
///		1. 'Build'
///		2. 'ReBuild'
///		3. 'Clean'

///	- file popup
///		1. 'Build file'
///		2. 'Clean file'
///
/// Calls to :
///     - m_prebuild->detectQt(prj, withreport):1,
///     - m_prebuild->buildQt(prj, Ws, Rebuild):1
///		- m_prebuild->buildFileQt(prj, file):1,
///
void QtPregen::OnPregen(CodeBlocksEvent& event)
{

	if (!m_qtproject)
	{
		event.Skip(); return;
	}
	if (event.GetId() != 1)
	{
		event.Skip(); return;
	}
/*  debug
	// event.Getint() == 0 ?
Mes = Lf + _T("Call 'OnPregen' ...");
Mes += _T(" eventId = ") + (wxString() << event.GetId());
Mes += _T(", eventInt = ") + (wxString() << event.GetInt());
Mes += _T(", target name = ") + event.GetBuildTargetName();
printWarn(Mes);
*/

// the active project
	cbProject *prj = event.GetProject();
	// no project !!
	if(!prj)
	{
		Mes = _T("QtPregen -> no project supplied");
		//AppendToLog(Mes, Logger::error);
		printErr(Mes);
		event.Skip();
		return ;
	}
// missing m_builder 'm_prebuild'
	if (!m_prebuild)
	{
		Mes = _T("QtPregen -> no 'm_prebuild' !!!");
		//AppendToLog(Mes, Logger::error);
		printErr(Mes);
		event.Skip();
		return;
	}
// detect Qt project, no report
	bool valid = m_prebuild->detectQt(prj, false);
	// not Qt project
	if (! valid)
	{
		event.Skip();
		return;
	}
// test event.GetString()
	wxString file = event.GetString();
	bool CompileOneFile = !file.IsEmpty();
// test event.GetInt()
	/*  enum from 'sdk_events.h'  -> 'event.getInt()'
		enum cbFutureBuild
		{
			fbNone = 10, fbBuild, fbClean, fbRebuild, fbWorkspaceBuild,
			fbWorkspaceClean, fbWorkspaceReBuild
		};
		-> evt.SetInt(static_cast<int>(GetFutureBuild()))
		-> 10, 11, 12, 13, 14, 15, 16
	*/
	int eventInt = event.GetInt();
	int eventId = event.GetId();
	//bool CompileAllFiles = (eventInt > cbFutureBuild::fbNone) ;
	bool CompileAllFiles = (eventInt > fbNone) ;
///
/*  debug
Mes = Lf + _T("Call 'OnPregen' ...");
Mes += _T(" eventId = ") + (wxString() << eventId);
Mes += _T(" eventInt = ") + (wxString() << eventInt);
printWarn(Mes);

Mes =  _T(", CompileOneFile = ") ;
Mes += CompileOneFile ? _T("true -> '") + file + Quote : _T("false");
Mes += _T(", CompileAllFiles = ");
Mes += CompileAllFiles ? _T("true") : _T("false");
printWarn(Mes);
*/
///
	if (!CompileOneFile && !CompileAllFiles)
	{
		event.Skip(); return ;
	}
	cbFutureBuild FBuild = static_cast<cbFutureBuild>(eventInt);
// calculate future action
	bool Build	 = FBuild == fbBuild || FBuild == fbRebuild ; //|| FBuild == fbWorkspaceBuild ;
	bool Clean 	 = FBuild == fbClean || FBuild == fbRebuild ; //|| fbWorkspaceClean;
	bool Rebuild = FBuild == fbRebuild  ;//|| FBuild == fbWorkspaceReBuild;
	bool WsBuild = FBuild == fbWorkspaceBuild || FBuild == fbWorkspaceReBuild;
/*  debug
Mes = _T(" FBuild = ") + (wxString() << FBuild);
Mes += _T(", Build(11) = ") + (wxString() << (Build ? _T("true") : _T("false")) );
Mes += _T(", Clean(12) = ") + (wxString() << (Clean ? _T("true") : _T("false")));
Mes += _T(", Rebuild(13) = ") + (wxString() << (Rebuild ? _T("true") : _T("false")));
printWarn(Mes);
*/

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
	if (CompileAllFiles)
	{
	// log clear
		if (m_PregenLog && (prj != m_project) && ! WsBuild)
			m_PregenLog->Clear();
/// v0.9
		if (Build)
		{
//Mes =  _T("Build complement files ...");
//printWarn(Mes);
//PrintWarn(Mes);
			// realtarget
			wxString targetname = event.GetBuildTargetName() ;
			prj->SetActiveBuildTarget(targetname);
			// verify Qt target
			bool ok = m_prebuild->isGoodTargetQt(targetname);
			if (ok)
            {
                // preBuild active target !!!
                ok = m_prebuild->buildQt(prj, WsBuild, Rebuild);
                if (! ok)
                {
                    Mes = _("Error 'PreBuild' !!!");
                    AppendToLog(Mes, Logger::error);
                   // printErr(Mes);
                }
            }
            Build = false;
		}
	}
    else
///********************************
/// Build one file
///********************************
	if (CompileOneFile)
	{
		// log clear
		//if (m_PregenLog && prj != m_project)
		//	m_PregenLog->Clear();
		if (Rebuild && !Build)
		{
			Mes = _(" CompileOnefile && Rebuild ") + Space + Quote + file + Quote;
			printErr(Mes);
			// clean
			Clean = true;
			// build
			Build = true;
		}

		if (Clean)    // n'existe pas tout seul !!
		{
			Mes =  _T("CompileOneFile && Clean ...");
			printErr(Mes);
			Clean = false;
		}

		if (Build)
		{
			Mes =  _T("CompileOneFile && Build ...");
			printErr(Mes);
			// preCompile active file
			bool elegible = m_prebuild->buildFileQt(prj, file);

			Build = false;
		}
	}

// memorize last m_project
	m_project = prj;

	m_removingfirst = true;

// switch last build log
	CodeBlocksLogEvent evtSwitch(cbEVT_SWITCH_TO_LOG_WINDOW, m_Lastlog);
    Manager::Get()->ProcessEvent(evtSwitch);

/// The event processing system continues searching
	event.Skip();
}

///-----------------------------------------------------------------------------
/// Clean all complement files for Qt
///
/// Called by 'cbEVT_CLEAN_PROJECT_STARTED'
///
void QtPregen::OnCleanPregen(CodeBlocksEvent& event)
{
	if (!m_qtproject)
	{
		event.Skip(); return;
	}
/* debug
Mes = Lf + _T("Call 'OnCleanPregen' ... ") ;
Mes += _T(" eventId = ") + (wxString() << event.GetId();
Mes += _T(", eventInt = ") + (wxString() << event.GetInt());
printWarn(Mes);
*/
	Mes =  Lf +_T("Clean all complement files ...");
	printWarn(Mes);
	// Checks if tables are empty
	bool ok =  m_prebuild->isClean();
	if (!ok)
	{
		ok = m_prebuild->removeComplements();
		if (! ok)
		{
			Mes = _T("Error remove all complement files !!!");
			//AppendToLog(Mes, Logger::error);
			printErr(Mes);
		}
	}

	m_removingfirst = true;
}

///-----------------------------------------------------------------------------
/// Unregister complement file for Qt
///
/// Called by 'cbEVT_PROJECT_FILE_REMOVED'
///
///	 project popup menu or menu
///		1. 'Remove file from project'
///		2. 'Remove files ...'
///     3. 'Remove  dir\*'
///
void QtPregen::OnFileRemovedPregen(CodeBlocksEvent& event)
{
	if (!m_qtproject)
	{
		event.Skip(); return;;
	}
	// the active project
	cbProject *prj = event.GetProject();
	if (!prj)
	{
		event.Skip(); return; ;
	}
	// the removed file ( absolute path )
	wxString filename = event.GetString();
	if (filename.IsEmpty())
	{
		event.Skip(); return;
	}

	// switch  'Prebuild' log
	CodeBlocksLogEvent evtSwitch(cbEVT_SWITCH_TO_LOG_WINDOW, m_PregenLog);
    Manager::Get()->ProcessEvent(evtSwitch);

    // it's a complement file ?
	wxString file = filename.AfterLast(Slash) ;
	bool ok = m_prebuild->isComplementFile(file);
	bool complement = ok;
	if (!ok)
	{
		// it's a creator file ?
		ok = m_prebuild->isCreatorFile(file);
		if (!ok)
		{
			m_removingfirst = true;
			event.Skip();
			return;
		}
	}
    // unregisterer file
	ok = m_prebuild->unregisterProjectFile(filename, complement, m_removingfirst) ;
	if (!ok)
	{
		Mes = Lf;
		if (complement)
			Mes += _("Complement") ;
		else
			Mes +=  _("Creator");
		Mes += Space + _("file was NOT removed from project !!!") ;
		printErr(Mes);
		Mes = Tab + Quote + filename + Quote;
		print(Mes);
	}
	m_removingfirst = false;

/// The event processing system continues searching
	event.Skip();
}
///-----------------------------------------------------------------------------


