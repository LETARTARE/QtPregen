/*************************************************************
 * Name:      qtpregen.cpp
 * Purpose:   Code::Blocks plugin
 * Author:    LETARTARE
 * Created:   2015-10-17
 * Modified:  2019-04-27
 * Copyright: LETARTARE
 * License:   GPL
 *************************************************************
 */
#include <sdk.h> 	// Code::Blocks SDK
// for linux
#include <cbproject.h>
#include <projectmanager.h>
// <-
/// for Parser class
#include <parser/parser_base.h>
#include <parser/parser.h>
/// for cbEditor class
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
//13- closing a project :
/// NOT USED HERE
	cbEventFunctor<QtPregen, CodeBlocksEvent>* functorCloseProject =
		new cbEventFunctor<QtPregen, CodeBlocksEvent>(this, &QtPregen::OnCloseProject);
	//pm->RegisterEventSink(cbEVT_PROJECT_CLOSE, functorCloseProject);
//13- save a project :
/// NOT USED HERE
	cbEventFunctor<QtPregen, CodeBlocksEvent>* functorSaveProject =
		new cbEventFunctor<QtPregen, CodeBlocksEvent>(this, &QtPregen::OnSaveProject);
	//pm->RegisterEventSink(cbEVT_PROJECT_SAVE, functorSaveProject);
//13-2 editor save a file
	/// NOT USED HERE
	cbEventFunctor<QtPregen, CodeBlocksEvent>* functorSaveFileEditor =
		new cbEventFunctor<QtPregen, CodeBlocksEvent>(this, &QtPregen::OnSaveFileEditor);
	// pm->RegisterEventSink(cbEVT_EDITOR_SAVE, functorSaveFileEditor);
//13.3
	/// NOT USED HERE
	 cbEventFunctor<QtPregen, CodeBlocksEvent>* functorParserEnd =
	     new cbEventFunctor<QtPregen, CodeBlocksEvent>(this, &QtPregen::OnParserEnd);
	 //pm->RegisterEventSink(wxEVT_COMMAND_MENU_SELECTED, functorParserEnd);

//14- construct a new log
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

//15- construct the builder
	// construct a new 'm_pPrebuild' m_pProject == nullptr
	m_pPrebuild = new qtPrebuild(NamePlugin, m_LogPageIndex);
	if (m_pPrebuild)
	{
		Mes = _T("sdk => ") + Quote + m_pPrebuild->GetVersionSDK() + Quote;
		print(Mes);
/// for debug : none activated project
/*
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
	    printError(Mes);
	///  release plugin
	    OnRelease(false);
	}
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
	Mes += Space + _T("-> m_initDone = ") + strBool(m_initDone) ;
	printWarn(Mes) ;
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
	Mes += Space + _T("m_initDone = ") + strBool(m_initDone) ;
	printWarn(Mes) ;
*/
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
//Mes = _T("QtPregen::OnAppBeginShutDown(...) ...");
//printWarn(Mes) ;
	m_WithMessage = false;

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
/// just for debug
/*
	Mes = NamePlugin + _T("::OnActivateProject(CodeBlocksEvent& event) -> ");
	if (m_pseudoEvent) 	Mes += _("local call : ");
	else 				Mes += _("app call : ");
	Mes += _("app init done -> ") + Space + strBool(m_initDone);
	printWarn(Mes);
*/
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
		Mes += _("no project supplied !!");
		printError(Mes);
		_event.Skip(!m_pseudoEvent);
		m_pseudoEvent = false;
		return;
	}

	Mes = wxEmptyString;
// actualize the project here
	m_pProject = prj;
/// DEBUG
//* *********************************************************
//	m_pPrebuild->beginDuration(_T("OnActivateProject(...)"));
//* *********************************************************

// detect Qt project ... with report : feed 'qtpre::m_pProject'
	m_isQtProject = m_pPrebuild->detectQtProject(prj, WITH_REPORT);
