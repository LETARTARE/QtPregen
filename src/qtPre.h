/*************************************************************
 * Name:      qtPre.h
 * Purpose:   Code::Blocks plugin	'qtPregenForCB.cbp'  0.9
 * Author:    LETARTARE
 * Created:   2015-10-17
 * Modified:  2017-07-26
 * Copyright: LETARTARE
 * License:   GPL
 *************************************************************
 */
#define VERSION_QTP _T("0.9.0")

#ifndef _QTPRE_H_
#define _QTPRE_H_
//------------------------------------------------------------------------------
#include <wx/arrstr.h>
#include <time.h>
//------------------------------------------------------------------------------
class cbProject;
class CompileTargetBase;
class ProjectBuildTarget;
class MacrosManager;
class ProjectFile;
class TextCtrlLogger;
//------------------------------------------------------------------------------

/**	\class qtPre
 *	@brief The class that is used to detect Qt libraries
 *
 */
class qtPre
{
	public:

		/** Constructor
         * @param prj The active project.
         * @param logindex The active log page.
         */
		qtPre(cbProject * prj, int logindex);
		/** Destructor */
		virtual ~qtPre();

		/** Plugin name
		 * @return name
		 */
		wxString namePlugin();

		/** Give complement directory
		 * @return directory name
		*/
		wxString complementDirectory() const;
		/**
		 * @return true : is a complement file
		 */
        bool isComplementFile(wxString & filename);
        /**
		 * @return true : is a creator file
		 */
        bool isCreatorFile(wxString & filename);

        /**
         * @return true : tables is empty
         */
        bool isClean();

		/** Detects if the current project uses Qt libraries,
		 * refuses QT projects using a 'makefile'
         * @param prj : the active project.
         * @param report : display report if true.
         * @return true : if used
         */
		bool detectQt(cbProject * prj, bool report= false);

		/** Detects if the current project has complements
         * @param prj : the active project.
         */
		bool detectComplements(cbProject * prj);

		/** Get version from 'cbplugin.h'
		 * @return version du SDK ex: "1.19.0" for Code::Blocks 13.12
		 */
		wxString GetVersionSDK();

		/**  Set page index to log
		 */
		void SetPageIndex(int pageindex);

	protected :

		/** Give an array from another
         *	@param strarray : array name
         *	@return a copy, not an adress
         */
		wxArrayString copyArray (const wxArrayString& strarray)  ;

		/** Give an array from another
         *	@param strarray : array name
         *	@return a reverse,
         */
		wxArrayString reverseFileCreator(const wxArrayString& strarray) ;

	protected:
		/** name of plugin
		 */
		const wxString m_Thename;
		/** platforms Windows
		 */
		bool m_Win,
		/** platforms Linux
		 */
			 m_Linux,
		/** platforms Mac
		 */
			 m_Mac;
		/** the project
		 */
		cbProject * m_project	;
		/**  project directory
		 */
		wxString  m_dirproject,
		/**  generation directory
		 */
				  m_dirgen;
		/**  project name,
		 */
		wxString m_nameproject;
		/**  numbers files projects
		 */
		uint16_t m_nfilescreated;
		/** executable name files Qt : 'moc'
		 */
		wxString m_Mexe,
		/** executable name files Qt : 'uic'
		 */
				m_Uexe,
		/** executable name files Qt : 'rcc'
		 */
				m_Rexe,
		/** executable name files Qt : 'lrelease'
		 */
				m_Lexe;
		/** files prefix for 'moc'
		 */
		wxString m_Moc,
		/** files prefix for 'uic'
		 */
				 m_UI,
		/** files prefix for 'rcc'
		 */
				 m_Qrc,
		/** files extension for 'lrelease'
		 */
				 m_Lm;

		/**  macros manager
		 */
		MacrosManager  * m_mam;

		/** table contains libray name Qt
		 */
		wxArrayString  m_TablibQt ;

		/** String with "__nullptr__"
		 */
		 const wxString m_Devoid = _T("__nullptr__") ;

	/// the tables
			/**	files registered in the project
			 */
		wxArrayString	m_Registered,
			/** files to be created in the project
			 */
			m_Filestocreate,
			/** files created list
			 */
			m_Createdfile,
			/** creators files
			 */
			m_Filecreator,
			/** file was already created
			 */
			m_Filewascreated
		;
		/**  the table is empty
		 */
		bool m_clean = true;

		/**  messages to console
		 */
		wxString Mes;

		/** log page index
		 */
		int	m_LogPageIndex;

		/**  calculate duration  mS
		 */
		clock_t m_start;

	private:

        /**  Search Qt libraries in project, targets
         * @param container: 'cbProject * Project' or 'ProjectBuildTarget * buildtarget' both inherited of  'CompileTargetBase'
		 * @return true if finded
         */
		bool hasLibQt(CompileTargetBase * container) ;

		/**  Adjusts variable depending on the platform
		 */
		void platForm() ;

		/** Set version to 'manifest.xml'
		 * @param ver version du SDK ex: "1.19.0" for Code::Blocks 13.12
		 */
		void SetVersionSDK(const wxString& ver);


};

#endif // _QTPRE_H_
