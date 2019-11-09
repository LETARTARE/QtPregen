/*************************************************************
 * Name:      qtpregen.cpp
 * Purpose:   Code::Blocks plugin
 * Author:    LETARTARE
 * Created:   2015-10-17
 * Modified:  2019-11-09
 * Copyright: LETARTARE
 * License:   GPL
 *************************************************************
 */
#include <sdk.h> 	// Code::Blocks SDK
// for linux
#include <cbproject.h>
#include <projectmanager.h>
// <-
#include <cbeditor.h>
#include <editormanager.h>

#include <loggers.h>
#include "qtpregen.h"
#include "qtprebuild.h"
// not place change !
#include "defines.h"
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

//1- handle done startup application :
/// NOT USED HERE
	cbEventFunctor<QtPregen, CodeBlocksEvent>* functorDoneStartup =
		new cbEventFunctor<QtPregen, CodeBlocksEvent>(this, &QtPregen::OnAppDoneStartup);
	// pm->RegisterEventSink(cbEVT_APP_STARTUP_DONE, functorDoneStartup);
//2- plugin manually loaded
	cbEventFunctor<QtPregen, CodeBlocksEvent>* functorPluginLoaded =
		new cbEventFunctor<QtPregen, CodeBlocksEvent>(this, &QtPregen::OnPluginLoaded);
	pm->RegisterEventSink(cbEVT_PLUGIN_INSTALLED, functorPluginLoaded);
//3- handle loading plugin complete ( comes before 'cbEVT_APP_START_SHUTDOWN' )
	cbEventFunctor<QtPregen, CodeBlocksEvent>* functorPluginLoadingComplete =
		new cbEventFunctor<QtPregen, CodeBlocksEvent>(this, &QtPregen::OnPluginLoadingComplete);
	pm->RegisterEventSink(cbEVT_PLUGIN_LOADING_COMPLETE, functorPluginLoadingComplete);
//4- handle begin shutdown application
	cbEventFunctor<QtPregen, CodeBlocksEvent>* functorBeginShutdown =
		new cbEventFunctor<QtPregen, CodeBlocksEvent>(this, &QtPregen::OnAppBeginShutDown);
	pm->RegisterEventSink(cbEVT_APP_START_SHUTDOWN, functorBeginShutdown);
//5- opening a project :
/// NOT USED HERE
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
//10- handle build stop
	cbEventFunctor<QtPregen, CodeBlocksEvent>* functorAbortGen =
		new cbEventFunctor<QtPregen, CodeBlocksEvent>(this, &QtPregen::OnAbortAdding);
	pm->RegisterEventSink(cbEVT_COMPILER_FINISHED, functorAbortGen);
//11- a file was removed !
	cbEventFunctor<QtPregen, CodeBlocksEvent>* functorFileRemoved =
		new cbEventFunctor<QtPregen, CodeBlocksEvent>(this, &QtPregen::onProjectFileRemoved);
	pm->RegisterEventSink(cbEVT_PROJECT_FILE_REMOVED, functorFileRemoved);
//12- a project is renommed
	cbEventFunctor<QtPregen, CodeBlocksEvent>* functorRenameProject =
		new cbEventFunctor<QtPregen, CodeBlocksEvent>(this, &QtPregen::OnRenameProjectOrTarget);
//	pm->RegisterEventSink(cbEVT_PROJECT_RENAMED, functorRenameProject);
//13- a target is renommed
	cbEventFunctor<QtPregen, CodeBlocksEvent>* functorRenameTarget =
		new cbEventFunctor<QtPregen, CodeBlocksEvent>(this, &QtPregen::OnRenameProjectOrTarget);
	 pm->RegisterEventSink(cbEVT_BUILDTARGET_RENAMED, functorRenameTarget);
//14- closing a project :
/// NOT USED HERE
	cbEventFunctor<QtPregen, CodeBlocksEvent>* functorCloseProject =
		new cbEventFunctor<QtPregen, CodeBlocksEvent>(this, &QtPregen::OnCloseProject);
	//pm->RegisterEventSink(cbEVT_PROJECT_CLOSE, functorCloseProject);
//15- save a project :
/// NOT USED HERE
	cbEventFunctor<QtPregen, CodeBlocksEvent>* functorSaveProject =
		new cbEventFunctor<QtPregen, CodeBlocksEvent>(this, &QtPregen::OnSaveProject);
	//pm->RegisterEventSink(cbEVT_PROJECT_SAVE, functorSaveProject);
//16 editor save a file
	/// USED HERE for debug
	cbEventFunctor<QtPregen, CodeBlocksEvent>* functorSaveFileEditor =
		new cbEventFunctor<QtPregen, CodeBlocksEvent>(this, &QtPregen::OnSaveFileEditor);
	pm->RegisterEventSink(cbEVT_EDITOR_SAVE, functorSaveFileEditor);