//Mes = _T("m_qtproject = ") + strBool(m_isQtProject);
//printWarn(Mes);
	// advice
	Mes = Quote + prj->GetTitle() + Quote + Space;
	if (m_isQtProject)	Mes += _("has at least one target using Qt libraries...");
	else				Mes += _("is NOT a Qt project !!");
	printWarn(Mes);
	if (!m_isQtProject)
	{
		_event.Skip(!m_pseudoEvent);
		m_pseudoEvent = false ;
		return;
	}

/// detect if active target is QT or has a Qt target
	wxString nametarget = m_pProject->GetActiveBuildTarget();
	m_isQtActiveTarget = adviceTypeTarget(nametarget);

/// complements exists already ?
//	if (m_isQtActiveTarget)
	{
		m_pPrebuild->detectComplementsOnDisk(m_pProject, nametarget, WITH_REPORT);
		m_removingIsFirst = true;
	}
/// The event processing system continues searching if not a pseudo event
	_event.Skip(!m_pseudoEvent);
	m_pseudoEvent = false ;

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
	Mes = wxEmptyString;
/// just for debug
	//Mes = NamePlugin + _T("::OnActivateTarget(CodeBlocksEvent& _event) -> ");

// the active project
	cbProject *prj = _event.GetProject();
	if(!prj)
	{
		Mes += _("no project supplied !!");
		printError(Mes);
		_event.Skip();
		return;
	}
/// is it possible ??
// it's not the current project !
	if ( m_pProject != prj)
	{
/// just for debug
	//	Mes += Quote + prj->GetTitle() + Quote + _T(" : ") ;
	//	Mes += _("event project is not the current project !!");
	//	printWarn(Mes);
/// <=
		_event.Skip();
		return;
	}

// the active target
	wxString nametarget  =  _event.GetBuildTargetName() ;
	if (nametarget.IsEmpty() )
	{
	/// test if the project is open
		if (Manager::Get()->GetProjectManager()->IsProjectStillOpen(m_pProject))
		{
			Mes += _("!! no target supplied !!");
			printError(Mes);
		}
		_event.Skip();
		return;
	}
	// an advice
	m_isQtActiveTarget = adviceTypeTarget(nametarget);

/// The event processing system continues searching
	_event.Skip();
}

///-----------------------------------------------------------------------------
/// Give if a target is or has a type Qt target
///
/// Called by :
///		1. QtPregen::OnActivateProject(CodeBlocksEvent& _event):1,
///		2. QtPregen::OnActivateTarget(CodeBlocksEvent& _event):1,
///
/// Calls to :
///		1. qtprebuild::detectQtTarget(const wxString& _nametarget, cbProject * _pProject);
///
bool QtPregen::adviceTypeTarget(const wxString & _nameTarget)
{
 	// reals targets
	wxArrayString realsTargets;
	bool isQtTarget ;
// is a virtual target ?
	Mes = Tab + Quote + _T("::") + _nameTarget + Quote + Space;
	bool virtualtarget = m_pProject->HasVirtualBuildTarget(_nameTarget);
	if (virtualtarget)
	{
		Mes += _("(virtual target)");
		// search all real targets
		realsTargets =  m_pProject->GetExpandedVirtualBuildTargetGroup(_nameTarget);
		printWarn(Mes);
	}
	// is one real target
	else	realsTargets.Add(_nameTarget);

// all real targets ...
	for (wxString target : realsTargets)
	{
		if (virtualtarget)
			Mes =  Tab + Tab + _T(" => ") + Quote + _T("::") + target + Quote + Space;
	// detect Qt target
		isQtTarget= m_pPrebuild->detectQtTarget(target, m_pProject) ;
		// advices
		Mes += _("is") + Space;
		if(!isQtTarget)	Mes += _("NOT") + Space;
		Mes += _("a Qt target.");
		print(Mes);
	}

	return isQtTarget;
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
	Mes = wxEmptyString;
/// just for debug
	// Mes = NamePlugin + _T("::OnNewProject(CodeBlocksEvent& event) -> ");

// the new project
	cbProject *prj = _event.GetProject();
	if(!prj)
	{
		Mes += _("no project supplied !!");
		printError(Mes);
		_event.Skip();
		return;
	}

// actualize the project
	m_pProject = prj;

// just project created ?
	wxString nametarget = prj->GetActiveBuildTarget();
	if (nametarget.IsEmpty() )
	{
		Mes += _("no target supplied !!");
		printError(Mes);
		_event.Skip();
		return;
	}
//Mes = _T("project name ") + Quote + prj->GetTitle() + Quote ;
//Mes +=  Space + _T("nametarget = ") + Quote + nametarget + Quote ;
//printWarn(Mes);

// detect Qt project ... with report
	m_isQtProject = m_pPrebuild->detectQtProject(prj, WITH_REPORT);
//Mes = _T("m_isQtProject = ") + strBool(m_isQtProject);
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
	_event.Skip();
}

