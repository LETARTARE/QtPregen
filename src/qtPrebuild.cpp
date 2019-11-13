/***************************************************************
 * Name:      qtprebuild.cpp
 * Purpose:   Code::Blocks plugin
 * Author:    LETARTARE
 * Created:   2015-10-17
 * Modified:  2019-11-10
 * Copyright: LETARTARE
 * License:   GPL
 **************************************************************/
#include "qtprebuild.h"

#include <manager.h>
#include <cbproject.h>
#include <compiletargetbase.h>
#include <projectmanager.h>
#include <macrosmanager.h>
#include <editormanager.h>
#include <cbeditor.h>
#include <wx/datetime.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <projectfile.h>
// not change  !
#include "defines.h"
///-----------------------------------------------------------------------------
/// Constuctor

/// Called by :
///		1. QtPregen::OnAttach():1,
///
qtPrebuild::qtPrebuild(wxString & _namePlugin, int _logIndex)
	: qtPre(_namePlugin, _logIndex)
{
	/// !! not use  '+ _T(Slash)'   !!  because Slash = wxT(...)
	m_dirPreBuild = _T("adding") + wxString(Slash) ;
}
///-----------------------------------------------------------------------------
/// Destructor

/// Called by :
///		1. QtPregen::OnRelease():1,
///
qtPrebuild::~qtPrebuild()
{
	m_pProject = nullptr;  m_pMam = nullptr;
}

///-----------------------------------------------------------------------------
///  Build create start banner
///
/// Called by :
///		1.buildAllFiles(cbProject * _pProject, bool _workspace, bool _allbuild):1,
/// Calls to :
///		1. date():1,
///
void qtPrebuild::beginMesBuildCreate()
{
// begin
	if(!m_pProject)
		return;

// base directory
	m_dirProject = m_pProject->GetBasePath() ;
// directory changed
	wxFileName::SetCwd (m_dirProject);
	bool ok = createDir (m_dirPreBuild) ;
	if (!ok)
		return ;

	m_nameProject = m_pProject->GetTitle();
	m_nameActiveTarget = m_pProject->GetActiveBuildTarget();
	// for test
	//m_pBuildTarget = m_pProject->GetBuildTarget(m_nameActiveTarget);

// display log
	Mes = _T("-------------- ") ;
	Mes += _T("PreBuild :") ;
	Mes += quote( m_nameActiveTarget );
	Mes += _("in");
	Mes += quote( m_nameProject );
	Mes += _T("-------------- ") ;
	printWarn(Mes);
// date
	Mes = _("started on");
	Mes += _T(" : ") + date() ;
	print(Mes);
// for duration
	m_start = clock();
	Mes.Clear();
}

///-----------------------------------------------------------------------------
///  Build create end banner
///
/// Called by :
///		1. buildAllFiles(cbProject * _pProject, bool _workspace, bool _allbuild):1,
/// Calls to :
///		1. date():1,
///		2. duration():1,
///
void qtPrebuild::endMesBuildCreate()
{
// date and duration
	Mes = _("ended on") ;
	Mes += _T(" : ") + date() ;
	print(Mes) ;
	Mes =  _("duration") ;
	Mes += _T(" = ") + duration() ;
	printWarn(Mes);
	Mes.Clear();
}
///-----------------------------------------------------------------------------
/// Generating the complements files...
///
/// Called by :
///		1. QtPregen::OnAddComplements(CodeBlocksEvent& _event):1
///
/// Calls to :
///		1. beginMesBuildCreate():1,
///		2. findGoodfiles():1,
///		3. addAllFiles():1,
///		4. filesTocreate(bool _allrebuild) :1,
///		5. createFiles():1,
///		6. validCreated():1,
///		7. endMesBuildCreate():1,
///
bool qtPrebuild::buildAllFiles(cbProject * _pProject, bool _workspace, bool _allbuild)
{
	m_pProject = _pProject;
	bool ok = m_pProject != nullptr;
	if (!ok)
		return ok;

/// DEBUG
//* ****************************************
//	beginDuration(_T("buildAllFiles(...)"));
//* ****************************************
// begin banner
	beginMesBuildCreate();

	ok = false;
	m_clean = false;
	///***************************************
	///1- find good target with eligible files
	///***************************************
	// analyzing all project files
	Mes = _("Search creator file(s)") ;
	Mes += _T(" ...") ;
	printWarn(Mes);
	int nelegible =  findGoodfiles() ;
	if (nelegible > 0)
	{
		///******************************************
		///2-  registered all files in 'm_Registered'
		///******************************************
		int nfiles = addAllFiles() ;
		Mes = Tab + _T("-> ") + strInt(nfiles) + Space ;
		Mes += _("creator(s) file(s) registered in the plugin") ;
		print(Mes) ;
		if (nfiles > 0)
		{
			///****************************************
			///3-  files to create to 'm_Filestocreate'
			///****************************************
			uint16_t ntocreate = filesTocreate(_allbuild) ;
			if (ntocreate == 0 )
				ok = !_allbuild ;
			else
				ok = true;

			if (ok)
			{
				///**********************************************************
				///4- create files
				/// 	from 'FileCreator', 'm_Registered'
				/// 	1- adds files created in 'm_Createdfile'
				/// 	2- create additional files as needed  by 'create(qexe, fileori)'
				///***********************************************************
				Mes = _("Creating complement file(s)") ;
				Mes += _T(" ...") ;
				printWarn(Mes);
				bool good = createFiles() ;
				if (good)
				{
					///**************************
					///5- register files created
					///**************************
					good = validCreated() ;
					if (!good)
					{
						Mes = Tab + _("No file to create.") ;
						printWarn(Mes);

					}
					ok = true;
				} // end (good)
				else
				{
					if (m_abort)
					{
						Mes = Tab + _("Abort complement file(s) building") + _T(" ...") ;
						printWarn(Mes);
						m_abort = false;
					}
					else
					{
						Mes = Tab + _("Complement file creation error") + _T(" !!") ;
						printError(Mes);
					}
				}
				ok = good;
			}	//end (ntocreate > 0)
			else
			{
				Mes = Tab + _("File to create error") + _T(" !!") ;
				printError(Mes);
			}
		} // end (nfiles > 0)
		else
		{
			Mes = Tab + _("File registration error") + _T(" !!") ;
			printError(Mes);
		}
	} // end (nelegible > 0)
	else
	{
		Mes = Tab + _("No elegible file (with 'Q_OBJECT' or 'Q_GADGET')") + _T(" !!") ;
		printWarn(Mes);
		ok = true;
	}
/// debug
//print(allStrTable(_T("m_Registered"), m_Registered));


    m_clean = m_Registered.IsEmpty();
// end banner
	endMesBuildCreate();

/// DEBUG
//* **************************************
//	endDuration(_T("buildAllFiles(...)"));
//* **************************************
	Mes.Clear();
	return ok;
}
///-----------------------------------------------------------------------------
///  Build create one file start banner
///
/// Called by :
///		1. buildOneFile(cbProject * _pProject, const wxString& _fcreator) :1,
/// Calls to :
///		1. date():1,
///
void qtPrebuild::beginMesFileCreate()
{
// banner
	printLn;
	Mes = _T("-------------- ") ;
	Mes += _T("PreCompileFile :") ;
	Mes += quote( m_pProject->GetActiveBuildTarget() );
	Mes += _("in");
	Mes += quote( m_pProject->GetTitle() );
	Mes += _T(" :") + quote( m_filename );
	Mes += _T("-------------- ") ;
	printWarn(Mes);
// date
	Mes = _("started on : ") + date() ;
	print(Mes);
// for duration
	m_start = clock();
	Mes.Clear();
}
///-----------------------------------------------------------------------------
///  Build create one file end banner
///
/// Called by :
///		1. buildOneFile(cbProject * _pProject, const wxString& _fcreator) :1,
/// Calls to :
///		1. date():1,
///		2. duration():1,
///
void qtPrebuild::endMesFileCreate()
{
// date and duration
	Mes = _("ended on : ") + date() ;
	print (Mes) ;
	Mes =  _("duration = ") + duration() ;
	printWarn(Mes);
	Mes.Clear();
}
///-----------------------------------------------------------------------------
/// Generating only ONE complement file...
///
///  Called by :
///		1. QtPregen::OnAddComplements(CodeBlocksEvent& _event):1,
///
///  Calls to :
///		1.	beginMesFileCreate():1,
///		2.	isElegible(const wxString& _file):1,
///		3.	inProjectFileCB(const wxString& _file):1,
///		4.	compareDate(const wxString&  _fileref, const wxString&  _filetarget):1
///		5.  createFileComplement(const wxString& _qexe, const wxString& _fcreator, const wxString& _fout):1,
///		6.	endMesFileCreate():1,
///

