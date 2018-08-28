/*************************************************************
 * Name:      qtpregen.cpp
 * Purpose:   Code::Blocks plugin
 * Author:    LETARTARE
 * Created:   2015-10-17
 * Modified:  2018-06-23
 * Copyright: LETARTARE
 * License:   GPL
 *************************************************************
 */
#include <sdk.h> 	// Code::Blocks SDK
// for linux
#include <cbproject.h>
#include <projectmanager.h>
// <-
#include <loggers.h>
#include "qtpregen.h"
#include "qtprebuild.h"
// not place change !
#include "print.h"
//------------------------------------------------------------------------------
// Register the plugin with Code::Blocks.
// We are using an anonymous namespace so we don't litter the global one.
namespace
{
	/**  name to register this plugin with Code::Blocks
  	 */
  	wxString NamePlugin = _T("QtPregen");
	PluginRegistrant<QtPregen> reg(NamePlugin);
}
///-----------------------------------------------------------------------------
///	Load ressource 'QtPregen.zip'
///
QtPregen::QtPregen()
{
	wxString zip = NamePlugin + _T(".zip");
	if(!Manager::LoadResource(zip))
		NotifyMissingFile(zip);
}
///-----------------------------------------------------------------------------
/// Create handlers event and creating the pre-builders
///
void QtPregen::OnAttach()
{
// register event sinks
    Manager* pm = Manager::Get();

//1- handle done startup application :  NOT USED HERE
	cbEventFunctor<QtPregen, CodeBlocksEvent>* functorDoneStartup =
		new cbEventFunctor<QtPregen, CodeBlocksEvent>(this, &QtPregen::OnAppDoneStartup);
	//pm->RegisterEventSink(cbEVT_APP_STARTUP_DONE, functorDoneStartup);
//2- plugin manually loaded
	cbEventFunctor<QtPregen, CodeBlocksEvent>* functorPluginLoaded =
		new cbEventFunctor<QtPregen, CodeBlocksEvent>(this, &QtPregen::OnPluginLoaded);
	pm->RegisterEventSink(cbEVT_PLUGIN_INSTALLED, functorPluginLoaded);
//3- handle loading plugin complete
	cbEventFunctor<QtPregen, CodeBlocksEvent>* functorPluginLoadingComplete =
		new cbEventFunctor<QtPregen, CodeBlocksEvent>(this, &QtPregen::OnPluginLoadingComplete);
	pm->RegisterEventSink(cbEVT_WORKSPACE_LOADING_COMPLETE, functorPluginLoadingComplete);
//4- handle begin shutdown application
	cbEventFunctor<QtPregen, CodeBlocksEvent>* functorBeginShutdown =
		new cbEventFunctor<QtPregen, CodeBlocksEvent>(this, &QtPregen::OnAppBeginShutDown);
	pm->RegisterEventSink(cbEVT_APP_START_SHUTDOWN, functorBeginShutdown);
//5- opening a project : NOT USED HERE
	cbEventFunctor<QtPregen, CodeBlocksEvent>* functorOpenProject =
		new cbEventFunctor<QtPregen, CodeBlocksEvent>(this, &QtPregen::OnOpenProject);
	//pm->RegisterEventSink(cbEVT_PROJECT_OPEN, functorOpenProject);
//6- handle project activate
	cbEventFunctor<QtPregen, CodeBlocksEvent>* functorActivateProject =
		new cbEventFunctor<QtPregen, CodeBlocksEvent>(this, &QtPregen::OnActivateProject);
	pm->RegisterEventSink(cbEVT_PROJECT_ACTIVATE, functorActivateProject);
//7- handle target selected
	cbEventFunctor<QtPregen, CodeBlocksEvent>* functorTargetSelected =
		new cbEventFunctor<QtPregen, CodeBlocksEvent>(this, &QtPregen::OnActivateTarget);
	pm->RegisterEventSink(cbEVT_BUILDTARGET_SELECTED, functorTargetSelected);
//8- handle new project  (indicated by PECAN http://forums.codeblocks.org, 2017-12-18)
	cbEventFunctor<QtPregen, CodeBlocksEvent>* functorNewProject =
		new cbEventFunctor<QtPregen, CodeBlocksEvent>(this, &QtPregen::OnNewProject);
	pm->RegisterEventSink(cbEVT_PROJECT_NEW, functorNewProject);
//9- handle build project start
	cbEventFunctor<QtPregen, CodeBlocksEvent>* functorAdd =
		new cbEventFunctor<QtPregen, CodeBlocksEvent>(this, &QtPregen::OnAddComplements);
	pm->RegisterEventSink(cbEVT_ADD_COMPLEMENT_FILES, functorAdd);
//7- handle build stop
	cbEventFunctor<QtPregen, CodeBlocksEvent>* functorAbortGen =
		new cbEventFunctor<QtPregen, CodeBlocksEvent>(this, &QtPregen::OnAbortAdding);
	pm->RegisterEventSink(cbEVT_COMPILER_FINISHED, functorAbortGen);
//10- a file was removed !
	cbEventFunctor<QtPregen, CodeBlocksEvent>* functorFileRemoved =
		new cbEventFunctor<QtPregen, CodeBlocksEvent>(this, &QtPregen::onProjectFileRemoved);
	pm->RegisterEventSink(cbEVT_PROJECT_FILE_REMOVED, functorFileRemoved);
//11- a project is renommed
	cbEventFunctor<QtPregen, CodeBlocksEvent>* functorRenameProject =
		new cbEventFunctor<QtPregen, CodeBlocksEvent>(this, &QtPregen::OnRenameProjectOrTarget);
	pm->RegisterEventSink(cbEVT_PROJECT_RENAMED, functorRenameProject);
//12- a target is renommed
	cbEventFunctor<QtPregen, CodeBlocksEvent>* functorRenameTarget =
		new cbEventFunctor<QtPregen, CodeBlocksEvent>(this, &QtPregen::OnRenameProjectOrTarget);
	 pm->RegisterEventSink(cbEVT_BUILDTARGET_RENAMED, functorRenameTarget);
//13- closing a project :  NOT USED HERE
	cbEventFunctor<QtPregen, CodeBlocksEvent>* functorCloseProject =
		new cbEventFunctor<QtPregen, CodeBlocksEvent>(this, &QtPregen::OnCloseProject);
	//pm->RegisterEventSink(cbEVT_PROJECT_CLOSE, functorCloseProject);

//14- construct a new log
	m_LogMan = pm->GetLogManager();
	if(m_LogMan)
    {
    // add 'Prebuild log'
        m_PregenLog = new TextCtrlLogger(true);
        m_LogPageIndex = m_LogMan->SetLog(m_PregenLog);
        m_LogMan->Slot(m_LogPageIndex).title = _("PreBuild log");
        CodeBlocksLogEvent evtAdd1(cbEVT_ADD_LOG_WINDOW, m_PregenLog,
									m_LogMan->Slot(m_LogPageIndex).title);
        pm->ProcessEvent(evtAdd1);
    // memorize last log
        CodeBlocksLogEvent evtGetActive(cbEVT_GET_ACTIVE_LOG_WINDOW);
        pm->ProcessEvent(evtGetActive);
        m_Lastlog   = evtGetActive.logger;
        m_LastIndex = evtGetActive.logIndex;
    // display 'm_PregenLog'
        SwitchToLog(m_LogPageIndex);
    }

//15- construct the builder
	// construct a new 'm_pPrebuild'
	m_pPrebuild = new qtPrebuild(m_pProject, m_LogPageIndex, NamePlugin);
	if (m_pPrebuild)
	{
		Mes = _T("sdk => ") + Quote + m_pPrebuild->GetVersionSDK() + Quote;
		print(Mes);
	/* for debug
		Mes = _T("onAttach() : ");
		if (m_pProject)
			Mes += _T("Active project = ") + Quote + m_pProject->GetTitle() + Quote ;
		else
			Mes += _T(" no active project !!");
		print(Mes);
	*/
	}
	else
	{
		Mes = _("Error to create ") ;
		Mes += _T("m_pPrebuild") + Lf;
		Mes += _("The plugin is not operational !!");
	    printErr(Mes);
	}
}

