/***************************************************************
 * Name:      qtPrebuild.cpp
 * Purpose:   Code::Blocks plugin	'qtPregenForCB.cbp'  0.4.2
 * Author:    LETARTARE
 * Created:   2015-02-21
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
///	1. QtPregen::OnAttach():1,
///
qtPrebuild::qtPrebuild(cbProject * prj)
	: qtPre(prj)
	, m_Dexeishere(true), m_Identical(false)
{
	// not use  + _T(Slash)   !!
	m_dirgen = _T("qtprebuild") + wxString(Slash);
	m_Devoid = _T("__NULL__") ;
}
///-----------------------------------------------------------------------------
/// called by :
///	1. QtPregen::OnRelease():1,
///
qtPrebuild::~qtPrebuild()
{
	m_project = nullptr;  m_mam = nullptr;
}
///-----------------------------------------------------------------------------
/// called by :
///	1. buildQt():1,
///
void qtPrebuild::beginMesCreate() {
// begin '*.cre'
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
	wxString file = m_dirproject + m_nameproject ;
	closedit(file + _T(".cre"));
	m_Fileswithstrings.Clear();
// tittle
	Mes = _("Begin ") +  Quote + m_Thename + _T(" ") + VERSION_QTP + Quote + _T(" : ") ;
	//print(Mes) ;
	m_Fileswithstrings.Add(Mes, 1) ;
	// Qt path
	wxString qtpath = findpathProjectQt() ;
	Mes = Lf + Tab + Quote + _T("Qt") + Quote + _(" Installation path") + _T(" : ") ;
	Mes +=  Quote + qtpath + Quote  ;
	//print(Mes) ;
	m_Fileswithstrings.Add(Mes, 1)  ;

	Mes = Lf + Tab + _("Eligibles files ") ;
	Mes += _T("(*.h, *.hpp, *.cpp, *.ui, *.qrc)")  ;
//	print(Mes) ;
	m_Fileswithstrings.Add(Mes, 1)  ;
	Mes = _T("");
    if (m_project) {
		int nfp = m_project->GetFilesCount();
		Mes = Lf + Tab + _("The active m_project ") + Quote + m_nameproject + Quote ;
		Mes += _T(", ") + (wxString() << nfp) + _(" files") ;
		Mes += Lf + Tab + _("Work path : ") + Quote + m_dirproject + Quote ;
    }
	// date
	wxString Datebegin = date() ;
	Mes +=  Lf +  Tab + _("Date : ")+ Datebegin  ;
	//print(Mes);
	m_Fileswithstrings.Add(Mes, 1);
	// for duration
	m_start = clock();
}
///-----------------------------------------------------------------------------
/// called by :
///	1. buildQt():1,
///
void qtPrebuild::endMesCreate() {
// end '*.cre'
	Mes = Lf + _("End ") + Quote + m_Thename + Quote  ;
	m_Fileswithstrings.Add(Mes, 1) ;
	// date and duration
	Mes = date()  + _T(" -> ") + _("duration = ") + duration() ;
	//print (Mes) ;
	m_Fileswithstrings.Add(Mes, 1)  ;
// saving
	if (m_Savereport) {
		bool ok = saveFileswithstrings();
		if (!ok) {
			Mes = _("The message table can not be saved to disk !");
			printErr(Mes);
		}
	}
}
///-----------------------------------------------------------------------------
/// Generating the complements files...
///
/// called by :
///	1. QtPregen::OnPrebuild(CodeBlocksEvent& event):1
///
/// calls to :
///	1. findGoodfiles():1,
///	2. addRecording():1,
///	3. filesTocreate(allbuild):1,
///	4. createFiles():1,
///	5. validCreated():1,
///
bool qtPrebuild::buildQt(cbProject * prj, bool workspace, bool allbuild)
{
	m_project = prj;
	bool ok = m_project != nullptr;
	if (!ok) return ok;

	m_Savereport = ! workspace;
    // begin banner
	beginMesCreate();

	ok = false;
	Mes = Tab + _("Generating the complements files ") ;
	Mes += _("for the project ") + Quote + m_project->GetTitle() + Quote ;
	print(Mes);
	///***********************************************
	///1- find good target with eligible files
	///***********************************************
	wxString mes = _T("1- findGoodfiles() ...") ;
	// analyzing all project files
	int nelegible =  findGoodfiles() ;
	mes += Lf + Tab + _T("elegible files= ") + (wxString()<<nelegible) ;
	if (nelegible > 0) {
		///************************************************
		///2-  registered all files in 'm_Registered'
		///************************************************
		mes += Lf + _T("2- addRecording() ...");
		int nfiles = addRecording() ;
		mes += Lf + Tab + _T("recorded files = ") + (wxString()<<nfiles) ;
		if (nfiles > 0) {

			///************************************************
			///3-  files to create to 'm_Filestocreate'
			///************************************************
			mes += Lf + _T("3- filesTocreate(") ;
			mes += allbuild ? _T("true") : _T("false");
			mes += _T(") ...") ;
			uint16_t ntocreate = filesTocreate(allbuild) ;
			mes += Lf + Tab + _T("files to create = ") + (wxString()<<ntocreate) ;
			if (ntocreate == 0 )
				ok = !allbuild ;
			else
				ok = true;

			if (ok) {
				///**********************************************************
				///4- create files
				/// from 'FileCreator', 'm_Registered'
				/// 1- adds files created in 'm_Createdfile'
				/// 2- create additional files as needed  by 'create(qexe, fileori)'
				///***********************************************************
				mes +=  Lf + _T("4- createFiles() ...");
				bool good = createFiles() ;
				if (good) {

					///**********************************
					///5- register files created
					///**********************************
					mes += Lf + _T("5- validCreated() ...")  ;
					good = validCreated() ;
					if (good) {
                //  print(mes);
						///**********************************
						///6- ???
						///**********************************
						//print(_T("- empty ...") ) ;
						ok = true;

					}	// end (good)
				} // end (good)
			}	//end (ntocreate > 0)
		} // end (nfiles > 0)
	} // end (nelegible > 0)
    // end banner
	endMesCreate();

	return ok;
}
///-----------------------------------------------------------------------------
/// Generating one complement file...
///
/// - called by :
///	1. QtPregen::OnPrecompile(CodeBlocksEvent& event)
///
/// - calls to :
///   -#  isElegible(filename):1,
///   -#  inProjectFile(fout):1,
///   -#  CompareDate(fcreator, fout):1,
///   -#  createFileComplement(m_Mexe, fcreator, fout):1,
///
bool qtPrebuild::buildFileQt(cbProject * prj, const wxString& fcreator)
{
	if(!prj) return false;

	m_dirproject = prj->GetBasePath();

    bool elegible = isElegible(fcreator);
    if (elegible) {
		m_project = prj;
		m_nameactiveproject = prj->GetActiveBuildTarget() ;
     	wxString fout = m_dirgen + m_nameactiveproject + wxString(Slash) ;
		fout += nameCreated(fcreator) ;

		Mes = Tab + _(" One supplement file are registered in the project") ;
		print(Mes) ;
		Mes = Tab + Quote + fcreator + Quote + _T(" -> ") + Quote + fout + Quote ;
		print(Mes);
    // already registered
		bool inproject = inProjectFile(fout) ;
		if (!inproject) {
		// has included ?
			wxString extin = fcreator.AfterLast('.') ;
			wxString extout = fout.AfterLast('.')  ;
			bool include = false ;
			if ( extin.Matches(EXT_H) && extout.Matches(EXT_CPP) )
				include =  hasIncluded(fcreator) ;
		// add file : AddFile(Nametarget, file, compile, link, weight)
			ProjectFile* prjfile = m_project->AddFile(m_nameactiveproject, fout, !include, !include, 50);
			if (!prjfile) {
			// display message
				Mes  = _T("===> ") ;
				Mes += _("can not add this file ");
				Mes += Quote + fout + Quote + _(" to target ") + m_nameactiveproject ;
				printErr (Mes) ;
				cbMessageBox(Mes, _T("AddFile(...)"), wxICON_ERROR) ;
			}
		// display
			Mes = Tab + _("Add ") + Quote + fout + Quote ;
			if (include) {
				Mes += Lf + Tab + Tab + _T("*** ") + _("This file is included, ") ;
				Mes += _("attributes 'compile' and 'link' will be set to 'false'") ;
			}
			print(Mes);
			//m_Fileswithstrings.Add(Mes, 1) ;

			// svn 9501 : CB 13.12
			Manager::Get()->GetProjectManager()->GetUI().RebuildTree() ;
			// svn 8629 : CB 12.11
			//Manager::Get()->GetProjectManager()->RebuildTree() ;

		// create complement ...
			m_Identical = false;
		}
		else {
		// Check the date
			m_Identical = false ;
			//- verify datation on disk
			if (wxFileExists(fout))
				m_Identical = CompareDate(fcreator, fout) ;
		}
	// ! identical date -> create
		if (!m_Identical) {
			Mes = Tab + _(" One supplement file are created in the project ...") ;
			printWarn(Mes);
		// create file complement with 'moc'
            wxString strerror = createFileComplement(m_Mexe, fcreator, fout);
            if (!strerror.IsEmpty()) {
			// error message
				wxString title = _("Creating ") + Quote + fout + Quote ;
				title += _(" failed") ;
				title += _T(" ...");
				//1- error create directory  :
					// = _("Unable to create directory ")
				//2- error buildtarget no exist :
					// = Quote + m_nameactiveproject + Quote + _(" does not exist !!") ;
				//3- error create complement
				Mes =  _T("=> ") ;
				Mes += strerror.BeforeLast(Lf.GetChar(0)) ;
				printErr (Mes) ;
				m_Fileswithstrings.Add(Mes, 1) ;
				cbMessageBox(Mes, title) ;
            }
		}
    }

    return elegible;
}
///-----------------------------------------------------------------------------
/// called by :
///	1. endMesCreate():1,
///
/// calls to :
///	1. saveArray(...):1,
///
bool qtPrebuild::saveFileswithstrings() {
	bool good = false ;
	if (m_project) {
		wxString nameprj = m_project->GetTitle();
		good = saveArray(m_Fileswithstrings, nameprj + _T(".cre")) ;
	}
	return good;
}
///-----------------------------------------------------------------------------
/// Reference of libQt by target: "" or "4r" or "4d" or "5r" or "5d"
///  qt4 -> '4', qt5 -> '5', release -> 'r' and debug -> 'd'
///
/// called by  :
///	1. isGoodTargetQt():1,
///	2. findGoodfiles():1
///
wxString qtPrebuild::refTargetQt(ProjectBuildTarget * buildtarget) {
	wxString refqt = _T("") ;
	if (! buildtarget)
		return refqt  ;

	wxArrayString tablibs = buildtarget->GetLinkLibs();
	int nlib = tablibs.GetCount() ;
	if (!nlib)
		return refqt ;

	// search lib
	bool ok = false ;
	wxString str =  _T("") ;
	wxString namelib ;
	int u=0, index= -1, pos ;
	while (u < nlib && !ok ) {
		// lower, no extension
		namelib = tablibs.Item(u++).Lower().BeforeFirst('.') ;
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
		if (ok)  {
			if (namelib.GetChar(2) == '5') {
				refqt = _T("5") ;
				// the last
				str = namelib.Right(1) ;
				if ( str.Matches(_T("d") ) )
					refqt += _T("d") ;
				else
					refqt += _T("r") ;
			}
			else  {
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
	if (refqt.IsEmpty() ) {
		Mes = _("It is not a Qt target") + Lf + _("or nothing library") + _T(" ! ") ;
		Mes += Lf + _("Cannot continue.") ;
		cbMessageBox(Mes, _("QtPregen information"), wxICON_INFORMATION | wxOK)  ;
		//m_Fileswithstrings.Add(Mes, 1)
	}

	return refqt ;
}
///-----------------------------------------------------------------------------
/// Find path Qt installation
///
/// called by  :
///	1. beginMesCreate():1,
///
wxString qtPrebuild::findpathProjectQt() {
	wxString path = _("");
	if (!m_project)
		return path;

//1- analyze the Project	'cbproject* Project'
	path = pathQt(m_project) ;
	if (path.IsEmpty()) {
//2- analyze all targets
		bool ok = false ;
		ProjectBuildTarget * buildtarget ;
		int ntargets = m_project->GetBuildTargetsCount() ;
		int nt = 0 ;
		while (nt < ntargets && ! ok ) {
		// retrieve the target Qt libraries paths
			buildtarget = m_project->GetBuildTarget(nt++) ;
			if (! buildtarget)
				continue;

			path = pathQt(buildtarget) ;
			ok = ! path.IsEmpty() ;
		}
	}
	return path ;
}
///-----------------------------------------------------------------------------
/// Find path Qt container  ->  'CompileOptionsBase * container'
///
/// called by  :
///	1. findpathprojetQt():2,
///	2. findTargetQtexe(buildtarget):1,
///
wxString qtPrebuild::pathQt(CompileTargetBase * container) {
	wxString path = _T("");
	if (!container)
		return path;

	wxArrayString tablibdirs = container->GetLibDirs() ;
	int npath = tablibdirs.GetCount() ;
	wxString  path_nomacro ;
	if (npath > 0 ) {
		wxString path;
		bool ok = false ;
		int u = 0  ;
		while (u < npath && !ok ) {
			path = tablibdirs.Item(u++);
		//print(_T("path = ") + Quote+ path + Quote);
			// not correct !!
			ok = path.Find(_T("$QT")) != -1 ;
			if (ok) {
		//	print(_T("path = ") + Quote+ path + Quote);
				m_mam->ReplaceEnvVars(path) ;
		//	print(_T("path = ") + Quote+ path + Quote);
				path_nomacro =  path ;
				// remove "\lib"
				path_nomacro = path_nomacro.BeforeLast(Slash) ;
				path_nomacro += wxString(Slash)  ;
			}
			else {  // correct
				path.Lower();
				ok = path.Find(_T("#qt")) != -1 ;
				if (ok) {
			//	print(_T("path = ") + Quote+ path + Quote);
					m_mam->ReplaceMacros(path) ;
			//	print(_T("path = ") + Quote+ path + Quote);
					path_nomacro =  path ;
					// remove "\lib"
					path_nomacro = path_nomacro.BeforeLast(Slash) ;
					path_nomacro += wxString(Slash)  ;
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
///	1. createFiles():1,
///
bool qtPrebuild::findTargetQtexe(cbProject * parentbuildtarget)
{
	if (!parentbuildtarget)
		return false  ;

//	Mes = _T("name projet = ") + Quote + parentbuildtarget->GetTitle() + Quote;
//	printErr(Mes);
	wxString qtpath = pathQt(parentbuildtarget) ;
	if(qtpath.IsEmpty()) {
		Mes = _("Nothing path 'qt'") ;
		Mes += Lf + _("or nothing library !") + Lf + _("Cannot continue.") ;
		cbMessageBox(Mes, _T(""), wxICON_ERROR) ;
		return false ;
	}

//	Mes = _T("qtpath = ") + Quote + qtpath + Quote;
//	printErr(Mes);
	wxString qtexe = qtpath + _T("bin") + wxFILE_SEP_PATH  ;
	if (m_Win) {
		m_Mexe = qtexe + _T("moc.exe") ;
		m_Uexe = qtexe + _T("uic.exe") ;
		m_Rexe = qtexe + _T("rcc.exe") ;
		// compile *_fr.ts -> *_fr.qm
		m_Lexe = qtexe + _T("lrelease.exe") ;
	}
	else
	if (m_Linux) {	// check !
		m_Mexe = qtexe + _T("moc") ;
		m_Uexe = qtexe + _T("uic") ;
		m_Rexe = qtexe + _T("rcc") ;
		m_Lexe = qtexe + _T("lrelease") ;
	}
	else
	if (m_Mac) { 	// ???
		m_Mexe = qtexe + _T("moc");
		m_Uexe = qtexe + _T("uic") ;
		m_Rexe = qtexe + _T("rcc") ;
		m_Lexe = qtexe + _T("lrelease") ;
	}

//	Mes = _T("qtexe = ") + Quote + qtexe + Quote;
//	printErr(Mes);
	bool Findqtexe = wxFileExists(m_Mexe) && wxFileExists(m_Uexe) ;
	Findqtexe = Findqtexe && wxFileExists(m_Rexe) && wxFileExists(m_Lexe) ;
	if (!Findqtexe) {
		Mes = _("Could not query the executable Qt") ;
		Mes +=  _T(" !") ;
		Mes += Lf + _("Cannot continue.") ;
		cbMessageBox(Mes, _("Search executable Qt ..."), wxICON_ERROR) ;
	//	m_Fileswithstrings.Add(Mes, 1)
	}
	m_IncPathQt = pathIncludeMoc() ;
	m_DefinesQt = definesMoc() ;

	return Findqtexe ;
}
///-----------------------------------------------------------------------------
/// Save messages array to file
///
/// called by  :
///	1. SaveFileswithstrings():1,
/// calls to :
///	1. none
///
bool qtPrebuild::saveArray(const wxArrayString& table, wxString namefile)
{
	int n = table.GetCount() ;
	bool ok = n > 0 ;
	if (ok)  {
	// full name
		namefile = m_dirproject + namefile ;
	// exists ?
		ok = wxFileExists(namefile)  ;
		if (ok)  {
			wxRemoveFile(namefile) ;
		}
	// data to file
		wxString shortname = namefile.AfterLast(Slash) ;
		wxString mes ;
		wxString temp = GetStringFromArray(table, Eol, false ) ;
		ok = WriteFileContents(namefile, temp ) ;
		if (!ok)  {
			mes =  Tab + _("Could not write file ") ;
			mes += Quote + _T("locale\\") + shortname + Quote + _T(" !!") ;
			mes += Eol + Tab + _("Check that the name not contain illegal characters ...") ;
			m_Fileswithstrings.Add(mes, 1)  ;
			return false ;
		}
		mes = Tab + _("Write file ") ;
		mes += Quote + shortname + Quote  + Lf  + Lf ;
		m_Fileswithstrings.Add(mes, 1) ;
	// open file into editor
		ok = openedit(namefile) ;
	}

	return ok ;
}
///-----------------------------------------------------------------------------
/// called by  :
///	1. q_object(...):1,hasIncluded(wxString):1,
///
/// calls to :
///		none
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
///	1. saveArray():1,
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
/// Open file to editor
///
/// called by  :
///	1. saveArray():1,
/// calls to :
///     none
///
bool qtPrebuild::openedit(const wxString& filename)
{
	bool ok = wxFileExists(filename) ;
	if (ok) {
		cbEditor * ed_open = Manager::Get()->GetEditorManager()->IsBuiltinOpen(filename) ;
		if (ed_open )
			ed_open->Close() ;

		ed_open = Manager::Get()->GetEditorManager()->Open(filename) ;
		ok = ed_open != nullptr ;
		if (!ok ) {
			Mes = _("Couldn't open ") + Quote + filename + Quote + _T(" !!") ;
			cbMessageBox(Mes, _T(""), wxICON_ERROR)  ;
			//print (Mes)
			return ok ;
		}
	}
	return ok ;
}
///-----------------------------------------------------------------------------
/// Close file to editor
///
/// called by  :
///	1. saveArray():1,
///
/// calls to :
///     none
///
bool qtPrebuild::closedit(const wxString& filename)
{
//print(file);
	bool ok = wxFileExists(filename) ;
	if (ok) {
		cbEditor * ed_open = Manager::Get()->GetEditorManager()->IsBuiltinOpen(filename) ;
		if (ed_open ) {
			ok = ed_open->Close()  ;
			if (!ok ) {
				Mes = _("Couldn't close ") + Quote + filename + Quote + _T(" !!") ;
				cbMessageBox(Mes, _T(""), wxICON_ERROR) ;
				// print (Mes) ;
				return ok ;
			}
		}
	}
	return ok ;
}
///-----------------------------------------------------------------------------
/// Get date
///
/// called by  :
///	1. begimMesCre():1,
///	2. endMesCre():1
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
///	1. endMesCre():1
///
wxString qtPrebuild::duration()
{
	return wxString::Format(_T("%ld ms"), clock() - m_start  );
}
///-----------------------------------------------------------------------------
/// Looking for eligible files to the active target,
/// 	meet to table 'm_Filecreator' : return eligible file number
///
/// called by  :
///	1. QtPregen::OnPrebuild( ...):1
///
/// calls to :
///	1. isVirtualTarget(m_nameactiveproject):1,
///	2. isGoodTargetQt(m_nameactiveproject):1,
///	3. isEligible(file):1,
///
uint16_t qtPrebuild::findGoodfiles()
{
	m_Filecreator.Clear();
// active target
	m_nameactiveproject = m_project->GetActiveBuildTarget() ;
	Mes = Lf + Tab + _("The active target ") ;
	Mes += Quote + m_nameactiveproject + Quote  + _T(" :") ;
	//print(Mes) ;
	m_Fileswithstrings.Add(Mes, 1) ;
// virtual ?
	if (isVirtualTarget(m_nameactiveproject) )
		return 0 ;

// no good target
	if (! isGoodTargetQt(m_nameactiveproject))
		return 0 ;

// around project files
	ProjectFile * prjfile ;
	wxString file ;
	wxArrayString tabtargets ;
	int nt, nft=0 ;
	int nfprj = m_project->GetFilesCount()  ;
// all files project
	for (int nf =0 ; nf < nfprj; nf++ ) {
		prjfile = m_project->GetFile(nf);
		if (!prjfile)
			continue  ;

	//	file name
		file = prjfile->relativeFilename ;
		if (file.IsEmpty() )
			continue  ;

	// copy  all targets
	//	tabtargets = copyarray(prjfile->buildTargets) ;
		tabtargets = prjfile->buildTargets ;
		nt = tabtargets.GetCount()  ;
		if (nt > 0) {
			wxString nametarget ;
			bool good ;
			// all file targets
			for (int t=0; t < nt; t++) {
				nametarget = tabtargets.Item(t) ;
				good = nametarget.Matches(m_nameactiveproject) ;
				if (good )
					break ;
			}
			if (good ) {
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
///	1. findGoodfiles():2,
///
/// calls to:
///	1. refTargetQt(buildtarget):1,
///
bool qtPrebuild::isGoodTargetQt(const wxString& nametarget)
{
	ProjectBuildTarget * buildtarget = m_project->GetBuildTarget(nametarget) ;
	bool ok = buildtarget != nullptr;
	if (!ok) return ok ;

	ok = buildtarget->GetTargetType() != ::ttCommandsOnly  ;
	wxString str = refTargetQt(buildtarget)  ;
	bool qt = ! str.IsEmpty()  ;

	return ok && qt ;
}
///-----------------------------------------------------------------------------
/// Search virtual target
///
/// called by  :
///	1. findGoodfiles():1,
///
bool qtPrebuild::isVirtualTarget(const wxString& nametarget) {
	// search virtual target : vtargetsTable <- table::wxArrayString
	wxString namevirtualtarget = _T("") ;
	wxArrayString vtargetsTable = m_project->GetVirtualBuildTargets () ;
	int nvirtual = vtargetsTable.GetCount() ;
	bool ok = nvirtual > 0 ;
	if (ok) {
		//PrintTable(_T("vtargetsTable"), vtargetsTable)
		ok = false  ;
	// virtual name
		for (int nv = 0; nv < nvirtual ; nv++ ) {
			namevirtualtarget = vtargetsTable.Item(nv) ;
			ok = nametarget.Matches(namevirtualtarget) ;
			if (ok)  {
				Mes = _T("'") + namevirtualtarget + _T("'") ;
				Mes += _(" is a virtual target !!") ;
				Mes += Lf + _("NOT YET IMPLEMENTED...") ;
				Mes += Lf + _("... you must use the real targets.")  ;
				Mes += Lf + _("Cannot continue.") ;
				cbMessageBox(Mes, _T(""), wxICON_ERROR)  ;
				m_Fileswithstrings.Add(Mes, 1)  ;
				break  ;
			}
		}
	}

	return ok  ;
}
///-----------------------------------------------------------------------------
/// Search elegible files
/// 'file' was created ?  ->  moc_*.cxx, ui_*.h, qrc_*.cpp
///
/// called by  :
///	1. findGoodfiles():1,
///
/// calls to :
/// 1. q_object(file, _T("Q_OBJECT") ):1;
///
bool qtPrebuild::isElegible(const wxString& file)
{
	wxString name = file.AfterLast(Slash) ;
	// eliminates complements files
	int pos = name.Find(_T("_")) ;
	if (pos != -1) {
		wxString pre  = name.Mid(0, pos)  ;
		if (pre.Matches(m_Moc) || pre.Matches(m_UI) || pre.Matches(m_Qrc) )
			return false  ;
	}
	wxString ext = name.AfterLast('.')  ;
	bool ok = ext.Matches(m_UI) || ext.Matches(m_Qrc) ;
	if (ext.Matches(EXT_H) || ext.Matches(EXT_HPP) || ext.Matches(EXT_CPP) ) {
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
///	1. q_object():2
///
int  qtPrebuild::qt_find (wxString tmp, const wxString& qt_text)
{
	//wxChar
	uint8_t tab = 0x9, espace = 0x20  ;
	int len_text = qt_text.length() ;
	int posq, number =0 ;
	bool ok, good, goodb, gooda ;

	do {
		posq = tmp.Find(qt_text) ;
		ok = posq != -1 ;
		if (ok) {
		// pred and next caracter
			//wxUChar
			uint8_t xcar = tmp.GetChar(posq-1) ;
			//wxUChar
			uint8_t carx = tmp.GetChar(posq + len_text)  ;
		/*
		Mes = _T(" char before '") ;
		Mes += wxString()<< xcar ;
		Mes +=_T("', char after '");
		Mes += wxString()<< carx ;
		Mes += _T("'");
		print (Mes);
		*/
		// only autorized caracters
			// before
			goodb = (xcar == espace || xcar == tab || xcar == Sepf) ;
			// next
			gooda = (carx == espace || carx == tab || carx == Sepd || carx == Sepf);
			gooda = gooda || carx == '(' || carx == '_' ;
			good = goodb && gooda  ;
			if (good)
				number++  ;
		// delete analyzed
			tmp.Remove(0, posq + len_text)  ;
		}
	} while (ok) ;

	return number ;
}
///-----------------------------------------------------------------------------
/// Return occurrences of "Q_OBJECT" + occurrences of "Q_GADGET"
///
/// called by  :
///	1. isEligible():1
///
int qtPrebuild::q_object(const wxString& filename, const wxString& qt_macro)
{
//1- the comments
	wxString CPP = _T("//") ;
	wxString DCC = _T("/*") ;
	wxString FCC = _T("*/") ;
	// end of line
	wxString SF, SD;
	SF.Append(Sepf); SD.Append(Sepd) ;
	// the result
	bool good= false ;
//2- verify exist
	wxString namefile = m_dirproject + filename ;
	if (! wxFileExists(namefile)) {
		Mes = Quote + namefile + Quote + _(" NOT FOUND") + _T(" !!!") + Lf ;
		print(Mes) ;
		m_Fileswithstrings.Add(Mes, 1);
		return -1  ;
	}
//3- read source 'namefile'
	wxString source = ReadFileContents(namefile) ;
	if (source.IsEmpty())  {
		Mes = Quote + namefile + Quote + _(" : file is empty") +  _T(" !!!") + Lf ;
		print(Mes) ;
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
	do {
		reste = temp.length() ;
	// first DCC next FCC
		posdcc = temp.Find(DCC) ;
		if (posdcc != -1) {
			reste = reste - posdcc ;
			after = temp.Mid(posdcc, reste) ;
			posfcc = after.Find(FCC) ;
			// 'posfcc' relative to 'posdcc' !!
		}
		// comments exists ?
		ok = posdcc != -1 && posfcc != -1  ;
		if (ok) {
		// delete full comment
			moins = posfcc + FCC.length() ;
			temp.Remove(posdcc, moins) ;
		}
	} while (ok)  ;
//5- delete C++ comments
	do {
		reste = temp.length()  ;
	// start of a comment C
		poscpp = temp.Find(CPP) ;
		if (poscpp != -1) {
		// verify the caracter before : 'http://www.xxx' !!
			wxChar xcar = temp.GetChar(poscpp-1) ;
			if (xcar != ':' ) {
				reste = reste - poscpp  ;
			// string comment over begin
				after = temp.Mid(poscpp, reste) ;
			// end comment
				posfcc = after.Find(SF) ;
			}
			else
				poscpp = -1  ;
		}
		ok = poscpp != -1 && posfcc != -1 ;
		if (ok) {
		// delete full comment
			temp.Remove(poscpp, posfcc) ;
		}
	} while (ok) ;
//6- find "Q_OBJECT" and/or "Q_GADJECT"
	int number = 0  ;
	wxString findtext = _T("Q_OBJECT") ;
	if ( qt_macro.Matches(findtext) ) {
		number = qt_find(temp, findtext)  ;
		findtext = _T("Q_GADGET") ;
		number += qt_find(temp, findtext)  ;
	}
	else  {
      // TODO ...
	}

	return number ;
}
///-----------------------------------------------------------------------------
/// To register all new files in project : 'm_Registered' is meet
///	return 'true' when right
///
/// called by  :
///	1. QtPregen::OnPrebuild():1,
///
/// calls to :
///	1. indexTarget(m_nameactiveproject):1,
///	2. nameCreated(fcreator):1,
///	3. inProjectFile(fout):1,
///	4. hasIncluded(fcreator):1,
///

uint16_t qtPrebuild::addRecording()
{
	m_Registered.Clear();
// local variables
	bool valid = false , inproject = false  ;
	wxString fout, fcreator, strindex = _T("0") ;
	uint16_t n=0, index, indextarget = indexTarget(m_nameactiveproject) ;
	ProjectFile *  prjfile;
// read file list to 'm_Filecreator'  (*.h, *.cpp, *.qrc, *.ui)
	uint16_t nfiles = m_Filecreator.GetCount() ;
	for (uint16_t u=0; u < nfiles ; u++ ) {
		// creator
		fcreator = m_Filecreator.Item(u)  ;
		//  this is where you add the name of the target
		fout = m_dirgen + m_nameactiveproject + wxString(Slash)  ;
	//print(fout);
		fout += nameCreated(fcreator) ;

		Mes = Tab + Quote + fcreator + Quote + _T(" -> ") +  Quote + fout + Quote;
print(Mes);
		// checks if the file exists in the project
		inproject = inProjectFile(fout) ;
		if (!inproject) {
			// has included ?
			wxString extin = fcreator.AfterLast('.') ;
			wxString extout = fout.AfterLast('.')  ;
			bool include = false ;
			if ( extin.Matches(EXT_H) && extout.Matches(EXT_CPP) )
				include =  hasIncluded(fcreator) ;

		// AddFile(Nametarget, file, compile, link, weight)
			prjfile = m_project->AddFile(m_nameactiveproject, fout, !include, !include, 50);
			valid = true;
			if (!prjfile) {
				valid = false;
			// display message
				Mes  = _T("===> ") ;
				Mes += _("can not add this file ");
				Mes += Quote + fout + Quote + _(" to target ") + m_nameactiveproject ;
				printErr (Mes) ;
				cbMessageBox(Mes, _T("AddFile(...)"), wxICON_ERROR) ;
			}
			// display
			n++ ;
			Mes = Tab + (wxString()<<n) + _T("- ") ;
			Mes += Quote + fout + Quote ;
			if (include) {
				Mes += Lf + Tab + Tab + _T("*** ") + _("This file is included, ") ;
				Mes += _("attributes 'compile' and 'link' will be set to 'false'") ;
			}
			//print(Mes);
			m_Fileswithstrings.Add(Mes, 1) ;
		}
		// valid
		if ( valid || inproject){
		// registered in table
			m_Registered.Add(fout, 1) ;
		}
	}
// tree refresh
	// svn 9501 : CB 13.12
	Manager::Get()->GetProjectManager()->GetUI().RebuildTree() ;
	// svn 8629 : CB 12.11
	//Manager::Get()->GetProjectManager()->RebuildTree() ;
//  end registering
	uint16_t nf = m_Registered.GetCount() ;
	Mes = Tab + Tab + _T("- ") + (wxString()<<nf) + _T(" ") ;
	Mes += _("supplements files are registered in the plugin") ;
	//print(Mes) ;
	m_Fileswithstrings.Add(Mes, 1) ;

	return  nf;
}
///-----------------------------------------------------------------------------
/// Returns the index of a target in the project
///
/// called by  :
///	1. addRecording():1,
///
int qtPrebuild::indexTarget(const wxString& nametarget)
{
	ProjectBuildTarget *  buildtarget ;
	uint16_t ntargets = m_project->GetBuildTargetsCount() ;
	if (ntargets < 1) return -1 ;

	for (uint16_t nt = 0; nt < ntargets; nt++) {
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
///	1. addRecording():1,
///
wxString qtPrebuild::nameCreated(const wxString& file)
{
	wxString name  = file.BeforeLast('.') ;
	wxString fout ;
	if (name.Find(Slash) > 0) {
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
		fout +=  name + _T(".moc") ;

	fout = fout.AfterLast(Slash) ;

	return fout  ;
}
///-----------------------------------------------------------------------------
/// Indicates whether a file is registered in the project
///
/// called by  :
///	1. addRecording():1,
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
///	1. addRecording():1,
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
	if (!include) {
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
///	1. createComplement():1,
///
bool qtPrebuild::createDir (const wxString&  dirgen)
{
	bool ok = true  ;
	if (! wxDirExists(dirgen)) {
		ok = wxMkdir(dirgen, 0) ;
		if (!ok) {
			Mes = _("Can't create directory ")  ;
			Mes += Quote + dirgen + Quote + _T(" !!") ;
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
	bool ok = wxDirExists(dirgen) ;
	if (ok) {
		ok = wxRmdir(dirgen)  ;
		if (!ok) {
			Mes = _("Can't remove directory ") ;
			Mes += Quote + dirgen + Quote + _T(" !!") ;
			cbMessageBox(Mes, _T(""), wxICON_ERROR) ;
		}
	}

	return ok ;
}
///-----------------------------------------------------------------------------
/// Must complete the table 'm_Filestocreate'
///
/// called by
///    1. QtPregen::m_prebuild (...)
///
uint16_t qtPrebuild::filesTocreate(bool allrebuild)
{
	m_Filestocreate.Clear();
// save all open files
	bool ok = Manager::Get()->GetEditorManager()->SaveAll();
	if (!ok ) {
		Mes = _("Couldn't save all files ") ;
		Mes += _T(" !!")  ;
		cbMessageBox(Mes, _T(""), wxICON_ERROR)  ;
		print (Mes);
		return ok  ;
	}
    uint16_t  ntocreate = 0;
// all rebuild : copy table
	if (allrebuild) {
		m_Filestocreate = copyArray(m_Registered) ;
		ntocreate = m_Filestocreate.GetCount();
	}
// build or rebuild not identical
	else {
	// local variables
		wxString inputfile, nameout ;				// wxString
		bool inproject ;						// bool
	// analyze all eligible files/targets
		uint16_t nfiles = m_Filecreator.GetCount() ; 	// integer
		for (uint16_t i=0; i < nfiles ; i++ ) {
		// relative name inputfile
			inputfile = m_Filecreator.Item(i) ;
		// file registered to create
			nameout = m_Registered.Item(i) ;
		// in project ?
			inproject =  inProjectFile(nameout) ;
			m_Identical = false ;
		//- verify datation on disk
			if (inproject && m_Dexeishere) {
			// verify if exists file
				if (wxFileExists(nameout)) {
				// test his modification date
					m_Identical = CompareDate(inputfile, nameout) ;
				}
			}
			// identical date -> no create
			if (m_Identical)
			// add 'm_Devoid' to 'Filetocreate' and continue
				m_Filestocreate.Add(m_Devoid, 1) ;
			else  {
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
///	1. createFiles():1,
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
///	1. createComplement(qexe, index):1
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
///	1. 'm_Filecreator' is meet by 'findGoodfiles()'
///	2. 'm_Registered is filled by 'addRecording()'
///	3. 'm_Filestocreate' filled by 'filesTocreate(allrebuild)'
///	4. meet 'm_Createdfile'
///
/// return true if all right
///
/// called by  :
///	1. buildQt(cbProject*, bool) :1,
///
///	calls to :
///	1. isEmpty(...):1,
///	2. findTargetQtexe(buildtarget->GetParentProject()):1,
///	3. createComplement(...):3,
///
bool qtPrebuild::createFiles()
{
	m_Createdfile.Clear();
// nothing to do ?
	bool emptiness = isEmpty (m_Filestocreate) ;
	if (emptiness) {
	// already created
		Mes = Tab + _T("- ") + _("thing to be done (all items are up-to-date)") ;
		print(Mes) ;
		m_Fileswithstrings.Add(Mes, 1) ;
		return emptiness  ;
	}

//  message "begin create"
	Mes = Lf + Tab + _T("=> ") + _("Begin create file to target")  ;
	m_Fileswithstrings.Add(Mes, 1) ;

// search target qt path for moc, ...
	ProjectBuildTarget * buildtarget = m_project->GetBuildTarget(m_nameactiveproject)  ;
	bool ok = buildtarget != nullptr ;
	if (!ok) return ok ;

	ok = findTargetQtexe(buildtarget->GetParentProject()) ;
	if (!ok) return ok ;

// local variables
	wxString nameout, ext ;
	bool created = false;
	wxString  strerror;
	// used by 'createComplement()'
	m_nfilescreated = 0 ;
// analyze all eligible files/target
	uint16_t  nfiles = m_Filestocreate.GetCount() ;
	for (uint16_t   i=0; i < nfiles ; i++ ) {
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

		created = strerror.IsEmpty();
	// more one
		if (created)
			m_Createdfile.Add(nameout, 1)  ;
		else  {
			m_Createdfile.Add(m_Devoid, 1)  ;
		// error message
			wxString title = _("Creating ") + Quote + m_Registered.Item(i) + Quote ;
			title += _(" failed") ;
			title += _T(" ...");
			//1- error create directory  :
				// = _("Unable to create directory ")
			//2- error buildtarget no exist :
				// = Quote + m_nameactiveproject + Quote + _(" does not exist !!") ;
			//3- error create complement
			Mes =  _T("=> ") ;
			Mes += strerror.BeforeLast(Lf.GetChar(0)) ;
			printErr (Mes) ;
			m_Fileswithstrings.Add(Mes, 1) ;

			Mes += Lf + Lf + _("Do you want to stop the generation ?");
			int16_t choice = cbMessageBox(Mes, title, wxICON_QUESTION | wxYES_NO) ;
			if (choice == wxID_YES) {
                Mes = _("Abandonment of the construction ...");
				printErr (Mes) ;
				m_Fileswithstrings.Add(Mes, 1) ;
				break;
			}
		}
	}  // end forManageprj.RebuildTree()

	uint16_t nf = m_Createdfile.GetCount() ;
	Mes =  Tab + _T("- ") + (wxString()<<nf) + _T(" ")  ;
	Mes += _("supplements files are created in the target") ;
	print(Mes)  ;

// message "end create"
	Mes = Tab + _T("<= ") + _("End create file") ;
	m_Fileswithstrings.Add(Mes, 1)  ;

	return created   ;
}
///-----------------------------------------------------------------------------
/// Search all 'm_Devoid' to an wxArrayString
///
/// called by  :
///	1. createFiles()!1,
///
bool qtPrebuild::isEmpty (const wxArrayString& arraystr)
{
	uint16_t nf = arraystr.GetCount() ;
	bool devoid = false ;
	if (nf > 0) {
		for (uint16_t u= 0; u < nf ; u++ ) {
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
///	1. createFiles():6,
///
wxString qtPrebuild::createFileComplement(const wxString&  qexe,
										  const wxString& fcreator,
										  const wxString& fout)
{
//3- build command
	wxString command = qexe  ;
	// add file name whithout extension
	if (qexe.Matches(m_Rexe)) {
		wxString name = fcreator.BeforeLast('.')  ;
		command +=  _T(" -name ") + name  ;
	}
	if (qexe.Matches(m_Mexe)) {
		command +=  m_DefinesQt + m_IncPathQt  + _T(" -D__GNUC__ -DWIN32") ;
	}
	// add input file
	command += _T(" \"") + fcreator + _T("\"") ;
	// add output file
	command += _T(" -o \"") + fout + _T("\" ") ;

//4- execute command line : use short file name
	wxString strerror = ExecuteAndGetOutputAndError(command, true)  ;
	bool created =  strerror.IsEmpty() ;
	m_nfilescreated++ ;
	Mes = Tab + (wxString()<<m_nfilescreated) + _T("- ") + Quote + fcreator + Quote  ;
	Mes +=  _T(" ---> ") + Quote + fout + Quote ;
	m_Fileswithstrings.Add(Mes, 1)  ;
	Mes = _T(">>") + command + _T("<<") ;
// log to debug
	//printMes) ;
	m_Fileswithstrings.Add(Mes, 1)  ;
	// create error
	if (! created) {
	// unregister the file 'Outputfile' in the project
		ProjectFile * prjfile =  m_project->GetFileByFilename(fout, true, false) ;
		if (!prjfile) {
			m_project->RemoveFile(prjfile) ;
			// svn 9501 : CB 13.12
			Manager::Get()->GetProjectManager()->GetUI().RebuildTree() ;
			// svn 8629 : CB 12.11
			//Manager::Get()->GetProjectManager()->RebuildTree() ;
		}
	}
	else {
		if (m_Dexeishere)
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
///	1. createFiles():6,
///
wxString qtPrebuild::createComplement(const wxString& qexe, const uint16_t index)
{
	bool created = false ;
//1- name relative input file
	wxString inputfile = m_Filecreator.Item(index) ;
	// create directory for m_nameactiveproject
	wxString dirgen = m_Registered.Item(index).BeforeLast(Slash)  ;
	dirgen +=  wxFILE_SEP_PATH ;
	bool ok = createDir(dirgen) ;
	if (!ok) {
		Mes = _("Unable to create directory ") + Quote + dirgen + Quote ;
		return Mes ;
	}

	// add search path for compiler
	ProjectBuildTarget * buildtarget = m_project->GetBuildTarget(m_nameactiveproject) ;
	if (!buildtarget)  {
		Mes = Quote + m_nameactiveproject + Quote + _(" does not exist !!") ;
		return Mes ;
	}

	// add include directory
	buildtarget->AddIncludeDir(dirgen) ;
//2- full path name  output file
	wxString Outputfile = m_Registered.Item(index) ;
//3- create on file complement
	wxString strerror = createFileComplement(qexe, inputfile, Outputfile) ;
//4- return  error  : good if strerror is empty
	return strerror ;
}
///-----------------------------------------------------------------------------
/// Give include path qt for 'm_Moc'
///
/// called by :
/// 1.findTargetQtexe(cbProject *):1,
///
wxString qtPrebuild::pathIncludeMoc()
{
	wxArrayString tabincludedirs = m_project->GetIncludeDirs() ;
//	PrintTable(_T("tabincludedirs"),tabincludedirs)
	wxArrayString tabpath ;
	wxString line ;
	uint16_t ncase = tabincludedirs.GetCount()  ;
	for (uint16_t u = 0; u < ncase ; u++) {
		line = tabincludedirs.Item(u) ;
	//	if (line.Find(_T("#qt")) != -1 ) {
			m_mam-> ReplaceMacros(line) ;
			line = 	_T("-I") + Quote + line + Quote  ;
			tabpath.Add(line, 1) ;
	//	}
	}
	//PrintTable(_T("tabpath"),tabpath)
	// build 'incpath'
	wxString incpath = _T(" ") + GetStringFromArray(tabpath, _T(" "), true) + _T(" ");
//	print(incpath)

	return incpath  ;
}
///-----------------------------------------------------------------------------
/// Give 'defines' qt for 'm_Moc'
///
/// called by :
///	1. findTargetQtexe(buildtarget):1,
///
wxString qtPrebuild::definesMoc()
{
	wxArrayString tabdefines = m_project->GetCompilerOptions() ;
	//PrintTable(_T("tabdefines"),tabdefines)
	wxArrayString tabdef ;
	wxString  line ;
	uint16_t ncase = tabdefines.GetCount()  ;
	for (uint16_t u = 0; u < ncase ; u++) {
		line = tabdefines.Item(u) ;
		if (line.Find(_T("-D")) != -1 )
			tabdef.Add(line, 1) ;
	}
	//PrintTable(_T("tabdef"),tabdef)
// build 'def'
	wxString def = _T(" ") + GetStringFromArray(tabdef, _T(" "), true) + _T(" ") ;

	return def ;
}
///-----------------------------------------------------------------------------
/// Verify if at least one valid file is saved in 'm_Createdfile'
///
/// called by  :
/// 1. buildQt(cbProject *, bool):1,
///
bool qtPrebuild::validCreated()
{
	bool ok = false  ;
	uint16_t ncase = m_Createdfile.GetCount()  ;
	while (ncase > 0 && !ok) {
		ok = ! m_Createdfile.Item(--ncase).Matches(m_Devoid)  ;
	}
	if (ok) {
		// svn 9501 : CB 13.12
		Manager::Get()->GetProjectManager()->GetUI().RebuildTree()  ;
		// svn 8629 : CB 12.11
		// Manager::Get()->GetProjectManager()->RebuildTree()  ;
		Mes = Tab + _("-> ") + _("Save project and rebuild of the tree") ;
		Mes += _T(" ...") ;
		print(Mes) ;
		m_Fileswithstrings.Add(Mes + Lf, 1) ;
	// save project
		ok = Manager::Get()->GetProjectManager()->SaveProject(m_project)  ;
	}
	return ok ;
}
///-----------------------------------------------------------------------------
/// Execute a command
///
/// called by :
///	1. createComplement(wxString, uint16_t)
///
wxString qtPrebuild::ExecuteAndGetOutputAndError(const wxString& command, bool prepend_error)
{
	wxArrayString output;
	wxArrayString error;
	wxExecute(command, output, error, wxEXEC_NODISABLE);

	wxString str_out;

	if ( prepend_error && !error.IsEmpty())
		str_out += GetStringFromArray(error,  _T("\n"));

	if (!output.IsEmpty())
		str_out += GetStringFromArray(output, _T("\n"));

	if (!prepend_error && !error.IsEmpty())
		str_out += GetStringFromArray(error,  _T("\n"));

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
	if (m_Win) {
		wxString illegal = _T("[:*?\"<>|]") ;
		uint16_t pos, ncar = illegal.length() ;
	// all illegal characters
		for (uint16_t u = 0; u < ncar; u++) {
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
