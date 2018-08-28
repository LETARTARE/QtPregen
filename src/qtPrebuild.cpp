/***************************************************************
 * Name:      qtprebuild.cpp
 * Purpose:   Code::Blocks plugin
 * Author:    LETARTARE
 * Created:   2015-10-17
 * Modified:  2018-08-28
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
#include <projectfile.h>
// not change  !
#include "print.h"
///-----------------------------------------------------------------------------
/// Constuctor

/// Called by :
///		1. QtPregen::OnAttach():1,
///
qtPrebuild::qtPrebuild(cbProject * prj, int logindex, wxString & nameplugin)
	: qtPre(prj, logindex, nameplugin)
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
///		1.buildAllFiles(cbProject * prj, bool workspace, bool allbuild):1,
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

// display log
	Mes = _T("-------------- ") ;
	Mes += _T("PreBuild : ") ;
	Mes += Quote + m_nameActiveTarget + Quote;
	Mes += _(" in ");
	Mes += Quote + m_nameProject + Quote;
	Mes += _T("-------------- ") ;
	printWarn(Mes);
// date
	Mes = _("started on : ")+ date()  ;
	print(Mes);
// for duration
	m_start = clock();
}

///-----------------------------------------------------------------------------
///  Build create end banner
///
/// Called by :
///		1. buildAllFiles(cbProject * prj, bool workspace, bool allbuild):1,
/// Calls to :
///		1. date():1,
///		2. duration():1,
///
void qtPrebuild::endMesBuildCreate()
{
// date and duration
	Mes = _("ended on : ") + date() ;
	print(Mes) ;
	Mes =  _("duration = ") + duration() ;
	printWarn(Mes);
}
///-----------------------------------------------------------------------------
/// Generating the complements files...
///
/// Called by :
///		1. QtPregen::OnAddComplements(CodeBlocksEvent& event):1
///
/// Calls to :
///		1. beginMesBuildCreate():1,
///		2. findGoodfiles():1,
///		3. addAllFiles():1,
///		4. filesTocreate(bool allrebuild) :1,
///		5. createFiles():1,
///		6. validCreated():1,
///		7. endMesBuildCreate():1,
///
bool qtPrebuild::buildAllFiles(cbProject * prj, bool workspace, bool allbuild)
{
	m_pProject = prj;
	bool ok = m_pProject != nullptr;
	if (!ok)
		return ok;

/// DEBUG
//* **********************************
//	beginDuration(_T("buildAllFiles(...)"));
//* *********************************
// begin banner
	beginMesBuildCreate();

	ok = false;
	m_clean = false;
	///***********************************************
	///1- find good target with eligible files
	///***********************************************
	// analyzing all project files
	Mes = _("Search creator files ...") ;
		printWarn(Mes);
	int nelegible =  findGoodfiles() ;
	if (nelegible > 0)
	{
		///************************************************
		///2-  registered all files in 'm_Registered'
		///************************************************
		int nfiles = addAllFiles() ;
		Mes = Tab + _T("-> ") + (wxString()<<nfiles) ;
		Mes += Space + _("creator(s) file(s) registered in the plugin") ;
			print(Mes) ;
		if (nfiles > 0)
		{
			///************************************************
			///3-  files to create to 'm_Filestocreate'
			///************************************************
			uint16_t ntocreate = filesTocreate(allbuild) ;
			if (ntocreate == 0 )
				ok = !allbuild ;
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
				Mes = _("Creating complement files ...") ;
				printWarn(Mes);
				bool good = createFiles() ;
				if (good)
				{
					///**********************************
					///5- register files created
					///**********************************
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
						Mes = Tab + _("Abort complement files building ...")  ;
						printWarn(Mes);
						m_abort = false;
					}
					else
					{
						Mes = Tab + _("Complement file creation error !!") ;
						printErr(Mes);
					}
				}
				ok = good;
			}	//end (ntocreate > 0)
			else
			{
				Mes = Tab + _("File to create error !!") ;
				printErr(Mes);
			}
		} // end (nfiles > 0)
		else
		{
			Mes = Tab + _("File registration error !!") ;
			printErr(Mes);
		}
	} // end (nelegible > 0)
	else
	{
		Mes = Tab + _("No elegible file (with 'Q_OBJECT' or 'Q_GADGET' !!") ;
		printWarn(Mes);
		ok = true;
	}

    m_clean = m_Registered.IsEmpty();
// end banner
	endMesBuildCreate();

/// DEBUG
//* **********************************
//	endDuration(_T("buildAllFiles(...)"));
//* *********************************

	return ok;
}
///-----------------------------------------------------------------------------
///  Build create one file start banner
///
/// Called by :
///		1. buildOneFile(cbProject * prj, const wxString& fcreator) :1,
/// Calls to :
///		1. date():1,
///
void qtPrebuild::beginMesFileCreate()
{
// banner
	printLn;
	Mes = _T("-------------- ") ;
	Mes += _T("PreCompileFile : ") ;
	Mes += Quote + m_pProject->GetActiveBuildTarget() + Quote;
	Mes += _(" in ");
	Mes += Quote + m_pProject->GetTitle() + Quote;
	Mes += _T(" : ") + Quote + m_filename + Quote;
	Mes += _T("-------------- ") ;
	printWarn(Mes);
// date
	Mes = _("started on : ") + date() ;
	print(Mes);
// for duration
	m_start = clock();
}
///-----------------------------------------------------------------------------
///  Build create one file end banner
///
/// Called by :
///		1. buildOneFile(cbProject * prj, const wxString& fcreator) :1,
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
}
///-----------------------------------------------------------------------------
/// Generating only ONE complement file...
///
///  Called by :
///		1. QtPregen::OnAddComplements(CodeBlocksEvent& event):1,
///
///  Calls to :
///		1.	beginMesFileCreate():1,
///		2.	isElegible(const wxString& file):1,
///		3.	inProjectFileCB(const wxString& file):1,
///		4.	compareDate(const wxString&  fileref, const wxString&  filetarget):1
///		5.  createFileComplement(const wxString& qexe, const wxString& fcreator, const wxString& fout):1,
///		6.	endMesFileCreate():1,
///

bool qtPrebuild::buildOneFile(cbProject * prj, const wxString& fcreator)
{
	if(!prj)
		return false;

	m_pProject = prj;
	m_filename = fcreator;

// directory changed
	m_dirProject = m_pProject->GetBasePath();
	wxFileName::SetCwd (m_dirProject);
	bool ok = createDir (m_dirPreBuild) ;
	if (!ok)
		return ok;

// file it is correct ?
    bool elegible = isElegible(fcreator);
    if (!elegible)
		return elegible ;

// begin pre-Compile
	beginMesFileCreate() ;
// init Qt tools
	ok = findTargetQtexe(prj) ;
	if (!ok)
		return ok ;

	m_nameActiveTarget =  m_pProject->GetActiveBuildTarget() ;
	// targets no virtual list
	wxArrayString rltargets;
// virtual target ?  without warning
	bool virt = isVirtualTarget(m_nameActiveTarget);
	if (virt)
		rltargets = listRealsTargets(m_nameActiveTarget) ;
	else
		rltargets.Add(m_nameActiveTarget);

	uint16_t ntargets = rltargets.GetCount();
	ProjectBuildTarget* buildtarget;
	wxString fout;
    for (uint16_t nt = 0 ; nt < ntargets ; nt++ )
    {
		m_nameActiveTarget = rltargets.Item(nt);
		buildtarget = m_pProject->GetBuildTarget(m_nameActiveTarget);
		if (buildtarget && buildtarget->GetTargetType() == ttCommandsOnly )
			continue;

		fout = complementDirectory() + nameCreated(fcreator) ;
		Mes = Tab + Quote + fcreator + Quote + _T(" -> ") + Quote + fout + Quote ;
		print(Mes);
	// already registered
		bool inproject = inProjectFileCB(fout) ;
		if (!inproject)
		{
		// has included ?
			wxString extin = fcreator.AfterLast('.') ;
			wxString extout = fout.AfterLast('.')  ;
			bool nocompile = false ;
			if ( extin.Matches(EXT_H) && extout.Matches(EXT_CPP) )
				nocompile =  hasIncluded(fcreator) ;
		// add file : AddFile(Nametarget, file, compile, link, weight)
			ProjectFile* prjfile = m_pProject->AddFile(m_nameActiveTarget, fout, !nocompile, !nocompile, 50);
			if (!prjfile)
			{
			// display message
				Mes  = _T("===> ") ;
				Mes += _("can not add this file ");
				Mes += Quote + fout + Quote + _(" to target ") + m_nameActiveTarget ;
				printErr (Mes) ;
				cbMessageBox(Mes, _T("AddFile(...)"), wxICON_ERROR) ;
			}
		// display
			if (nocompile)
			{
				Mes = Tab + _("Add ") + Quote + fout + Quote ;
				Mes += Lf + Tab + Tab + _T("*** ") + _("This file is included, ") ;
				Mes += _("attributes 'compile' and 'link' will be set to 'false'") ;
				print(Mes);
			}
			// svn 9501 : CB 13.12  and >
			Manager::Get()->GetProjectManager()->GetUI().RebuildTree() ;

		// create complement ...
			m_Identical = false;
		}
		else
		{
		// Check the date
			m_Identical = false ;
			//- verify datation on disk
			if (wxFileExists(fout))
				m_Identical = compareDate(fcreator, fout) ;
		}

		if (m_Identical)
		{
			Mes = Tab + _("Nothing to do (complement file exist on disk)") ;
			print(Mes) ;
		}
	// ! identical date -> create
		else
		{
			Mes = Tab + _(" One complement file are created in the project ...") ;
			printWarn(Mes);
		// create file complement with 'moc'
			wxString strerror = createFileComplement(m_Mexe, fcreator, fout);
			if (!strerror.IsEmpty())
			{
			// error message
				wxString title = _("Creating ") + Quote + fout + Quote ;
				title += _(" failed") ;
				title += _T(" ...");
				//1- error create directory  :
					// = _("Unable to create directory ")
				//2- error buildtarget no exist :
					// = Quote + m_nameActiveTarget + Quote + _(" does not exist !!") ;
				//3- error create complement
				Mes =  _T("=> ") ;
				Mes += strerror.BeforeLast(Lf.GetChar(0)) ;
				printErr (Mes) ;
				cbMessageBox(Mes, title) ;
			}
		}
	}
// end preCompile
	endMesFileCreate() ;

    return elegible;
}

///-----------------------------------------------------------------------------
/// Reference of libQt by target
///		"" or "4r" or "4d" or "5r" or "5d"
///  	qt4 -> '4', qt5 -> '5', release -> 'r' and debug -> 'd'
///
/// Called by  :
///		-# isGoodTargetQt(const ProjectBuildTarget * target):1,
///
wxString qtPrebuild::refTargetQt(const ProjectBuildTarget * buildtarget)
{
	wxString refqt ;
	if (! buildtarget)
		return refqt  ;
//Mes = _T("qtPrebuild::refTargetQt(...)");
//printWarn(Mes);
	wxArrayString tablibs = buildtarget->GetLinkLibs();
	int nlib = tablibs.GetCount() ;
//Mes = _T("nlib = ") + (wxString() << nlib);
//printWarn(Mes);
    if (!nlib)
    {
        tablibs = m_pProject->GetLinkLibs();
        nlib = tablibs.GetCount() ;
        if (!nlib)
        {
            Mes =  _("This target has no linked library !!");
            printErr(Mes);
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
//printWarn(namelib);
		// no prefix "lib"
		pos = namelib.Find(_T("lib"));
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
		{
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
			{
				refqt = _T("4") ;
				str = namelib.Right(2);
				if (str.Matches(_T("d4") ) )
					refqt += _T("d") ;
				else
					refqt += _T("r") ;
			}
			break;
		}
	}

	return refqt ;
}

///-----------------------------------------------------------------------------
/// Find path Qt container  ->  'CompileOptionsBase * container'
///
/// Called by  :
///		1. findTargetQtexe(CompileTargetBase * buildtarget):1,
///
wxString qtPrebuild::pathlibQt(CompileTargetBase * container)
{
	wxString path;
	if (!container)
		return path;

	wxArrayString tablibdirs = container->GetLibDirs() ;
	int npath = tablibdirs.GetCount() ;
//Mes = _T("path lib number = ") + wxString()<<npath;
//printErr(Mes);
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
//Mes = Lf + _("$qt => ") + Quote + path + Quote;
//printWarn(Mes);
				m_pMam->ReplaceEnvVars(path) ;
//Mes = Lf + _("Local variable path Qt => '") + path; Mes += _T("'");
//print(Mes);
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
//Mes = Lf + _("#qt => ") + Quote + path + Quote;
//printWarn(Mes);
					m_pMam->ReplaceMacros(path) ;
//Mes = Lf + _("Global variable path Qt => '") + path;Mes += _T("'");
//print(Mes);
					path_nomacro =  path ;
					// remove "\lib"
					path_nomacro = path_nomacro.BeforeLast(Slash) ;
					path_nomacro += wxString(Slash)  ;
				}
				// no variable ! , absolute path ?
				else
				{
			//	Mes = Lf + _("Path Qt => '") + path ; Mes += _T("'");
			//	printWarn(Mes);
					path_nomacro += path;
				}
			}
		}
	}

	return path_nomacro  ;
}

///-----------------------------------------------------------------------------
/// Find path Qt exe : 'ProjectBuildTarget* buildtarget'
///
/// Called by  :
///		1.	createFiles():1
///		2.	buildOneFile(cbProject * prj, const wwxFileExistsxString& fcreator):1
///

bool qtPrebuild::findTargetQtexe(CompileTargetBase * buildtarget)
{
	if (! buildtarget)
		return false  ;

	wxString qtpath = pathlibQt(buildtarget) ;
	if(qtpath.IsEmpty())
	{
		Mes = _("No library path QT 'in the target") + Lf + _("or nothing library !")  ;
		Mes += Lf +  _("Check that you have a correct path to the Qt path libraries !") ;
		Mes += Lf + _("Cannot continue.") ;
		printErr(Mes);
	//	cbMessageBox(Mes, _T(""), wxICON_ERROR) ;
		return false ;
	}
	// Qt path
//	Mes = Lf + _("QT path for the target = '") + qtpath + _T("'");
//	print(Mes);
	wxString qtexe = qtpath + _T("bin") + wxFILE_SEP_PATH  ;
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

	bool Findqtexe = wxFileExists(m_Mexe) && wxFileExists(m_Uexe) ;
	Findqtexe = Findqtexe && wxFileExists(m_Rexe) && wxFileExists(m_Lexe) ;
	if (!Findqtexe)
	{
		Mes = _("Could not query the executable Qt") ;
		Mes +=  _T(" !") ;
		Mes += Lf + _("Cannot continue,") + Space + _("Verify your installation Qt.");
		printErr(Mes);
		cbMessageBox(Mes, _("Search executable Qt ..."), wxICON_ERROR) ;
	}
	m_IncPathQt = pathIncludeMoc(m_pProject) +  pathIncludeMoc(buildtarget);
	m_DefinesQt = definesMoc(m_pProject) + definesMoc(buildtarget);

	return Findqtexe ;
}
///-----------------------------------------------------------------------------
///	Read content file
///
/// Called by  :
/// 	1. q_object(filename, qt_macro):1,
///		2. hasIncluded(const wxString& fcreator):1,
///
wxString qtPrebuild::ReadFileContents(const wxString& filename)
{
	wxFileName fname(Manager::Get()->GetMacrosManager()->ReplaceMacros(filename));
	NormalizePath(fname, wxEmptyString);
	wxFile f(fname.GetFullPath());

	return cbReadFileContents(f);
}
///-----------------------------------------------------------------------------
///	Write content file
///
/// Called by  : none
///
bool qtPrebuild::WriteFileContents(const wxString& filename, const wxString& contents)
{
	wxFileName fname(Manager::Get()->GetMacrosManager()->ReplaceMacros(filename));
	NormalizePath(fname, wxEmptyString);
	wxFile f(fname.GetFullPath(), wxFile::write);

	return cbWrite(f, contents);
}

///-----------------------------------------------------------------------------
/// Looking for eligible files to the active target (around project files),
/// 	meet to table 'm_Filecreator' : return eligible file number
///
/// Called by  :
///		1. buildAllFiles(cbProject * prj, bool workspace, bool allbuild):1
///
/// Calls to :
///		1. isVirtualTarget(m_nameactivetarget):1,
///		2. isGoodTargetQt(m_nameActiveTarget):1,
///		3. isElegible(file):1,
///
uint16_t qtPrebuild::findGoodfiles()
{
//	Mes = _("Search creator files ...") ;
//	printWarn(Mes);
	/// DEBUG
//* **********************************
//	beginDuration(_T("findGoodFiles(...)"));
//* *********************************
	m_Filecreator.Clear();
// active target
	m_nameActiveTarget = m_pProject->GetActiveBuildTarget() ;
	if (m_nameActiveTarget.IsEmpty() )
		return 0;
// virtual ?
	if (isVirtualTarget(m_nameActiveTarget, true) )
		return 0 ;
// is good target  ?
	ProjectBuildTarget * target = m_pProject->GetBuildTarget(m_nameActiveTarget);
	if (!target)
		return 0 ;
	if (! isGoodTargetQt(target))
		return 0 ;

// **********************
// Around project files
// **********************
	ProjectFile * prjfile ;
	wxArrayString tabtargets ;
	wxString file, nametarget;
	int nt, nfprj = m_pProject->GetFilesCount()  ;
	bool good ;
	/// DEBUG
//* **********************************
//	beginDuration(_T("findGoodFiles:: all files)"));
//* *********************************
// all files project
	for (int nf =0 ; nf < nfprj; nf++ )
	{
		prjfile = m_pProject->GetFile(nf);
		if (!prjfile)
			continue  ;
	//	file name
		file = prjfile->relativeFilename ;
		if (file.IsEmpty() )
			continue  ;
		// no complement file
		if (file.StartsWith(m_dirPreBuild) )
			continue  ;
//Mes = _T("file = ") + file;
//print(Mes);
		//m_pm->Yield() ;
	// all targets
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
		//	Mes = _T("nametarget = ") + nametarget;
		//	print(Mes);
//Mes = Tab + _T("file finded ") + (wxString()<<nf) + _T(" = ") + file;
//printWarn(Mes);
			// not eligible ?
				if (! isElegible(file))
					continue  ;
//Mes = Tab + _T("file elegible ") + (wxString()<<nf) + _T(" = ") + file;
//printWarn(Mes);
			// add the file and it target
				m_Filecreator.Add(file, 1) ;
//Mes = _T("elegible file = ") + file;
//print(Mes);
			}
			else
			{
				// no target active !!
//Mes = _T("no target active = ") + nametarget;
//print(Mes);
			}
		}
	// execute pending events
		m_pm->ProcessPendingEvents();
	}
	/// DEBUG
//* **********************************
//	endDuration(_T("findGoodFiles:: all files)"));
//* *********************************

//  number file eligible
	return m_Filecreator.GetCount()  ;
}
///-----------------------------------------------------------------------------
/// Give the good type of target for Qt
///
/// Called by  :
///		1. QtPregen::OnAddComplements(CodeBlocksEvent& event):1,
///		2. qtPrebuild::findGoodfiles():1,
///		3. qtPrebuild::findGoodfilesTarget():1,
///
/// Calls to:
///		1. refTargetQt(ProjectBuildTarget * buildtarget):1,
///
bool qtPrebuild::isGoodTargetQt(const ProjectBuildTarget * target)
{
	if (!target)
		return false ;

	bool ok = target->GetTargetType() != ::ttCommandsOnly  ;
	wxString str = refTargetQt(target)  ;
//Mes = _T("str = ") + Quote + str + Quote;
//printWarn(Mes);
	bool qt = ! str.IsEmpty()  ;
	if (!qt)
	{   // TODO ...
		Mes = Tab + _("This target have nothing library Qt !") ;
		Mes += Lf + Tab + _("PreBuild cannot continue.") ;
		printWarn(Mes);
	}

	return ok && qt ;
}

///-----------------------------------------------------------------------------
/// Search elegible files
/// 'file' was created ?  ->  moc_*.cxx, ui_*.h, qrc_*.cpp
///
/// Called by  :
///		1. findGoodfilesTarget():1,
///		2. buildOneFile(cbProject * prj, const wxString& fcreator):1,
///
/// Calls to :
/// 	1. q_object(const wxString& filename, const wxString& qt_macro):1;
///
bool qtPrebuild::isElegible(const wxString& file)
{
	wxString ext = file.AfterLast('.')  ;
	bool ok = ext.Matches(m_UI) || ext.Matches(m_Qrc) ;
	if (ok)
		return ok;

	ok = false ;
	if (ext.Matches(EXT_H) || ext.Matches(EXT_HPP) || ext.Matches(EXT_CPP) )
		ok = q_object(file, _T("Q_OBJECT") ) > 0  ;

	return ok ;
}

///-----------------------------------------------------------------------------
/// Return occurrences of "Q_OBJECT" + occurrences of "Q_GADGET"
///
/// Called by  :
///		1. isElegible(const wxString& file):1
///
int qtPrebuild::q_object(const wxString& filename, const wxString& qt_macro)
{
		/// DEBUG
//* **********************************
//	beginDuration(_T("q_object(") + filename + _T(", ") + qt_macro + _T(")"));
//* *********************************

//1- the comments
	wxString CPP = _T("//") ;
	wxString BCC = _T("/*"), ECC = _T("*/") ;
	// ends of line
	wxString SA, SD;
	SA.Append(SepA); SD.Append(SepD) ;