//17- construct a new log
	m_LogMan = pm->GetLogManager();
	if(m_LogMan)
    {
    // add 'Prebuild log'
        m_PregenLog = new TextCtrlLogger(FIX_LOG_FONT);
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

//18- construct the builder
	// construct a new 'm_pPrebuild' m_pProject == nullptr
	m_pPrebuild = new qtPrebuild(NamePlugin, m_LogPageIndex);
	if (m_pPrebuild)
	{
		Mes = _T("sdk =>") + quote(m_pPrebuild->GetVersionSDK()); print(Mes);
/// for debug : none activated project
/*
		Mes = _T("onAttach() : ");
		if (m_pProject)
			Mes += _T("Active project = ") + quote(m_pProject->GetTitle());
		else
			Mes += _T(" no active project !!");
		print(Mes);
*/
	}
	else
	{
		Mes = _("Error to create") + Space + _T("m_pPrebuild") ;
		Mes += Lf + _("The plugin is not operational") + _T(" !!"); printError(Mes);
	///  release plugin
	    OnRelease(false);
	}
	Mes.Clear();
}

///-----------------------------------------------------------------------------
/// Validate messages to 'Prebuild log'
///		- called only when the plugin is manually loaded
///
///	Called by :
///		1. cbEVT_PLUGIN_INSTALLED
///
void QtPregen::OnPluginLoaded(CodeBlocksEvent& _event)
{
	m_WithMessage = true;
// init done
	m_initDone = true;
/// just for debug
/*
	Mes = _T("QtPregen::OnPluginLoaded(...) -> ");
	Mes +=  _T(" 'qtPregen' is manually loaded");
	Mes += Space + _T("-> m_initDone = ") + strBool(m_initDone) ; printWarn(Mes) ;
*/
// the active project
	m_pProject = Manager::Get()->GetProjectManager()->GetActiveProject();
	if (m_pProject)
	{
	// pseudo event
		m_pseudoEvent = true;
		CodeBlocksEvent evt(cbEVT_PROJECT_ACTIVATE, 0, m_pProject, 0, this);
		OnActivateProject(evt);
	}

	Mes.Clear();
/// The event processing system continues searching
	_event.Skip();
}

///-----------------------------------------------------------------------------
/// Validate messages to 'Prebuild log'
///
///	Called by : DISCONNECTED !
///		1. cbEVT_WORKSPACE_LOADING_COMPLETE
///
void QtPregen::OnPluginLoadingComplete(CodeBlocksEvent& _event)
{
	m_WithMessage = true;
	// using plugins
	m_initDone = true;
/// just for debug
/*
	Mes = _T("QtPregen::OnPluginLoadingComplete(...) -> ");
	Mes += _("all plugins are loaded");
	Mes += Space + _T("m_initDone = ") + strBool(m_initDone) ; printWarn(Mes) ;
*/
	Mes.Clear();
/// The event processing system continues searching
	_event.Skip();
}

///-----------------------------------------------------------------------------
/// Invalid the messages to 'Prebuild log'
///
/// Called by :
///		1. event 'cbEVT_APP_START_SHUTDOWN'
///
void QtPregen::OnAppBeginShutDown(CodeBlocksEvent& _event)
{
//Mes = _T("QtPregen::OnAppBeginShutDown(...) ..."); printWarn(Mes) ;
	m_WithMessage = false;

	Mes.Clear();
/// The event processing system continues searching
	_event.Skip();
}

