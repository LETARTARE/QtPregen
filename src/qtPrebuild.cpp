/***************************************************************
 * Name:      qtprebuild.cpp
 * Purpose:   Code::Blocks plugin	'qtPregen.cbp'  1.0
 * Author:    LETARTARE
 * Created:   2015-10-17
 * Modified:  2017-12-09
 * Copyright: LETARTARE
 * License:   GPL
 **************************************************************/
#include "qtprebuild.h"
#include <sdk.h>
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
/// called by :
///		1. QtPregen::OnAttach():1,
///
qtPrebuild::qtPrebuild(cbProject * prj, int logindex, wxString & nameplugin)
	: qtPre(prj, logindex, nameplugin)
	, m_Identical(false)
{
	/// !! not use  + _T(Slash)   !!
	m_dirgen = _T("qtprebuild") + wxString(Slash) ;
}
///-----------------------------------------------------------------------------
/// called by :
///		1. QtPregen::OnRelease():1,
///
qtPrebuild::~qtPrebuild()
{
	m_project = nullptr;  m_mam = nullptr;
}
///-----------------------------------------------------------------------------
/// called by :
///		1. buildQt():1,
///
void qtPrebuild::beginMesBuildCreate()
{
// begin
	if(!m_project)
		return;

// base directory
	m_dirproject = m_project->GetBasePath() ;
// directory changed
	wxFileName::SetCwd (m_dirproject);
	bool ok = createDir (m_dirgen) ;
	if (!ok)
		return ;

	m_nameproject = m_project->GetTitle();
	m_nameactivetarget = m_project->GetActiveBuildTarget();

// display log
	//printLn;
	Mes = _T("-------------- ") ;
	Mes += _T("PreBuild : ") ;
	Mes += Quote + m_nameactivetarget + Quote;  // realtarget
	Mes += _(" in ");
	Mes += Quote + m_nameproject + Quote;
	Mes += _T("-------------- ") ;
	printWarn(Mes);
// date
	Mes = _("started on : ")+ date()  ;
	print(Mes);
// for duration
	m_start = clock();
}
///-----------------------------------------------------------------------------
/// called by :
///	1. buildQt():1,
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
/// called by :
///		1. QtPregen::OnPregen(CodeBlocksEvent& event):1
///
/// calls to :
///		1. beginMesBuildCreate():1,
///		2. findGoodfiles():1,
///		3. addRecording():1,
///		4. filesTocreate(bool allrebuild) :1,
///		5. createFiles():1,
///		6. vzlidCreated():1,
///		7. endMesBuildCreate():1,
///
bool qtPrebuild::buildQt(cbProject * prj, bool workspace, bool allbuild)
{
	m_project = prj;
	bool ok = m_project != nullptr;
	if (!ok)
		return ok;

// begin banner
	beginMesBuildCreate();

	ok = false;
	///***********************************************
	///1- find good target with eligible files
	///***********************************************
	// analyzing all project files
	int nelegible =  findGoodfiles() ;
	if (nelegible > 0)
	{
		///************************************************
		///2-  registered all files in 'm_Registered'
		///************************************************
		int nfiles = addRecording() ;
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
				/// from 'FileCreator', 'm_Registered'
				/// 1- adds files created in 'm_Createdfile'
				/// 2- create additional files as needed  by 'create(qexe, fileori)'
				///***********************************************************
				bool good = createFiles() ;
				if (good)
				{
					///**********************************
					///5- register files created
					///**********************************
					good = validCreated() ;
					if (good)
					{
						///**********************************
						///6- ???
						///**********************************
						//print(_T("- empty ...") ) ;
						ok = true;

					}	// end (good)
					else
					{
						Mes = Tab + _("No file to create.") ;
					//	printWarn(Mes);
					}
				} // end (good)
				else
				{
					Mes = Tab + _("File creation error !!") ;
					printErr(Mes);
				}
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
	}
    // end banner
	endMesBuildCreate();

	return ok;
}
///-----------------------------------------------------------------------------
/// called by :
///		1. buildFileQt():1,
///
void qtPrebuild::beginMesFileCreate()
{
// banner
	printLn;
	Mes = _T("-------------- ") ;
	Mes += _T("PreCompileFile : ") ;
	Mes += Quote + m_project->GetActiveBuildTarget() + Quote;
	Mes += _(" in ");
	Mes += Quote + m_project->GetTitle() + Quote;
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
/// called by :
///		1. buildFileQt():1,
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
/// Generating one complement file...
///
///  called by :
///		1. QtPregen::OnPregen(CodeBlocksEvent& event):1,
///
///  calls to :
//		-#  beginMesFileCreate():1,
///   	-#  isElegible(const wxString& file):1,
///   	-#  inProjectFile(const wxString& file):1,
///   	-#  CompareDate(const wxString&  fileref, const wxString&  filetarget) :1,
///   	-#  createFileComplement(const wxString& qexe, const wxString& fcreator, const wxString& fout) :1,
///		-#  endMesFileCreate():1,
///
bool qtPrebuild::buildFileQt(cbProject * prj, const wxString& fcreator)
{
	if(!prj)
		return false;

	m_project = prj;
	m_filename = fcreator;

// directory changed
	m_dirproject = m_project->GetBasePath();
	wxFileName::SetCwd (m_dirproject);
	bool ok = createDir (m_dirgen) ;
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

	m_nameactivetarget =  m_project->GetActiveBuildTarget() ;
	// targets no virtual list
	wxArrayString ltargets;
// virtual target ?
	bool virt = isVirtualTarget(m_nameactivetarget);
	if (virt)
		ltargets = listRealsTargets(m_nameactivetarget) ;
	else
		ltargets.Add(m_nameactivetarget);

	uint16_t ntargets = ltargets.GetCount();
	ProjectBuildTarget* buildtarget;
    for (uint16_t nt = 0 ; nt < ntargets ; nt++ )
    {
		m_nameactivetarget = ltargets.Item(nt);
		buildtarget = m_project->GetBuildTarget(m_nameactivetarget);
		if (buildtarget && buildtarget->GetTargetType() == ttCommandsOnly )
			continue;

		wxString fout = m_dirgen + m_nameactivetarget + wxString(Slash) ;
		fout += nameCreated(fcreator) ;
		Mes = Tab + Quote + fcreator + Quote + _T(" -> ") + Quote + fout + Quote ;
		print(Mes);
	// already registered
		bool inproject = inProjectFile(fout) ;
		if (!inproject)
		{
		// has included ?
			wxString extin = fcreator.AfterLast('.') ;
			wxString extout = fout.AfterLast('.')  ;
			bool include = false ;
			if ( extin.Matches(EXT_H) && extout.Matches(EXT_CPP) )
				include =  hasIncluded(fcreator) ;
		// add file : AddFile(Nametarget, file, compile, link, weight)
			ProjectFile* prjfile = m_project->AddFile(m_nameactivetarget, fout, !include, !include, 50);
			if (!prjfile)
			{
			// display message
				Mes  = _T("===> ") ;
				Mes += _("can not add this file ");
				Mes += Quote + fout + Quote + _(" to target ") + m_nameactivetarget ;
				printErr (Mes) ;
				cbMessageBox(Mes, _T("AddFile(...)"), wxICON_ERROR) ;
			}
		// display
			if (include)
			{
				Mes = Tab + _("Add ") + Quote + fout + Quote ;
				Mes += Lf + Tab + Tab + _T("*** ") + _("This file is included, ") ;
				Mes += _("attributes 'compile' and 'link' will be set to 'false'") ;
				print(Mes);
			}
			// svn 9501 : CB 13.12  and >
			Manager::Get()->GetProjectManager()->GetUI().RebuildTree() ;
			// svn 8629 : CB 12.11
			//Manager::Get()->GetProjectManager()->RebuildTree() ;

		// create complement ...
			m_Identical = false;
		}
		else
		{
		// Check the date
			m_Identical = false ;
			//- verify datation on disk
			if (wxFileExists(fout))
				m_Identical = CompareDate(fcreator, fout) ;
		}

		if (m_Identical)
		{
			Mes = Tab + _("Nothing to do (complement file exist on disk)") ;
			print(Mes) ;
		}
	// ! identical date -> create
		else
		{
//Mes = Tab + _(" One complement file are created in the project ...") ;
//printWarn(Mes);
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
					// = Quote + m_nameactivetarget + Quote + _(" does not exist !!") ;
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
/// Reference of libQt by target: "" or "4r" or "4d" or "5r" or "5d"
///  qt4 -> '4', qt5 -> '5', release -> 'r' and debug -> 'd'
///
/// called by  :
///		-# isGoodTargetQt():1,
///
wxString qtPrebuild::refTargetQt(ProjectBuildTarget * buildtarget)
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
        tablibs = m_project->GetLinkLibs();
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
/// Find path Qt installation
///
/// called by  :
///		-# beginMesCreate():1,
///
wxString qtPrebuild::findpathProjectQt()
{
	wxString path ;
	if (!m_project)
		return path;

//1- analyze the Project 'cbproject* Project'
	path = pathlibQt(m_project) ;
	if (path.IsEmpty())
	{
//2- analyze all targets
		bool ok = false ;
		ProjectBuildTarget * buildtarget ;
		int ntargets = m_project->GetBuildTargetsCount() ;
		int nt = 0 ;
		while (nt < ntargets && ! ok )
		{
		// retrieve the target Qt libraries paths
			buildtarget = m_project->GetBuildTarget(nt++) ;
			if (! buildtarget)
				continue;

			path = pathlibQt(buildtarget) ;
			ok = ! path.IsEmpty() ;
		}
	}

	return path ;
}
///-----------------------------------------------------------------------------
/// Find path Qt container  ->  'CompileOptionsBase * container'
///
/// called by  :
///		1. findpathProjectQt():2,
///		2. findTargetQtexe(CompileTargetBase * buildtarget):1,
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
				m_mam->ReplaceEnvVars(path) ;
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
					m_mam->ReplaceMacros(path) ;
                Mes = Lf + _("Global variable path Qt => '") + path;Mes += _T("'");
				print(Mes);
					path_nomacro =  path ;
					// remove "\lib"
					path_nomacro = path_nomacro.BeforeLast(Slash) ;
					path_nomacro += wxString(Slash)  ;
				}
				// no variable ! , absolute path ?
				else
				{
				Mes = Lf + _("Path Qt => '") + path ; Mes += _T("'");
				printWarn(Mes);
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
/// called by  :
/// 	-# createFiles():1
///		-# buildFileQt(cbProject * prj, const wwxFileExistsxString& fcreator):1
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
Mes = Lf + _("qtpath = '") + qtpath + _T("'");
printWarn(Mes);

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
		Mes += Lf + _("Cannot continue.") ;
		printErr(Mes);
		cbMessageBox(Mes, _("Search executable Qt ..."), wxICON_ERROR) ;
	}
	m_IncPathQt = pathIncludeMoc(m_project) +  pathIncludeMoc(buildtarget);
	m_DefinesQt = definesMoc(m_project) + definesMoc(buildtarget);

	return Findqtexe ;
}
///-----------------------------------------------------------------------------
/// called by  :
/// 	1. q_object(filename,qt_macro):1,
///		2. hasIncluded(const wxString& file):1,
///
wxString qtPrebuild::ReadFileContents(const wxString& filename)
{
	wxFileName fname(Manager::Get()->GetMacrosManager()->ReplaceMacros(filename));
	NormalizePath(fname, wxEmptyString);
	wxFile f(fname.GetFullPath());

	return cbReadFileContents(f);
}
///-----------------------------------------------------------------------------
/// called by  :
///		none
/// calls to :
///		none
///
bool qtPrebuild::WriteFileContents(const wxString& filename, const wxString& contents)
{
	wxFileName fname(Manager::Get()->GetMacrosManager()->ReplaceMacros(filename));
	NormalizePath(fname, wxEmptyString);
	wxFile f(fname.GetFullPath(), wxFile::write);

	return cbWrite(f, contents);
}

///-----------------------------------------------------------------------------
/// Get date
///
/// called by  :
///		1. beginMesBuildCreate():1,
///		2. endMesBuildCreate():1
///
wxString qtPrebuild::date()
{
	wxDateTime now = wxDateTime::Now();
	//wxString str = now.FormatISODate() + _T("-") + now.FormatISOTime();
	wxString str = now.FormatDate() + _T("-") + now.FormatTime();
    return str ;
}
///-----------------------------------------------------------------------------
/// Execution time
///
/// called by  :
///		1. endMesBuildCreate():1
///
wxString qtPrebuild::duration()
{
    clock_t dure = clock() - m_start;
    if (m_Linux)
        dure /= 1000;

     return wxString::Format(_T("%ld ms"), dure);
}
///-----------------------------------------------------------------------------
/// Looking for eligible files to the active target,
/// 	meet to table 'm_Filecreator' : return eligible file number
///
/// called by  :
///		1. buildQt(cbProject * prj, bool workspace, bool allbuild):1
///
/// calls to :
///		1. isVirtualTarget(m_nameactivetarget):1,
///		2. isGoodTargetQt(m_nameactivetarget):1,
///		3. isElegible(file):1,
///
uint16_t qtPrebuild::findGoodfiles()
{
	m_Filecreator.Clear();
// active target
	m_nameactivetarget = m_project->GetActiveBuildTarget() ;
// virtual ?
	if (isVirtualTarget(m_nameactivetarget, true) )
		return 0 ;

// no good target
	if (! isGoodTargetQt(m_nameactivetarget))
		return 0 ;

// around project files
	ProjectFile * prjfile ;
	wxString file ;
	wxArrayString tabtargets ;
	int nt, nft=0 ;
	int nfprj = m_project->GetFilesCount()  ;
// all files project
	for (int nf =0 ; nf < nfprj; nf++ )
	{
		prjfile = m_project->GetFile(nf);
		if (!prjfile)
			continue  ;

	//	file name
		file = prjfile->relativeFilename ;
		if (file.IsEmpty() )
			continue  ;

//Mes = _T("file = ") + file;
//printWarn(Mes);
	// copy  all targets
	//	tabtargets = copyarray(prjfile->buildTargets) ;
		tabtargets = prjfile->buildTargets ;
		nt = tabtargets.GetCount()  ;
		if (nt > 0)
		{
			wxString nametarget ;
			bool good ;
			// all file targets
			for (int t=0; t < nt; t++)
			{
				nametarget = tabtargets.Item(t) ;
				good = nametarget.Matches(m_nameactivetarget) ;
				if (good )
					break ;
			}
			if (good )
			{
			// not eligible ?
				if (! isElegible(file))
					continue  ;

			// add the file and it target
				m_Filecreator.Add(file, 1) ;
			}
		}
	}

//  number file eligible
	return m_Filecreator.GetCount()  ;
}
///-----------------------------------------------------------------------------
/// Give the good type of target for Qt
///
/// called by  :
/// 	1. QtPregen::OnPregen(CodeBlocksEvent& event):1,
///		2. findGoodfiles():2,
///
/// calls to:
///		1. refTargetQt(ProjectBuildTarget * buildtarget):1,
///
bool qtPrebuild::isGoodTargetQt(const wxString& nametarget)
{
	ProjectBuildTarget * buildtarget = m_project->GetBuildTarget(nametarget) ;
	if (!buildtarget) return false ;

	bool ok = buildtarget->GetTargetType() != ::ttCommandsOnly  ;
	wxString str = refTargetQt(buildtarget)  ;
//Mes = _T("str = ") + Quote + str + Quote;
//printWarn(Mes);
	bool qt = ! str.IsEmpty()  ;
	if (!qt)
	{   // TODO ...
		Mes = _("This target have nothing library Qt !") ;
		Mes += Lf + _("PreBuild cannot continue.") ;
		printErr(Mes);
	}

	return ok && qt ;
}
/*
///-----------------------------------------------------------------------------
/// Search virtual target
///
/// called by  :
///		1. findGoodfiles():1,
///
bool qtPrebuild::isVirtualTarget(const wxString& nametarget, bool warning)
{
// search virtual target : vtargetsTable <- table::wxArrayString
	wxString namevirtualtarget = _T("") ;
	wxArrayString vtargetsTable = m_project->GetVirtualBuildTargets () ;
	int nvirtual = vtargetsTable.GetCount() ;
	bool ok = nvirtual > 0 ;
	if (ok)
	{
		ok = false  ;
	// virtual name
		for (int nv = 0; nv < nvirtual ; nv++ )
		{
			namevirtualtarget = vtargetsTable.Item(nv) ;
			ok = nametarget.Matches(namevirtualtarget) ;
			if (!ok)
				continue;

			if (warning)			{
				Mes = _T("'") + namevirtualtarget + _T("'") ;
				Mes += _(" is a virtual target !!") ;
				Mes += Lf + _("NOT YET IMPLEMENTED...") ;
				Mes += Lf + _("... you must use the real targets.")  ;
				Mes += Lf + _("Cannot continue.") ;
				printErr(Mes);
				cbMessageBox(Mes, _T(""), wxICON_ERROR)  ;
			}
			break;
		}
	}

	return ok  ;
}
*/
/*
///-----------------------------------------------------------------------------
/// Search all not virtual target
///
/// called by  :
///		1. buildFileQt(cbProject * prj, const wxString& fcreator):1;
///
wxArrayString qtPrebuild::listRealsTargets(const wxString& nametarget)
{
	wxArrayString realsTargets;
	bool ok = isVirtualTarget(nametarget);
	if(ok)
	{
		// list virtual targets
		wxArrayString vtargetsTable = m_project->GetVirtualBuildTargets () ;
		int nvirtual = vtargetsTable.GetCount() ;
		ok = nvirtual > 0 ;
		if (ok)
		{
			wxString namevirtualtarget = _T("") ;
			ok = false  ;
		// virtual name
			for (int nv = 0; nv < nvirtual ; nv++ )
			{
				namevirtualtarget = vtargetsTable.Item(nv) ;
				ok = nametarget.Matches(namevirtualtarget) ;
				if (!ok)
					continue;
            // targets no virtuals
				realsTargets = m_project->GetVirtualBuildTargetGroup(nametarget) ;
				break;
			}
		}
	}
	else
	{
	    realsTargets.Add(nametarget);
    }

	return realsTargets;
}
*/
///-----------------------------------------------------------------------------
/// Search elegible files
/// 'file' was created ?  ->  moc_*.cxx, ui_*.h, qrc_*.cpp
///
/// called by  :
///		1. findGoodfiles():1,
///
/// calls to :
/// 	1. q_object(const wxString& filename, const wxString& qt_macro):1;
///
bool qtPrebuild::isElegible(const wxString& file)
{
	wxString name = file.AfterLast(Slash) ;
	// eliminates complements files
	int pos = name.Find(_T("_")) ;
	if (pos != -1)
	{
		wxString pre  = name.Mid(0, pos)  ;
		if (pre.Matches(m_Moc) || pre.Matches(m_UI) || pre.Matches(m_Qrc) )
			return false  ;
	}
	wxString ext = name.AfterLast('.')  ;
	bool ok = ext.Matches(m_UI) || ext.Matches(m_Qrc) ;
	if (ext.Matches(EXT_H) || ext.Matches(EXT_HPP) || ext.Matches(EXT_CPP) )
	{
		bool good = q_object(file, _T("Q_OBJECT") ) > 0  ;
		ok = ok || good  ;
	}
	return ok ;
}
///-----------------------------------------------------------------------------
/// Find valid "Q_OBJECT" or "Q_GADGET"
///
/// Looks for occurrences of "Q_OBJECT" or "Q_GADGET" valid in the file.
///	Remarks:
///		Qt documentation indicates another keyword METATOBJETS -> "Q_GADGET"
///		like "Q_OBJECT" but when the class inherits from a subclass of QObject.
///	Algorithm:
///		1 - eliminates comments type C
///		2 - eliminates comments type Cpp  '// .... eol'
///		3 - one seeks "Q_OBJECT" then 'xQ_OBJECTy' to verify that 'x' in
///			[0xA, 0x9, 0x20] and 'y' in [0xd (or 0xA), 0x9, 0x20]
///		4 - one seeks "Q_GADGET" in the same way
///
///-----------------------------------------------------------------------------
/// Search 'qt_text' in 'tmp'
///
/// called by  :
///		1. q_object(const wxString& filename, const wxString& qt_macro):2
///
int  qtPrebuild::qt_find (wxString tmp, const wxString& qt_text)
{
	//wxChar
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
			// next
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
/// Return occurrences of "Q_OBJECT" + occurrences of "Q_GADGET"
///
/// called by  :
///		1. isElegible(const wxString& file):1
///
int qtPrebuild::q_object(const wxString& filename, const wxString& qt_macro)
{
//1- the comments
	wxString CPP = _T("//") ;
	wxString BCC = _T("/*") ;
	wxString ECC = _T("*/") ;
	// ends of line
	wxString SA, SD;
	SA.Append(SepA); SD.Append(SepD) ;
	// the result
	bool good= false ;
//2- verify exist
	wxString namefile = m_dirproject + filename ;
	if (! wxFileExists(namefile))
	{
		Mes = Quote + namefile + Quote + _(" NOT FOUND") + _T(" !!!") + Lf ;
		printErr(Mes) ;
		return -1  ;
	}
//3- read source 'namefile'

//printWarn(namefile);
	wxString source = ReadFileContents(namefile) ;
	if (source.IsEmpty())
	{
		Mes = Quote + namefile + Quote + _(" : file is empty") +  _T(" !!!") + Lf ;
		printErr(Mes) ;
		return -1 ;
	}
	// wxString
	int reste = source.length() ;
	// a string copy
	wxString temp = source.Mid(0, reste ) ;
	// pos
	int posdcc, posfcc, poscpp , moins ;
	// wxString
	wxString sup , after ;
	// boolean
	bool ok ;
//4- delete C comments
	do
	{
		reste = temp.length() ;
	// first DCC next FCC
		posdcc = temp.Find(BCC) ;
		if (posdcc != -1)
		{
			reste = reste - posdcc ;
			after = temp.Mid(posdcc, reste) ;
			posfcc = after.Find(ECC) ;
			// 'posfcc' relative to 'posdcc' !!
		}
		// comments exists ?
		ok = posdcc != -1 && posfcc != -1  ;
		if (ok)
		{
		// delete full comment
			moins = posfcc + ECC.length() ;
			temp.Remove(posdcc, moins) ;
		}
	}
	while (ok)  ;
//5- delete C++ comments
	do
	{
		reste = temp.length()  ;
	// start of a comment C
		poscpp = temp.Find(CPP) ;
		if (poscpp != -1)
		{
		// verify the caracter before : 'http://www.xxx' !!
			wxChar xcar = temp.GetChar(poscpp-1) ;
			if (xcar != ':' )
			{
				reste = reste - poscpp  ;
			// string comment over begin
				after = temp.Mid(poscpp, reste) ;
			// end comment
				posfcc = after.Find(SA) ;
			}
			else
				poscpp = -1  ;
		}
		ok = poscpp != -1 && posfcc != -1 ;
		if (ok)
		{
		// delete full comment
			temp.Remove(poscpp, posfcc) ;
		}
	}
	while (ok) ;

//6- find "Q_OBJECT" and/or "Q_GADJECT"
	int number = 0  ;
	wxString findtext = _T("Q_OBJECT") ;
	if ( qt_macro.Matches(findtext) )
	{
		number = qt_find(temp, findtext)  ;
		findtext = _T("Q_GADGET") ;
		number += qt_find(temp, findtext)  ;
	}
	else
	{
      // TODO ...
	}

	return number ;
}

///-----------------------------------------------------------------------------
/// To remove all complement files in project :
///		1. 'm_Registered' is clear
///		2. disk : "qtprebuild\nametarget" is removed
///	return 'true' when right
///
/// called by :
///		1. QtPregen::OnCleanPregen(CodeBlocksEvent& event)
///

bool qtPrebuild::removeComplements()
{
	bool ok = true;
// directory changed
	m_dirproject = m_project->GetBasePath() ;
	wxFileName::SetCwd (m_dirproject);
/// 1. Unregister the file 'Outputfile' in the project
/// 2. we will do a loop of 'm_Registered' to delete the file
	wxString name;
	uint16_t nfiles, nf = m_Registered.GetCount() ;
	nfiles = nf;
//Mes = Tab + (wxString() << nf) + _T(" files(s) to remove ...") ;
//printWarn(Mes);
	while (nf-- && ok)
	{
		name = m_Registered.Item(nf);
//Mes = Tab + (wxString() << nf) + Space + name  ;
//printWarn(Mes);
		ok = ::wxFileExists(name);
//Mes = Tab +  name + _T(" exists = ") + (wxString() << ok)  ;
//printWarn(Mes);
		if(ok)
		{
		// 1- Unregister project file
			ok = unregisterProjectFileToCB(name);
			if (ok)
			{
			// 2- Delete file from disk
				ok = ::wxRemoveFile(name);
	//Mes = Tab +  name + _(" removed = ") + (wxString() << ok)  ;
	//printWarn(Mes);
				if (!ok)
				{
					Mes = Tab + _("Can not delete") + Space + Quote + name + Quote + _T(" !!");
					printErr(Mes) ;
				}
			}
			else {
				Mes = Tab + _("Can not unregister") + Space + Quote + name + Quote + _T(" !!");
				printErr(Mes) ;
			}
		}
		else
		{
			Mes = Tab + _("File") + Quote + name + Quote + Space + _("not Found !!");
			printErr(Mes) ;
		}
	}

    if (ok)
	{
		if (nfiles)
		{
		//Rebuil C::B project manager tree
			Manager::Get()->GetProjectManager()->GetUI().RebuildTree() ;
		// clear 'QtPregen' table
			m_Registered.Clear();
			m_clean = true;
			Mes = Tab + _T("- ") + (wxString()<< nfiles) + Space ;
			Mes += _("complement(s) file(s) are deleted on disk") ;

		/*	it's not necessary
			// Remove directory 'qtprebuil\target\*.*'
			ok =  removeDir (dirgen);
			if (ok)
			{
				Mes = Tab + _T("Removed directory : ") + Quote + dirgen + Quote ;
				printErr(Mes) ;
			}
		*/
		}
		else
		{
			Mes = Tab + _("Nothing to delete on disk.");
		}
		print(Mes);
	}
	else
	{
		//Mes = _T("ok : ") + Quote + (wxString() << ok) + Quote ;
		//printErr(Mes) ;
	}

	return  ok;
}

///-----------------------------------------------------------------------------
/// To remove one complement files in project :
///		1. 'm_Registered.item(pos)' is clear
///		2. disk : "qtprebuild\nametarget\cmplement" is removed
///	return 'true' when right
///
/// called by :
///		1. unregisterProjectFile(filename, complement, m_removingfirst)
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
///	To unregister a project file (creator or complement) to 'qtPregen'
/// return 'true' when right
///
/// called by :
///		-# QtPregen::OnFileRemovedPregen(CodeBlocksEvent& event):1,
///  calls to :
///		-# removeComplementToDisk(const wxString & filename):1,
///		-# qtpre::complementDirectory():1,
///
bool qtPrebuild::unregisterProjectFile(const wxString & file, bool complement, bool first)
{
	wxString filename = file.AfterLast(Slash) ;
//Mes = _T("filename = ") + Quote + filename + Quote;
//Mes += Tab +  _T("first = ") + (wxString() << first);
//printWarn(Mes);

	bool ok = false;
	bool creator  = !complement;
	int16_t index = wxNOT_FOUND;

/// 1- Creator complement file  ?
	if (creator)
	{
		index = m_Filecreator.Index (filename);
		ok = index != wxNOT_FOUND;
		if (!ok)
		{
			Mes =  Lf +_("Creator file(s) was removed from project ...") ;
			printWarn(Mes);
		}
		if (first)
		{
			Mes =  Lf +_("Creator file(s) was removed from project ...") ;
			printWarn(Mes);
		}
		Mes = Tab + (wxString() << index) +_T(": ") + Quote + filename + Quote;
		print(Mes);
		// remove complement file to disk
		filename = nameCreated(filename) ;
		complement = true;
	}

/// 2- Complement file  ?
	if (complement)
	{
		wxString dircomplement = complementDirectory();
		// find file removed to CB
		wxString supplement = dircomplement + filename ;
/*
Mes = _T("supplement = ") + Quote + supplement + Quote;
Mes += Tab +  _T("first = ") + (wxString() << first);
printWarn(Mes);
*/
		index = m_Registered.Index (supplement);
		ok = index != wxNOT_FOUND;
		if (ok)
		{
		// remove file in all tables
			m_Registered.RemoveAt(index, 1);
			m_Filestocreate.RemoveAt(index, 1);
			m_Createdfile.RemoveAt(index, 1);
			m_Filecreator.RemoveAt(index, 1);
			m_Filewascreated.RemoveAt(index, 1);
			// no file
			m_clean = m_Registered.IsEmpty();
			if(first)
			{
				Mes = _("Complement file(s) was removed from project ...") ;
				printWarn(Mes);
			}
			Mes = Tab + (wxString() << index) +_T(": ") + Quote + supplement + Quote;
			print(Mes);
            // remove complement to disk
			if (creator)
			{
				ok =  removeComplementToDisk(supplement, index);
			}
		}
	}

	return ok;
}

///-----------------------------------------------------------------------------
///	To unregister a project file to 'CB'
/// return 'true' when right
///
/// called by :
///		-# removeComplements():1,

bool qtPrebuild::unregisterProjectFileToCB(const wxString & complement)
{
	bool ok = false;
	// GetFileByFilename(filename, relative filename, is Unix filename)
	ProjectFile * prjfile =  m_project->GetFileByFilename(complement, true, false) ;
	if (prjfile)
	{
       ok = m_project->RemoveFile(prjfile) ;
	}

	return ok;
}

///-----------------------------------------------------------------------------
/// To register all new files in project : 'm_Registered' is meet
///	return 'true' when right
///
/// called by  :
///		1. buildQt(cbProject * prj, bool workspace, bool allbuild):1,
///
/// calls to :
///		1. indexTarget(const wxString& nametarget):1,
///		2. nameCreated(const wxString& file):1,
///		3. inProjectFile(const wxString& file):1,
///		4. hasIncluded(const wxString& file):1,
///

uint16_t qtPrebuild::addRecording()
{
	Mes = _("Search complement files to create ...") ;
	printWarn(Mes);

	m_Registered.Clear();
// local variables
	bool valid = false , inproject = false  ;
	wxString fout, fcreator, strindex = _T("0") ;
	uint16_t n=0, index, indextarget = indexTarget(m_nameactivetarget) ;
	ProjectFile * prjfile;
// read file list to 'm_Filecreator'  (*.h, *.cpp, *.qrc, *.ui)
	uint16_t nfiles = m_Filecreator.GetCount() ;
	for (uint16_t u=0; u < nfiles ; u++ )
	{
		// creator
		fcreator = m_Filecreator.Item(u)  ;
//Mes = Tab + _T("fcreator = ") + fcreator ;
//printWarn(Mes);
		//  this is where you add the name of the target
		fout = m_dirgen + m_nameactivetarget + wxString(Slash)  ;
		fout += nameCreated(fcreator) ;
//Mes = Tab + Quote + fcreator + Quote + _T(" -> ") +  Quote + fout + Quote;
//printWarn(Mes);
		// checks if the file exists in the project
		inproject = inProjectFile(fout) ;
		if (!inproject)
		{
			// has included ?
			wxString extin = fcreator.AfterLast('.') ;
			wxString extout = fout.AfterLast('.')  ;
			bool include = false ;
			if ( extin.Matches(EXT_H) && extout.Matches(EXT_CPP) )
				include =  hasIncluded(fcreator) ;

		// AddFile(Nametarget, file, compile, link, weight)
			prjfile = m_project->AddFile(m_nameactivetarget, fout, !include, !include, 50);
			valid = true;
			if (!prjfile)
			{
				valid = false;
			// display message
				Mes  = _T("===> ") ;
				Mes += _("can not add this file ");
				Mes += Quote + fout + Quote + _(" to target ") + m_nameactivetarget ;
				printErr (Mes) ;
				cbMessageBox(Mes, _T("AddFile(...)"), wxICON_ERROR) ;
			}
			// display
			n++ ;
			Mes = Tab + (wxString()<<n) + _T("- ") ;
			Mes += Quote + fout + Quote ;
			if (include)
			{
				Mes += Lf + Tab + Tab + _T("*** ") + _("This file is included, ") ;
				Mes += _("attributes 'compile' and 'link' will be set to 'false'") ;
			}
			print(Mes);
		}
		// valid
		if ( valid || inproject)
		{
		// registered in table
			m_Registered.Add(fout, 1) ;
		}
	}
//	printLn;
// tree refresh
	// >= svn 9501 : CB 13.12
	Manager::Get()->GetProjectManager()->GetUI().RebuildTree() ;
	// svn 8629 : CB 12.11
	//Manager::Get()->GetProjectManager()->RebuildTree() ;

//  end registering
	uint16_t nf = m_Registered.GetCount() ;

//	Mes = Tab + _T("- ") + (wxString()<<nf) + _T(" ") ;
//	Mes += _("complement(s) file(s) are registered in the plugin") ;
//	printWarn(Mes) ;

	return  nf;
}
///-----------------------------------------------------------------------------
/// Returns the index of a target in the project
///
/// called by  :
///		1. addRecording():1,
///
int qtPrebuild::indexTarget(const wxString& nametarget)
{
	ProjectBuildTarget *  buildtarget ;
	uint16_t ntargets = m_project->GetBuildTargetsCount() ;
	if (ntargets < 1) return -1 ;

	for (uint16_t nt = 0; nt < ntargets; nt++)
	{
		buildtarget = m_project->GetBuildTarget(nt) ;
		if (!buildtarget)
			continue ;

		if (buildtarget->GetTitle().Matches(nametarget) ) ;
			return nt  ;
	}
	return -1 ;
}
///-----------------------------------------------------------------------------
/// Gives the name of the file to create on
///
/// called by  :
///		1. addRecording():1,
///
wxString qtPrebuild::nameCreated(const wxString& file)
{
	wxString name  = file.BeforeLast('.') ;
	wxString fout ;
	if (name.Find(Slash) > 0)
	{
	// short name
		name = name.AfterLast(Slash) ;
	// before path
		fout += file.BeforeLast(Slash) + wxString(Slash) ;
	}
	wxString ext  = file.AfterLast('.')  ;
//1- file *.ui  (forms)
	if ( ext.Matches(m_UI) )
		fout += m_UI + _T("_") + name + DOT_EXT_H  ;
	else
//2- file *.qrc  (resource)
	if (ext.Matches(m_Qrc) )
		fout += m_Qrc + _T("_") + name + DOT_EXT_CPP  ;
	else
//3- file *.h or *hpp with 'Q_OBJECT' and/or 'Q_GADGET'
	if (ext.Matches(EXT_H) || ext.Matches(EXT_HPP)  )
		fout +=  m_Moc + _T("_") + name + DOT_EXT_CPP ;
	else
//4- file *.cpp with 'Q_OBJECT' and/or 'Q_GADGET'
	if (ext.Matches(EXT_CPP) )
		fout +=  name + DOT_EXT_MOC ;

	fout = fout.AfterLast(Slash) ;

	return fout  ;
}
///-----------------------------------------------------------------------------
/// Indicates whether a file is registered in the project
///
/// called by  :
///		1. addRecording():1,
///
bool qtPrebuild::inProjectFile(const wxString& file)
{
    // relative filename !!!
	ProjectFile * prjfile = m_project->GetFileByFilename (file, true, false) ;
	return prjfile != nullptr;
}

///-----------------------------------------------------------------------------
///	Search included file
// *.cpp contains  (#include "moc_*.cpp") or (#include "*.moc")
///
/// called by  :
///		1. addRecording():1,
///
bool qtPrebuild::hasIncluded(const wxString& file)
{
// searcch into '*.cpp' -> "#include "moc_*.cpp" or "#include "*.moc"
	wxString namefile = file.BeforeLast('.') + DOT_EXT_CPP;
// load file  xxxx.cpp
	if (! wxFileExists(namefile) )
		 return false  ;

	wxString source = ReadFileContents(namefile) ;
	if (source.IsEmpty())
		return false ;

// search >"moc_namefile.cpp"<
	namefile = namefile.AfterLast(Slash) ;
	wxString txt = _T("\"moc_") + namefile + _T("\"") ;
	bool include =  source.Find(txt) != -1 ;
	if (!include)
	{
// search >"namefile.moc"<
		txt = _T("\"") + namefile + _T(".moc\"") ;
		include  = source.Find(txt) != -1 ;
	}

	return  include ;
}
///-----------------------------------------------------------------------------
///  Create directory for "qtbrebuilt\name-target\"
///
///  called by :
///		1. createComplement(const wxString& qexe, const uint16_t index):1,
///
bool qtPrebuild::createDir (const wxString&  dirgen)
{
//Mes = _T("qtPrebuild::createDir(") + Quote + dirgen + Quote + _T(")") ;
//printWarn(Mes);

	bool ok = true  ;
	if (! wxDirExists(dirgen))
	{
	    // droit d'accès = 0x0777
		ok = wxMkdir(dirgen) ;
		if (!ok)
		{
			Mes = _("Can't create directory ")  ;
			Mes += Quote + dirgen + Quote + _T(" !!") ;
			printErr(Mes);
			cbMessageBox(Mes, _T("createDir()"), wxICON_ERROR) ;
		}
	}

	return ok ;
}
///-----------------------------------------------------------------------------
///  Remove directory for "qtbrebuilt\name-target\"
///
///  called by :
///		none
///
bool qtPrebuild::removeDir(const wxString&  dirgen )
{
	bool ok = ::wxDirExists(dirgen) ;
	if (ok)
	{
Mes = Quote + dirgen + Quote + _T(" !!") ;
printWarn(Mes);
//ok = true;
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
/// called by
///    1. buildQt(cbProject * prj, bool workspace, bool allbuild)
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
		wxString inputfile, nameout ;				// wxString
		bool inproject ;						// bool
	// analyze all eligible files/targets
		uint16_t nfiles = m_Filecreator.GetCount() ; 	// integer
		for (uint16_t i=0; i < nfiles ; i++ )
		{
		// relative name inputfile
			inputfile = m_Filecreator.Item(i) ;
		// file registered to create
			nameout = m_Registered.Item(i) ;
		// in project ?
			inproject =  inProjectFile(nameout) ;
			m_Identical = false ;
		//- verify datation on disk
			if (inproject)
			{
			// verify if exists file
				if (wxFileExists(nameout))
				{
				// test his modification date
					m_Identical = CompareDate(inputfile, nameout) ;
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
/// called by  :
///		1. createFiles():1,
///
bool qtPrebuild::CompareDate(const wxString& fileref, const wxString&  filetarget)
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
/// called by  :
///		1. createComplement(const wxString& qexe, const uint16_t index):1
///
bool qtPrebuild::ModifyDate(const wxString&  fileref, const wxString& filetarget)
{
	if ( !(wxFileExists(fileref) && wxFileExists(filetarget) ) )
		return false;

	wxFileName refname(fileref);
	wxDateTime refdate = refname.GetModificationTime();

	wxFileName target(filetarget);
	wxDateTime targetdate = target.GetModificationTime();

	bool ok  = target.SetTimes(0, &refdate, 0);

	return ok ;
}
///-----------------------------------------------------------------------------
/// Create files before build project :
///	- additions built files before generation
///	-# 'm_Filecreator' is meet by 'findGoodfiles()'
///	-# 'm_Registered is filled by 'addRecording()'
///	-# 'm_Filestocreate' filled by 'filesTocreate(bool allrebuild)'
///	-# meet 'm_Createdfile'
///
/// return true if all right
///
/// called by  :
///	 	-# buildQt(cbProject*, bool) :1,
///
///	calls to :
///		-# isEmpty(const wxArrayString& arraystr):1,
///		-# findTargetQtexe(buildtarget->GetParentProject()):1,
///		-# createComplement(const wxString& qexe, const uint16_t index):3,
///
bool qtPrebuild::createFiles()
{
	m_Createdfile.Clear();
// nothing to do ?
	bool emptiness = isEmpty (m_Filestocreate) ;
	if (emptiness)
	{
	// already created
		Mes = Tab + _("Nothing to do (all complement files exist on disk)") ;
		print(Mes) ;
		return emptiness  ;
	}

// search target qt path for moc, ...
	ProjectBuildTarget * buildtarget = m_project->GetBuildTarget(m_nameactivetarget)  ;
	bool ok = buildtarget != nullptr ;
	if (!ok)
		return ok ;
//printWarn(buildtarget->GetParentProject()->GetTitle());
//printWarn(m_project->GetTitle());
	ok = findTargetQtexe(buildtarget) ;
	if (!ok)
		return ok ;

	Mes = _("Creating complement files ...") ;
	printWarn(Mes);
// local variables
	wxString nameout, ext ;
	bool created = false;
	wxString  strerror;
	// used by 'createComplement()'
	m_nfilescreated = 0 ;
// analyze all eligible files/target
	uint16_t  nfiles = m_Filestocreate.GetCount() ;
	for (uint16_t   i=0; i < nfiles ; i++ )
	{
		nameout = m_Filestocreate.Item(i) ;
//Mes = Tab + _T("=> to create : ") + nameout;
//printWarn(Mes);
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

//Mes = Tab + _T("=> strerror : ") + strerror;
//printWarn(Mes);
		// more one
		created = strerror.IsEmpty();
		if (created)
		{
//Mes = Tab + Tab + _T("=> created ") + nameout;
//printWarn(Mes);
			m_Createdfile.Add(nameout, 1)  ;
		}
		else
		{
			m_Createdfile.Add(m_Devoid, 1)  ;
		// error message
			wxString title = _("Creating ") + Quote + m_Registered.Item(i) + Quote ;
			title += _(" failed") ;
			title += _T(" ...");
			//1- error create directory  :
				// = _("Unable to create directory ")
			//2- error buildtarget no exist :
				// = Quote + m_nameactivetarget + Quote + _(" does not exist !!") ;
			//3- error create complement
			Mes =  _T("=> ") ;
			Mes += strerror.BeforeLast(Lf.GetChar(0)) ;
			printErr (Mes) ;

			Mes += Lf + Lf + _("Do you want to stop the generation ?");
			int16_t choice = cbMessageBox(Mes, title, wxICON_QUESTION | wxYES_NO) ;
			if (choice == wxID_YES)
			{
                Mes = _("Abandonment of the construction ...");
				printErr (Mes) ;
				break;
			}
		}
	}  // end forManageprj.RebuildTree()
/*
	uint16_t nf = m_Createdfile.GetCount() ;
	Mes =  Tab + _T("- ") + (wxString()<<nf) + _T(" ")  ;
	Mes += _("complement(s) file(s) are created in the target") ;
	printWarn(Mes)  ;
*/
	return created   ;
}
///-----------------------------------------------------------------------------
/// Search all 'm_Devoid' to an wxArrayString
///
/// called by  :
///		1. createFiles()!1,
///
bool qtPrebuild::isEmpty (const wxArrayString& arraystr)
{
	uint16_t nf = arraystr.GetCount() ;
	bool devoid = false ;
	if (nf > 0)
	{
		for (uint16_t u= 0; u < nf ; u++ )
		{
			devoid  = arraystr.Item(u).Matches(m_Devoid) ;
			if (!devoid)
				break  ;
		}
	}

	return devoid ;
}
///-----------------------------------------------------------------------------
///	Execute commands 'moc', 'uic', 'rcc' : return _("") if file created
///	else return an error string
///
/// called by  :
///		1. createFiles():6,
///
///	calls to :
///		1. ModifyDate(const wxString&  fcreator, const wxString& fout) ;
///
wxString qtPrebuild::createFileComplement(const wxString& qexe,
										  const wxString& fcreator,
										  const wxString& fout)
{
//3- build command
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
		//command += _T(" -IO:/DevCpp/Qt-5.9/5.9/mingw53_32/include/QtWidgets ");
		//if (m_Win)
		   // command += Space + _T("-D__GNUC__ -DWIN32 -DQ_OS_WIN") + Space;
	}
	// add input file
	//if(m_Win)
    //    command += _T(" \"") + fcreator + _T("\"") ;
    //else
        command += Space + fcreator ;
    // add output file
    //if(m_Win)
    //    command += _T(" -o \"") + fout + _T("\" ") ;
    //else
       // command += _T(" -o '") + fout + _T("'");
        command += _T(" -o ") + fout;
//Mes = Tab + _T("=> command = *>") + command  + _T("<*");
//printWarn(Mes);
//4- execute command line : use short file name
	wxString strerror = ExecuteAndGetOutputAndError(command, true)  ;
	bool created =  strerror.IsEmpty() ;
	m_nfilescreated++ ;
	Mes = Tab + (wxString()<< m_nfilescreated) + _T("- ") + Quote + fcreator + Quote  ;
	Mes +=  _T(" ---> ") + Quote + fout + Quote ;
	print(Mes);

	wxString exe = qexe.AfterLast(Slash);
	command.Replace(qexe, exe) ;
/// log to debug
//Mes = command;
//print(Mes) ;
	// create error
	if (! created)
	{
	// unregister the file 'Outputfile' in the project
		ProjectFile * prjfile =  m_project->GetFileByFilename(fout, true, false) ;
		if (!prjfile)
		{
			m_project->RemoveFile(prjfile) ;
			// svn 9501 : CB 13.12
			Manager::Get()->GetProjectManager()->GetUI().RebuildTree() ;
			// svn 8629 : CB 12.11
			//Manager::Get()->GetProjectManager()->RebuildTree() ;
		}
	}
	else
	{
		// modify date of created file
		m_Identical = ModifyDate(fcreator ,fout) ;
	}

//5- return  error  : good if strerror is empty
	return strerror ;
}
///-----------------------------------------------------------------------------
///	Execute commands 'moc', 'uic', 'rcc' : return _("") if file created
///	else return an error string
///
/// called by  :
///		1. createFiles():6,
///
wxString qtPrebuild::createComplement(const wxString& qexe, const uint16_t index)
{
	bool created = false ;
//1- name relative input file
	wxString inputfile = m_Filecreator.Item(index) ;
	// create directory for m_nameactivetarget
	wxString dirgen = m_Registered.Item(index).BeforeLast(Slash)  ;
	dirgen +=  wxFILE_SEP_PATH ;
//Mes = Tab + _T("=> dirgen = ") + dirgen;
//printWarn(Mes);
	bool ok = createDir(dirgen) ;
	if (!ok)
	{
		Mes = _("Unable to create directory ") + Quote + dirgen + Quote ;
		return Mes ;
	}

	// add search path for compiler
	ProjectBuildTarget * buildtarget = m_project->GetBuildTarget(m_nameactivetarget) ;
	if (!buildtarget)
	{
		Mes = Quote + m_nameactivetarget + Quote + _(" does not exist !!") ;
		return Mes ;
	}

	// add include directory
	buildtarget->AddIncludeDir(dirgen) ;
//2- full path name  output file
	wxString Outputfile = m_Registered.Item(index) ;
//3- create on file complement
//Mes = Tab + _T("=> call to ") + qexe + _T(", ") + inputfile + _T(", ") + Outputfile;
//printWarn(Mes);
	wxString strerror = createFileComplement(qexe, inputfile, Outputfile) ;
//Mes = Tab + _T("=> strerror = ") + strerror;
//printWarn(Mes);

//4- return  error  : good if strerror is empty
	return strerror ;
}
///-----------------------------------------------------------------------------
/// Give include path qt for 'm_Moc'
///
/// called by :
/// 	1.findTargetQtexe(CompileTargetBase * buildtarget):1,
///
wxString qtPrebuild::pathIncludeMoc(CompileTargetBase * container)
{
	wxArrayString tabincludedirs = container->GetIncludeDirs(), tabpath;
	wxString incpath;
	uint16_t ncase = tabincludedirs.GetCount()  ;
	if (!ncase)
		return incpath;

	wxString line ;
	for (uint16_t u = 0; u < ncase ; u++)
	{
		line = tabincludedirs.Item(u) ;
	//	if (line.Find(_T("#qt")) != -1 )
	//	{
			m_mam->ReplaceMacros(line) ;
		/// with qt-5.9 error with 'moc' pour 'Q_INTERFACES(...)'
			//line = 	_T("-I") + Quote + line + Quote  ;
			//line = 	_T("-I") + Dquote + line + Dquote ;
			line = 	_T("-I") + line ;
			tabpath.Add(line, 1) ;
	//	}
	}
	// build 'incpath'
	incpath = Space + GetStringFromArray(tabpath, Space, true) + Space;

	return incpath  ;
}
///-----------------------------------------------------------------------------
/// Give 'defines' qt for 'm_Moc'
///
/// called by :
///		1. findTargetQtexe (CompileTargetBase * buildtarget):1,
///
wxString qtPrebuild::definesMoc(CompileTargetBase * container)
{
	wxArrayString tabdefines = container->GetCompilerOptions(), tabdef ;
	wxString def ;
	uint16_t ncase = tabdefines.GetCount();
	if (!ncase)
		return def;

	wxString  line;
	for (uint16_t u = 0; u < ncase ; u++)
	{
		line = tabdefines.Item(u) ;
		if (line.Find(_T("-D")) != -1 )
			tabdef.Add(line, 1) ;
	}

// build 'def'
	def = _T(" ") + GetStringFromArray(tabdef, _T(" "), true) + _T(" ") ;

	return def ;
}
///-----------------------------------------------------------------------------
/// Verify if at least one valid file is saved in 'm_Createdfile'
///
/// called by  :
/// 	1. buildQt(cbProject *, bool):1,
///
bool qtPrebuild::validCreated()
{
	bool ok = false  ;
	uint16_t ncase = m_Createdfile.GetCount()  ;
	while (ncase > 0 && !ok)
	{
		ok = ! m_Createdfile.Item(--ncase).Matches(m_Devoid)  ;
	}
//Mes =  _T("validCreated() = ") + (wxString() << ok) ;
//printWarn(Mes);

	if (ok)
	{
		// svn >= svn 9501 : CB 13.12
		Manager::Get()->GetProjectManager()->GetUI().RebuildTree()  ;
		// svn 8629 : CB 12.11
		// Manager::Get()->GetProjectManager()->RebuildTree()  ;
	// save project
		ok = Manager::Get()->GetProjectManager()->SaveProject(m_project)  ;
	}

	return ok ;
}
///-----------------------------------------------------------------------------
/// Execute a command
///
/// called by :
///		1. createComplement(const wxString& qexe, const uint16_t index)
///
wxString qtPrebuild::ExecuteAndGetOutputAndError(const wxString& command, bool prepend_error)
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
/// Check that the filename not contain illegal characters for windows !
///
/// called by :
///		 none
///
wxString qtPrebuild::filenameOk(const wxString& namefile)
{
	wxString banned = _T("") ;
	if (m_Win)
	{
		wxString illegal = _T("[:*?\"<>|]") ;
		uint16_t pos, ncar = illegal.length() ;
	// all illegal characters
		for (uint16_t u = 0; u < ncar; u++)
		{
			banned += illegal.GetChar(u) ;
			pos = namefile.Find(banned) ;
			if (pos != -1)
				break ;

			banned = _T("")  ;
		}
	}

	return banned  ;
}
///-----------------------------------------------------------------------------