bool qtPrebuild::buildOneFile(cbProject * _pProject, const wxString& _fcreator)
{
	if(!_pProject)
		return false;

	m_pProject = _pProject;
	m_filename = _fcreator;

// directory changed
	m_dirProject = m_pProject->GetBasePath();
	wxFileName::SetCwd (m_dirProject);
	bool ok = createDir (m_dirPreBuild) ;
	if (!ok)
		return ok;

// file it is correct ?
    bool elegible = isElegible(_fcreator);
    if (!elegible)
		return elegible ;

// begin pre-Compile
	beginMesFileCreate() ;

// init Qt tools
	ok = findTargetQtexe(_pProject) ;
	if (!ok)
		return ok ;

	m_nameActiveTarget =  m_pProject->GetActiveBuildTarget() ;
	/*  TO REVIEW ...
	// targets no virtual list
	wxArrayString rltargets;
// virtual target ?  without warning
	bool virt = m_pProject->HasVirtualBuildTarget(m_nameActiveTarget);
	if (virt)
		rltargets = compileableVirtualTargets(m_nameActiveTarget);
	else
		rltargets.Add(m_nameActiveTarget);

	uint16_t ntargets = rltargets.GetCount();
	ProjectBuildTarget* pBuildTarget;
	wxString fout;
    for (uint16_t nt = 0 ; nt < ntargets ; nt++ )
    {
		m_nameActiveTarget = rltargets.Item(nt);
		// all targets are reals
		pBuildTarget = m_pProject->GetBuildTarget(m_nameActiveTarget);
		if (isCommandTarget(pBuildTarget) ) 	continue;

		fout = complementDirectory() + nameCreated(_fcreator) ;
		Mes = Tab + quote( _fcreator + quote( _T(" -> ") ) + fout ) ;
		print(Mes);
	// already registered
		bool inproject = inProjectFileCB(fout) ;
		if (!inproject)
		{
		// has included ?
			wxString extin = _fcreator.AfterLast('.') ;
			wxString extout = fout.AfterLast('.')  ;
			bool nocompile = false ;
			if ( extin.Matches(EXT_H) && extout.Matches(EXT_CPP) )
				nocompile =  hasIncluded(_fcreator) ;
		// add file : AddFile(Nametarget, file, compile, link, weight)
			ProjectFile* prjfile = m_pProject->AddFile(m_nameActiveTarget, fout, !nocompile, !nocompile, 50);
			if (!prjfile)
			{
			// display message
				Mes  = _T("===> ") ;
				Mes += _("can not add this file ");
				Mes += quote( fout ) + _(" to target ") + m_nameActiveTarget ;
				printError (Mes) ;
				cbMessageBox(Mes, _T("AddFile(...)"), wxICON_ERROR) ;
			}
		// display
			if (nocompile)
			{
				Mes = Tab + _("Add ") + quote( fout ) ;
				Mes += Lf + Tab + Tab + _T("*** ") + _("This file is included, ") ;
				Mes += _("attributes 'compile' and 'link' will be set to 'false'") ;
				print(Mes);
			}
			// svn 9501 : CB 13.12  and >// verify if complements exists already
			Manager::Get()->GetProjectManager()->GetUI().RebuildTree() ;

		// create complement ...
			m_Identical = false;
		}
		else
		{
		// Check the date
			m_Identical = false ;
			//- verify datation on disk
			if (::wxFileExists(fout))
				m_Identical = compareDate(_fcreator, fout) ;
		}

		if (m_Identical)
		{
			Mes = Tab + _("Nothing to do (complement file exist on disk)") ;
			print(Mes) ;
		}
	// ! identical date -> create
		else
		{
			Mes = Tab + _(" One complement file is created in the project ...") ;
			printWarn(Mes);
		// create file complement with 'moc'
			wxString strerror = createFileComplement(m_Mexe, _fcreator, fout);
			if (!strerror.IsEmpty())
			{
			// error message
				wxString title = _("Creating ") + quote( fout ) ;
				title += _(" failed") ;
				title += _T(" ...");
				//1- error create directory  :
					// = _("Unable to create directory ")
				//2- error buildtarget no exist :
					// = quote( m_nameActiveTarget ) + _(" does not exist !!") ;
				//3- error create complement
				Mes =  _T("=> ") ;
				Mes += strerror.BeforeLast(Lf.GetChar(0)) ;
				printError (Mes) ;
				cbMessageBox(Mes, title) ;
			}
		}
	}
*/
// end preCompile
	endMesFileCreate() ;

	Mes.Clear();

    return elegible;
}

///-----------------------------------------------------------------------------
/// Find path Qt container  ->  'CompileOptionsBase * container'
///
/// Called by  :
///		1. findTargetQtexe(CompileTargetBase * _buildtarget):1,
///
wxString qtPrebuild::pathlibQt(CompileTargetBase * _pContainer)
{
	wxString path;
	if (!_pContainer)
		return path;

	wxArrayString tablibdirs = _pContainer->GetLibDirs() ;
	int npath = tablibdirs.GetCount() ;
//Mes = _T("path lib number = ") + strInt(npath); printError(Mes);
	if (!npath)
		return path;

	wxString  path_nomacro ;
	if (npath > 0 )
	{
		bool ok = false ;
		int u = 0  ;
		while (u < npath && !ok )
		{
			path = tablibdirs.Item(u++);
			path.Lower();
			// used local variable ?
			ok = path.Find(_T("$qt")) != -1 ;
			if (ok)
			{
//Mes = Lf + _("$qt => ") + quote( path ); printWarn(Mes);
				m_pMam->ReplaceEnvVars(path) ;
//Mes = Lf + _("Local variable path Qt => '") + path; Mes += _T("'"); print(Mes);
				path_nomacro =  path ;
				// remove "\lib"
				path_nomacro = path_nomacro.BeforeLast(Slash) ;
				path_nomacro += wxString(Slash)  ;
			}
			else
			{  // use global variable ?
				ok = path.Find(_T("#qt")) != -1 ;
				if (ok)
				{
//	debug qt
//Mes = Lf + _("#qt => ") + quote( path ); printWarn(Mes);
					m_pMam->ReplaceMacros(path) ;
//Mes = Lf + _("Global variable path Qt => '") + path;Mes += _T("'"); print(Mes);
					path_nomacro =  path ;
					// remove "\lib"
					path_nomacro = path_nomacro.BeforeLast(Slash) ;
					path_nomacro += wxString(Slash)  ;
				}
				// no variable ! , absolute path ?
				else
				{
//Mes = Lf + _("Path Qt => '") + path ; Mes += _T("'");	printWarn(Mes);
					path_nomacro += path;
				}
			}
		}
	}
	Mes.Clear();

	return path_nomacro  ;
}

///-----------------------------------------------------------------------------
/// Find path Qt exe : 'ProjectBuildTarget* _pBuildTarget'
///
/// Called by  :
///		1.	createFiles():1
///		2.	buildOneFile(cbProject * _pProject, const wwxFileExistsxString& _fcreator):1
///

bool qtPrebuild::findTargetQtexe(CompileTargetBase * _pBuildTarget)
{
	if (! _pBuildTarget)
		return false  ;

	wxString qtpath = pathlibQt(_pBuildTarget) ;
// Mes = Lf + _("QT path for the target = ") + quote(qtpath); print(Mes);
	if(qtpath.IsEmpty() || qtpath == _T("\\") )
	{
		Mes = _("No library path QT 'in the target") + Lf + _("or nothing library !")  ;
		Mes += Lf +  _("Check that you have a correct path to the Qt path libraries !") ;
		Mes += Lf + _("Cannot continue.") ;
		printError(Mes);
	//	cbMessageBox(Mes, _T(""), wxICON_ERROR) ;
		return false ;
	}

	wxString qtexe = qtpath + _T("bin") + wxFILE_SEP_PATH  ;
//	Mes = _T("qtexe = ") + quote(qtexe) ;  printWarn(Mes);
	if (m_Win)
	{
		m_Mexe = qtexe + _T("moc.exe") ;
		m_Uexe = qtexe + _T("uic.exe") ;
		m_Rexe = qtexe + _T("rcc.exe") ;
		// compile *_fr.ts -> *_fr.qm
		m_Lexe = qtexe + _T("lrelease.exe") ;
	}
	else
	if (m_Linux)
	{	// check !
		m_Mexe = qtexe + _T("moc") ;
		m_Uexe = qtexe + _T("uic") ;
		m_Rexe = qtexe + _T("rcc") ;
		m_Lexe = qtexe + _T("lrelease") ;
	}
	else
	if (m_Mac)
	{ 	// ???
		m_Mexe = qtexe + _T("moc");
		m_Uexe = qtexe + _T("uic") ;
		m_Rexe = qtexe + _T("rcc") ;
		m_Lexe = qtexe + _T("lrelease") ;
	}
//	Mes = _T("m_Mexe = ") + quote(m_Mexe) ;  printWarn(Mes);
//	Mes = _T("m_Uexe = ") + quote(m_Uexe) ;  printWarn(Mes);
//	Mes = _T("m_Rexe = ") + quote(m_Rexe) ;  printWarn(Mes);
//	Mes = _T("m_Lexe = ") + quote(m_Lexe) ;  printWarn(Mes);
    bool Findqtexe = true, dquote = false;
	bool FindMexe  = ::wxFileExists(m_Mexe) ;
	bool FindUexe  = ::wxFileExists(m_Uexe) ;
	bool FindRexe  = ::wxFileExists(m_Rexe);
	bool FindLexe  = ::wxFileExists(m_Lexe) ;
	if (!FindMexe)
    {
		Mes = _("Could not query the executable Qt") +  quote(m_Mexe)  ;
		Findqtexe = false;
		dquote = m_Mexe.find(Dquote) != -1 ;
    }
    else
    if (!FindUexe)
    {
        Mes = _("Could not query the executable Qt") +  quote(m_Uexe)  ;
        dquote = m_Mexe.find(Dquote);
        Findqtexe = false;
    }
    else
    if (!FindRexe)
    {
        Mes = _("Could not query the executable Qt") +  quote(m_Rexe)  ;
        dquote = m_Rexe.find(Dquote);
        Findqtexe = false;
    }
    else
    if (!FindLexe)
    {
        Mes = _("Could not query the executable Qt") +  quote(m_Lexe)  ;
        dquote = m_Lexe.find(Dquote);
        Findqtexe = false;
    }
// error on path ?
    if (!Findqtexe )
    {
		Mes += Lf + _("Cannot continue") ;
		Mes += _T(", ") ;
		Mes += _("Verify your installation Qt");
		if (dquote)
		{
            Mes += _T(" (") ;
            Mes += _("!! you have a quote in path !!");
            Mes += _T(")") ;
		}
		Mes += _T(".") ;
		printError(Mes);

		wxString title = _("Search executable Qt") ;
		title += _T(" ...") ;
		cbMessageBox(Mes, title, wxICON_ERROR) ;
	}
	m_IncPathQt = pathIncludeMoc(m_pProject) +  pathIncludeMoc(_pBuildTarget);
	m_DefinesQt = definesMoc(m_pProject) + definesMoc(_pBuildTarget);

	Mes.Clear();



	return Findqtexe ;
}
///-----------------------------------------------------------------------------
///	Read content file
///
/// Called by  :
/// 	1. q_object(filename, qt_macro):1,
///		2. hasIncluded(const wxString& fcreator):1,
///
wxString qtPrebuild::ReadFileContents(const wxString& _filename)
{
	wxFileName fname(Manager::Get()->GetMacrosManager()->ReplaceMacros(_filename));
	NormalizePath(fname, wxEmptyString);
	wxFile f(fname.GetFullPath());

	return cbReadFileContents(f);
}
///-----------------------------------------------------------------------------
///	Write content file
///
/// Called by  : none
///
bool qtPrebuild::WriteFileContents(const wxString& _filename, const wxString& _contents)
{
	wxFileName fname(Manager::Get()->GetMacrosManager()->ReplaceMacros(_filename));
	NormalizePath(fname, wxEmptyString);
	wxFile f(fname.GetFullPath(), wxFile::write);

	return cbWrite(f, _contents);
}