///-----------------------------------------------------------------------------
///	Delete the pre-builders and do de-initialization for plugin
///
/// Called by :
///
void QtPregen::OnRelease(bool appShutDown)
{
//1- delete builder"
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
void QtPregen::AppendToLog(const wxString& _Text, Logger::level _lv)
{
    if(m_PregenLog && m_WithMessage)
    {
        CodeBlocksLogEvent evtSwitch(cbEVT_SWITCH_TO_LOG_WINDOW, m_PregenLog);
        Manager::Get()->ProcessEvent(evtSwitch);
        m_PregenLog->Append(_Text, _lv);
    }
}

///-----------------------------------------------------------------------------
/// Switch to a log
///
/// Called by :
///		1. OnAttach():1,
///		2. OnAddComplements(CodeBlocksEvent& _event):1,
///		3. onProjectFileRemoved(CodeBlocksEvent& _event):1,
///
void QtPregen::SwitchToLog(int _indexLog)
{
// display a log
	CodeBlocksLogEvent evtSwitch(cbEVT_SWITCH_TO_LOG_WINDOW, _indexLog);
	Manager::Get()->ProcessEvent(evtSwitch);
	Manager::Yield();
}

///-----------------------------------------------------------------------------
/// Activate a project
///		called before a 'cbEVT_PROJECT_NEW' !!
///
/// Called by :
///		1. event 'cbEVT_PROJECT_ACTIVATE'
///
/// Calls to :
///		1. qtPre::detectQtProject(cbProject * _pProject, bool _report):1,
///		2. qtPre::detectComplementsOnDisk(cbProject * _pProject, const wxString & _nametarget,  bool _report):1,
///
void QtPregen::OnActivateProject(CodeBlocksEvent& _event)
{
/// plugins are loaded ?
	if (!m_initDone)
	{
		_event.Skip() ; return;
	}
// wait for message validation
	if (!m_WithMessage)
	{
		_event.Skip(!m_pseudoEvent);
		m_pseudoEvent = false;
		return;
	}
// the active project
	cbProject *prj = _event.GetProject();
	if(!prj)
	{
		Mes += _("no project supplied") + _T(" !!"); printError(Mes);
		_event.Skip(!m_pseudoEvent);
		m_pseudoEvent = false;
		return;
	}

// actualize the project here
	m_pProject = prj;
/// DEBUG
//* *********************************************************
//	m_pPrebuild->beginDuration(_T("OnActivateProject(...)"));
//* *********************************************************
// detect Qt project ... with report : feed 'qtpre::m_pProject'
	m_isQtProject = m_pPrebuild->detectQtProject(m_pProject, WITH_REPORT);
//Mes = _T("m_qtproject = ") + strBool(m_isQtProject); printWarn(Mes);
	// advice
	Mes = _T("The project") +  quote(m_pProject->GetTitle());
	if (m_isQtProject)	Mes += _("has at least one target using Qt libraries") + _T("...");
	else				Mes += _("is NOT a Qt project") + _T(" !!");
	printWarn(Mes);
	if (!m_isQtProject)
	{
		_event.Skip(!m_pseudoEvent);
		m_pseudoEvent = false ;
		return;
	}

/// here : project has a Qt target
	// virtual target ?
	wxString nametarget = m_pProject->GetActiveBuildTarget();
	m_isQtActiveTarget = m_pPrebuild->detectQtTarget(nametarget);
//Mes = _T("OnActivateProject :: active name target") + quote(nametarget); print(Mes);
//Mes = _T("QtPregen::OnActivateProject() : m_isQtActiveTarget = ") + strBool(m_isQtActiveTarget); print(Mes);
	/// search if complements exists already on disk ?
		// ok = true => files Qt exists on disk
		bool ok = m_pPrebuild->detectComplementsOnDisk(m_pProject, nametarget, !m_isNewProject);
		m_removingIsFirst = true;
		// it' now an old project
	//	m_isNewProject = false;

/// The event processing system continues searching if not a pseudo event
	_event.Skip(!m_pseudoEvent);
	m_pseudoEvent = false ;

/// DEBUG
//* *******************************************************
//	m_pPrebuild->endDuration(_T("OnActivateProject(...)"));
//* *******************************************************
	Mes.Clear();
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
///		1. adviceTypeTarget(nametarget):1,
///
void QtPregen::OnActivateTarget(CodeBlocksEvent& _event)
{
/// a Qt target can exist in a non-Qt project!
// not a Qt current project
	if (!m_isQtProject)
	{
		_event.Skip(); return;
	}
// wait for message validation
	if (!m_WithMessage)
	{
		_event.Skip(); return;
	}

/// just for debug
//Mes = NamePlugin + _T("::OnActivateTarget(CodeBlocksEvent& _event) -> "); printError(Mes);
///
// the active project
	cbProject *prj = _event.GetProject();
	if(!prj)
	{
		Mes += Space + _("no project supplied") + _T(" !!"); printError(Mes);
		_event.Skip();
		return;
	}
/// is it possible ??
// it's not the current project !
	if ( m_pProject != prj)
	{
/// just for debug
	//	Mes += quote(prj->GetTitle()) + _T(" : ") ;
	//	Mes += _("event project is not the current project !!"); 	printWarn(Mes);
/// <=
		_event.Skip();
		return;
	}

/// DEBUG
//* *********************************************************
//	m_pPrebuild->beginDuration(_T("OnActivateTarget(...)"));
//* *********************************************************

// the active target
	wxString nametarget  =  _event.GetBuildTargetName() ;
	if (nametarget.IsEmpty() )
	{
	/// test if the project is open
		if (Manager::Get()->GetProjectManager()->IsProjectStillOpen(m_pProject))
		{
			Mes += Space + _("no target supplied") + _T(" !!"); printError(Mes);
		}
		_event.Skip();
		return;
	}
	// is a command target
	if (m_pPrebuild-> isCommandTarget(nametarget))
	{
		Mes =  Tab + quote(_T("::") + nametarget);
		Mes += _("is a command target") + _T(" !!") ; printWarn(Mes);
		_event.Skip();
		return;
	}
	// all targets : real and virtual ...
	m_isQtActiveTarget = m_pPrebuild->detectQtTarget(nametarget, WITH_REPORT);
//Mes = _T("QtPregen::OnActivateTarget() : m_isQtActiveTarget = ") + strBool(m_isQtActiveTarget); print(Mes);

	Mes.Clear();
/// The event processing system continues searching
	_event.Skip();

/// DEBUG
//* *******************************************************
//	m_pPrebuild->endDuration(_T("OnActivateTarget(...)"));
//* *******************************************************
}

///-----------------------------------------------------------------------------
/// Create a new project
///
/// Called by :
///		1. event 'cbEVT_PROJECT_NEW'
///
/// Calls to :
///		1. qtpre::detectQtProject(const wxString& _nametarget, cbProject * _pProject):1,
///		2. qtpre::detectQtTarget(const wxString& _nametarget, cbProject * _pProject):1,
///
void QtPregen::OnNewProject(CodeBlocksEvent& _event)
{
// wait for message validation
	if (!m_initDone || !m_WithMessage)
	{
		_event.Skip(); return;
	}
/// just for debug
	//Mes = NamePlugin + _T("::OnNewProject(CodeBlocksEvent& event) -> ");
	//printError(Mes);
// the new project
	cbProject *pProject = _event.GetProject();
	if(!pProject)
	{
		Mes += _("no project supplied") + _T(" !!"); printError(Mes);
		_event.Skip(); return;
	}

// actualize the project
	m_pProject = pProject;
	m_isNewProject = true;
// just project created ?
	wxString nametarget = m_pProject->GetActiveBuildTarget();
	if (nametarget.IsEmpty() )
	{
		Mes += _("no target supplied") + _T(" !!"); printError(Mes);
		_event.Skip(); return;
	}
//Mes = _T("project name") + quote(m_pProject->GetTitle()) ;
//Mes +=  Space + _T("nametarget =") + quote(nametarget); printWarn(Mes);

// detect Qt project ... with report
	m_isQtProject = m_pPrebuild->detectQtProject(m_pProject, WITH_REPORT);
	// advice
	Mes = _("The New project") + quote(m_pProject->GetTitle());
	if (m_isQtProject)	Mes += _("has at least one target using Qt libraries") + _T("...");
	else				Mes += _("is NOT a Qt project") + _T(" !!");
	printWarn(Mes);

// detect Qt active Target ...
	m_isQtActiveTarget = m_pPrebuild->detectQtTarget(nametarget, NO_REPORT);
	// advice
	Mes = Tab + quote( _T("::") + nametarget ) + Space ;
	Mes += _("is") + Space;
	if(!m_isQtActiveTarget)		Mes += _("NOT") + Space;
	Mes += _("a Qt target");
	print(Mes);

	Mes.Clear();
/// The event processing system continues searching
	_event.Skip();
}

///-----------------------------------------------------------------------------
/// Receive a renaming of a project or a target
///		at CB start we receive "cbEVT_PROJECT_RENAMED" for each registered project !!!
///		after all plugins are loaded and before the first project activation
///
/// Called by :
///		1. event 'cbEVT_PROJECT_RENAMED'
///		2. event 'cbEVT_BUILDTARGET_RENAMED'
///		3- before 'cbEVT_PROJECT_TARGETS_MODIFIED'
///
/// Calls to :
///		1. qtpre::detectQtProject(const wxString& _nametarget, cbProject * _pProject):1,
///		2. qtpre::detectQtTarget(const wxString& _nametarget, cbProject * _pProject):1,
///		3. qtPre::detectComplementsOnDisk(cbProject * _pProject, const wxString & _nametarget,  bool _report):2,
///		4. qtPre::isTargetWithComplement(cbProject * _pProject, const wxString & _target):1,
///		5. qtPre::newNameComplementDirTarget(wxString & _newname, wxString & _oldname):1,
///
void QtPregen::OnRenameProjectOrTarget(CodeBlocksEvent& _event)
{
	// no project is activated yet
	if (!m_pProject)
	{
/// Debug
//Mes = NamePlugin + _T("::OnRenameProjectOrTarget(CodeBlocksEvent& event) -> ");
//Mes +=  quote(_event.GetProject()->GetTitle()) + _T(" : ") ;
//Mes += _T(" but 'm_pProject' is null !! ") ; printWarn(Mes);
/// <=
		_event.Skip(); return;
	}

// the project used
	cbProject *prj = _event.GetProject();
	if (!prj)
	{
		_event.Skip(); return; ;
	}

/// is it possible ?? it's just a new name
// it's not the current project !
	if ( m_pProject != prj)
	{
/// Debugdebug
//	Mes += quote(prj->GetTitle()) + _T(" : ") ;
//		Mes += _("event project is not the current project !!");
//		printError(Mes);
/// <==
		_event.Skip();
		return;
	}

// wait for message validation
	if (!m_WithMessage)
	{
		_event.Skip(); return;
	}
	// global
	Mes = wxEmptyString;
/// virtual target ??
	// the name target :  new name ?
	wxString nametarget = _event.GetBuildTargetName()
			,oldnametarget = _event.GetOldBuildTargetName()
			;
//Mes  = _T("!!! nameProject =") + quote(prj->GetTitle());
//Mes += _T(", newtarget =") + quote(nametarget);
//Mes += _T(", oldtarget =") + quote(oldnametarget); printWarn(Mes);

	// it's a command target
	if (m_pPrebuild-> isCommandTarget(nametarget))
	{
		Mes =  Tab + quote( nametarget ) + _("is a command target") + _T(" !!") ;
		printWarn(Mes);
		_event.Skip();
		return;
	}

	bool okqt = false;
/// //////////////////////////
/// it's a new name project
/// //////////////////////////
	if (nametarget.IsEmpty() && oldnametarget.IsEmpty())
	{
		// detect Qt project ... with report
		m_isQtProject = m_pPrebuild->detectQtProject(m_pProject, WITH_REPORT);
//Mes = _T("==> m_isQtProject = ") + strBool(m_isQtProject); printWarn(Mes);
		if (!m_isQtProject)
		{
			_event.Skip(); return;
		}
		Mes = _T("=== ") + _("A new name project") + quote(m_pProject->GetTitle());
		printWarn(Mes);
		Mes = quote(prj->GetTitle());
		if (m_isQtProject)
		{ // advice
			Mes += _("has at least one target using Qt libraries") + _T("...");
			printWarn(Mes);
			// complements exists already ?
			m_pPrebuild->detectComplementsOnDisk(m_pProject, nametarget, !m_isNewProject);
			// init
			m_removingIsFirst = true;
		}
	}
/// ////////////////////////
/// it's a new name target
/// ////////////////////////
	else
	if (!nametarget.IsEmpty() )
	{
		m_pProject = prj;
	// verify target
		wxString actualtarget = m_pProject->GetActiveBuildTarget() ;
//Mes = _T("!! actualtarget = ") + quote(actualtarget);
		// update m_pProject, ..., m_pBuildTarget
		m_pPrebuild->setProject(m_pProject);

	// new active target ?
		wxString activetarget = m_pProject->GetActiveBuildTarget() ;
//Mes += _T("!! activetarget =") + quote(activetarget);
//Mes += _T(", nametarget =") + quote(nametarget); printWarn(Mes);

		// advice
		Mes = _T("=== ") + _("Old target name") + quote(oldnametarget)  ;
		Mes += _T("=> ") + _("New target name") + quote(nametarget);
		printWarn(Mes);

		if (!activetarget.Matches(nametarget))
		{
		// detect Qt target ...
			okqt = m_pPrebuild->detectQtTarget(nametarget, NO_REPORT);
			Mes = quoteNS(_T("::") + nametarget) + Space;
			Mes += _("is") + Space;
			if(!okqt)		Mes += _("NOT") + Space;
			Mes += _("a Qt target");
			printWarn(Mes);
		}
	/// updates of the new target :
		// new build target
		ProjectBuildTarget * pBuildTarget = m_pProject->GetBuildTarget(nametarget);
		bool ok = m_pPrebuild->updateNewTargetName(pBuildTarget, oldnametarget);
		/// ... TO FINISH

	}
	Mes.Clear();
/// The event processing system continues searching
	_event.Skip();
}

///-----------------------------------------------------------------------------
/// Stop compiling request to 'CompilerGCC'
///
///	Called by :
///		1. OnAddComplements(CodeBlocksEvent& _event):1,

void QtPregen::compilingStop(int _idAbort)
{
//Mes = _T("QtPregen::compilingStop(") + strInt(_idAbort) + _T(")"); printError(Mes);
	CodeBlocksEvent evtMenu(wxEVT_COMMAND_MENU_SELECTED, _idAbort);
	// if comes from 'QtPregen' for 'CompilerGCC' !
	evtMenu.SetInt(_idAbort);
	Manager::Get()->ProcessEvent(evtMenu);
// Not mandatory
//	Manager::Yield();
}

///-----------------------------------------------------------------------------
///  Abort compiling complement files from 'CompilerGCC'
///
/// Called by :
///		1. event 'cbEVT_COMPILER_FINISH' from 'CompilerGCC::AbortPreBuild()'
///	       with button-menu
///		2. 'setAbort'
///
///	Call to :
///		1. qtPrebuild::setAbort():1,
///
void QtPregen::OnAbortAdding(CodeBlocksEvent& _event)
{
// not a Qt current project
	if (!m_isQtProject)
	{
		_event.Skip(); return;
	}
// it's for QtPregen ?
	if (_event.GetId() != 1)
	{
		_event.Skip(); return;
	}
	//Mes = wxEmptyString;
/// just for debug
	Mes = NamePlugin + _T("::OnAbortAdding(CodeBlocksEvent& event) -> ");
///
// the active project
	cbProject *prj = _event.GetProject();
	// no project !!
	if(!prj)
	{
		Mes += _("no project supplied"); printError(Mes);
		_event.Skip();
		return ;
	}
/// just for debug
	if (m_pProject != prj)
	{
//Mes += _("event project target is not the current project !!");	printError(Mes);
		_event.Skip();
		return;
	}
// Abort complement files creating
	m_pPrebuild->setAbort();
// cleaning plugin  : TODO
	/// ...
	Mes.Clear();
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
///		-# qtPre::detectComplementsOnDisk(cbProject * _pProject, const wxString & _nametarget,  bool _report):1,
///
void QtPregen::OnAddComplements(CodeBlocksEvent& _event)
{
//Mes = _T("QtPregen::OnAddComplements(...)"); printError(Mes);
// not a Qt current project  :: TO REVIEW !!
	if (!m_isQtProject)
	{
		_event.Skip(); return;
	}
// it's for QtPregen ?
	if (_event.GetId() != 1)
	{
		_event.Skip(); return;
	}
	if (!m_isQtActiveTarget )
	{
		_event.Skip(); return;
	}
//Mes = _T("QtPregen::OnAddComplements(...):m_isQtActiveTarget : ") + strBool(m_isQtActiveTarget);
//printError(Mes);

/// DEBUG
//* ********************************************************
	m_pPrebuild->beginDuration(_T("OnAddComplements(...)"));
//* ********************************************************
///  Debug
	// _event.Getint() == 0 ?
//Mes = Lf + _T("Call 'OnAddComplements' ...");
//Mes += _T(" eventId = ") + strInt(_event.GetId());
//Mes += _T(", eventInt = ") + strInt(_event.GetInt());
//Mes += _T(", target name = ") + _event.GetBuildTargetName(); printWarn(Mes);
/// <=
	Mes = NamePlugin + _T("::OnAddComplements(CodeBlocksEvent& event) -> ");
// the active project
	cbProject * prj = m_pProject ; //event.GetProject();
	// no project !!
	if(!prj )
	{
		Mes += _("no project supplied"); printError(Mes);
		_event.Skip();
		return ;
	}

// just project created ?
	wxString nametarget = prj->GetActiveBuildTarget();
	if (nametarget.IsEmpty() )
	{
		Mes += _("no target supplied !!"); printError(Mes);
		_event.Skip();
		return;
	}

//Mes = _T("project name") + quote(prj->GetTitle()) ;
//Mes +=  Space + _T("nametarget =") + quote(nametarget) ; printWarn(Mes);

// detect Qt project, no report
	bool valid = m_pPrebuild->detectQtProject(prj, NO_REPORT);
	// not Qt project
	if (! valid)
	{
		_event.Skip(); return;
	}
// allright !
//Mes = _T("m_isNewProject is ") + strBool(m_isNewProject);
	// verify if complements exists already
	m_pPrebuild->detectComplementsOnDisk(prj, nametarget, !m_isNewProject);
	// it' now an old project with complement files on disk
	m_isNewProject = false;
	// init
	m_removingIsFirst = false;

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
	int eventInt = _event.GetInt();
//	int eventId = event.GetId();
	int idMenuKillProcess ;
// test file == event.GetString()
	wxString file = _event.GetString();
	bool CompileOneFile = !file.IsEmpty()
		,CompileAllFiles = !CompileOneFile && (eventInt > fbNone)
		;
/// debug
//Mes = Lf + _T("Call 'OnAddComplements' ...");
//Mes += _T(" eventId = ") + strInt(eventId) ;
//Mes += _T(" eventInt = ") + strInt(eventInt); printWarn(Mes);

//Mes =  _T(", CompileOneFile = ") ;
//Mes += CompileOneFile ? _T("true ->") + quote(file) : _T("false");
//Mes += _T(", CompileAllFiles = ") + strBool(CompileAllFiles); printWarn(Mes);
/// <==
	if (!CompileOneFile && !CompileAllFiles)
	{
		_event.Skip(); return ;
	}

	cbFutureBuild FBuild = static_cast<cbFutureBuild>(eventInt);
// calculate future action
	bool Build	 = FBuild == fbBuild || FBuild == fbRebuild ; //|| FBuild == fbWorkspaceBuild ;
	bool Clean 	 = FBuild == fbClean || FBuild == fbRebuild ; //|| fbWorkspaceClean;
	bool Rebuild = FBuild == fbRebuild  ;//|| FBuild == fbWorkspaceReBuild;
	bool WsBuild = FBuild == fbWorkspaceBuild || FBuild == fbWorkspaceReBuild;

/// Debug
//Mes = _T(" FBuild = ") + strBool(FBuild);
//Mes += _T(", Build(11) = ") + strBool(Build);
//Mes += _T(", Clean(12) = ") + strBool(Clean);
//Mes += _T(", Rebuild(13) = ") + strBool(Rebuild);
//printWarn(Mes);
/// <==
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

		Mes = _("Wait please") + _T(" ...");
		AppendToLog(Mes);
		//printWarn(Mes);

	// event.GetX()
		idMenuKillProcess = _event.GetX();
//Mes = Lf + _T("Call 'OnAddComplements' ...");
//Mes += _T(" eventX = ") + strInt(idMenuKillProcess); printWarn(Mes);

		if (Build)
		{
		// real target
            wxString nametarget = _event.GetBuildTargetName() ;
			prj->SetActiveBuildTarget(nametarget);
			ProjectBuildTarget * pBuildTarget = prj->GetBuildTarget(nametarget);
			// verify Qt target
			bool ok = m_pPrebuild->isGoodTargetQt(pBuildTarget);
			// if a virtual target !! => '!pBuildTarget' => ok = false
			if (ok)
            {
				// prebuil all complements
				if (Rebuild)
					ok = m_pPrebuild->buildAllFiles(prj, WsBuild, ALLBUILD);
				// only preBuild when date complements < date creator
				else
					ok = m_pPrebuild->buildAllFiles(prj, WsBuild, NO_ALLBUILD);
                if (! ok)
                {
                //	Mes = Tab + _T("m_pPrebuild->buildAllFiles(...) => ");
                //  Mes += _("Error 'PreBuild' !!!");   printError(Mes);
                    valid = false;
                }
            }
            else
            {
                Mes = _("It's not a Qt target") + _T(" !!!");
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
			Mes = _("CompileOnefile && Rebuild") + quote(file); printError(Mes);
			// clean
			Clean = true;
			// build
			Build = true;
		}

		if (Clean)    // does not exist alone !!
		{
			Mes =  _("CompileOneFile && Clean") + _T(" ..."); printWarn(Mes);
			Clean = false;
		}

		if (Build)
		{
			Mes =  _("CompileOneFile && Build") + _T(" ..."); printWarn(Mes);
			// preCompile active file
			// bool elegible =
			m_pPrebuild->buildOneFile(prj, file);

			Build = false;
		}
	}

	m_removingIsFirst = true;

// switch last build log
	CodeBlocksLogEvent evtSwitch(cbEVT_SWITCH_TO_LOG_WINDOW, m_Lastlog);
    Manager::Get()->ProcessEvent(evtSwitch);

    if (!valid)
	{
	// call compiling stop
		compilingStop(idMenuKillProcess);
	}

/// DEBUG
//* ******************************************************
	m_pPrebuild->endDuration(_T("OnAddComplements(...)"));
//* ******************************************************
	Mes.Clear();
// The event processing system continues searching
	_event.Skip();
}

///-----------------------------------------------------------------------------
/// Unregister complement file for Qt
///	==> The file is already removed of the project
///
/// Called by
///		1. event 'cbEVT_PROJECT_FILE_REMOVED'
///
///		by project popup menu or menu
///		2. 'Remove file from project'
///		3. 'Remove files ...'
///		4. 'Remove  dir\\*'
///
///	Calls to
///		1. qtPre::isComplementFile(const wxString & _file):1,
///		2. qtPrebuild::unregisterComplementFile(wxString & _file, bool _first):1,
///		2. qtPre::isCreatorFile(const wxString & _file):1,
///		3. qtPrebuild::unregisterCreatorFile(wxString & _file, bool _first):1,
///
void QtPregen::onProjectFileRemoved(CodeBlocksEvent& _event)
{
/// just for debug
//Mes = _T("\nQtPregen::onProjectFileRemoved(...)");printError(Mes);
///
	// the project used
	cbProject *prj = _event.GetProject();
	if (!prj)
	{
		_event.Skip(); return; ;
	}
// it's not the current project !
	if ( m_pProject != prj)
	{
/// Debug
//	Mes = quote(prj->GetTitle()) + _T(": ") ;
//	Mes += _("event project is not the current project") + _T(" !!");  printError(Mes);
/// <==
		_event.Skip();	return;
	}

// actualize the project
	m_pProject = prj;
	// call  setBuildTarget(...)
	m_pPrebuild->setProject(prj);

	// the removed file ( absolute path )
	wxString filename = _event.GetString();
	if (filename.IsEmpty())
	{
		_event.Skip(); return;
	}

	// switch  'Prebuild' log
    SwitchToLog(m_LogPageIndex);
// only name less path
	wxString file = filename.AfterLast(Slash) ;
//Mes = Lf + _T("filename = ") + quote(filename);
//Mes += _T(",file =") + quote(file); print(Mes);
	bool ok = false;
	Mes = wxEmptyString;
// it's a complement file ?
	if (m_pPrebuild->isComplementFile(file))
	{
	// unregisterer one complement file
		ok = m_pPrebuild->unregisterComplementFile(filename,  m_removingIsFirst) ;
		if (!ok)
			Mes = Lf + _("Complement") + Space;
	}
	else
// it's a creator file ?
	if (m_pPrebuild->isCreatorFile(file))
	{
//Mes = _T("file =") + quote(file) + _("is a creator file"); print(Mes);
	// unregisterer one creator file and its complement file
		ok = m_pPrebuild->unregisterCreatorFile(filename, m_removingIsFirst) ;
		if (!ok)
			Mes = Lf + _("Creator") + Space;
	}
// it's an another file
	else
	{
		m_removingIsFirst = true;
		_event.Skip();	return;
	}

	if (!ok)
	{
		Mes += quote(filename) + _("file was NOT removed from project") + _T(" !!!") ;
		printError(Mes);
//Mes = Tab + quote(filename); print(Mes);
	}

	m_removingIsFirst = false;
	Mes.Clear();
/// The event processing system continues searching
	_event.Skip();
}

///-----------------------------------------------------------------------------
/// Validate messages to 'Prebuild log'
///
///	Called by : DISCONNECTED
///		1. event 'cbEVT_APP_STARTUP_DONE'
///
void QtPregen::OnAppDoneStartup(CodeBlocksEvent& _event)
{
	m_WithMessage = true;
	//m_initDone = true;
/// just for debug
//	Mes = _T("QtPregen::OnAppDoneStartup(...) ... ");
//	Mes +=  _("app done startup");
//	Mes += Space + _T("-> m_initDone = ") + strBool(m_initDone) ;
/// <=

	// ***********************************
	// this line hang code completion ??
	// ***********************************
	m_pProject = Manager::Get()->GetProjectManager()->GetActiveProject();
	if (m_pProject)
	{
	// pseudo event
	//	m_pseudoEvent = true;
	//	CodeBlocksEvent evt(cbEVT_PROJECT_ACTIVATE, 0, m_pProject, 0, this);
	//	OnActivateProject(evt);
	}

	if (m_pProject)
		Mes +=  Lf + Tab + _("Active project") + _T(" =") + quote(m_pProject->GetTitle() );
	else
		Mes +=  Lf + Tab + _("No active project") + _T("  !!");
	//printWarn(Mes) ;

	Mes.Clear();
/// The event processing system continues searching
	_event.Skip();
}

///-----------------------------------------------------------------------------
/// Open a project
///
/// Called by : DISCONNECTED
///		1. event 'cbEVT_PROJECT_OPEN'
///
/// Calls to : none
///

void QtPregen::OnOpenProject(CodeBlocksEvent& _event)
{
// wait for message validation
	if (!m_WithMessage)
	{
		_event.Skip(); return;
	}
/// Debug
//	Mes = NamePlugin + _T("::OnOpenProject(CodeBlocksEvent& event) -> ");
//	printError(Mes);
/// <=
// the active project
	cbProject *prj = _event.GetProject();
	if(!prj)
	{
		Mes += _("no project supplied") + _T(" !!"); printError(Mes);
		_event.Skip();
		return;
	}
/// DEBUG
//* *****************************************************
//	m_pPrebuild->beginDuration(_T("OnOpenProject(...)"));
//* *****************************************************
// active target
	wxString nametarget = prj->GetActiveBuildTarget();
	Mes += quote(prj->GetTitle()) ;
	if (nametarget.IsEmpty() )
	{
		Mes += Space + _("no target supplied") + _T(" !!"); printError(Mes);
		_event.Skip();
		return;
	}

// messages
	Mes = quote(prj->GetTitle() + _T("::") + nametarget);
	Mes += _("is opened") + _T(" !!"); printWarn(Mes);

/// The event processing system continues searching
	_event.Skip();

/// DEBUG
//* ***************************************************
//	m_pPrebuild->endDuration(_T("OnOpenProject(...)"));
//* ***************************************************
	Mes.Clear();
}

///-----------------------------------------------------------------------------
/// Save a project
///
/// Called by :
///		1. event 'cbEVT_PROJECT_SAVE'
///
/// Calls to : none
///
void QtPregen::OnSaveProject(CodeBlocksEvent& _event)
{
// wait for message validation
	if (!m_WithMessage)
	{
		_event.Skip(); return;
	}
/// just for debug
	Mes = NamePlugin + _T("::OnSaveProject(CodeBlocksEvent& event) -> ");
	//printWarn(Mes);
// the project
	cbProject *prj = _event.GetProject();
	if(!prj)
	{
		Mes += _("no project supplied") + _T(" !!"); printError(Mes);
		_event.Skip();
		return;
	}
/// messages
	Mes += quote(prj->GetTitle()) + _("is saved") + _T(" !!"); printWarn(Mes);

/// The event processing system continues searching
	_event.Skip();

/// DEBUG
//* ***************************************************
//	m_pPrebuild->endDuration(_T("OnSaveProject(...)"));
//* ***************************************************
	Mes.Clear();
}
///-----------------------------------------------------------------------------
/// Editor save a file
///
/// Called by :
///		1. event 'cbEVT_EDITOR_SAVE'
///
/// Calls to : none
///
void QtPregen::OnSaveFileEditor(CodeBlocksEvent& _event)
{
// wait for message validation
	if (!m_WithMessage)
	{
		_event.Skip(); return;
	}
// not editor
	if (!_event.GetEditor())
	{
		Mes += _("no editor supplied") + _T(" !!"); printError(Mes);
		_event.Skip(); return;
	}
/// Debug
//	Mes = NamePlugin + _T("::OnSaveFileEditor(CodeBlocksEvent& event) -> ");
//	printWarn(Mes);
/// <=
// the editor
	EditorManager * em = EditorManager::Get();
	cbEditor * ed = em->GetBuiltinEditor(_event.GetEditor());
	if(!ed)
	{
		Mes += _("no editor supplied") + _T(" !!"); printError(Mes);
		_event.Skip();
		return;
	}
	wxString filename = ed->GetFilename();

/// messages
	Mes += quote(ed->GetTitle()) + _("is saved") ;
	//Mes +=  quote(filename);
	Mes += _T(": ") + m_pPrebuild->date();
	printWarn(Mes);

/// The event processing system continues searching
	_event.Skip();

/// DEBUG
//* ******************************************************
//	m_pPrebuild->endDuration(_T("OnSaveFileEDitor(...)"));
//* ******************************************************
	Mes.Clear();
}

///-----------------------------------------------------------------------------
/// Close a project
///
/// Called by : DISCONNECTED !
///		1. event 'cbEVT_PROJECT_CLOSE'
///
/// Calls to : none
///
void QtPregen::OnCloseProject(CodeBlocksEvent& _event)
{
// wait for message validation
	if (!m_WithMessage)
	{
		_event.Skip(); return;
	}
/// Debug
//	Mes = NamePlugin + _T("::OnCloseProject(CodeBlocksEvent& event) -> ");
//	printWarn(Mes);
/// <==
// the project
	cbProject *prj = _event.GetProject();
	if(!prj)
	{
		Mes += _("no project supplied") + _T(" !!"); printError(Mes);
		_event.Skip();
		return;
	}
/// messages
	Mes = quote(prj->GetTitle()) + _("is closed") + _T(" !!"); printWarn(Mes);

/// The event processing system continues searching
	_event.Skip();

/// DEBUG
//* ****************************************************
//	m_pPrebuild->endDuration(_T("OnCloseProject(...)"));
//* ****************************************************
	Mes.Clear();
}
///-----------------------------------------------------------------------------