//2- verify exist  : 'filename' is relative file name
	wxString namefile = m_dirProject + filename ;
	if (! wxFileExists(namefile))
	{
		Mes = Quote + namefile + Quote + _(" NOT FOUND") + _T(" !!!") + Lf ;
		printErr(Mes) ;
		return -1  ;
	}
//3- read source 'namefile'
	wxString source = ReadFileContents(namefile) ;
	if (source.IsEmpty())
	{
		Mes = Quote + namefile + Quote + _(" : file is empty") +  _T(" !!!") + Lf ;
		printErr(Mes) ;
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
		if ( qt_macro.Matches(findtext) )
		{
			number = qt_find(temp, findtext)  ;
			findtext = _T("Q_GADGET") ;
			number += qt_find(temp, findtext)  ;
		}
		else
		{
			Mes = _("The file") + Space + filename + Space;
			Mes += _("is not a good creator !!") ;
			printErr(Mes);
		}

	}
	/// DEBUG
//* **********************************
//	endDuration(_T("q_object(...)"));
//* *********************************

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
int  qtPrebuild::qt_find (wxString tmp, const wxString& qt_text)
{
	uint8_t tab = 0x9, espace = 0x20  ;
	int len_text = qt_text.length() ;
	int posq, number = 0;
	bool ok, good, goodb, gooda ;
	do
	{
		posq = tmp.Find(qt_text) ;
		ok = posq != -1 ;
		if (ok)
		{
		// pred and next caracter
			//wxUChar
			uint8_t xcar = tmp.GetChar(posq-1) ;
			//wxUChar
			uint8_t carx = tmp.GetChar(posq + len_text)  ;
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
			tmp.Remove(0, posq + len_text)  ;
		}
	}
	while (ok) ;

	return number ;
}