///-----------------------------------------------------------------------------
/// Looking for eligible files to the active target (around project files),
/// 	meet to table 'm_Filecreator' : return eligible file number
///
/// Called by  :
///		1. qtPrebuild::buildAllFiles(cbProject * prj, bool workspace, bool allbuild):1
///
/// Calls to :
///		1. virtualToFirstRealTarget(m_nameactivetarget):1,
///		2. isGoodTargetQt(m_nameActiveTarget):1,
///		3. isElegible(file):1,
///
uint16_t qtPrebuild::findGoodfiles()
{
//	Mes = _("Search creator file(s) ...") ; printWarn(Mes);
	/// DEBUG
//* ****************************************
//	beginDuration(_T("findGoodFiles(...)"));
//* ****************************************
	m_Filecreator.Clear();
// active target ( virtual or real )
	m_nameActiveTarget = m_pProject->GetActiveBuildTarget() ;
	if (m_nameActiveTarget.IsEmpty() )
		return 0;

	// => peek the first real target of the virtual
	virtualToFirstRealTarget(m_nameActiveTarget);
// is good target  ?
	ProjectBuildTarget * pBuildTarget = m_pProject->GetBuildTarget(m_nameActiveTarget);
	// virtual target ?
	if (!pBuildTarget)	return 0 ;

	if (! isGoodTargetQt(pBuildTarget))		return 0 ;

// **********************
// Around project files
// **********************
	ProjectFile * prjfile ;
	wxArrayString tabtargets ;
	wxString file, nametarget;
	int nt, nfprj = m_pProject->GetFilesCount()  ;
	bool good ;
	/// DEBUG
//* ************************************************
//	beginDuration(_T("findGoodFiles:: all files)"));
//* ************************************************
// all files project
	for (int nf =0 ; nf < nfprj; nf++ )
	{
		prjfile = m_pProject->GetFile(nf);
		if (!prjfile) 	continue  ;
	//	file name
		file = prjfile->relativeFilename ;
		if (file.IsEmpty() )	continue  ;
		// no complement file
		if (file.StartsWith(m_dirPreBuild) )	continue  ;
//Mes = _T("file = ") + file; print(Mes);
	// all targets of file
		tabtargets = prjfile->GetBuildTargets() ;
		nt = tabtargets.GetCount()  ;
		if (nt > 0)
		{
		// all file targets
			for (int nft=0; nft < nt; nft++)
			{
				nametarget = tabtargets.Item(nft) ;
				good = nametarget.Matches(m_nameActiveTarget) ;
				if (good )
					break ;
			}
		// target == active target
			if (good )
			{
//Mes = _T("nametarget = ") + nametarget;	print(Mes);
//Mes = Tab + _T("file finded ") + strInt(nf) + _T(" = ") + file;  printWarn(Mes);
			// not eligible ?
				if (! isElegible(file))		continue  ;
//Mes = Tab + _T("file elegible ") + strInt(nf) + _T(" = ") + file; printWarn(Mes);
			// add the file and it target
				m_Filecreator.Add(file, 1) ;
//Mes = _T("elegible file = ") + file; print(Mes);
			}
			else
			{
				// no target active !!
//Mes = _T("no target active = ") + nametarget; print(Mes);
			}
		}
	// execute pending events
		m_pm->ProcessPendingEvents();
	}
	/// DEBUG
//* **********************************************
//	endDuration(_T("findGoodFiles:: all files)"));
//* **********************************************
	Mes.Clear();

//  number file eligible
	return m_Filecreator.GetCount()  ;
}

///-----------------------------------------------------------------------------
/// Reference of libQt by target
///		"" or  "5r" or "5d" or "0" (for 'qtmain')
///   	qt5 -> '5', release -> 'r' and debug -> 'd'
///		'qtmain' -> '0'
///
/// Called by  :
///		-# isGoodTargetQt(const ProjectBuildTarget * _pBuildtarget):1,
///
wxString qtPrebuild::refTargetQt(const ProjectBuildTarget * _pBuildTarget)
{
	wxString refqt ;
	if (! _pBuildTarget)
		return refqt  ;

//Mes = _T("qtPrebuild::refTargetQt(...)"); printWarn(Mes);
	wxArrayString tablibs = _pBuildTarget->GetLinkLibs();
	int nlib = tablibs.GetCount() ;
//Mes = _T("nlib = ") + strInt(nlib); printWarn(Mes);
    if (!nlib)
    {
        tablibs = m_pProject->GetLinkLibs();
        nlib = tablibs.GetCount() ;
        if (!nlib)
        {
            Mes = _("This target has no linked library") ;
            Mes += _T(" !!");
            printError(Mes);
            return refqt ;
        }
    }
// search lib
	bool ok = false ;
	wxString str =  _T("") ;
	wxString namelib ;
	int u=0, index= -1, pos ;
	while (u < nlib && !ok )
	{
		// lower, no extension
		namelib = tablibs.Item(u++).Lower().BeforeFirst('.') ;
//Mes = strInt(u) + _T("- namelib = " + quote( namelib )); printWarn(Mes);
		// no prefix "lib"
		pos = namelib.Find(_T("lib"));
		if (pos == 0)
			namelib.Remove(0, 3) ;
		// begin == "qt"
		pos = namelib.Find(_T("qt")) ;
		if (pos != 0)	continue ;
//Mes = strInt(pos) ; printWarn(Mes);
		// compare
		index = m_TablibQt.Index(namelib);
//Mes = strInt(index) ; printWarn(Mes);
		ok = index != -1 ;
		// first finded
		if (ok)
		{
			// == 'qtmain' or 'qtmaind'
			if ( namelib.StartsWith(_T("qtmain") ) )
			{
				refqt = _T("0");
			}
			else
			// qt5xxxx or qt5xxxd
			if (namelib.GetChar(2) == '5')
			{
				refqt = _T("5") ;
				// the last
				str = namelib.Right(1) ;
				if ( str.Matches(_T("d") ) )
					refqt += _T("d") ;
				else
					refqt += _T("r") ;
			}
			else
				refqt = _T("");

			break;
		}
	}
//Mes = _T(" return refqt = " + quote( refqt )); printWarn(Mes);
	Mes.Clear();

	return refqt ;
}
///-----------------------------------------------------------------------------
/// Give the good type of target for Qt
///
/// Called by  :
///		1. QtPregen::OnAddComplements(CodeBlocksEvent& event):1,
///		2. qtPrebuild::findGoodfiles():1,
///
/// Calls to:
///		1. refTargetQt(ProjectBuildTarget * _pBuildtarget):1,
///
bool qtPrebuild::isGoodTargetQt(const ProjectBuildTarget * _pBuildTarget)
{
	if (!_pBuildTarget)
		return false ;

	bool ok = ! isCommandTarget(_pBuildTarget);
	wxString str = refTargetQt(_pBuildTarget)  ;
//Mes = _T("str = ") + quote( str ); printWarn(Mes);
	bool isqt = ! str.IsEmpty()  ;
	if (!isqt)
	{   // TODO ...
		Mes = Tab + _("This target have nothing library Qt") + _T(" !") ;
		Mes += Lf + Tab + _("PreBuild cannot continue.") ;
		printWarn(Mes);
	}
//Mes = _T("isGoodTargetQt  = ") + strBool( ok && isqt); printWarn(Mes);
	Mes.Clear();

	return ok && isqt ;
}

/// /////////////////////////// Search into files //////////////////////////////

///-----------------------------------------------------------------------------
/// Search elegible files
/// 'file' was created ?  ->  moc_*.cxx, ui_*.h, qrc_*.cpp
///
/// Called by  :
///		1. buildOneFile(cbProject * prj, const wxString& fcreator):1,
///
/// Calls to :
/// 	1. q_object(const wxString& filename, const wxString& qt_macro):1;
///
bool qtPrebuild::isElegible(const wxString& _file)
{
	wxString ext = _file.AfterLast('.')  ;
	bool ok = ext.Matches(m_UI) || ext.Matches(m_Qrc) ;
	if (ok)
		return ok;

	ok = false ;
	if (ext.Matches(EXT_H) || ext.Matches(EXT_HPP) || ext.Matches(EXT_CPP) )
		ok = q_object(_file, _T("Q_OBJECT") ) > 0  ;

	return ok ;
}

