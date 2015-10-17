/***************************************************************
 * Name:      qtPrebuild.h
 * Purpose:   Code::Blocks plugin	'qtPregenForCB.cbp'  0.8.5
 * Author:    LETARTARE
 * Created:   2015-10-17
 * Copyright: LETARTARE
 * License:   GPL
 **************************************************************/
#ifndef _QTPREBUILD_H
#define _QTPREBUILD_H
//------------------------------------------------------------------------------
#include "qtpre.h"
//------------------------------------------------------------------------------
/**	@brief The class is used to pre-Build additional Qt files
 *
 */
class qtPrebuild  : public qtPre
{
	public :

		/** Constructor
         * @param prj The active project.
         * @param logindex The active log
         */
		qtPrebuild(cbProject * prj, int logindex);

		/** Destructor
         */
		virtual ~qtPrebuild();

		/** Built all files necessary complements
		 * @param prj The active project.
		 * @param workspace is true -> no report, no popup messages.
		 * @param allbuild is true -> rebuild complement files.
		 * @return	true if building is correct
		 */
		bool buildQt(cbProject * prj, bool workspace, bool allbuild);

		/** Built one file elegible necessary complement
		 * @param prj The active project.
		 * @param fcreator  creator file
		 * @return	true if building is correct
		 */
		bool buildFileQt(cbProject * prj, const wxString& fcreator);

		/**  Give if target is a Qt target
		 *	 @param nametarget : target name
		 *   @return true if it's
		 */
		bool isGoodTargetQt(const wxString& nametarget);

	private:

		/** Startup banner pre-building
         */
		void beginMesBuildCreate();

		/** Banner End pre-building
         */
		void endMesBuildCreate();

		/** Startup banner pre-compiling file
         */
		void beginMesFileCreate();

		/** Banner End pre-compiling file
         */
		void endMesFileCreate();

		/**	Search the eligible files (which requires additional files)
		 * @return  the number of eligible files
         */
		uint16_t findGoodfiles();

		/**  Give if file  is elegible
		 *	 @param file : file name
		 *   @return true if it's
		 */
		bool isElegible(const wxString& file);

		/**	Records in an internal table to build additional files
		 * @return	the number of recorded files
         */
		uint16_t addRecording();

		/**	Searches for files to be created
		 * @param allrebuild  : true -> rebuild everything, false -> reconstruct
		 *						only files whose creators (eligible) have changed
		 * @return	the number of files to be created
         */
		uint16_t filesTocreate(bool allrebuild) ;

        /**  Creating complements files
         * @return	true if creating is correct
         */
		bool createFiles() ;

		/**  Checking complements files
         * @return	true if checking is correct
         */
		bool validCreated() ;

		/**  Checking that the filename not contain illegal characters for windows !
		 * @param namefile : file name preoject
         * @return  _T("") it's correct else illegal character
         */
		wxString filenameOk(const wxString& namefile) ;

        /**  Find project path
		 *   @return path
		 */
		wxString findpathProjectQt() ;

		/**  Reference string of libQt by target: "" or "4r" or "4d" or "5r" or "5d"
		 *	 @param buildtarget : 'ProjectBuildTarget*'
		 *   @return reference string
		 */
		wxString refTargetQt(ProjectBuildTarget * buildtarget) ;

		/**  Find container path
		 *	 @param container : 'cbProject*' or 'ProjectBuildTarget*'
		 * 	 	both inherited of  'CompileTargetBase'
		 *   @return path
		 */
		wxString pathQt(CompileTargetBase * container) ;

		/**  Is whether the QT executables exist
		 *	 @param parentbuildtarget : 'cbProject*' of used target
		 *   @return true if they exist all
		 */
	//	bool findTargetQtexe(cbProject * parentbuildtarget) ;
		bool findTargetQtexe(CompileTargetBase * buildtarget) ;

		/**  Read contents file
		 *	 @param filename : file name
		 *   @return contents file
		*/
		wxString ReadFileContents(const wxString& filename) ;

		/**  Write a file
		 *	 @param filename : file name
		 *	 @param contents : str contents
		 *   @return true if it's correct
		*/
		bool WriteFileContents(const wxString& filename, const wxString& contents);