///-----------------------------------------------------------------------------
/// To remove one complement files in project :
///		(1. 'm_Registered.item(pos)' is clear)
///		is it necessary ?
///		(2. disk : "qtprebuild\nametarget\cmplement" is removed)
///		return 'true' when right
///
/// Called by : none
///

bool qtPrebuild::removeComplementToDisk(const wxString & filename, int16_t index)
{
	bool ok = ::wxFileExists(filename);
	if (ok)
	{
		ok = ::wxRemoveFile(filename);
		if (ok)
		{
			Mes = _("Complement file(s) was removed from disk ...") ;
			printWarn(Mes);
			Mes = Tab + (wxString() << index) +_T(": ") + Quote + filename + Quote;
			print(Mes);
		}
		else
		{
			Mes = Tab + _("Can not delete") + Space + Quote + filename + Quote + _T(" !!");
			printErr(Mes) ;
		}
	}
	else
	{
		Mes = Tab + _("File") + Quote + filename + Quote + Space + _("not Found !!");
		printErr(Mes) ;
	}

	return ok;
}

///-----------------------------------------------------------------------------
///	To unregister a project file (creator and complements) in 'qtPregen'
///		format 'm_Filecreator' ->  'dircreator\xxx.h'  ( src\xxx.h)
/// 	return 'true' when right
///
///	Called by :
///		1. QtPregen::onProjectFileRemoved(CodeBlocksEvent& event):1,
///
///	Calls to :
///		1. unregisterFileComplement(const wxString & file, bool first):1,
///