///-----------------------------------------------------------------------------
/// Return occurrences of "Q_OBJECT" + occurrences of "Q_GADGET"
///
/// Called by  :
///		1. isElegible(const wxString& file):1
///
int qtPrebuild::q_object(const wxString& _filename, const wxString& _qt_macro)
{
/// DEBUG
//* **************************************************************************
//	beginDuration(_T("q_object(") + filename + _T(", ") + qt_macro + _T(")"));
//* **************************************************************************

//1- the comments
	wxString CPP = _T("//") ;
	wxString BCC = _T("/*"), ECC = _T("*/") ;
	// ends of line
	wxString SA, SD;
	SA.Append(SepA); SD.Append(SepD) ;

//2- verify exist  : 'filename' is relative file name
	wxString namefile = m_dirProject + _filename ;
	if (! ::wxFileExists(namefile))
	{
		Mes = quote( namefile ) + _("NOT FOUND") + _T(" !!!") + Lf ;
		printError(Mes) ;
		return -1  ;
	}
//3- read source 'namefile'
	wxString source = ReadFileContents(namefile) ;
	if (source.IsEmpty())
	{
		Mes = quote( namefile ) + _T(": ") + _T("file is empty") +  _T(" !!!") + Lf ;
		printError(Mes) ;
		return -1 ;
	}
///----------------------------------------------------------------------
/// 1st test before the detailed analysis
	int number = 0  ;
	number = qt_find(source, _T("Q_OBJECT"))  ;
	number += qt_find(source, _T("Q_GADGET") )  ;
///-----------------------------------------------------------------------
	if (number)
	{
		// wxString
		int rest = source.length() ;
		// string copy
		wxString temp = source.Mid(0, rest ), after ;
		// pos
		int posbcc, posecc, poscpp;
		// boolean
		bool ok ;
	//4- delete C comments
		do
		{
			posecc = -1;
			rest = temp.length() ;
		// first DCC next FCC
			posbcc = temp.Find(BCC) ;
			if (posbcc != -1)
			{
				rest -= posbcc ;
				after = temp.Mid(posbcc, rest) ;
				posecc = after.Find(ECC) ;
				// 'posecc' relative to 'posbcc' !!
			}
			// comments exists ?
			ok = posbcc != -1 && posecc != -1  ;
			if (ok)
			{
			// delete full comment
				temp.Remove(posbcc, posecc + ECC.length()) ;
			}
		}
		while (ok)  ;
	//5- delete C++ comments
		do
		{
			posecc = -1;
			rest = temp.length()  ;
		// start of a comment C
			poscpp = temp.Find(CPP) ;
			if (poscpp != -1)
			{
			// verify the caracter before : 'http://www.xxx' !!
				wxChar xcar = temp.GetChar(poscpp-1) ;
				if (xcar != ':' )
				{
					rest -= poscpp  ;
				// string comment over begin
					after = temp.Mid(poscpp, rest) ;
				// end comment
					posecc = after.Find(SA) ;
				}
				else
					poscpp = -1  ;
			}
			ok = poscpp != -1 && posecc != -1 ;
			if (ok)
			{
			// delete full comment
				temp.Remove(poscpp, posecc) ;
			}
		}
		while (ok) ;
	//6- delete all text == "...."
		if (temp.Contains(Dquote) )
		{
			int posb, pose ;
			do
			{
				pose = -1;
				rest = temp.length() ;
			// first Dquote,
				posb = temp.Find(Dquote);
				if (posb != -1)
				{
					rest -= posb;
					after = temp.Mid(posb + 1, rest) ;
				// next Dquote
					pose = after.Find(Dquote);
				}
				// text exists ?
				ok = posb != -1 && pose != -1 ;
				if (ok)
				{
				// delete full text = "..."
					temp.Remove(posb, pose + 2) ;
				}

			} while (ok);
		}

//6- find "Q_OBJECT" and/or "Q_GADJECT"
		number = 0  ;
		wxString findtext = _T("Q_OBJECT") ;
		if ( _qt_macro.Matches(findtext) )
		{
			number = qt_find(temp, findtext)  ;
			findtext = _T("Q_GADGET") ;
			number += qt_find(temp, findtext)  ;
		}
		else
		{
			Mes = _("The file") + Space + _filename + Space;
			Mes += _("is not a good creator !!") ;
			printError(Mes);
		}

	}
	/// DEBUG
//* *********************************
//	endDuration(_T("q_object(...)"));
//* *********************************
	Mes.Clear();

	return number ;
}

///-----------------------------------------------------------------------------
/// Find valid "Q_OBJECT" or "Q_GADGET"  ( ex : QKeySequence )
///
/// Looks for occurrences of "Q_OBJECT" or "Q_GADGET" valid in the file.
///	Remarks:
///		Qt documentation indicates another METATOBJETS keyword -> "Q_GADGET"
///		like "Q_OBJECT" but when the class inherits from a subclass of QObject.
///	Algorithm:
///		1 - eliminates comments type C   '/*... ...*/'
///		2 - eliminates comments type Cpp  '// .... eol'
///		3 - one seeks "Q_OBJECT" then 'xQ_OBJECTy' to verify that 'x' in
///			[0xA, 0x9, 0x20] and 'y' in [0xd (or 0xA), 0x9, 0x20]
///		4 - one seeks "Q_GADGET" in the same way
///
///-----------------------------------------------------------------------------
/// Search 'qt_text' in 'tmp'
///
/// Called by  :
///		1. q_object(const wxString& filename, const wxString& qt_macro):4
///
int  qtPrebuild::qt_find (wxString _tmp, const wxString& _qt_text)
{
	uint8_t tab = 0x9, espace = 0x20  ;
	int len_text = _qt_text.length() ;
	int posq, number = 0;
	bool ok, good, goodb, gooda ;
	do
	{
		posq = _tmp.Find(_qt_text) ;
		ok = posq != -1 ;
		if (ok)
		{
		// pred and next caracter
			//wxUChar
			uint8_t xcar = _tmp.GetChar(posq-1) ;
			//wxUChar
			uint8_t carx = _tmp.GetChar(posq + len_text)  ;
		// only autorized caracters
			// before
			goodb = (xcar == espace || xcar == tab || xcar == SepA) ;
			// after
			gooda = (carx == espace || carx == tab || carx == SepD || carx == SepA);
			gooda = gooda || carx == '(' || carx == '_' ;
			good = goodb && gooda  ;
			if (good)
				number++  ;
		// delete analyzed
			_tmp.Remove(0, posq + len_text)  ;
		}
	}
	while (ok) ;

	return number ;
}

/// //////////////////////// End search into files /////////////////////////////

/// /////////////////////// Unregistered files /////////////////////////////////

///-----------------------------------------------------------------------------
///	To unregister a project file (creator and complements) in 'qtPregen'
///		format 'm_Filecreator' ->  'dircreator\xxx.h'  ( src\xxx.h)
/// 	return 'true' when right
///
///	Called by :
///		1. QtPregen::onProjectFileRemoved(CodeBlocksEvent& _event):1,
///
///	Calls to :
///		1. unregisterComplementFile(const wxString & _file, bool _first):1,
///
bool qtPrebuild::unregisterCreatorFile(wxString & _file, bool _first)
{
	// copy file, absolute project path
	wxString filename = _file, pathproject = m_pProject->GetCommonTopLevelPath();
	// relative file path
	filename.Replace(pathproject, _T("") ) ;
//Mes = _T(" ==> creator filename to find =") + quote( filename ); print(Mes);
	int16_t index = m_Filecreator.Index (filename);
	bool ok = index != wxNOT_FOUND;
	if (!ok)
		return ok;
	// replace creator by 'm_Devoid'
	m_Filecreator.RemoveAt(index); m_Filecreator.Insert(m_Devoid, index);
	if (_first)
	{
		Mes =  Lf +_("Creator file(s) was removed from project") + _T(" ...") ;
		printWarn(Mes);
	}
	Mes = Tab + _T("-") + quote( filename );
	print(Mes);

// print(allStrTable(_T("m_Filecreator"), m_Filecreator));
// find complement in m_Registered
    filename = m_Registered.Item(index);
// remove complement if it's exists
	ok = filename.Matches(m_Devoid);
	if (!ok)
	{
		ok = unregisterComplementFile(filename, _first);
	}
	Mes.Clear();

	return ok;
}
///-----------------------------------------------------------------------------
///	To unregister project file(s) (complements) in 'qtPregen'
///		format 'm_Registered' ->  'dircomplement\\moc_xxx.cpp'  (m_dirPreBuild\\debug\\moc_xxx.cpp)
/// 	return 'true' when right
///
///	Called by :
///		1. QtPregen::onProjectFileRemoved(CodeBlocksEvent& _event):1,
///		2. unregisterCreatorFile(const wxString & _file, bool _first):n,
///
///	Calls to :
///		1. qtPre::nameCreated(const wxString& _file):1,
///
bool qtPrebuild::unregisterComplementFile(wxString & _file, bool _first)
{
// copy file, absolute project path
	wxString filename = _file, pathproject = m_pProject->GetCommonTopLevelPath();
	// relative file path
	filename.Replace(pathproject, _T("") ) ;
//Mes = _T(" qtPrebuild::unregisterComplementFile() ==> filename to find = ") + quote( filename ); printWarn(Mes);
// we look for the number of 'complement' occurrences in 'm_Registered'
	bool ok = false;
	uint16_t nf = m_Registered.GetCount();
//Mes =  _T(" from ") + quote( strInt(nf) )  + _T(" file(s) ...") ; print(Mes);
	wxString item;
	while (nf--)
	{
		item = m_Registered.Item(nf);
//Mes =  Tab + strInt(nf) +  _T(" ==> ") + quote(item); print(Mes);
		if (!item.Contains(filename))	continue ;
	// replace table complement by 'm_Devoid'
		m_Registered.RemoveAt(nf); 		m_Registered.Insert(m_Devoid, nf);
		m_Filestocreate.RemoveAt(nf); 	m_Filestocreate.Insert(m_Devoid, nf);
		m_Createdfile.RemoveAt(nf);		m_Createdfile.Insert(m_Devoid, nf);
		m_Filewascreated.RemoveAt(nf);  m_Filewascreated.Insert(m_Devoid, nf);
	// remove from disk
//Mes =  Tab + Tab + strInt(nf) +  _T(" ==> ") + quote( item );
//Mes += _T(" is here..."); print(Mes);
		ok = removeComplementToDisk(filename, _first);
		if (!ok)
		{
			Mes = _T(" ==> ");
			Mes += _("error filename") ;
			Mes += + _T(" =") + quote( filename );
		//	printError(Mes);
		//	break;
		/// TO DO an other method ...
			ok = true;
		}
	}
	// no file  ?
	m_clean = m_Registered.IsEmpty();
	// return short name
	_file = filename ;
	Mes.Clear();

	return ok;
}

