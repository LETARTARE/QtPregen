/*************************************************************
 * Name:      qtPre.h
 * Purpose:   Code::Blocks plugin	'qtPregenForCB.cbp'  0.2.2
 * Author:    LETARTARE
 * Created:   2015-02-10
 * Copyright: LETARTARE
 * License:   GPL
 *************************************************************
 */
#define VERSION_QTP _T("0.2.2")

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
//-----------------------------------------------------------------------------
/**	@brief The class that is used to detect Qt libraries
 *
 */
class qtPre
{
	public:

		/** Constructor
         * @param prj The active project.
         */
		qtPre(cbProject *);
		/** Destructor */
		virtual ~qtPre();

		/** Plugin name
		 * @return name
		 */
		 wxString namePlugin();

		/** Detects if the current project uses Qt libraries,
		 * refuses QT projects using a 'makefile'
         * @param prj : the active project.
         * @return true : if used
         */
		bool detectQt(cbProject * prj);

	protected:
		/** name of plugin
		 */
		const wxString Thename;
		/** platforms Windows
		 */
		bool Win,
		/** platforms Linux
		 */
			 Linux,
		/** platforms Mac
		 */
			 Mac;
		/** the project
		 */
		cbProject * Project	;
		/**  project directory
		 */
		wxString  DirProject,
		/**  generation directory
		 */
				  Dirgen;
		/**  project name,
		 */
		wxString Nameproject,
		/**  active target name
		 */
				Nameactivetarget;
		/**  numbers files projects
		 */
		uint16_t Nfilescreated;
		/** executable name files Qt : 'moc'
		 */
		wxString Mexe,
		/** executable name files Qt : 'uic'
		 */
				Uexe,
		/** executable name files Qt : 'rcc'
		 */
				Rexe,
		/** executable name files Qt : 'lrelease'
		 */
				Lexe;
		/** files prefix for 'moc'
		 */
		wxString Moc,
		/** files prefix for 'uic'
		 */
				 Ui,
		/** files prefix for 'rcc'
		 */
				 Qrc,
		/** files extension for 'lrelease'
		 */
				 Lm;
		/**  managers
		 */
		MacrosManager  * Mam;

		/** table contains libray name Qt
		 */
		wxArrayString  TablibQt ;
		/** table contains report build
		 */
		wxArrayString Fileswithstrings ;
	/// the tables
		wxArrayString
			/**	 files registered in the project
			 */
			Registered,
			/** files to be created in the project
			 */
			Filestocreate,
			/** files created list
			 */
			Createdfile,
			/** creators files
			 */
			Filecreator,
			/** files index to project
			 */
			Indexproject,
			/** file was already created
			 */
			Filewascreated
					;
		/**  messages to console
		 */
		wxString Mes;

		/**  calculate duration  mS
		 */
		clock_t Start;

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
		void setVersion(const wxString& ver);
};

#endif // _QTPRE_H_