bool qtPrebuild::unregisterFileCreator(const wxString & file, bool first)
{
	// copy file, absolute project path
	wxString filename = file, pathproject = m_pProject->GetCommonTopLevelPath();
	// relative file path
	filename.Replace(pathproject, _T("") ) ;
//Mes = _T(" ==> creator filename to find = ") + Quote + filename + Quote;
//print(Mes);
	int16_t index = m_Filecreator.Index (filename);
	bool ok = index != wxNOT_FOUND;
	if (!ok)
		return ok;
	// remove creator
	m_Filecreator.RemoveAt(index);
	if (first)
	{
		Mes =  Lf +_("Creator file(s) was removed from plugin ...") ;
		printWarn(Mes);
	}
	Mes = Tab + _T("- ") + Quote + filename + Quote;
	print(Mes);
    // remove complements
	ok = unregisterFileComplement(file, true, first);

	return ok;
}

///-----------------------------------------------------------------------------
///	To unregister project file(s) (complements) in 'qtPregen'
///		format 'm_Registered' ->  'dircomplement\moc_xxx.cpp'  (adding\debug\moc_xxx.cpp)
/// 	return 'true' when right
///
///	Called by :
///		1. QtPregen::onProjectFileRemoved(CodeBlocksEvent& event):1,
///		2. unregisterFileCreator(const wxString & file, bool first):n,
///
///	Calls to :
///		1. qtPre::nameCreated(const wxString& file):1,
///
bool qtPrebuild::unregisterFileComplement(const wxString & file, bool creator, bool first)
{
	// copy file, absolute project path
	wxString filename = file, pathproject = m_pProject->GetCommonTopLevelPath();
	// relative file path
	filename.Replace(pathproject, _T("") ) ;
//Mes = Lf + _T(" ==> filename to find = ") + Quote + filename + Quote;
//print(Mes);
	if (creator)
		filename = nameCreated(filename.AfterLast(Slash));
//Mes = _T(" ==> complement to find = ") + Quote + filename + Quote;
//print(Mes);
	if(first)
	{
		Mes = _("Complement file(s) was removed from plugin ...") ;
		printWarn(Mes);
	}
	// we look for the number of 'complement' occurrences in 'm_Registered'
	bool ok = false;
	uint16_t nf = m_Registered.GetCount();
	wxString item;
	while (nf--)
	{
		item = m_Registered.Item(nf);
//Mes =  Tab + (wxString() << nf) +  _T(" ==> ") + Quote + item + Quote;
//print(Mes);
		if (!item.Contains(filename))
			continue ;
	// remove table complement
		m_Registered.RemoveAt(nf);
		m_Filestocreate.RemoveAt(nf);
		m_Createdfile.RemoveAt(nf);
		m_Filewascreated.RemoveAt(nf);

        if(creator)
			unregisterComplementToCB(item);

		Mes = Tab + _T("- ") + Quote + item + Quote;
		print(Mes);
		ok = true;
	}
	// no file  ?
	m_clean = m_Registered.IsEmpty();

	return ok;
}

