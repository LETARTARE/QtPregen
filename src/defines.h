/***************************************************************
 * Name:      defines.h
 * Purpose:   Code::Blocks plugin
 * Author:    LETARTARE
 * Created:   2015-10-17
 * Modified:  2019-11-09
 * Copyright: LETARTARE
 * License:   GPL
 **************************************************************/

///-----------------------------------------------------------------------------
#ifndef _DEFINES_H_
	#define _DEFINES_H_

///-----------------------------------------------------------------------------
/** Version
 */
#define VERSION_QP _T("2.5.0")  // not used !

/** @brief end of line for Win/Linux/OX
 */
#define 	Cr 		wxString(_T("\r"))
#define 	Lf 		wxString(_T("\n"))
#define 	CrLf 	wxString(_T("\r\n"))
#define 	Eol 	wxString(_T("\r\n"))
#define 	Quote 	wxString(_T("'"))
#define		Dquote  wxString(_T("\""))
#define 	Tab		wxString(_T("\t"))
#define 	Space	wxString(_T(" "))
#define 	Point	wxString(_T("."))
/** \brief text separator
 */
#define 	SepD 	char(13) 	// 0xD, \n
#define 	SepA 	char(10)	// 0xA, \r
#define 	SizeSep 2
#define		SizeLe	16
/** @brief surrounded by 'Quote'
 */
#define 	quote(a)	(Space + Quote + wxString(a) + Quote + Space)
#define 	quoteNS(a)	(Quote + wxString(a) + Quote)
/** \brief  for print an integer and a boolean
 */
#define strInt(a)	(wxString()<<a)
#define strBool(a)	(wxString()<<(a==0 ? _("false" ): _("true") ))

#include <wx/filefn.h>
/** @brief directory separator
 *  use _T("....")  +  wxString (Slash ) : not  _T("....")  +  Slash !!
 */
#define 	Slash 	wxFILE_SEP_PATH
///-----------------------------------------------------------------------------
#include <logmanager.h>
#define lm			Manager::Get()->GetLogManager()
/** @brief messages  -> 'Code::Blocks log'
 */
#define Print		lm->Log
#define PrintLn		lm->Log(_T(""))
#define PrintWarn	lm->LogWarning
#define PrintError	lm->LogError
/** @brief messages  -> 'Code::Blocks Debug log'
 */
#define DPrint		lm->DebugLog
#define DPrintErr	lm->DebugLogError

/** @brief messages  -> 'PreBuild log'
 */
#define p               m_LogPageIndex
#define print(a)	    lm->Log(a, p)
#define printLn		    lm->Log(_T(""), p)
#define printWarn(a)	lm->LogWarning(a, p)
#define printError(a)	lm->LogError(a, p)
///-----------------------------------------------------------------------------
/** @brief news extensions
 */
#define		EXT_UI			_T("ui")
#define		EXT_MOC			_T("moc")
#define		EXT_QRC			_T("qrc")
#define		DOT_EXT_UI		_T(".ui")
#define		DOT_EXT_MOC		_T(".moc")
#define		DOT_EXT_QRC		_T(".qrc")

#include <filefilters.h>
#define  	EXT_H 			FileFilters::H_EXT
#define  	EXT_HPP 		FileFilters::HPP_EXT
#define  	EXT_CPP 		FileFilters::CPP_EXT
#define 	DOT_EXT_H 		FileFilters::H_DOT_EXT
#define 	DOT_EXT_CPP 	FileFilters::CPP_DOT_EXT
///-----------------------------------------------------------------------------
/** @brief booleans
 */
#define IS_COMPLEMENT		true
#define IS_NO_COMPLEMENT	false

#define IS_CREATOR			true
#define IS_NO_CREATOR		false

#define WITH_REPORT			true
#define NO_REPORT			false

#define FIX_LOG_FONT		true
#define NO_FIX_LOG_FONT		false

#define SEPARATOR_AT_END	true
#define NO_SEPARATOR_AT_END	false

#define IS_RELATIVE			true
#define NO_RELATIVE			false

#define IS_UNIX_FILENAME	true
#define NO_UNIX_FILENAME	false

#define PREPEND_ERROR		true
#define NO_REPEND_ERROR		true

#define WITH_WARNING		true
#define NO_WARNING			false

#define ALLBUILD			true
#define NO_ALLBUILD			false

#define WITH_DEBUG			true
#define NO_DEBUG			false

#define FIRST				true
#define NO_FIRST			false
///-----------------------------------------------------------------------------

#endif // _DEFINES_H_