		/**  Give a date
		 *   @return date
		 */
		wxString date();

		/**  Give a duration
		 *   @return duration mS
		 */
		wxString duration();

		/**  Give if target is virtual
		 *	 @param nametarget : target name
		 *	 @param warning : indicate a message
		 *   @return true if it's virtual
		 */
		bool isVirtualTarget(const wxString& nametarget, bool warning=false) ;

		/**  Give targets list for a virtual target
		 *	 @param nametarget : virtual target name
		 *   @return a table
		 */
		wxArrayString listTargets(const wxString& nametarget);

		/** Search a macro inside file
		 *	 @param filename : file name
		 *	 @param qt_macro : the macro (Q_OBJECT, Q_GADGET ...)
		 */
		int q_object(const wxString& filename, const wxString& qt_macro) ;

		/** Search a text inside a string
		 *	 @param tmp : string
		 *	 @param qt_text : the text
		 */
		int  qt_find (wxString tmp, const wxString& qt_text) ;

		/**  Give the index of target
		 *   @param nametarget : target name
		 *   @return an index
		 */
		int indexTarget(const wxString& nametarget) ;

		/** Indicates whether the file is already registered in the project
		 *  @param file : file name
		 *  @return if it's
		 */
		bool inProjectFile(const wxString& file) ;
			//  called by  'indexTarget()'
			wxString nameCreated(const wxString& file);

		/** Indicates whether the file is already included
		 *	 @param file : file name
		 *   @return true if it's
		 */
		bool hasIncluded(const wxString& file) ;

		/** Create a directory
		 *	@param dirgen : dir name
		 *  @return true if it's correct
		 */
		bool createDir (const wxString& dirgen) ;

		/** Remove a directory
		 *	@param dirgen : directory name
		 *  @return true if it's correct
		 */
		bool removeDir (const wxString&  dirgen ) ;

		/**  Compare both date files
		 *	@param  fileref : file name reference
		 * 	@param  filetarget : file name target
		 *  @return true if identical dates
		 */
		bool CompareDate(const wxString&  fileref, const wxString&  filetarget) ;

		/**  Modify date file target
		 *	@param  fileref : file name reference
		 * 	@param  filetarget : file name target
		 *  @return true if changing it's correct
		 */
		bool ModifyDate(const wxString& fileref, const wxString& filetarget)  ;

		/** Indicates whether a table is empty
		 *  @param arraystr : a table
		 *  @return if it's
		 */
		bool isEmpty (const wxArrayString& arraystr) ;

        /** Execute commands 'moc', 'uic', 'rcc'
         *	 @param qexe  : executable name
         *	 @param index : position creator file inside 'm_Filecreator'
         *   @return  _("") if file created else error string
         */
		wxString createComplement(const wxString& qexe, const uint16_t index) ;

		/** Execute commands 'moc', 'uic', 'rcc'
         *	 @param qexe : executable name
         *	 @param fcreator : file name creator
         *	 @param fout : file name to create
         *   @return _("") if file created else error string
         */
		wxString createFileComplement(const wxString& qexe, const wxString& fcreator,
									  const wxString& fout) ;

		/** Retrieve path include table for executable
		 * @return string path include
		 */
		wxString pathIncludeMoc();

		/** Retrieve all defines table for executables
		 * @return string all defines
		 */
		wxString definesMoc() ;

        /**  Launch a command line
         *	 @param command : command line
         *	 @param prepend_error : prepend error to return
         *   @return  empty string if it's correct else error message
         */
		wxString ExecuteAndGetOutputAndError(const wxString& command, bool prepend_error = true) ;

	private:

		/** Indicate if identical dates
		 */
		bool m_Identical;

		/** String with "__DEVOID__"
		 */
		wxString m_Devoid,
		/** Contains all defines Qt for executables
		 */
			m_DefinesQt,
		/** Contains all path include Qt for executables
		 */
			m_IncPathQt,
		/** Contains active target name
		 */
			m_nameactivetarget,
		/** Contains file name
		 */
			m_filename
			;

};

#endif // _QTPREBUILD_H