///-----------------------------------------------------------------------------
///	To unregister a complement file to 'CB' after unregiter a creator file
/// return 'true' when right
///
/// Called by :
///		1. qtPrebuild::unregisterFileComplement(const wxString & file, bool creator, bool first):1,

bool qtPrebuild::unregisterComplementToCB(const wxString & complement)
{
	bool ok = false;
	// GetFileByFilename(filename, relative filename, is Unix filename)
	ProjectFile * prjfile =  m_pProject->GetFileByFilename(complement, true, false) ;
	if (prjfile)
	{
       ok = m_pProject->RemoveFile(prjfile) ;
	}

	return ok;
}

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
		{
		// registered in table
			m_Registered.Add(fout, 1) ;
		}
	}
// tree refresh
	// >= svn 9501 : CB 13.12
	Manager::Get()->GetProjectManager()->GetUI().RebuildTree() ;

//  end registering
	uint16_t nf = m_Registered.GetCount() ;
//	Mes = Tab + _T("-> ") + (wxString()<<nf) ;
//	Mes += Space + _("creator(s) file(s) registered in the plugin") ;
//	print(Mes) ;

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
bool qtPrebuild::addOneFile(const wxString& fcreator, const wxString& fout)
{
// local variables
	bool valid = false, nocompile = false, weight = 50;
// has included ?
	wxString extin = fcreator.AfterLast('.') , extout = fout.AfterLast('.')  ;
	if ( (extin.Matches(EXT_H) && extout.Matches(EXT_CPP)) ||
		 (extin.Matches(EXT_CPP) && extout.Matches(EXT_MOC)) )
		nocompile =  hasIncluded(fcreator) ;
	else
	// ui_*.h file
	if (extin.Matches(EXT_UI) && extout.Matches(EXT_H) )  {
		nocompile = true;
		weight = 40; // before all files
	}
//Mes = Tab + Quote + fcreator + Quote + _T(" -> ") +  Quote + fout + Quote ;
//Mes += _T(" : include = ") +  Quote + (wxString()<<nocompile) + Quote ;
//printWarn(Mes);

// find projectfile in project ?
	ProjectFile * prjfile = m_pProject->GetFileByFilename(fout);
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
		prjfile = m_pProject->AddFile(m_nameActiveTarget, fout, !nocompile, !nocompile, weight);
		valid = prjfile != nullptr;
		if (!valid)
		{
		// display message
			Mes  = _T("===> ") ;
			Mes += _("can not add this file ");
			Mes += Quote + fout + Quote + _(" to target ") + m_nameActiveTarget ;
			printErr (Mes) ;
			cbMessageBox(Mes, _T("AddFile(...)"), wxICON_ERROR) ;
		}
	}

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
bool qtPrebuild::hasIncluded(const wxString& fcreator)
{
// search only in '*.cpp'
	wxString ext = fcreator.AfterLast('.');
	if (!ext.Matches(EXT_CPP))
		return false;

	wxString namefile = fcreator.BeforeLast('.') + DOT_EXT_CPP;
	if (! wxFileExists(namefile) )
		 return false  ;

// load file  'xxxx.cpp'
	wxString source = ReadFileContents(namefile) ;
	if (source.IsEmpty())
		return false ;

	namefile = namefile.AfterLast(Slash).BeforeLast('.')  ;
// search "#include "moc_*.cpp" or "#include "*.moc"
	//1- search >"moc_namefile.cpp"<
	wxString txt = _T("moc_") + namefile + DOT_EXT_CPP;
	// find from end
	bool include = verifyIncluded(source, txt) ;
	if (!include)
	{
	//2- search >"namefile.moc"<
		txt = namefile + DOT_EXT_MOC ;
		include = verifyIncluded(source, txt) ;
	}
//Mes = _T("Creator ") + Quote + fcreator + Quote ;
//Mes += _T(" : hasIncluded(...) -> ") + (wxString() << include);
//printWarn(Mes);
	return  include ;
}
/// Called by  :
///		1. hasIncluded(const wxString& fcreator):2
///
bool qtPrebuild::verifyIncluded(wxString& source, wxString& txt)
{
	wxString temp;
	int32_t pos = source.Find(txt);
	bool include =  pos != -1 ;
	if (include)
	{
	// delete 'txt' from pos
		source.Remove(pos);
	// search from the end, the last character of endline
		pos = source.Find('\n', true);
		if (pos >=0)
   		{
   		// give from pos ( the last line )
			temp = source.Mid(pos+1);
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
		//	Mes = _T("line = ") + Quote + temp + Quote;
		//	print(Mes);
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
bool qtPrebuild::inProjectFileCB(const wxString& file)
{
//Mes = Tab + Quote + file + Quote ;
    // relative filename !!!
	ProjectFile * prjfile = m_pProject->GetFileByFilename (file, true, false) ;
//Mes += _T(" -> ") + here;
//printWarn(Mes);
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
bool qtPrebuild::createDir (const wxString&  dircomplement)
{
//Mes = _T("qtPrebuild::createDir(") + Quote + dircomplement + Quote + _T(")") ;
//printWarn(Mes);
	bool ok = true  ;
	if (! wxDirExists(dircomplement))
	{
	    // Linux -> droit d'accès = 0x0777
		ok = wxMkdir(dircomplement) ;
		if (!ok)
		{
			Mes = _("Can't create directory ")  ;
			Mes += Quote + dircomplement + Quote + _T(" !!") ;
			printErr(Mes);
			cbMessageBox(Mes, _T("createDir()"), wxICON_ERROR) ;
		}
	}

	return ok ;
}
///-----------------------------------------------------------------------------
///  Remove directory for "qtbrebuilt\name-target\"
///
///  Called by : none
///
bool qtPrebuild::removeDir(const wxString&  dirgen )
{
	bool ok = ::wxDirExists(dirgen) ;
	if (ok)
	{
//Mes = Quote + dirgen + Quote + _T(" !!") ;
//printWarn(Mes);
		ok = ::wxRmdir(dirgen)  ;
		if (!ok)
		{
			Mes = _("Can't remove directory ") ;
			Mes += Quote + dirgen + Quote + _T(" !!") ;
			printErr(Mes);
			cbMessageBox(Mes, _T(""), wxICON_ERROR) ;
		}
	}

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
uint16_t qtPrebuild::filesTocreate(bool allrebuild)
{
	m_Filestocreate.Clear();
// save all open files
	bool ok = Manager::Get()->GetEditorManager()->SaveAll();
	if (!ok )
	{
		Mes = _("Couldn't save all files ") ;
		Mes += _T(" !!")  ;
		printErr (Mes);
		cbMessageBox(Mes, _T(""), wxICON_ERROR)  ;
		return ok  ;
	}

    uint16_t  ntocreate = 0;
// all rebuild : copy table
	if (allrebuild)
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
				if (wxFileExists(nameout))
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
			{
			// to create
				m_Filestocreate.Add(nameout, 1)  ;
				ntocreate++;
			}
		} // end for
	}

	return ntocreate;
}
///-----------------------------------------------------------------------------
/// Compare file date
///
/// Called by  :
///		1. buildOneFile(cbProject * prj, const wxString& fcreator):1,
///		1. filesTocreate(bool allrebuild):1,
///
bool qtPrebuild::compareDate(const wxString& fileref, const wxString&  filetarget)
{
	if ( !(wxFileExists(fileref) && wxFileExists(filetarget) ) )
		return false;

	wxFileName refname(fileref);
	wxDateTime refdate = refname.GetModificationTime();

	wxFileName target(filetarget);
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
bool qtPrebuild::modifyDate(const wxString&  fileref, const wxString& filetarget)
{
	if ( !(wxFileExists(fileref) && wxFileExists(filetarget) ) )
		return false;

	wxFileName refname(fileref);
	wxDateTime refdate = refname.GetModificationTime();

	wxFileName target(filetarget);
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
///		1. noFile(const wxArrayString& arraystr):1,
///		2. findTargetQtexe(buildtarget->GetParentProject()):1,
///		3. createComplement(const wxString& qexe, const uint16_t index):3,
///
bool qtPrebuild::createFiles()
{
// search target qt path for moc, ...
	ProjectBuildTarget * buildtarget = m_pProject->GetBuildTarget(m_nameActiveTarget)  ;
	bool ok = buildtarget != nullptr ;
	if (!ok)
		return ok ;

	ok = findTargetQtexe(buildtarget) ;
	if (!ok)
		return ok ;

// -> debug
//Mes = allStrTable(_T("m_Filestocreate"), m_Filestocreate);
//print(Mes);

// nothing to do ?
	// 'nfilesTocreate' count good files ( no 'm_Devoid' )
	uint32_t nftocreate = nfilesToCreate(m_Filestocreate)
			, nfcreated = m_Createdfile.GetCount()
			, nftodisk = m_Filewascreated.GetCount();
//Mes = _T("nftocreate = ")  + (wxString()<<nftocreate) ;
//Mes += _T(", nfcreated = ")  + (wxString()<<nfcreated) ;
//Mes += _T(", nftodisk = ")  + (wxString()<<nftodisk) ;
//printWarn(Mes);
// no 'abort' during last build
	if (nftocreate == nfcreated )
	{
		m_Createdfile.Clear();
		nfcreated = 0;
	//	m_nfilesCreated = 0;
	}
    // nothing to do ?
	bool emptiness = noFile(m_Filestocreate) ;
	if (emptiness)
	{
	// already created
		Mes = Tab + _("Nothing to do (") + (wxString()<<nftodisk);
		Mes += Space + _("correct complement files exist on disk)") ;
		print(Mes) ;

		return emptiness  ;
	}

	Mes = Tab + _T("=> ") + _("There are");
	Mes += Space + (wxString()<<nftocreate) + Space ;
	Mes += _("complement files to rebuild") ;
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
		if (nameout.Matches(m_Devoid) )
			continue  ;

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
			wxString title = _("Creating ") + Quote + m_Registered.Item(i) + Quote ;
			title += _(" failed") ;
			title += _T(" ...");
			// error create complement
			Mes =  Tab +_T("=> ") ;
			Mes += strerror.BeforeLast(Lf.GetChar(0)) ;
			printErr (Mes) ;
        // message box
            Mes = strerror.BeforeLast(Lf.GetChar(0)) ;
			Mes += Lf + _("Do you want to stop pre-construction only ?");
			int16_t choice = cbMessageBox(Mes, title, wxICON_QUESTION | wxYES_NO) ;
			abandonment = choice == wxID_YES ;
			if (abandonment)
			{
                Mes = Tab  + _("Abandonment of the pre-construction ...");
				printErr (Mes) ;
				break;
			}
			else
			{
				Mes = Tab  + _("Continue building ...");
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
		Mes = Tab + _T("=> ") + _("You have created normally") + Space + (wxString()<<ncreated) + _T("/") ;
		Mes += (wxString()<<nfiles) + Space + _("file(s)");
		print(Mes);
    // adversing
		Mes = _("A stop was requested during the creation of the complement files !!");
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
//Mes =  Tab + _T("- ") + (wxString()<<nf) + _T(" ")  ;
//Mes += _("complement(s) file(s) are created in the target") ;
//printWarn(Mes)  ;
	}

	return created ;
}
///-----------------------------------------------------------------------------
/// Search all 'm_Devoid' to an wxArrayString
///
/// Called by  :
///		1. createFiles():1,
///
bool qtPrebuild::noFile (const wxArrayString& arraystr)
{
	bool devoid = false ;
	for (const auto item : arraystr)
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
uint16_t qtPrebuild::nfilesToCreate(const wxArrayString& arraystr)
{
	uint16_t nftocreate = 0;
	for (const auto item : arraystr)
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
wxString qtPrebuild::createFileComplement(const wxString& qexe,
										  const wxString& fcreator,
										  const wxString& fout)
{
// build command
	wxString command = qexe  ;
//Mes = _T("qtPrebuild::createFileComplement() => command = *>") + command  + _T("<*");
//printWarn(Mes);
// add file name whithout extension
	if (qexe.Matches(m_Rexe))
	{
		wxString name = fcreator.BeforeLast('.')  ;
		command +=  _T(" -name ") + name  ;
	}
	if (qexe.Matches(m_Mexe))
	{
		command +=  m_DefinesQt + m_IncPathQt ;
	}
// add input file
	command += Space + fcreator ;
// add output file
	command += _T(" -o ") + fout;
//Mes = Tab + _T("=> command = *>") + command  + _T("<*");
//printWarn(Mes);

// execute command line : use short file name
	wxString strerror = executeAndGetOutputAndError(command, true)  ;
	bool created =  strerror.IsEmpty() ;
// file-project              : params = name, is relative, is Unixfilename
	ProjectFile * prjfile =  m_pProject->GetFileByFilename(fout, true, false) ;
	if (!prjfile)
	{
		Mes = fout + _(" is no correct !!");
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
		m_Identical = modifyDate(fcreator ,fout) ;
    // more one file
		++m_nfilesCreated ;
	// display
		bool nocompile = ! prjfile->compile
			,nolink = ! prjfile->link;

		Mes = Tab + (wxString()<< m_nfilesCreated) + _T("- ") + Quote + fcreator + Quote  ;
		Mes +=  _T(" ---> ") + Quote + fout + Quote ;
		if (nocompile && nolink)
		{
			Mes += Tab + _T("(") +  _("no compiled, no linked") + _T(")");
			printWarn(Mes);
		}
		else
			print(Mes);
	}

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
wxString qtPrebuild::createComplement(const wxString& qexe, const uint16_t index)
{
//1- name relative input file  "src\filecreator.xxx"
	wxString inputfile = m_Filecreator.Item(index) ;
	// create directory for m_nameActiveTarget
	wxString dircomplement = m_Registered.Item(index).BeforeLast(Slash)  ;
	dircomplement +=  wxFILE_SEP_PATH ;

	if (!createDir(dircomplement))
	{
		Mes = _("Unable to create directory ") + Quote + dircomplement + Quote ;
		return Mes ;
	}
//2- add search path for compiler
	ProjectBuildTarget * buildtarget = m_pProject->GetBuildTarget(m_nameActiveTarget) ;
	if (!buildtarget)
	{
		Mes = Quote + m_nameActiveTarget + Quote + _(" does not exist !!") ;
		return Mes ;
	}

	// add include directory
//	bool incdir = buildtarget->GetIncludeInTargetAll(); // ??
	buildtarget->AddIncludeDir(dircomplement) ;
//3- full path name  output file
	wxString outputfile = m_Registered.Item(index) ;
//4- create on file complement
	wxString strerror = createFileComplement(qexe, inputfile, outputfile) ;
//5- return  error  : good if 'strerror' is empty
	return strerror ;
}
///-----------------------------------------------------------------------------
/// Give include path qt for 'm_Moc'
///
/// Called by :
///		1. findTargetQtexe(CompileTargetBase * buildtarget):1,
///
wxString qtPrebuild::pathIncludeMoc(CompileTargetBase * container)
{
	wxArrayString tabincludedirs = container->GetIncludeDirs(), tabpath;
	wxString incpath;
	for (wxString& line : tabincludedirs)
	{
	//	if (line.Find(_T("#qt")) != -1 )
	//	{
			m_pMam->ReplaceMacros(line) ;
		// with qt-5.9 error with 'moc' pour 'Q_INTERFACES(...)'
			//line = 	_T("-I") + Quote + line + Quote  ;
			//line = 	_T("-I") + Dquote + line + Dquote ;
			//line = 	_T("-I") + line ;
			line.Prepend(_T("-I"));
			tabpath.Add(line, 1) ;
	//	}
	}
	// build 'incpath'
	if (tabpath.GetCount())
		incpath = Space + GetStringFromArray(tabpath, Space, true) + Space;

	return incpath  ;
}
///-----------------------------------------------------------------------------
/// Give 'defines' qt for 'm_Moc'
///
/// Called by :
///		1. findTargetQtexe (CompileTargetBase * buildtarget):1,
///
wxString qtPrebuild::definesMoc(CompileTargetBase * container)
{
	wxArrayString tabdefines = container->GetCompilerOptions(), tabdef ;
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
//Mes =  _T("validCreated() -> ") + (wxString()<<ncase) + _T(" = ") +  (wxString()<<tocreate) ;
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
			printErr(Mes);
		}
	}

	return ok ;
}
///-----------------------------------------------------------------------------
/// Execute another program (always synchronous).
///
/// Called by :
///		1. createFileComplement(const wxString& qexe, const wxString& fcreator, const wxString& fout):1,
///
wxString qtPrebuild::executeAndGetOutputAndError(const wxString& command, bool prepend_error)
{
	wxArrayString output;
	wxArrayString error;
	wxExecute(command, output, error, wxEXEC_NODISABLE);

	wxString str_out;

	if ( prepend_error && !error.IsEmpty())
		str_out += GetStringFromArray(error, Lf);

	if (!output.IsEmpty())
		str_out += GetStringFromArray(output, Lf);

	if (!prepend_error && !error.IsEmpty())
		str_out += GetStringFromArray(error,  Lf);

	return  str_out;
}

///-----------------------------------------------------------------------------