///-----------------------------------------------------------------------------
/// Validate messages to 'Prebuild log'
///		- called only when the plugin is manually loaded
///
///	Called by :
///		1. cbEVT_PLUGIN_INSTALLED
///
void QtPregen::OnPluginLoaded(CodeBlocksEvent& event)
{
	m_WithMessage = true;
/// just for debug
//	Mes = _T("QtPregen::OnPluginLoaded(...) -> ");
//	Mes +=  _T(" 'qtPregen' is manually loaded");
//	printWarn(Mes) ;
// the active project
	m_pProject = Manager::Get()->GetProjectManager()->GetActiveProject();
	if (m_pProject)
	{
		CodeBlocksEvent evt(cbEVT_PROJECT_ACTIVATE, 0, m_pProject, 0, this);
		OnActivateProject(evt);
	}


/// The event processing system continues searching
	event.Skip();
}

///-----------------------------------------------------------------------------
/// Validate messages to 'Prebuild log'
///
///	Called by :
///		1. cbEVT_WORKSPACE_LOADING_COMPLETE
///
void QtPregen::OnPluginLoadingComplete(CodeBlocksEvent& event)
{
	m_WithMessage = true;
/// just for debug
//	Mes = _T("QtPregen::OnPluginLoadingComplete(...) -> ");
//	Mes +=  _T("all plugins are loaded");
//	printWarn(Mes) ;
	m_pProject = Manager::Get()->GetProjectManager()->GetActiveProject();
	if (m_pProject)
	{
		CodeBlocksEvent evt(cbEVT_PROJECT_ACTIVATE, 0, m_pProject, 0, this);
		OnActivateProject(evt);
	}
/// The event processing system continues searching
	event.Skip();
}