///-----------------------------------------------------------------------------
/// Receive a renaming of a project or a target
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
//Mes = NamePlugin + _T("::OnRenameProjectOrTarget(CodeBlocksEvent& event) -> ");
//printWarn(Mes);
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
/// just for debug
	//	Mes = Quote + prj->GetTitle() + Quote + _T(" : ") ;
	//	Mes += _("event project is not the current project !!");
	//	printError(Mes);
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

// the name target :  new name ?
	wxString targetname = _event.GetBuildTargetName()
			,oldtargetname = _event.GetOldBuildTargetName()
			;
//Mes  = _T("!!! nameProject =") + Quote + prj->GetTitle() + Quote;
//Mes += _T(", newtarget =") + Quote + targetname + Quote;
//Mes += _T(", oldtarget =") + Quote + oldntargetname + Quote;
//printWarn(Mes);
	bool okqt = false;
/// //////////////////////////
/// it's a new name project
/// //////////////////////////
	if (targetname.IsEmpty() && oldtargetname.IsEmpty())
	{
		// detect Qt project ... with report
		m_isQtProject = m_pPrebuild->detectQtProject(prj, WITH_REPORT);
//Mes = _T("==> m_isQtProject = ") + strBool(m_isQtProject);
//printWarn(Mes);
		if (!m_isQtProject)
		{
			_event.Skip();
			return;
		}
		Mes = _T("===") + Space + _("A new name project") ;
		Mes += Space + Quote + prj->GetTitle() + Quote;
		printWarn(Mes);
		Mes = Quote + prj->GetTitle() + Quote;
		if (m_isQtProject)
		{ // advice
			Mes += Space + _("has at least one target using Qt libraries...");
			printWarn(Mes);
			// complements exists already ?
			m_pPrebuild->detectComplementsOnDisk(prj, targetname, WITH_REPORT);
			// init
			m_removingIsFirst = true;
		}
	}
/// ////////////////////////
/// it's a new name target
/// ////////////////////////
	else
	if (!targetname.IsEmpty() )
	{
		m_pProject = prj;
	// verify target
		wxString actualtarget = prj->GetActiveBuildTarget() ;
	Mes = _T("!! actualtarget = ") + Quote + actualtarget + Quote;;
		// update m_pProject, ..., m_pBuildTarget
		m_pPrebuild->setProject(prj);

	// new active target ?
		wxString activetarget = prj->GetActiveBuildTarget() ;
	Mes += _T("!! activetarget = ") + Quote + activetarget + Quote;
	Mes += _T(", targetname = ") + Quote + targetname + Quote;;
	printWarn(Mes);

		// advice
		Mes = _T("===") + Space + _("Old target name") + Space ;
		Mes += Quote + oldtargetname + Quote + _T(" => ") ;
		Mes += _("New target name") + Space + Quote + targetname+ Quote;
		printWarn(Mes);

		if (!activetarget.Matches(targetname))
		{
		// detect Qt target ...
			okqt = m_pPrebuild->detectQtTarget(targetname, prj);
			Mes = Quote + _T(":: ") + targetname + Quote + Space;
			if(okqt)	Mes += _("is a Qt target...");
			else		Mes += _("is NOT a Qt target !!");
		}
	/// updates of the new target :
		// new build target
		ProjectBuildTarget * pBuildTarget = prj->GetBuildTarget(targetname);
		bool ok = m_pPrebuild->updateNewTargetName(pBuildTarget, oldtargetname);
		/// ... TO FINISH

	}
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
//Mes = _T("QtPregen::compilingStop(") + strInt(_idAbort) + _T(")");
//printError(Mes);
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
///		1. event 'cbEVT_COMPILER_FINISH' from 'CompilerGCC::AbortPreBuild()' with
///	       bouton-menu
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