///-----------------------------------------------------------------------------
///	To unregister all complements file to 'CB' after rename one target
/// return 'true' when right
///
/// Called by :
///		1. updateNewTargetName(ProjectBuildTarget * _pBuildTarget,
///							   const wxString & _oldTargetName):1,
///
/// Call to :
///		1.
///
bool qtPrebuild::unregisterAllComplementsToCB(const wxString & _oldTargetName,
											  cbProject * _pProject, bool _first)
{
/// Debug
//Mes = _T("=> unregisterAllComplementsToCB( => An old name target =") ;
//Mes += quote( _oldTargetName) + _T(")"); printWarn(Mes);
/// <=
	bool ok = false;
	wxString filename ;
	ProjectFile * prjfile ;
	int index;
// begin remove files
	_pProject->BeginRemoveFiles();
	bool withObject;
	/// all complement are registerd to 'm_Registered'
	for (wxString fregistered : m_Registered)
	{
//Mes = _T(" fregistered = ") + quote( fregistered  ) ; print(Mes);
		if (fregistered.Matches(m_Devoid))	continue;

		prjfile = _pProject->GetFileByFilename(fregistered);
		if (prjfile)
		{
			filename = prjfile->relativeFilename ;
			if (filename.IsEmpty())		continue  ;
			// memorize compiled complement before remove from project...
			withObject	= prjfile->compile || prjfile->link;
			// unregister file from CB
			ok = _pProject->RemoveFile(prjfile);
			if (!ok)
			{
				Mes = _T(" ==> RemoveFile() : ") ;
				Mes += _("error with file") ;
				Mes += _T(" =") + quote( filename );
				printError(Mes);
				break;
			}
			else
			{
			// update  'm_Registered'
				index = m_Registered.Index(fregistered);
				// replace 'fregistered' by 'm_Devoid'
				m_Registered.RemoveAt(index); m_Registered.Insert(m_Devoid, index);

			// also remove file from disk
//Mes = _T(" file = ") + quote( file ) ; print(Mes);
				//ok = removeComplementToDisk(filename, _first, compiledOrLinked);
				ok = removeComplementToDisk(filename, _first, withObject);
				if (_first)
					_first = false;
				if (!ok)
				{
					Mes = _T(" ==> removeComplementToDisk() : ");
					Mes += _("error with file") ;
					Mes += _T(" =") + quote( filename );
					printError(Mes);
					break;
				}
			}
		}
	}
// end remove files
	_pProject->EndRemoveFiles();
// mandatory ! tree refresh
	Manager::Get()->GetProjectManager()->GetUI().RebuildTree() ;
	Mes.Clear();

	return ok;
}

///-----------------------------------------------------------------------------
/// To remove one complement files in project :
///		(1. 'm_Registered.item(pos)' is clear)
///		is it necessary ?  => yes in 2.4.0
///		(2. disk : (m_dirPreBuild\\nametarget\\m_dirPreBuild) is removed)
///		return 'true' when right
///
/// Called by :
///		1. unregisterComplementFile(const wxString & _file, bool _creator, bool _first):1,
///		2. unregisterAllComplementsToCB(const wxString & _oldTargetName, cbProject * _project, bool _first):1,

//bool qtPrebuild::removeComplementToDisk(const wxString & _filename, bool _first)
bool qtPrebuild::removeComplementToDisk(const wxString & _filename, bool _first, bool _withobject)
{
/// Debug
//Mes = _T("qtPrebuild::removeComplementToDisk : _filename = ") + quote( _filename ); print(Mes);
/// <==
// complement exists
	bool ok = ::wxFileExists(_filename);
// Mes = _T("ok = ") + strBool(ok); print(Mes);
	if (ok)
	{
	/// remove complement file
		ok = ::wxRemoveFile(_filename);
		if (ok)
		{
			if (_first)
			{
				Mes = Tab + _("Complement file(s) was removed from C::B and disk");
				Mes += _T(" ...") ;
				print(Mes);
			}
			Mes = Tab +  _T("-") + quote( _filename );
		// if 'ui_xxx.h' => no compiled file '*.o'
			wxString beginfile = _filename.AfterLast('\\').BeforeFirst('_');
			if (beginfile.Matches(_T("ui")) )
			{
				printWarn(Mes);
				return ok;
			}

			// extract the used target
			wxString  pathadding, targethere;
			// find target here
			int32_t pos = _filename.Find(m_dirPreBuild);
			if (pos >=0)
			{
				targethere  = _filename.Mid(pos).BeforeLast('\\').AfterLast('\\');
			}
/// it is also necessary to delete the compiled file from the old complement
/// ATTENTION : the active target may be different of target used !!
			wxString pathObjects, ofile = _filename.Before('.')  + _T(".o");
//Mes = _T(" m_dirObjects  =") + quote( m_dirObjects ) ; printWarn(Mes);
			pathObjects = m_dirObjects + ofile;
			m_pMam->ReplaceMacros(pathObjects);
			if (!targethere.IsEmpty())
				pathObjects.Replace(m_nameActiveTarget, targethere);
//Mes = _T(" To delete => pathObjects  =") + quote( pathObjects ) ; print(Mes);
			// is there a compiled complement file ?
				ok = ::wxFileExists(pathObjects);
				if (ok) {
					ok = ::wxRemoveFile(pathObjects);
					if (ok)
					{
						Mes += _T(",") + quote( pathObjects );
						print(Mes);
					}
					// error to removing :
					else
					{
						Mes = Tab + Tab + _T("-> ") + _("not removed") + quote( pathObjects );
						printError(Mes);
					}
				}
				// no compiled complement
				else
				{
				// the compiled complement file is missing
					if (_withobject)
					{
						Mes = Tab + Tab + _T("-> ") + _("not exists") + quote( pathObjects );
						printError(Mes);

					}

				// no complement has been compiled
					else
					{
					/// TO REVIEW to an other method ...
						print(Mes);
					}
				}
		/// <=
		}
		else
		{
			Mes = Tab + _("Can not delete") + quote( _filename ) + _T(" !!");
			printError(Mes) ;
		}
	}
	else
	{
		Mes = Tab + _("File") + quote( _filename ) + _("not found") + _T(" !!");
		printError(Mes) ;
	}
	Mes.Clear();

	return ok;
}
///-----------------------------------------------------------------------------
/// To remove one complement '*.o' file in project :
///		(1. disk : is removed)
///		return 'true' when right
///
/// Called by :
///		1. qtPrebuild::removeComplementToDisk(const wxString & _filename, bool _first):1,

bool qtPrebuild::removeComplementObjToDisk(const wxString & _filename)
{
	bool ok = ::wxFileExists(_filename);
	/// TO FINISH ...


	return ok;
}

///-----------------------------------------------------------------------------
///	To remove all complement's directory after rename one target
/// return 'true' when right
///
/// Called by :
///		1. unregisterAllComplementsToCB(const wxString & _oldTargetName,
///										cbProject * _pProject, bool _first):1,
///
/// Call to :
///		1. bool qtPrebuild::recursRmDir(wxString _rmDir):2,
///
bool qtPrebuild::removeComplementDirToDisk(const wxString & _oldTargetName)
{
	bool ok ;
/// delete old complements directory : '"m_dirPreBuild"\_oldTargetName\*.*'
	wxString file = m_dirPreBuild + _oldTargetName;
	Mes = Tab + _("Complement directory was removed from disk") + _T(" ...");
	printWarn(Mes);

	ok = recursRmDir(file);
	Mes = Tab + _T("- ") + quoteNS( file + Slash + _T("*.*") ) ;
	if(ok)
	{
		Mes += Space + _("is deleted");
		print(Mes);
	}
	else
	{
		Mes += _T(" ==> ") ;
		Mes +=  _(" cannot be deleted !!!");
		printError(Mes);
	}
/// delete old complements objects directory : 'obj\_oldTargetName\"m_dirPreBuild"\*.*'
	file = m_dirObjects + m_dirPreBuild ;
//Mes = _T("==> to delete objects") + Space + quote( file ) ; printWarn(Mes);
Mes = Tab + _T("Object complement directory was removed from disk");
//	Mes +=  Space + quote( file );
printWarn(Mes);
	ok = recursRmDir(file);
	Mes = Tab + _T("-") + quote( file + _T("*.*") ) ;
	if (ok)
	{
		Mes += Space + _("is deleted");
		print(Mes);
	}
	if (!ok)
	{
		Mes += _T(" ==>") ;
		Mes += _(" cannot be deleted");
		Mes += _T(" !!!");
		printError(Mes);
	}
	Mes.Clear();

	return ok;
}

///-----------------------------------------------------------------------------
/// Remove old executable from old target
///
/// Called by :
///		1. updateNewTargetName(ProjectBuildTarget * _pBuildTarget,
///									 const wxString & _oldTargetName):1,
///

bool qtPrebuild::removeOldExecutable(ProjectBuildTarget* _pBuildTarget, const wxString & _oldTargetName)
{
	bool ok = _pBuildTarget != nullptr;
	if (ok)
	{
		wxString oldexename = _pBuildTarget->GetOutputFilename();
		oldexename.Replace(_T("$(TARGET_NAME)"), _oldTargetName);
//Mes = _T(" old outputname = ") + quote( oldexename ); printWarn(Mes);
		ok = ::wxFileExists(oldexename);
		if (ok)
		{
		// remove executable file
			ok = ::wxRemoveFile(oldexename);
			if (ok)
			Mes = Tab + _("Old executable") + quote( oldexename ) ;
			if(ok)
			{
				Mes += _("is deleted");
				print(Mes);
			}
			else
			{
				Mes += _T(" ==> ") ;
				Mes += _("cannot be deleted") ;
				Mes += _T(" !!!");
				printError(Mes);
			}
		}
	}
	Mes.Clear();

	return ok;
}

///-----------------------------------------------------------------------------
/// Remove old include path qt for CB
///
/// Called by :
///		1. updateNewTargetName(ProjectBuildTarget * _pBuildTarget,
///									 const wxString & _oldTargetName):1,
///

bool qtPrebuild::removeOldPathtoCB(ProjectBuildTarget * _pContainer, const wxString & _oldTargetName)
{
//Mes = _T(" *** removeOldPathtoCB(...) from") + quote( _oldTargetName ) ; printWarn(Mes);
	bool ok = false;
	// read target include path
	wxArrayString tabincludedirs = _pContainer->GetIncludeDirs(), tabpath;
//print(allStrTable( _T(" include dir"), tabincludedirs));
	if (tabincludedirs.GetCount())
	{
		for (wxString& line : tabincludedirs)
		{
			m_pMam->ReplaceMacros(line) ;
//Mes = _T("** line =") + quote(line); print(Mes);
			// it's an old path
			if (line.Contains(_oldTargetName))	continue;
			// memorize the line
			tabpath.Add(line, 1);
		}
//print(allStrTable( _T(" include dir"), tabpath));
		// rewrite the paths without the old ones
		_pContainer->SetIncludeDirs(tabpath);
	}

	return ok;
}
///-----------------------------------------------------------------------------
/// Recursively deletes all directories and files
///
/// Called by :
///		1. bool qtPrebuild::recursRmDir(wxString _rmDir):1,
///		2. bool removeComplementDirToDisk(const wxString & _oldTargetName):2,
///