///-----------------------------------------------------------------------------
/// Validate messages to 'Prebuild log'
///
///	Called by :
///		1. cbEVT_APP_STARTUP_DONE
///     2. cbEVT_PLUGIN_INSTALLED
///
void QtPregen::OnAppDoneStartup(CodeBlocksEvent& event)
{
	m_WithMessage = true;
/// just for debug
//	Mes = _T("QtPregen::OnAppDoneStartup(...) ...");
//	Mes +=  _T("app done startup");
//	printWarn(Mes) ;
	if (m_pProject)
	{
		CodeBlocksEvent evt(cbEVT_PROJECT_ACTIVATE, 0, m_pProject, 0, this);
		OnActivateProject(evt);
	}
/// The event processing system continues searching
	event.Skip();
}

///-----------------------------------------------------------------------------
/// Invalid the messages to 'Prebuild log'
///
/// Called by :
///		1. event 'cbEVT_APP_START_SHUTDOWN'
///
void QtPregen::OnAppBeginShutDown(CodeBlocksEvent& event)
{
//Mes = _T("QtPregen::OnAppBeginShutDown(...) ...");
//printWarn(Mes) ;
	m_WithMessage = false;

/// The event processing system continues searching
	event.Skip();
}

///-----------------------------------------------------------------------------
///	Delete the pre-builders and do de-initialization for plugin
///
/// Called by :
///
void QtPregen::OnRelease(bool appShutDown)
{
//1- delete builder
	if (m_pPrebuild)
	{
		delete m_pPrebuild;
		m_pPrebuild = nullptr;
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
/// Called by :
///		-# all printxxx(wxString)
///
void QtPregen::AppendToLog(const wxString& Text, Logger::level lv)
{
    if(m_PregenLog && m_WithMessage)
    {
        CodeBlocksLogEvent evtSwitch(cbEVT_SWITCH_TO_LOG_WINDOW, m_PregenLog);
        Manager::Get()->ProcessEvent(evtSwitch);
        m_PregenLog->Append(Text, lv);
    }
}

///-----------------------------------------------------------------------------
/// Switch to a log
///
/// Called by :
///		1. OnAttach():1,
///		2. OnAddComplements(CodeBlocksEvent& event):1,
///		3. onProjectFileRemoved(CodeBlocksEvent& event):1,
///
void QtPregen::SwitchToLog(int indexlog)
{
// display a log
	CodeBlocksLogEvent evtSwitch(cbEVT_SWITCH_TO_LOG_WINDOW, indexlog);
	Manager::Get()->ProcessEvent(evtSwitch);
	Manager::Yield();
}

///-----------------------------------------------------------------------------
/// Open a project
///
/// Called by :
///		1. event 'cbEVT_PROJECT_OPEN'
///
/// Calls to : none
///
void QtPregen::OnOpenProject(CodeBlocksEvent& event)
{
// wait for message validation
	if (!m_WithMessage)
	{
		event.Skip(); return;
	}
	Mes = wxEmptyString;
/// just for debug
	//Mes = NamePlugin + _T("::OnOpenProject(CodeBlocksEvent& event) -> ");
// the active project
	cbProject *prj = event.GetProject();
	if(!prj)
	{
		Mes += _("no project supplied !!");
		printErr(Mes);
		event.Skip();
		return;
	}
/// DEBUG
//* **********************************
//	m_pPrebuild->beginDuration(_T("OnOpenProject(...)"));
//* *********************************
// active target
	wxString nametarget = prj->GetActiveBuildTarget();
	Mes += Quote + prj->GetTitle() + _T("::") ;
	if (nametarget.IsEmpty() )
	{
		Mes += Quote + Space + _("no target supplied !!");
		printErr(Mes);
		event.Skip();
		return;
	}
// messages
	Mes += nametarget + Quote + Space + _("is opened !!");
	printWarn(Mes);

/// The event processing system continues searching
	event.Skip();

/// DEBUG
//* **********************************
//	m_pPrebuild->endDuration(_T("OnOpenProject(...)"));
//* *********************************
}

///-----------------------------------------------------------------------------
/// Close a project
///
/// Called by :
///		1. event 'cbEVT_PROJECT_CLOSE'
///
/// Calls to : none
///
void QtPregen::OnCloseProject(CodeBlocksEvent& event)
{
// wait for message validation
	if (!m_WithMessage)
	{
		event.Skip(); return;
	}
	Mes = wxEmptyString;
/// just for debug
	//Mes = NamePlugin + _T("::OnCloseProject(CodeBlocksEvent& event) -> ");
// the project
	cbProject *prj = event.GetProject();
	if(!prj)
	{
		Mes += _("no project supplied !!");
		printErr(Mes);
		event.Skip();
		return;
	}
/// messages
	Mes += Quote + prj->GetTitle() + Quote + Space + _("is closed !!");
	printWarn(Mes);

/// The event processing system continues searching
	event.Skip();

/// DEBUG
//* **********************************
//	m_pPrebuild->endDuration(_T("OnCloseProject(...)"));
//* *********************************
}

///-----------------------------------------------------------------------------
/// Activate a project
///		called before 'cbEVT_PROJECT_NEW' !!
///
/// Called by :
///		1. event 'cbEVT_PROJECT_ACTIVATE'
///
/// Calls to :
///		1. qtPre::detectQtProject(cbProject * prj, bool report):1,
///		2. qtpre::detectQtTarget(const wxString& nametarget, cbProject * prj):1,
///		3. qtPre::detectComplementsOnDisk(cbProject * prj, const wxString & nametarget,  bool report):1,
///
void QtPregen::OnActivateProject(CodeBlocksEvent& event)
{
// wait for message validation
	if (!m_WithMessage)
	{
		event.Skip(); return;
	}
	Mes = wxEmptyString;
/// just for debug
	//Mes = NamePlugin + _T("::OnActivateProject(CodeBlocksEvent& event) -> ");
// missing builder 'm_pPrebuild'!
	if (!m_pPrebuild)
	{
		Mes += _("'m_pPrebuild' is null !!!");
		printErr(Mes);
		event.Skip();
		return;
	}
// the active project
	cbProject *prj = event.GetProject();
	if(!prj)
	{
		Mes += _("no project supplied !!");
		printErr(Mes);
		event.Skip();
		return;
	}
// only by this method : 'OnActivateProject(...)'
	m_pProject = prj;
/// DEBUG
//* **********************************
//	m_pPrebuild->beginDuration(_T("OnActivateProject(...)"));
//* *********************************

/// creating a new project : this event arrived before 'cbEVT_PROJECT_NEW' !!
// just project created ?
	wxString nametarget = prj->GetActiveBuildTarget();
	if (nametarget.IsEmpty() )
	{
		Mes += _("no target supplied !!");
		printErr(Mes);
		event.Skip();
		return;
	}
//Mes = _T("project name ") + Quote + prj->GetTitle() + Quote ;
//Mes +=  Space + _T("nametarget = ") + Quote + nametarget + Quote ;
//printWarn(Mes);

/// activate an old project ...
//Mes = NamePlugin +  _T(" -> ") + _("OnActivateProject for an OLD project ...");
//PrintWarn(Mes);

// detect Qt project ... with report
	m_isQtProject = m_pPrebuild->detectQtProject(prj, true);
//Mes = _T("m_qtproject = ") + (wxString() << (int)m_isQtProject);
//printWarn(Mes);
	// advice
	Mes += Quote + prj->GetTitle() + Quote + Space;
	if (m_isQtProject)	Mes += _("has at least one target using Qt libraries...");
	else				Mes += _("is NOT a Qt project !!");
	printWarn(Mes);

	if (m_isQtProject)
	{
	// detect Qt active Target ...
		m_isQtActiveTarget = m_pPrebuild->detectQtTarget(nametarget, prj);
		// advice
		Mes = Tab + Quote + _T("::") + nametarget + Quote + Space ;
		if(m_isQtActiveTarget)	Mes += _("is a Qt target...");
		else					Mes += _("is NOT a Qt target !!");
		printWarn(Mes);

		// complements exists already ?
		m_pPrebuild->detectComplementsOnDisk(prj, nametarget, true);
		m_removingfirst = true;
	}

/// The event processing system continues searching
	event.Skip();

/// DEBUG
//* **********************************
//	m_pPrebuild->endDuration(_T("OnActivateProject(...)"));
//* *********************************
}

///-----------------------------------------------------------------------------
/// Activate a target
///		called after a project is closed !!
///	    called before a project is opened  !!
///
/// Called by :
///		1. event 'cbEVT_BUILDTARGET_SELECTED'
///
/// Calls to :
///		1. qtpre::detectQtTarget(const wxString& nametarget, cbProject * prj):1,
///		2. qtPre::detectComplementsOnDisk(cbProject * prj, const wxString & nametarget,  bool report):1,
///
void QtPregen::OnActivateTarget(CodeBlocksEvent& event)
{
// not a Qt current project
	if (!m_isQtProject)
	{
		event.Skip(); return;
	}
// wait for message validation
	if (!m_WithMessage)
	{
		event.Skip(); return;
	}
	Mes = wxEmptyString;
/// just for debug
	//Mes = NamePlugin + _T("::OnActivateTarget(CodeBlocksEvent& event) -> ");
// missing builder 'm_pPrebuild'!
	if (!m_pPrebuild)
	{
		Mes += _("'m_pPrebuild' is null !!!");
		printErr(Mes);
		event.Skip();
		return;
	}
// the active project
	cbProject *prj = event.GetProject();
	if(!prj)
	{
		Mes += _("no project supplied !!");
		printErr(Mes);
		event.Skip();
		return;
	}
// it's not the current project !
	if ( m_pProject != prj)
	{
/// just for debug
		Mes += Quote + prj->GetTitle() + Quote + _T(" : ") ;
		Mes += _("event project is not the current project !!");
	//	printErr(Mes);
		event.Skip();
		return;
	}

// the active target
	wxString nametarget  =  event.GetBuildTargetName() ;
	Mes += Tab + Quote + _T("::") + nametarget + Quote + Space;
	if (nametarget.IsEmpty() )
	{
	/// test if the project is open
		if (Manager::Get()->GetProjectManager()->IsProjectStillOpen(m_pProject))
		{
			Mes += _("!! no target supplied !!");
			printErr(Mes);
		}
		event.Skip();
		return;
	}
// detect Qt target
	m_isQtActiveTarget = m_pPrebuild->detectQtTarget(nametarget, prj) ;
	// advices
	if(m_isQtActiveTarget)	Mes += _("is a Qt target...");
	else					Mes += _("is NOT a Qt target !!");
	printWarn(Mes);

/// The event processing system continues searching
	event.Skip();
}

///-----------------------------------------------------------------------------
/// Create a new project
///
/// Called by :
///		1. event 'cbEVT_PROJECT_NEW'
///
/// Calls to :
///		1. qtpre::detectQtProject(const wxString& nametarget, cbProject * prj):1,
///		2. qtpre::detectQtTarget(const wxString& nametarget, cbProject * prj):1,
///
void QtPregen::OnNewProject(CodeBlocksEvent& event)
{
// wait for message validation
	if (!m_WithMessage)
	{
		event.Skip(); return;
	}
	Mes = wxEmptyString;
/// just for debug
	// Mes = NamePlugin + _T("::OnNewProject(CodeBlocksEvent& event) -> ");
// missing builder 'm_pPrebuild'!
	if (!m_pPrebuild)
	{
		Mes += _("'m_pPrebuild' is null !!!");
		printErr(Mes);
		event.Skip();
		return;
	}
// the new project
	cbProject *prj = event.GetProject();
	if(!prj)
	{
		Mes += _("no project supplied !!");
		printErr(Mes);
		event.Skip();
		return;
	}
	m_pProject = prj;
// just project created ?
	wxString nametarget = prj->GetActiveBuildTarget();
	if (nametarget.IsEmpty() )
	{
		Mes += _("no target supplied !!");
		printErr(Mes);
		event.Skip();
		return;
	}
//Mes = _T("project name ") + Quote + prj->GetTitle() + Quote ;
//Mes +=  Space + _T("nametarget = ") + Quote + nametarget + Quote ;
//printWarn(Mes);

// detect Qt project ... with report
	m_isQtProject = m_pPrebuild->detectQtProject(prj, true);
//Mes = _T("m_isQtProject = ") + (wxString() << (int)m_isQtProject);
//printWarn(Mes);
	// advice
	Mes += Quote + prj->GetTitle() + Quote + Space;
	if (m_isQtProject)	Mes += _("has at least one target using Qt libraries...");
	else				Mes += _("is NOT a Qt project !!");
	printWarn(Mes);

// detect Qt active Target ...
	m_isQtActiveTarget = m_pPrebuild->detectQtTarget(nametarget, prj);
	// advice
	Mes = Tab + Quote + prj->GetTitle() + _T("::") + nametarget + Quote + Space ;
	if(m_isQtActiveTarget)	Mes += _("is a Qt target...");
	else					Mes += _("is NOT a Qt target !!");
	printWarn(Mes);

/// The event processing system continues searching
	event.Skip();
}

///-----------------------------------------------------------------------------
/// Receive a renaming of a project or a target
///
/// Called by :
///		1. event 'cbEVT_PROJECT_RENAMED'
///		2. event 'cbEVT_BUILDTARGET_RENAMED'
///		3- before 'cbEVT_PROJECT_ '
///
/// Calls to :
///		1. qtpre::detectQtProject(const wxString& nametarget, cbProject * prj):1,
///		2. qtpre::detectQtTarget(const wxString& nametarget, cbProject * prj):1,
///		3. qtPre::detectComplementsOnDisk(cbProject * prj, const wxString & nametarget,  bool report):2,
///		4. qtPre::isIndependentTarget(cbProject * prj, const wxString & target):1,
///		5. qtPre::newNameComplementDirTarget(wxString & newname, wxString & oldname):1,
///
void QtPregen::OnRenameProjectOrTarget(CodeBlocksEvent& event)
{
// not a Qt current project
	if (!m_isQtProject)
	{
		event.Skip(); return;
	}
// wait for message validation
	if (!m_WithMessage)
	{
		event.Skip(); return;
	}
	Mes = wxEmptyString;
/// just for debug
	//Mes = NamePlugin + _T("::OnRenameProjectOrTarget(CodeBlocksEvent& event) -> ");
// missing builder 'm_pPrebuild'!
	if (!m_pPrebuild)
	{
		Mes += _("'m_pPrebuild' is null !!!");
		printErr(Mes);
		event.Skip();
		return;
	}
// the active project :  new name ?
	cbProject *prj = event.GetProject();
	if(!prj)
	{
		Mes += _("no project supplied !!");
		printErr(Mes);
		event.Skip();
		return;
	}
// it's not the current project
	if ( m_pProject != prj)
	{
/// just for debug
	//	Mes += Quote + prj->GetTitle() + Quote + _T(" : ") ;
	//	Mes += _("event project is not the current project !!");
	//	printErr(Mes);
		event.Skip();
		return;
	}
// the name target :  new name ?
	wxString nametarget =  event.GetBuildTargetName()
			,oldnametarget = event.GetOldBuildTargetName()
			;
	bool okqt = false;
	if (nametarget.IsEmpty() && oldnametarget.IsEmpty())
	{
	// it's a new name project
		Mes += _T("!!") + Space + _("A new name project") ;
		Mes += Space + Quote + prj->GetTitle() + Quote;
		printWarn(Mes);
	// detect Qt project ... with report
		m_isQtProject = m_pPrebuild->detectQtProject(prj, true);
		Mes = Quote + prj->GetTitle() + Quote;
		if (m_isQtProject)
		{
			// advice
			Mes += Space + _("has at least one target using Qt libraries...");
			printWarn(Mes);
			// complements exists already ?
			m_pPrebuild->detectComplementsOnDisk(prj, nametarget, true);
			// init
			m_removingfirst = true;
		}
		else
		{
			// advice
			Mes += Space + _("is NOT a Qt project !!");
			printWarn(Mes);
		}
	}
	else
	{
	// it's a new name target
		Mes = _T("!!") + Space + _("A new name target") ;
		Mes += Space + Quote + prj->GetTitle() + _T("::") + nametarget + Quote;
		Mes += Space + _T("(old name :") + Quote + oldnametarget + Quote + _T(")");
		// advice
		if (!nametarget.IsEmpty() )
		{
		// active target
			wxString activetarget = prj->GetActiveBuildTarget() ;
			if (!activetarget.Matches(nametarget))
			{
			// detect Qt target ...
				okqt = m_pPrebuild->detectQtTarget(nametarget, prj);
				Mes += _T(" : ") ;
				if(okqt)	Mes += _("is a Qt target...");
				else		Mes += _("is NOT a Qt target !!");
			}
		}
		printWarn(Mes);
		if (!nametarget.IsEmpty() )
		{
		// check if the target is independent
			bool indtarget = m_pPrebuild->isIndependentTarget(prj, nametarget) ;
			if (indtarget)
			{
            //  create and refresh the subdirectory name
				m_pPrebuild->newNameComplementDirTarget(nametarget, oldnametarget);
            //	refresh the tables
				m_pPrebuild->detectComplementsOnDisk(prj, nametarget, true);
			}
		}
	}

/// The event processing system continues searching
	event.Skip();
}

///-----------------------------------------------------------------------------
/// Stop compiling request to 'CompilerGCC'
///
///	Called by :
///		1. OnAddComplements(CodeBlocksEvent& event):1,

void QtPregen::compilingStop(int idAbort)
{
//Mes = _T("QtPregen::compilingStop(") + (wxString()<<idAbort) + _T(")");
//printErr(Mes);
	CodeBlocksEvent evtMenu(wxEVT_COMMAND_MENU_SELECTED, idAbort);
	// if comes from 'QtPregen' for 'CompilerGCC' !
	evtMenu.SetInt(idAbort);
	Manager::Get()->ProcessEvent(evtMenu);
// Not mandatory
//	Manager::Yield();
}

///-----------------------------------------------------------------------------
///  Abort compiling complement files from 'CompilerGCC'
///
/// Called by :
///		1. event 'cbEVT_COMPILER_FINISH' from 'CompilerGCC::AbortPreBuild()' with
///	    bouton-menu
///		2. 'Abort'
///
///	Call to :
///		1. qtPrebuild::Abort():1,
///
void QtPregen::OnAbortAdding(CodeBlocksEvent& event)
{
// not a Qt current project
	if (!m_isQtProject)
	{
		event.Skip(); return;
	}
// it's for QtPregen ?
	if (event.GetId() != 1)
	{
		event.Skip(); return;
	}
	//Mes = wxEmptyString;
/// just for debug
	Mes = NamePlugin + _T("::OnAbortAdding(CodeBlocksEvent& event) -> ");
// missing m_builder 'm_pPrebuild'
	if (!m_pPrebuild)
	{
		Mes += _T("no 'm_pPrebuild' is null !!!");
		printErr(Mes);
		event.Skip();
		return;
	}
// the active project
	cbProject *prj = event.GetProject();
	// no project !!
	if(!prj)
	{
		Mes += _("no project supplied");
		printErr(Mes);
		event.Skip();
		return ;
	}

	if ( m_pProject != prj)
	{
		Mes += _("event project target is not the current project !!");
		printErr(Mes);
		event.Skip();
		return;
	}
// Abort complement files creating
	m_pPrebuild->Abort();
// cleaning plugin  : TODO
	/// ...
}

///-----------------------------------------------------------------------------
/// Build all complement files for Qt
///
/// Called by :
///		1. event : 'cbEVT_ADD_COMPLEMENT_FILES':2, from class 'CompilerGCC'
///
///	'CompilerGCC::AddComplementFiles()' with
///	- bouton-menu
///		1. 'Build'
///		2. 'Run'
///		3. 'Build and Run'
///		4. 'ReBuild'
///
///	- project menu
///		1. 'Build->Build'
///		2. 'Build->Compile current file'
///		3. 'Build->Run'
///		4. 'Build and Run'
///		5. 'Build->Rebuild'
///		6. 'Build->Clean'
///		7. 'Build->Build workspace'
///		8. 'Build->Rebuild workspace'
///		9. 'Build->Clean workspace'
///
///	- project popup
///		1. 'Build'
///		2. 'ReBuild'
///		(3. 'Clean')
///
///	- file popup
///		1. 'Build file'
///		2. 'Clean file'
///
/// Calls to :
///		-# qtpre::detectQtProject(prj, withreport):1,
///		-# qtPrebuild::buildAllFiles(prj, Ws, Rebuild):1,
///		-# qtPrebuild::buildOneFile(prj, file):1,
///		-# compilingStop(int idAbort):1,
///		-# qtPre::detectComplementsOnDisk(cbProject * prj, const wxString & nametarget,  bool report):1,
///
void QtPregen::OnAddComplements(CodeBlocksEvent& event)
{
//Mes = _T("QtPregen::OnAddComplements(...)");
//printWarn(Mes);
// not a Qt current project
	if (!m_isQtProject)
	{
		event.Skip(); return;
	}
// it's for QtPregen ?
	if (event.GetId() != 1)
	{
		event.Skip(); return;
	}
	if (!m_isQtActiveTarget )
	{
		event.Skip(); return;
	}

/// DEBUG
//* **********************************
//	m_pPrebuild->beginDuration(_T("OnAddComplements(...)"));
//* *********************************
//  debug
	// event.Getint() == 0 ?
//Mes = Lf + _T("Call 'OnAddComplements' ...");
//Mes += _T(" eventId = ") + (wxString() << event.GetId());
//Mes += _T(", eventInt = ") + (wxString() << event.GetInt());
//Mes += _T(", target name = ") + event.GetBuildTargetName();
//printWarn(Mes);

	Mes = NamePlugin + _T("::OnAddComplements(CodeBlocksEvent& event) -> ");
// missing m_builder 'm_pPrebuild'
	if (!m_pPrebuild)
	{
		Mes += _T("no 'm_pPrebuild' is null !!!");
		printErr(Mes);
		event.Skip();
		return;
	}

// the active project
	cbProject * prj = m_pProject ; //event.GetProject();
	// no project !!
	if(!prj )
	{
		Mes += _("no project supplied");
		printErr(Mes);
		event.Skip();
		return ;
	}

// just project created ?
	wxString nametarget = prj->GetActiveBuildTarget();
	if (nametarget.IsEmpty() )
	{
		Mes += _("no target supplied !!");
		printErr(Mes);
		event.Skip();
		return;
	}

//Mes = _T("project name ") + Quote + prj->GetTitle() + Quote ;
//Mes +=  Space + _T("nametarget = ") + Quote + nametarget + Quote ;
//printWarn(Mes);

// detect Qt project, no report
	bool valid = m_pPrebuild->detectQtProject(prj, false);
	// not Qt project
	if (! valid)
	{
		event.Skip(); return;
	}
// allright !
	// complements exists already ?
	m_pPrebuild->detectComplementsOnDisk(prj, nametarget, false);
	// init
	m_removingfirst = false;

// test event.GetInt()
	/* definitions
		enum from 'sdk_events.h'  -> 'event.getInt()'
		enum cbFutureBuild
		{
			fbNone = 10, fbBuild, fbClean, fbRebuild, fbWorkspaceBuild,
			fbWorkspaceClean, fbWorkspaceReBuild
		};
		-> evt.SetInt(static_cast<int>(GetFutureBuild()))
		-> 10, 11, 12, 13, 14, 15, 16
	*/
	int eventInt = event.GetInt();
//	int eventId = event.GetId();
	int idMenuKillProcess ;
// test file == event.GetString()
	wxString file = event.GetString();
	bool CompileOneFile = !file.IsEmpty()
		,CompileAllFiles = !CompileOneFile && (eventInt > fbNone)
		;
///
  //debug
//Mes = Lf + _T("Call 'OnAddComplements' ...");
//Mes += _T(" eventId = ") + (wxString() << eventId);
//Mes += _T(" eventInt = ") + (wxString() << eventInt);
//printWarn(Mes);

//Mes =  _T(", CompileOneFile = ") ;
//Mes += CompileOneFile ? _T("true -> '") + file + Quote : _T("false");
//Mes += _T(", CompileAllFiles = ");
//Mes += CompileAllFiles ? _T("true") : _T("false");
//printWarn(Mes);
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
  //debug
/*
Mes = _T(" FBuild = ") + (wxString() << FBuild);
Mes += _T(", Build(11) = ") + (wxString() << (Build ? _T("true") : _T("false")) );
Mes += _T(", Clean(12) = ") + (wxString() << (Clean ? _T("true") : _T("false")));
Mes += _T(", Rebuild(13) = ") + (wxString() << (Rebuild ? _T("true") : _T("false")));
printWarn(Mes);
*/
// last build log
	CodeBlocksLogEvent evtGetActive(cbEVT_GET_ACTIVE_LOG_WINDOW);
	Manager::Get()->ProcessEvent(evtGetActive);
	m_Lastlog   = evtGetActive.logger;
	m_LastIndex = evtGetActive.logIndex;
// to PreBuild Log
	SwitchToLog(m_LogPageIndex);

///********************************
/// Build all complement files
///********************************
	if (CompileAllFiles)
	{
	// log clear
		if (m_PregenLog && (prj != m_pProject) && ! WsBuild)
			m_PregenLog->Clear();

		Mes = _T("Wait please ...");
		AppendToLog(Mes);
		//printWarn(Mes);

	// event.GetX()
		idMenuKillProcess = event.GetX();
//Mes = Lf + _T("Call 'OnAddComplements' ...");
//Mes += _T(" eventX = ") + (wxString() << idMenuKillProcess );
//printWarn(Mes);

		if (Build)
		{
		// real target
            wxString targetname = event.GetBuildTargetName() ;
			prj->SetActiveBuildTarget(targetname);
			ProjectBuildTarget * target = prj->GetBuildTarget(targetname);
			// verify Qt target
			bool ok = m_pPrebuild->isGoodTargetQt(target);
			if (ok)
            {
            // preBuild active target !!!
			//  no preBuild
				// ok = m_pPrebuild->buildAllFiles(prj, WsBuild, Rebuild);
			// always prebuild
               // ok = m_pPrebuild->buildAllFiles(prj, WsBuild, Build);
			// only preBuild when date complements < date creator
				ok = m_pPrebuild->buildAllFiles(prj, WsBuild, false);
                if (! ok)
                {
                //	Mes = Tab + _T("m_pPrebuild->buildAllFiles(...) => ");
                //  Mes += _("Error 'PreBuild' !!!");
                //  printErr(Mes);
                    valid = false;
                }
            }
            else
            {
                Mes = _("It's not a Qt target !!!");
                printWarn(Mes);

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
		if (Rebuild && !Build)
		{
			Mes = _(" CompileOnefile && Rebuild ") + Space + Quote + file + Quote;
			printErr(Mes);
			// clean
			Clean = true;
			// build
			Build = true;
		}

		if (Clean)    // does not exist alone !!
		{
			Mes =  _T("CompileOneFile && Clean ...");
			printWarn(Mes);
			Clean = false;
		}

		if (Build)
		{
			Mes =  _T("CompileOneFile && Build ...");
			printWarn(Mes);
			// preCompile active file
			// bool elegible =
			m_pPrebuild->buildOneFile(prj, file);

			Build = false;
		}
	}

	m_removingfirst = true;

// switch last build log
	CodeBlocksLogEvent evtSwitch(cbEVT_SWITCH_TO_LOG_WINDOW, m_Lastlog);
    Manager::Get()->ProcessEvent(evtSwitch);

    if (!valid)
	{
	// call compiling stop
		compilingStop(idMenuKillProcess);
	}

/// DEBUG
//* **********************************
//	m_pPrebuild->endDuration(_T("OnAddComplements(...)"));
//* *********************************

// The event processing system continues searching
	event.Skip();
}


///-----------------------------------------------------------------------------
/// Unregister complement file for Qt
///
/// Called by
///		1. event 'cbEVT_PROJECT_FILE_REMOVED'
///
///		by project popup menu or menu
///		2. 'Remove file from project'
///		3. 'Remove files ...'
///		4. 'Remove  dir\*'
///
///	Calls to
///		1. qtPre::isComplementFile(const wxString & file):1,
///		2. qtPre::isCreatorFile(const wxString & file):1,
///		3. qtPrebuild::unregisterProjectFile(const wxString & file, bool complement, bool first):1,
///
void QtPregen::onProjectFileRemoved(CodeBlocksEvent& event)
{
//Mes = _T("QtPregen::onProjectFileRemoved(...)");
//printWarn(Mes);
	// 	not a Qt current project
	if (!m_isQtProject)
	{
		event.Skip(); return;;
	}
	// the project used
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
    SwitchToLog(m_LogPageIndex);
// only name less path
	wxString file = filename.AfterLast(Slash) ;
//Mes = _T("filename = ") + filename ;
//Mes += _T(" ,file = ") + file + _T(", complement = ") + (wxString() << complement);
//print(Mes);
	bool ok = false;
	Mes = Lf;
	// it's a complement file ?
	if (m_pPrebuild->isComplementFile(file))
	{
	// unregisterer one complement file
		ok = m_pPrebuild->unregisterFileComplement(filename, false, m_removingfirst) ;
		if (!ok)
			Mes += _("Complement") ;
	}
// it's a creator file ?
	else
	if (m_pPrebuild->isCreatorFile(file))
	{
	// unregisterer one creator file and one ? complement file
		ok = m_pPrebuild->unregisterFileCreator(filename, m_removingfirst) ;
		if (!ok)
			Mes +=  _("Creator");
	}
	else
	{
		m_removingfirst = true;
		event.Skip();
		return;
	}
	if (!ok)
	{
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
