/***************************************************************
 * Name:      print.h
 * Purpose:   Code::Blocks plugin	'qtPregenForCB.cbp'  0.8.5
 * Author:    LETARTARE
 * Created:   2015-10-17
 * Copyright: LETARTARE
 * License:   GPL
 **************************************************************/

///-----------------------------------------------------------------------------
#ifndef _PRINT_H_
	#define _PRINT_H_

///-----------------------------------------------------------------------------
/// end of line for Win/Linux/OX
#define 	Cr 		wxString(_T("\r"))
#define 	Lf 		wxString(_T("\n"))
#define 	CrLf 	wxString(_T("\r\n"))
#define 	Eol 	wxString(_T("\r\n"))
#define 	Quote 	wxString(_T("'"))
#define 	Tab		wxString(_T("\t"))
/// text separator
#define 	Sepd 	13 	// 0xD
#define 	Sepf 	10	// 0xA
#define 	Sizesep 2
/// directory separator
#include <wx/filefn.h>
/// use _T("....")  +  wxString (Slash ) : not  _T("....")  +  Slash !!
#define 	Slash 	wxFILE_SEP_PATH
///-----------------------------------------------------------------------------
#include <logmanager.h>
/// debug messages  -> 'Code::Blocks log'
#define lm			Manager::Get()->GetLogManager()
#define Print		lm->Log
#define PrintLn		lm->Log(_T(""))
#define PrintWarn	lm->LogWarning
#define PrintErr	lm->LogError
/// messages  -> 'PreBuild log'
#define p               m_LogPageIndex
#define print(a)	    lm->Log(a, p)
#define printLn		    lm->Log(_T(""), p)
#define printWarn(a)	lm->LogWarning(a, p)
#define printErr(a)	    lm->LogError(a, p)
///------------------------------------------------------------------------------
#include <filefilters.h>
#define  	EXT_H 		FileFilters::H_EXT
#define  	EXT_HPP 	FileFilters::HPP_EXT
#define  	EXT_CPP 	FileFilters::CPP_EXT
#define 	DOT_EXT_H 	FileFilters::H_DOT_EXT
#define 	DOT_EXT_CPP FileFilters::CPP_DOT_EXT
///-----------------------------------------------------------------------------
#endif // PRINT_H_INCLUDED
