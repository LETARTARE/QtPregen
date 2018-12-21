/*************************************************************
 * Name:      qtpre.h
 * Purpose:   Code::Blocks plugin
 * Author:    LETARTARE
 * Created:   2015-10-17
 * Modified:  2018-20-117
 * Copyright: LETARTARE
 * License:   GPL
 *************************************************************
 */
#define VERSION_QP _T("2.3.1")  // not used !

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
class Manager;
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

		/** \brief Constructor
         * @param _prj The active project.
         * @param _logindex The active log page.
         * @param _nameplugin  The plugin name.
         */
		qtPre(cbProject * _prj, int l_ogindex, const wxString & _nameplugin);
		/** Destructor
		  */
		virtual ~qtPre();

		/** \brief Plugin name
		 * @return name
		 */
		wxString namePlugin();

		/** \brief Set 'm_pBuildTarget'
		 * @param _pbuildtarget :
		 */
		void setBuildTarget(ProjectBuildTarget * _pbuildtarget);

		/** \brief Give a full complement filename
		 * @param _filename : short name
		 * @return a full filename
		 */
		wxString fullFilename(const wxString & _filename);
		/** \brief Test if complement filename
		 * @param _filename : short name
		 * @return true : is a complement file
		 */
        bool isComplementFile(const wxString & _filename);
        /** \brief Test if creator filename
		 * @param _filename : short name
		 * @return true if is a creator file
		 */
        bool isCreatorFile(const wxString & _filename);
        /** \brief Generate the name of the complement file
		 *	 @param _file : file name creator
		 *   @return file name complement
		 */
		wxString nameCreated(const wxString& _file);

        /** \brief Give complement directory
		 * @return directory name
		*/
		wxString complementDirectory() const;

        /**
         * @return true if tables are empties
         */
        bool isClean();

        /** \brief Give if target is virtual
		 *	 @param _nametarget : target name
		 *	 @param _warning : indicate a message
		 *   @return true if it's virtual
		 */
		bool isVirtualTarget(const wxString& _nametarget, bool _warning=false) ;
		/**  \brief Give reals targets list for a virtual target
		 *	 @param _nametarget : virtual target name
		 *   @return a table : if not a virtual target -> target name
		 */
		wxArrayString listRealsTargets(const wxString& _nametarget);

		/** \brief Detects if the project has at least one target using Qt libraries
		 * refuses QT projects using a 'makefile'
         * @param _prj : the active project.
         * @param _report : display report if true.
         * @return true : if used
         */
		bool detectQtProject(cbProject * _prj, bool _report = false);
		/** \brief Detects if the current target uses Qt libraries,
         * @param _nametarget : the active target.
         * @param _prj : the active project.
         * @return true : if used
         */
		bool detectQtTarget(const wxString& _nametarget, cbProject * _prj);

		/** \brief Check if the target is independent
		 * @param _prj : the active project.
		 * @param _target : the active target.
		 * @return true if is an independant target
		 */
		bool isIndependentTarget(cbProject * _prj, const wxString & _target) ;

		/**  \brief Create a new directory, transfer all files to the new directory
		 * @param _newname : the new target name.
		 * @param _oldname : the old target name.
		 * @return true if is allright
		 */
		bool newNameComplementDirTarget(wxString & _newname, wxString & _oldname) ;

		/** \brief Detects if the project has complements on disk
         * @param _prj : the active project.
         * @param _nametarget : the active target.
         * @param _report : display report if true.
         * @return true : if complement on disk
         */
		bool detectComplementsOnDisk(cbProject * _prj, const wxString & _nametarget=wxString(), bool _report = true);

		/** \brief refresh all tables from 'm_Filewascreated'
		 * @param _debug : display report if true.
		 * @return true if ok
		 */
		bool refreshTables(bool _debug= false);

		/** \brief Get SDK version from 'cbplugin.h'
		 * @return version du SDK ex: "1.19.0" for Code::Blocks 13.12
		 */
		wxString GetVersionSDK();

		/**  \brief Set page index to log
		 * @param _pageindex : page index
		 */
		void SetPageIndex(int _pageindex);

		/** \brief Abort complement file creating
		 */
		void Abort() ;

		/** \brief Startup duration
		 * @param _namefunction : used function name
         */
		void beginDuration(const wxString & _namefunction = wxString());

		/** \brief End duration
		 * @param _namefunction : used function name
         */
		void endDuration(const wxString & _namefunction = wxString());

	protected :

		/** \brief Give an array from another
         *	@param _strarray : array name
         *	@param _nlcopy : number of lines to copy, if 0 -> all
         *	@return a copy, not an adress
         */
		wxArrayString copyArray (const wxArrayString& _strarray,  int _nlcopy = 0)  ;

		/** \brief give a string of table 'title' for debug
		 *	@param _title : titlr name
		 *	@param _strarray : array name
		 *  @return one string of array
		 */
		wxString allStrTable(const wxString& _title, const wxArrayString& _strarray);

		/** \brief Construct 'm_FilesCreator' from 'm_Filewascreated'
         *	@return a reverse,
         */
		wxArrayString wasCreatedToCreator() ;
		/** \brief Give an string from another
         *	@param _fcreated : file created name
         *	@return a reverse,
         */
		wxString toFileCreator(const wxString& _fcreated) ;
		/** \brief Give a full relative name
         *	@param _fcreator : file name
         *	@return relative name
         */
		wxString fullFileCreator(const wxString&  _fcreator) ;

		/**  \brief Give a date
		 *   @return date
		 */
		wxString date();

		/**  \brief Give a duration
		 *   @return duration mS
		 */
		wxString duration();

	protected:

		/** \brief name of plugin
		 */
		wxString m_namePlugin;
		/** \brief platforms Windows
		 */
		bool m_Win = false,
		/** \brief platforms Linux
		 */
			 m_Linux = false,
		/** \brief platforms Mac
		 */
			 m_Mac = false
			 ;
		/** \brief the active project
		 */
		cbProject * m_pProject = nullptr;
		/** \brief the active build target
		 */
		ProjectBuildTarget * m_pBuildTarget = nullptr;

		/**  \brief project directory absolute path
		 */
		wxString  m_dirProject = wxEmptyString,
		/**  \brief generation directory
		 */
				  m_dirPreBuild = wxEmptyString,
		/**  \brief project name,
		 */
                  m_nameProject = wxEmptyString,
		/** \brief Contains active target name
		 */
                  m_nameActiveTarget = wxEmptyString;
		/**  \brief numbers files projects
		 */
		uint16_t m_nfilesCreated = 0;
		/** \brief executable name files Qt : 'moc'
		 */
		wxString m_Mexe,
		/** \brief executable name files Qt : 'uic'
		 */
				m_Uexe,
		/** \brief executable name files Qt : 'rcc'
		 */
				m_Rexe,
		/** \brief executable name files Qt : 'lrelease'
		 */
				m_Lexe;
		/** \brief files prefix for 'moc'
		 */
		wxString m_Moc,
        /** \brief files prefix for 'uic'
		 */
				 m_UI,
		/** \brief files prefix for 'rcc'
		 */
				 m_Qrc,
		/** \brief files extension for 'lrelease'
		 */
				 m_Lm;

		/**  \brief manager
		 */
		Manager * m_pm = nullptr;
		/**  \brief macros manager
		 */
		MacrosManager  * m_pMam = nullptr;

		/** \brief table contains libray name Qt
		 */
		wxArrayString  m_TablibQt ;

		/** \brief String with "__nullptr__"
		 */
		const wxString m_Devoid = _T("__nullptr__") ;

		/// the tables
		wxArrayString
			/** \brief Creators files
			 */
			m_Filecreator,
			/** \brief Files to be created (complement) in the project
			 */
			m_Filestocreate,
			/** \brief Files created list
			 */
			m_Createdfile,
			/**	\brief Files registered in the project
			 */
			m_Registered,
			/** \brief File was already created
			 */
			m_Filewascreated
		;
		/**  \brief the table is empty
		 */
		bool m_clean = false;

		/**  \brief messages to console
		 */
		wxString Mes = wxEmptyString;

		/** \brief log page index
		 */
		int	m_LogPageIndex = 0;

		/**  \brief calculate duration  mS
		 */
		clock_t m_start;

		/**  \brief Abort complement file creating
		 */
		bool m_abort = false;

	private:

	    /**  \brief Search Qt libraries in targets of m_pProject
         * @return true if finded
		 */
		bool findLibQtToTargets();

        /**  \brief Search Qt libraries in project or target
         * @param _container: 'cbProject * Project' or 'ProjectBuildTarget * buildtarget'
         *		both inherited of 'CompileTargetBase'
		 * @return true if finded
         */
		bool hasLibQt(CompileTargetBase * _container) ;

		/**  \brief Adjusts variable depending on the platform
		 */
		void platForm() ;
};

#endif // _QTPRE_H_