bool qtPrebuild::recursRmDir(wxString _rmDir)
{
// For safety, you don't empty a complete disk ('..' or '.')
    if (_rmDir.length() <= 3)
		return false;

// We make sure that the directory to be deleted exists
    if(!wxDir::Exists(_rmDir))
		return false;

// We make sure that the name of the folder to be deleted ends with a separator
	// if (_rmDir.Last() != wxFILE_SEP_PATH )
	/// patch find in "https://github.com/vslavik/poedit"
	if (_rmDir.empty() || (_rmDir.Last() != wxFILE_SEP_PATH) )
		_rmDir += wxFILE_SEP_PATH;
//Mes = quote( _rmDir ); printWarn(Mes);

// Creating the 'wxDir*' object
    wxDir* pDir = new wxDir(_rmDir);
    if (!pDir)
		return false;

/// Retrieving the first item in the folder to be deleted
    wxString item;
    bool ok = pDir->GetFirst(&item);
    if (ok)
    {
        do
        {
            // If the item found is a directory
            if (wxDirExists(_rmDir + item))
				/// recursive call ...
                recursRmDir(_rmDir + item);
            else
			// Otherwise, we delete the file
			if(!wxRemoveFile(_rmDir + item))
			{
				//Error during deletion (access rights ?)
				Mes = _T("Could not remove item") + quote(_rmDir + item);
				printError(Mes);
			}
        }
        while (pDir->GetNext(&item));
    }
/// From now on, the folder is empty, so we can delete it
    // But first you have to destroy the wxDir variable that always points to it
    delete pDir;
    // You can now delete the folder passed as a parameter
    ok = wxFileName::Rmdir(_rmDir);
    if (!ok)
    {
        //Error during deletion (access rights ?)
		Mes = _("Could not remove directory") + quote(_rmDir);
		printError(Mes);
    }
    Mes.Clear();

    return ok;
}

///-----------------------------------------------------------------------------
/// Total updates of the new target from old target
/// TO FINISH ...
///
/// Called by :
///		1. QtPregen::OnRenameProjectOrTarget(CodeBlocksEvent& _event):1,
///
/// Calls to :
///		1. unregisterAllComplementsToCB(const wxString & _oldTargetName,
///											  cbProject * _project, bool _first):1,
///		2. removeComplementDirToDisk(const wxString & _oldTargetName):1,
///		3. removeOldPathtoCB(ProjectBuildTarget * _pContainer, const wxString & _oldTargetName):1,
///		4. removeOldExecutable(ProjectBuildTarget* _pBuildTarget, const wxString & _oldTargetName):1,

bool qtPrebuild::updateNewTargetName(ProjectBuildTarget * _pBuildTarget,
									 const wxString & _oldTargetName)
{
	bool ok = true;

	//1- unregister complements in 'm_dirPreBuild\\oldNameTarget\\*.*'
	//2- remove complements from disk
	ok = unregisterAllComplementsToCB(_oldTargetName, m_pProject, FIRST);

	//3- remove old directory to disk
	ok = removeComplementDirToDisk(_oldTargetName);

	//4- remove old paths to new build target
	ok = removeOldPathtoCB(_pBuildTarget, _oldTargetName);

	//5- remove 'BuildCB\\oldNameTarget\\app.exe'
	ok = removeOldExecutable(_pBuildTarget, _oldTargetName);

	return ok;
}


/// //////////////////////// End Unregisterd files /////////////////////////////

///-----------------------------------------------------------------------------
/// To register ALL new files in project : 'm_Registered' is meet
///	return 'true' when right
///
/// Called by  :
///		1. buildAllFiles(cbProject * prj, bool workspace, bool allbuild):1,
///
/// Calls to :
///		1. addOneFile(const wxString& fcreator, const wxString& fout):1,
///
uint16_t qtPrebuild::addAllFiles()
{
	m_Registered.Clear();
// local variables
	bool valid = false;
	wxString fout;
// read file list to 'm_Filecreator'  (*.h, *.cpp, *.qrc, *.ui)
    for (wxString fcreator : m_Filecreator)
	{
	// complement file
		fout = complementDirectory() + nameCreated(fcreator) ;
	// record one complement file
		valid = addOneFile(fcreator, fout) ;
	// valid
		if (valid)
		{	// registered in table
			m_Registered.Add(fout, 1) ;
		}
	}
// tree refresh
	// >= svn 9501 : CB 13.12
	Manager::Get()->GetProjectManager()->GetUI().RebuildTree() ;

//  end registering
	uint16_t nf = m_Registered.GetCount() ;
//	Mes = Tab + _T("-> ") + strInt(nf) ;
//	Mes += Space + _("creator(s) file(s) registered in the plugin") ; print(Mes) ;

	return  nf;
}
///-----------------------------------------------------------------------------
/// To register One file in project and 'm_Registered' is actualized
///	return 'true' when right
///
/// Called by  :
///		1.  addAllFiles():1,
///
/// Calls to :
///		1. nameCreated(const wxString& file):1,
///		2. hasIncluded(const wxString& fcreator):1,
///
bool qtPrebuild::addOneFile(const wxString& _fcreator, const wxString& _fout)
{
// local variables
	bool valid = false, nocompile = false, weight = 50;
// has included ?
	wxString extin = _fcreator.AfterLast('.') , extout = _fout.AfterLast('.')  ;
	if ( (extin.Matches(EXT_H) && extout.Matches(EXT_CPP)) ||
		 (extin.Matches(EXT_CPP) && extout.Matches(EXT_MOC)) )
		nocompile =  hasIncluded(_fcreator) ;
	else
	// ui_*.h file
	if (extin.Matches(EXT_UI) && extout.Matches(EXT_H) )  {
		nocompile = true;
		weight = 40; // before all files
	}
//Mes = Tab + quote(_fcreator) + _T("->") +  quote( _fout ) ;
//Mes += _T(" : include =") +  quote( strBool(nocompile) ) ; printWarn(Mes);

// find projectfile in project ?
	ProjectFile * prjfile = m_pProject->GetFileByFilename(_fout);
	valid = prjfile != nullptr;
	if (valid) {
	// and add target to this projectfile
		prjfile->AddBuildTarget(m_nameActiveTarget);
		valid = prjfile != nullptr;
		if (valid)
		{
		// included ?
			prjfile->compile = !nocompile;
			prjfile->link = !nocompile;
			prjfile->weight = weight;
		}
	}
	else {
	// then create a projectfile with the file to target
		// cbProject::AddFile(Nametarget, file, compile, link, weight)
		prjfile = m_pProject->AddFile(m_nameActiveTarget, _fout, !nocompile, !nocompile, weight);
		valid = prjfile != nullptr;
		if (!valid)
		{
		// display message
			Mes  = _T("===> ") ;
			Mes += _("can not add this file");
			Mes += quote( _fout ) + _("to target") + Space + m_nameActiveTarget ;
			printError (Mes) ;
			cbMessageBox(Mes, _T("AddFile(...)"), wxICON_ERROR) ;
		}
	}
	Mes.Clear();

	return  valid;
}

///-----------------------------------------------------------------------------
///	Search included file
///  *.cpp contains  (# include "moc_*.cpp") or (# include "*.moc") at the file end
///
/// Called by  :
///		1. buildOneFile(cbProject * prj, const wxString& fcreator):1
///		2. addOneFile(const wxString& fcreator, const wxString& fout):1,
///
/// Calls to :
///		1. verifyIncluded(wxString& source, wxString& txt)
///
bool qtPrebuild::hasIncluded(const wxString& _fcreator)
{
//Mes = Tab + quote( fcreator ) ; printWarn(Mes);
// if '_fcreator' == 'xxxx.h' -> search in 'xxxx.cpp'
// if '_fcreator' == 'xxxx.cpp'  -> search in 'xxxx.cpp'
	wxString namefile = _fcreator.BeforeLast('.') + DOT_EXT_CPP;
	if (! ::wxFileExists(namefile) )
		 return false  ;

// load file  'xxxx.cpp'
	wxString source = ReadFileContents(namefile) ;
	if (source.IsEmpty())
		return false ;

// ext
	wxString ext = _fcreator.AfterLast('.');
//Mes = Tab + Tab + _T("ext = ") + quote( ext ) ; print(Mes);
	namefile = namefile.AfterLast(Slash).BeforeLast('.')  ;
	wxString txt = _T("");
	bool include = false;
	//1- ext == EXT_H =>  search >"moc_namefile.cpp"<
	if (ext.Matches(EXT_H))
	{
		txt = _T("moc_") + namefile + DOT_EXT_CPP;
	}
	else
	//2- ext == EXT_CPP => search >"namefile.moc"<
	if (ext.Matches(EXT_CPP) )
	{
		txt = namefile + DOT_EXT_MOC ;
	}
//Mes = Tab + Tab + _T("txt = ") + Dquote + txt + Dquote ; print(Mes);
// find from end
	include = verifyIncluded(source, txt) ;

//Mes = _T("Creator ") + quote( fcreator ) ;
//Mes += _T(" : hasIncluded(...) -> ") + strInt(include); printWarn(Mes);
	return  include ;
}
/// Called by  :
///		1. hasIncluded(const wxString& fcreator):2
///
bool qtPrebuild::verifyIncluded(wxString& _source, wxString& _txt)
{
	wxString temp;
	int32_t pos = _source.Find(_txt);
	bool include =  pos != -1 ;
	if (include)
	{
	// delete '_txt' from pos
		_source.Remove(pos);
	// search from the end, the last character of endline
		pos = _source.Find('\n', true);
		if (pos >=0)
   		{
   		// give from pos ( the last line )
			temp = _source.Mid(pos+1);
		// search "//" or "/*"
			pos = temp.Find(_T("//"));
			if (pos >= 0 )
			{
				include = false;
			}
			else
			{
				pos = temp.Find(_T("/*"));
				include = pos == -1;
			}
		//	Mes = _T("line = ") + quote( temp ); 	print(Mes);
		}
		else
		{
			// ??
		}
	}

	return include;
}

///-----------------------------------------------------------------------------
/// Indicates whether a file is registered in the project
///
/// Called by  :
///		1. buildOneFile(cbProject * prj, const wxString& fcreator):1,
///		2. filesTocreate(bool allrebuild):1,
///
bool qtPrebuild::inProjectFileCB(const wxString& _file)
{
//Mes = Tab + quote( file ) ;
    // relative filename !!!
	ProjectFile * prjfile = m_pProject->GetFileByFilename (_file, IS_RELATIVE, IS_UNIX_FILENAME) ;
//Mes += _T(" -> ") + here; printWarn(Mes);
	return prjfile != nullptr;
}