// the active project
	cbProject *prj = _event.GetProject();
	// no project !!
	if(!prj)
	{
		Mes += _("no project supplied");
		printError(Mes);
		_event.Skip();
		return ;
	}
/// just for debug
	if (m_pProject != prj)
	{
	//	Mes += _("event project target is not the current project !!");
	//	printError(Mes);
		_event.Skip();
		return;
	}
// Abort complement files creating
	m_pPrebuild->setAbort();
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
///		-# qtPre::detectComplementsOnDisk(cbProject * _pProject, const wxString & _nametarget,  bool _report):1,
///
void QtPregen::OnAddComplements(CodeBlocksEvent& _event)
{
//Mes = _T("QtPregen::OnAddComplements(...)");
//printWarn(Mes);
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

/// DEBUG
//* **********************************
//	m_pPrebuild->beginDuration(_T("OnAddComplements(...)"));
//* *********************************
//  debug
	// _event.Getint() == 0 ?
//Mes = Lf + _T("Call 'OnAddComplements' ...");
//Mes += _T(" eventId = ") + strInt(_event.GetId());
//Mes += _T(", eventInt = ") + strInt(_event.GetInt());
//Mes += _T(", target name = ") + _event.GetBuildTargetName();
//printWarn(Mes);

	Mes = NamePlugin + _T("::OnAddComplements(CodeBlocksEvent& event) -> ");

// the active project
	cbProject * prj = m_pProject ; //event.GetProject();
	// no project !!
	if(!prj )
	{
		Mes += _("no project supplied");
		printError(Mes);
		_event.Skip();
		return ;
	}

// just project created ?
	wxString nametarget = prj->GetActiveBuildTarget();
	if (nametarget.IsEmpty() )
	{
		Mes += _("no target supplied !!");
		printError(Mes);
		_event.Skip();
		return;
	}

//Mes = _T("project name ") + Quote + prj->GetTitle() + Quote ;
//Mes +=  Space + _T("nametarget = ") + Quote + nametarget + Quote ;
//printWarn(Mes);

// detect Qt project, no report
	bool valid = m_pPrebuild->detectQtProject(prj, NO_REPORT);
	// not Qt project
	if (! valid)
	{
		_event.Skip(); return;
	}
// allright !
	// complements exists already ?
	m_pPrebuild->detectComplementsOnDisk(prj, nametarget, NO_REPORT);
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
///
  //debug
//Mes = Lf + _T("Call 'OnAddComplements' ...");
//Mes += _T(" eventId = ") + strInt(eventId) ;
//Mes += _T(" eventInt = ") + strInt(eventInt);
//printWarn(Mes);

//Mes =  _T(", CompileOneFile = ") ;
//Mes += CompileOneFile ? _T("true -> '") + file + Quote : _T("false");
//Mes += _T(", CompileAllFiles = ") + strBool(CompileAllFiles);
//printWarn(Mes);
///
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
  //debug
/*
Mes = _T(" FBuild = ") + strBool(FBuild);
Mes += _T(", Build(11) = ") + strBool(Build);
Mes += _T(", Clean(12) = ") + strBool(Clean);
Mes += _T(", Rebuild(13) = ") + strBool(Rebuild);
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
		idMenuKillProcess = _event.GetX();
//Mes = Lf + _T("Call 'OnAddComplements' ...");
//Mes += _T(" eventX = ") + strInt(idMenuKillProcess);
//printWarn(Mes);

		if (Build)
		{
		// real target
            wxString targetname = _event.GetBuildTargetName() ;
			prj->SetActiveBuildTarget(targetname);
			ProjectBuildTarget * pBuildTarget = prj->GetBuildTarget(targetname);
			// verify Qt target
			bool ok = m_pPrebuild->isGoodTargetQt(pBuildTarget);
			if (ok)
            {
			// only preBuild when date complements < date creator
				ok = m_pPrebuild->buildAllFiles(prj, WsBuild, NO_ALLBUILD);
                if (! ok)
                {
                //	Mes = Tab + _T("m_pPrebuild->buildAllFiles(...) => ");
                //  Mes += _("Error 'PreBuild' !!!");
                //  printError(Mes);
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
			printError(Mes);
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
//* **********************************
//	m_pPrebuild->endDuration(_T("OnAddComplements(...)"));
//* *********************************

// The event processing system continues searching
	_event.Skip();
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
///		4. 'Remove  dir\\*'
///
///	Calls to
///		1. qtPre::isComplementFile(const wxString & _file):1,
///		2. qtPre::isCreatorFile(const wxString & _file):1,
///		3. qtPrebuild::unregisterProjectFile(const wxString & _file, bool _complement, bool _first):1,
///
void QtPregen::onProjectFileRemoved(CodeBlocksEvent& _event)
{
//Mes = _T("\nQtPregen::onProjectFileRemoved(...)");
//printWarn(Mes);
	// the project used
	cbProject *prj = _event.GetProject();
	if (!prj)
	{
		_event.Skip(); return; ;
	}
// it's not the current project !
	if ( m_pProject != prj)
	{
/// just for debug
//	Mes = Quote + prj->GetTitle() + Quote + _T(" : ") ;
//	Mes += _("event project is not the current project !!");
//	printError(Mes);
		_event.Skip();	return;
	}

// actualize the project
	m_pProject = prj;
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
//Mes = _T("filename = ") + Quote + filename + Quote;
//Mes += _T(" ,file = ") + Quote + file  + Quote;
//print(Mes);
	bool ok = false;
	Mes = Lf;
// it's a complement file ?
	if (m_pPrebuild->isComplementFile(file))
	{
	// unregisterer one complement file
		ok = m_pPrebuild->unregisterFileComplement(filename,  m_removingIsFirst) ;
		if (!ok)
			Mes += _("Complement ") + Quote + file + Quote;;
	}
	else
// it's a creator file ?
	if (m_pPrebuild->isCreatorFile(file))
	{
	//Mes = _T("file = ") + Quote + file + Quote + _T(" is a creator file");
	//print(Mes);
	// unregisterer one creator file and its complement file
		ok = m_pPrebuild->unregisterFileCreator(filename, m_removingIsFirst) ;
		if (!ok)
			Mes =  _("Creator ") + Quote + file + Quote;
	}
// it's an another file
	else
	{
		m_removingIsFirst = true;
		_event.Skip();	return;
	}

	if (!ok)
	{
		Mes += Space + _(" file was NOT removed from project !!!") ;
		printError(Mes);
		Mes = Tab + Quote + filename + Quote;
		print(Mes);
	}

	m_removingIsFirst = false;

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
	Mes = _T("QtPregen::OnAppDoneStartup(...) ... ");
	Mes +=  _("app done startup");
	Mes += Space + _T("-> m_initDone = ") + strBool(m_initDone) ;

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
		Mes +=  Lf + Tab + _T("Active project = ") + Quote + m_pProject->GetTitle() + Quote ;
	else
		Mes +=  Lf + Tab + _T("No active project !!");
	//printWarn(Mes) ;

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
/// just for debug
	Mes = NamePlugin + _T("::OnOpenProject(CodeBlocksEvent& event) -> ");
	printWarn(Mes);
// the active project
	cbProject *prj = _event.GetProject();
	if(!prj)
	{
		Mes += _("no project supplied !!");
		printError(Mes);
		_event.Skip();
		return;
	}
/// DEBUG
//* *****************************************************
//	m_pPrebuild->beginDuration(_T("OnOpenProject(...)"));
//* *****************************************************
// active target
	wxString nametarget = prj->GetActiveBuildTarget();
	Mes += Quote + prj->GetTitle() ;
	if (nametarget.IsEmpty() )
	{
		Mes += Quote + Space + _("no target supplied !!");
		printError(Mes);
		_event.Skip();
		return;
	}

// messages
	Mes = Quote + prj->GetTitle() + _T("::") ;
	Mes += nametarget + Quote + Space + _("is opened !!");
	printWarn(Mes);

/// The event processing system continues searching
	_event.Skip();

/// DEBUG
//* **********************************
//	m_pPrebuild->endDuration(_T("OnOpenProject(...)"));
//* *********************************
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
		Mes += _("no project supplied !!");
		printError(Mes);
		_event.Skip();
		return;
	}
/// messages
	Mes += Quote + prj->GetTitle() + Quote + Space + _("is saved !!");
	printWarn(Mes);

/// The event processing system continues searching
	_event.Skip();

/// DEBUG
//* **********************************
//	m_pPrebuild->endDuration(_T("OnSaveProject(...)"));
//* *********************************
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
		Mes += _("no editor supplied !!");
		printError(Mes);
		_event.Skip(); return;
	}
/// just for debug
	Mes = NamePlugin + _T("::OnSaveFileEditor(CodeBlocksEvent& event) -> ");
//	printWarn(Mes);
// the editor
	EditorManager * em = EditorManager::Get();
	cbEditor * ed = em->GetBuiltinEditor(_event.GetEditor());
	if(!ed)
	{
		Mes += _("no editor supplied !!");
		printError(Mes);
		_event.Skip();
		return;
	}
	wxString filename = ed->GetFilename();

/// messages
	Mes += Quote + ed->GetTitle() + Quote + Space + _(" is saved") ;
	//Mes += Space + Quote + filename + Quote;
	printWarn(Mes);

/// The event processing system continues searching
	_event.Skip();

/// DEBUG
//* **********************************
//	m_pPrebuild->endDuration(_T("OnSaveFileEDitor(...)"));
//* *********************************
}

///-----------------------------------------------------------------------------
/// Parser End
///
/// Called by :
///		1. event 'wxEVT_COMMAND_MENU_SELECTED'
///
/// Calls to : none
///
void QtPregen::OnParserEnd(CodeBlocksEvent& _event)
{
	Mes = NamePlugin + _T("::OnParserEnd(CodeBlocksEvent& event)");
	printError(Mes);

	ParserBase* parser = reinterpret_cast<ParserBase*>(_event.GetEventObject());
    cbProject* project = static_cast<cbProject*>(_event.GetClientData());
    wxString prj = (project ? project->GetTitle() : _T("*NONE*"));
    const ParserCommon::ParserState state = static_cast<ParserCommon::ParserState>(_event.GetInt());
    wxString job = _event.GetString();
    // job is finish
    if (state == ParserCommon::ptCreateParser)
	{
		Mes = NamePlugin + _T("::OnParserEnd(CodeBlocksEvent& event) -> ");
		Mes += job + Space + _("in project") + Space + Quote + prj + Quote;
		print(Mes);
	}

/// The event processing system continues searching
	_event.Skip();
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
/// just for debug
	Mes = NamePlugin + _T("::OnCloseProject(CodeBlocksEvent& event) -> ");
	printWarn(Mes);
// the project
	cbProject *prj = _event.GetProject();
	if(!prj)
	{
		Mes += _("no project supplied !!");
		printError(Mes);
		_event.Skip();
		return;
	}
/// messages
	Mes = Quote + prj->GetTitle() + Quote + Space + _("is closed !!");
	printWarn(Mes);

/// The event processing system continues searching
	_event.Skip();

/// DEBUG
//* **********************************
//	m_pPrebuild->endDuration(_T("OnCloseProject(...)"));
//* *********************************
}
///-----------------------------------------------------------------------------