///-----------------------------------------------------------------------------
///  Create directory for "qtbrebuilt\"
///
///  Called by :
///		1. beginMesBuildCreate():1,
///		2. buildOneFile(cbProject * prj, const wxString& fcreator):1,
///		1. createComplement(const wxString& qexe, const uint16_t index):1,
///
bool qtPrebuild::createDir (const wxString&  _dircomplement)
{
//Mes = _T("qtPrebuild::createDir(") + quote( dircomplement ) + _T(")") ;
//printWarn(Mes);
	bool ok = true  ;
	if (! wxDirExists(_dircomplement))
	{
	    // Linux -> droit d'accès = 0x0777
		ok = wxMkdir(_dircomplement) ;
		if (!ok)
		{
			Mes = _("Can't create directory")  ;
			Mes += quote( _dircomplement ) + _T("!!") ;
			printError(Mes);
			cbMessageBox(Mes, _T("createDir()"), wxICON_ERROR) ;
		}
	}
	Mes.Clear();

	return ok ;
}
///-----------------------------------------------------------------------------
///  Remove directory for '"m_dirPreBuild"\name-target\'
///
///  Called by : none
///
bool qtPrebuild::removeDir(const wxString& _dirgen )
{
	bool ok = ::wxDirExists(_dirgen) ;
	if (ok)
	{
//Mes = quote( dirgen ) + _T("!!") ; printWarn(Mes);
		ok = ::wxRmdir(_dirgen)  ;
		if (!ok)
		{
			Mes = _("Can't remove directory") ;
			Mes += quote( _dirgen) + _T("!!") ;
			printError(Mes);
			cbMessageBox(Mes, _T(""), wxICON_ERROR) ;
		}
	}
	Mes.Clear();

	return ok ;
}
///-----------------------------------------------------------------------------
/// Must complete the table 'm_Filestocreate'
///
/// Called by
///		1. buildAllFiles(cbProject * prj, bool workspace, bool allbuild)
///
///	Calls to
///		1. copyArray(const wxArrayString& strarray):1,
///		2. inProjectFileCB(const wxString& file):1,
///		3. compareDate(const wxString&  fileref, const wxString&  filetarget):1,
///
uint16_t qtPrebuild::filesTocreate(bool _allrebuild)
{
	m_Filestocreate.Clear();
// save all open files
	bool ok = Manager::Get()->GetEditorManager()->SaveAll();
	if (!ok )
	{
		Mes = _("Couldn't save all files ") ;
		Mes += _T(" !!")  ;
		printError (Mes);
		cbMessageBox(Mes, _T(""), wxICON_ERROR)  ;
		return ok  ;
	}

    uint16_t  ntocreate = 0;
// all rebuild : copy table
	if (_allrebuild)
	{
		m_Filestocreate = copyArray(m_Registered) ;
		ntocreate = m_Filestocreate.GetCount();
	}
// build or rebuild not identical
	else
	{
	// local variables
		wxString inputfile, nameout ;
		bool inproject ;
	// analyze all eligible files/targets
		uint16_t nfiles = m_Filecreator.GetCount() ;
		for (uint16_t i=0; i < nfiles ; i++ )
		{
		// relative name inputfile
			inputfile = m_Filecreator.Item(i) ;
		// file registered to create
			nameout = m_Registered.Item(i) ;
		// already registered
			inproject =  inProjectFileCB(nameout) ;
			m_Identical = false ;
		//- verify datation on disk
			if (inproject)
			{
			// verify if exists file
				if (::wxFileExists(nameout))
				{
				// test his modification date
					m_Identical = compareDate(inputfile, nameout) ;
				}
			}
			// identical date -> no create
			if (m_Identical)
			// add 'm_Devoid' to 'Filetocreate' and continue
				m_Filestocreate.Add(m_Devoid, 1) ;
			else
			{	// to create
				m_Filestocreate.Add(nameout, 1)  ;
				ntocreate++;
			}
		} // end for
	}
	Mes.Clear();

	return ntocreate;
}
///-----------------------------------------------------------------------------
/// Compare file date
///
/// Called by  :
///		1. buildOneFile(cbProject * prj, const wxString& fcreator):1,
///		1. filesTocreate(bool allrebuild):1,
///
bool qtPrebuild::compareDate(const wxString& _fileref, const wxString&  _filetarget)
{
	if ( !(::wxFileExists(_fileref) && ::wxFileExists(_filetarget) ) )
		return false;

	wxFileName refname(_fileref);
	wxDateTime refdate = refname.GetModificationTime();

	wxFileName target(_filetarget);
	wxDateTime targetdate = target.GetModificationTime();

	bool ok = refdate.IsEqualTo(targetdate);

	return ok ;
}
///-----------------------------------------------------------------------------
/// Set date to target
///
/// Called by  :
///		1. createFileComplement(const wxString& qexe, const wxString& fcreator, const wxString& fout):1,
///
bool qtPrebuild::modifyDate(const wxString&  _fileref, const wxString& _filetarget)
{
	if ( !(::wxFileExists(_fileref) && ::wxFileExists(_filetarget) ) )
		return false;

	wxFileName refname(_fileref);
	wxDateTime refdate = refname.GetModificationTime();

	wxFileName target(_filetarget);
	//wxDateTime targetdate = target.GetModificationTime();

	bool ok  = target.SetTimes(0, &refdate, 0);

	return ok ;
}
///-----------------------------------------------------------------------------
/// Create files before build project :
///		- additions built files before generation
/// Algorithm:
///		(-# 'm_Filecreator' is meet by 'findGoodfiles()')
///		(-# 'm_Registered is filled by 'addAllFiles()')
///		(-# 'm_Filestocreate' filled by 'filesTocreate(bool allrebuild)')
///		(-# meet 'm_Createdfile')
///
/// return true if all right
///
/// Called by  :
///		1. buildAllFiles(cbProject * prj, bool workspace, bool allbuild):1,
///
///	Calls to :
///		1. isNoFile(const wxArrayString& arraystr):1,
///		2. findTargetQtexe(buildtarget->GetParentProject()):1,
///		3. createComplement(const wxString& qexe, const uint16_t index):3,
///
bool qtPrebuild::createFiles()
{
// search target qt path for moc, ...
	ProjectBuildTarget * pBuildTarget = m_pProject->GetBuildTarget(m_nameActiveTarget)  ;
	bool ok = pBuildTarget != nullptr ;
	if (!ok)
		return ok ;

	ok = findTargetQtexe(pBuildTarget) ;
	if (!ok)
		return ok ;

// -> debug
//Mes = allStrTable(_T("m_Filestocreate"), m_Filestocreate); print(Mes);

// nothing to do ?
	// 'nfilesTocreate' count good files ( no 'm_Devoid' )
	uint32_t nftocreate = nfilesToCreate(m_Filestocreate)
			, nfcreated = m_Createdfile.GetCount()
			, nftodisk = m_Filewascreated.GetCount();
//Mes = _T("nftocreate = ")  + strInt(nftocreate) ;
//Mes += _T(", nfcreated = ")  + strInt(nfcreated) ;
//Mes += _T(", nftodisk = ")  + strInt(nftodisk) ; printWarn(Mes);
// no 'abort' during last build
	if (nftocreate == nfcreated )
	{
		m_Createdfile.Clear();
		nfcreated = 0;
	//	m_nfilesCreated = 0;
	}
    // nothing to do ?
	bool emptiness = isNoFile(m_Filestocreate) ;
	if (emptiness)
	{
	// already created
		Mes = Tab + _("Nothing to do (") + strInt(nftodisk);
		Mes += Space + _("correct complement file(s) exist on disk)") ;
		print(Mes) ;

		return emptiness  ;
	}

	Mes = Tab + _T("=> ") + _("There are");
	Mes += Space + strInt(nftocreate) + Space ;
	Mes += _("complement file(s) to rebuild") ;
	Mes += _T("...");
	printWarn(Mes);

// used by 'createComplement()'
//	if (m_clean)
//		m_nfilesCreated = nfcreated;
	m_nfilesCreated = 0;
// local variables
	wxString nameout, ext, strerror ;
	bool created = false, abandonment = false ;
	uint16_t nfiles = m_Filestocreate.GetCount() ;
	int32_t ncreated = 0 ;
// analyze all eligible files/target
	for (uint16_t i = 0; i < nfiles ; i++ )
	{
		if (m_abort) {
		// if 'i==0' -> i-1 < 0 !!
			created = false;
			ncreated = i-1;
			break;
		}
		nameout = m_Filestocreate.Item(i) ;
		if (nameout.Matches(m_Devoid) )		continue  ;

	// extension creator
		ext = m_Filecreator.Item(i).AfterLast('.') ;
	// create file
		//1- file *.ui : launch 'uic.exe'
		if ( ext.Matches(m_UI) )
			strerror = createComplement(m_Uexe, i) ;
		else
		//2- file *.qrc : launch 'rcc.exe'
		if (ext.Matches(m_Qrc) )
			strerror = createComplement(m_Rexe, i)  ;
		else
		//3- file with 'Q_OBJECT' and/or 'Q_GADGET' : launch 'moc.exe'
		if (ext.Matches(EXT_H) || ext.Matches(EXT_HPP) || ext.Matches(EXT_CPP) )
			strerror = createComplement(m_Mexe, i)  ;
		// more one
		created = strerror.IsEmpty();
		if (created)
		{
			m_Createdfile.Add(nameout, 1)  ;
		}
		else
		{
		// 'validCreated()' detect 'm_Devoid'  !!
			m_Createdfile.Add(m_Devoid, 1)  ;
		// error message
			wxString title = _("Creating") + quote( m_Registered.Item(i)) ;
			title += _("failed") ;
			title += _T(" ...");
			// error create complement
			Mes =  Tab +_T("=> ") ;
			Mes += strerror.BeforeLast(Lf.GetChar(0)) ;
			printError (Mes) ;
        // message box
            Mes = strerror.BeforeLast(Lf.GetChar(0)) ;
			Mes += Lf + _("Do you want to stop pre-construction only ?");
			int16_t choice = cbMessageBox(Mes, title, wxICON_QUESTION | wxYES_NO) ;
			abandonment = choice == wxID_YES ;
			if (abandonment)
			{
                Mes = Tab  + _("Abandonment of the pre-construction") + _T(" ...");
				printError (Mes) ;
				break;
			}
			else
			{
				Mes = Tab  + _("Continue building") + _T(" ...");;
				printWarn (Mes) ;
				created = true;
			}
		}
	}
// Abandonment of the construction of supplements
	if (abandonment)
	{
	// the last files is 'm_devoid'
    // ...
	}
// Abortion of the construction of supplements
	if (m_abort)
	{
	// 'ncreated' files were created normally
		Mes = Tab + _T("=> ") + _("You have created normally") + Space + strInt(ncreated) + _T("/") ;
		Mes += strInt(nfiles) + Space + _("file(s)");
		print(Mes);
    // adversing
		Mes = _("A stop was requested during the creation of the complement file(s) !!");
		printWarn(Mes) ;
		cbMessageBox(Mes, _T("Create File(...)"), wxICON_WARNING ) ;
	// adjust 'm_Createdfile'
		wxArrayString  temp ;
		temp = copyArray(m_Createdfile, ncreated);
			m_Createdfile.Clear();
			m_Createdfile = copyArray(temp);
	// adjust 'm_Registered'
		temp = copyArray(m_Registered, ncreated);
			m_Registered.Clear();
			m_Registered= copyArray(temp);
	// remove old file on disk
    // ...
 // Debug
//uint16_t nf = m_Createdfile.GetCount() ;
//Mes =  Tab + _T("- ") + strInt(nf) + Space  ;
//Mes += _("complement(s) file(s) are created in the target") ; printWarn(Mes)  ;
	}
	Mes.Clear();

	return created ;
}
///-----------------------------------------------------------------------------
/// Search all 'm_Devoid' to an wxArrayString
///
/// Called by  :
///		1. createFiles():1,
///
bool qtPrebuild::isNoFile (const wxArrayString& _arraystr)
{
	bool devoid = false ;
	for (const auto item : _arraystr)
	{
		devoid = item.Matches(m_Devoid) ;
		if (!devoid)
			break  ;
	}

	return devoid ;
}
///-----------------------------------------------------------------------------
/// Search all files to create into an wxArrayString
///
/// Called by  :
///		1. createFiles():1,
///
uint16_t qtPrebuild::nfilesToCreate(const wxArrayString& _arraystr)
{
	uint16_t nftocreate = 0;
	for (const auto item : _arraystr)
	{
		if(! item.Matches(m_Devoid) )
			nftocreate++;
	}

	return nftocreate;
}
///-----------------------------------------------------------------------------
///	Execute commands 'moc', 'uic', 'rcc' : return _("") if file created
///	else return an error string
///
/// Called by  :
///		1. buildOneFile(cbProject * prj, const wxString& fcreator):1,
///		2. createComplement(const wxString& qexe, const uint16_t index):1,
///
///	Calls to :
///		1. executeAndGetOutputAndError():1,
///		2. modifyDate(const wxString&  fcreator, const wxString& fout):1,
///
wxString qtPrebuild::createFileComplement(const wxString& _qexe,
										  const wxString& _fcreator,
										  const wxString& _fout)
{
// build command
	wxString command = _qexe  ;
//Mes = _T("qtPrebuild::createFileComplement() => command = *>") + command  + _T("<*");
//printWarn(Mes);
// add file name whithout extension
	if (_qexe.Matches(m_Rexe))
	{
		wxString name = _fcreator.BeforeLast('.')  ;
		command +=  _T(" -name ") + name  ;
	}
	if (_qexe.Matches(m_Mexe))
	{
		command +=  m_DefinesQt + m_IncPathQt ;
	}
// add input file
	command += Space + _fcreator ;
// add output file
	command += _T(" -o ") + _fout;
//Mes = Tab + _T("=> command = *>") + command  + _T("<*"); printWarn(Mes);

// execute command line : use short file name
	wxString strerror = executeAndGetOutputAndError(command, PREPEND_ERROR)  ;
	bool created =  strerror.IsEmpty() ;
// file-project              : params = name, is relative, is Unixfilename
	ProjectFile * prjfile =  m_pProject->GetFileByFilename(_fout, IS_RELATIVE, IS_UNIX_FILENAME) ;
	if (!prjfile)
	{
		Mes = _fout + _(" is no correct !!");
		return  Mes;
	}
// request abort ?
	if (m_abort)
	{
		created = false;
	}
// create error
	if (! created )
	{
	// unregister the last file 'fout' in the project
		m_pProject->RemoveFile(prjfile) ;
		//  >= svn 9501 : CB 13.12
		Manager::Get()->GetProjectManager()->GetUI().RebuildTree() ;
	}
	else
	{
	// modify date of created file 'fout'
		m_Identical = modifyDate(_fcreator ,_fout) ;
    // more one file
		++m_nfilesCreated ;
	// display
		bool nocompile = ! prjfile->compile
			,nolink = ! prjfile->link;

		Mes = Tab + strInt(m_nfilesCreated) + _T("-") + quote(_fcreator)  ;
		Mes +=  _T("--->") + quote(_fout) ;
		if (nocompile && nolink)
		{
			Mes += Tab + _T("(") + _("no compiled, no linked") + _T(")");
			printWarn(Mes);
		}
		else
			print(Mes);
	}
	Mes.Clear();

// return  error  : good if strerror is empty
	return strerror ;
}
///-----------------------------------------------------------------------------
///	Execute commands 'moc', 'uic', 'rcc' : return _("") if file created
///	else return an error string
///
/// Called by  :
///		1. createFiles():3,
///	Calls to :
///		1. createFileComplement(const wxString& qexe, cnst wxString& fcreator, const wxString& fout):1,
///
wxString qtPrebuild::createComplement(const wxString& _qexe, const uint16_t _index)
{
//Mes = "qtPrebuild::createComplement(...)" ; print(Mes);
//1- name relative input file  "src\filecreator.xxx"
	wxString inputfile = m_Filecreator.Item(_index) ;
	// create directory for m_nameActiveTarget
	wxString dircomplement = m_Registered.Item(_index).BeforeLast(Slash)  ;
	dircomplement +=  wxFILE_SEP_PATH ;

	if (!createDir(dircomplement))
	{
		Mes = _("Unable to create directory") + quote( dircomplement) ;
		return Mes ;
	}
//2- add search path for compiler
	ProjectBuildTarget * pBuildTarget = m_pProject->GetBuildTarget(m_nameActiveTarget) ;
	if (!pBuildTarget)
	{
		Mes = quote(m_nameActiveTarget) + _("does not exist") + _T(" !!") ;
		return Mes ;
	}

	// add include directory
//	bool incdir = pBuildTarget->GetIncludeInTargetAll(); // ??
	pBuildTarget->AddIncludeDir(dircomplement) ;
//3- full path name  output file
	wxString outputfile = m_Registered.Item(_index) ;
//4- create on file complement
	wxString strerror = createFileComplement(_qexe, inputfile, outputfile) ;
//5- return  error  : good if 'strerror' is empty
	return strerror ;
}
///-----------------------------------------------------------------------------
/// Give include path qt for 'm_Moc'
///
/// Called by :
///		1. findTargetQtexe(CompileTargetBase * buildtarget):1,
///
wxString qtPrebuild::pathIncludeMoc(const CompileTargetBase * _pContainer)
{
	wxArrayString tabincludedirs = _pContainer->GetIncludeDirs(), tabpath;
	wxString incpath;
	for (wxString& line : tabincludedirs)
	{
		m_pMam->ReplaceMacros(line) ;
		line.Prepend(_T("-I"));
		tabpath.Add(line, 1);
	}
	// build 'incpath'
	if (tabpath.GetCount())
		incpath = Space + GetStringFromArray(tabpath, Space, SEPARATOR_AT_END) + Space;

	return incpath  ;
}

///-----------------------------------------------------------------------------
/// Give 'defines' qt for 'm_Moc'
///
/// Called by :
///		1. findTargetQtexe (CompileTargetBase * buildtarget):1,
///
wxString qtPrebuild::definesMoc(CompileTargetBase * _pContainer)
{
	wxArrayString tabdefines = _pContainer->GetCompilerOptions(), tabdef ;
	wxString def ;
	for (const wxString line : tabdefines)
	{
		if (line.Find(_T("-D")) != -1 )
			tabdef.Add(line, 1) ;
	}
	// build 'def'
	if (tabdef.GetCount())
		def = _T(" ") + GetStringFromArray(tabdef, _T(" "), true) + _T(" ") ;

	return def ;
}
///-----------------------------------------------------------------------------
/// Verify if at least one valid file is saved in 'm_Createdfile'
///
/// Called by  :
/// 	1. buildAllFiles(cbProject * prj, bool workspace, bool allbuild):1,
///
bool qtPrebuild::validCreated()
{
	bool tocreate = false, ok = true;
	uint16_t ncase = m_Createdfile.GetCount()  ;
	while (ncase > 0 && !tocreate)
	{
		tocreate = ! m_Createdfile.Item(--ncase).Matches(m_Devoid)  ;
//Mes =  _T("validCreated() -> ") + strInt(ncase) + _T(" = ") +  strBool(tocreate) ;
//printWarn(Mes);
	}
	if (tocreate)
	{
		// svn >= svn 9501 : CB 13.12
		Manager::Get()->GetProjectManager()->GetUI().RebuildTree()  ;
	// save project
		ok = Manager::Get()->GetProjectManager()->SaveProject(m_pProject)  ;
		if(!ok)
		{
			Mes = _("Save project is not possible !!");
			printError(Mes);
		}
	}
	Mes.Clear();

	return ok ;
}
///-----------------------------------------------------------------------------
/// Execute another program (always synchronous).
///
/// Called by :
///		1. createFileComplement(const wxString& qexe, const wxString& fcreator, const wxString& fout):1,
///
wxString qtPrebuild::executeAndGetOutputAndError(const wxString& _command, bool _prepend_error)
{
	wxArrayString output;
	wxArrayString error;
	wxExecute(_command, output, error, wxEXEC_NODISABLE);

	wxString str_out;

	if ( _prepend_error && !error.IsEmpty())
		str_out += GetStringFromArray(error, Lf);

	if (!output.IsEmpty())
		str_out += GetStringFromArray(output, Lf);

	if (!_prepend_error && !error.IsEmpty())
		str_out += GetStringFromArray(error,  Lf);

	return  str_out;
}

///-----------------------------------------------------------------------------
