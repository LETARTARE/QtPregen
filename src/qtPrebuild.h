/***************************************************************
 * Name:      qtprebuild.h
 * Purpose:   Code::Blocks plugin
 * Author:    LETARTARE
 * Created:   2015-10-17
 * Modified:  2018-04-15
 * Copyright: LETARTARE
 * License:   GPL
 **************************************************************/
#ifndef _QTPREBUILD_H
#define _QTPREBUILD_H
//------------------------------------------------------------------------------
#include "qtpre.h"
//------------------------------------------------------------------------------
/**	\class qtPrebuild
 *	@brief The class is used to pre-Build additional Qt files
 *
 */
class qtPrebuild  : public qtPre
{
	public :

		/** \brief Constructor
         * @param prj The active project.
         * @param logindex The active log
         * @param nameplugin The plugin name
         */
		qtPrebuild(cbProject * prj, int logindex, wxString & nameplugin);

		/** \brief Destructor
         */
		virtual ~qtPrebuild();

		/** \brief Built all files necessary complements
		 * @param prj The active project.
		 * @param workspace is true -> no report, no popup messages.
		 * @param allbuild is true -> rebuild complement files.isGoodTargetQt
		 * @return	true if building is correct
		 */
		bool buildAllFiles(cbProject * prj, bool workspace, bool allbuild);

		/** \brief Built one file elegible necessary complement
		 * @param prj The active project.
		 * @param fcreator  creator file
		 * @return	true if building is correct
		 */
		bool buildOneFile(cbProject * prj, const wxString& fcreator);

		/** \brief  Give if target is a Qt target
		 *	 @param target : a project target
		 *   @return true if it's
		 */
		bool isGoodTargetQt(const ProjectBuildTarget * target);

		/**	\brief Remove one complement file to disk directory "qtprebuild\"
		 *  @param  filename : complement file name
		 *  @param  index :  table index
		 * @return	true if correct
         */
		bool removeComplementToDisk(const wxString & filename, int16_t index);

		/** \brief Unregister a project file creator to 'qtPregen'
		 *  @param file : file name  (creator)
		 *  @param first : the first call to 'unregisterProjectFile(...)'
		 *  @return	true if correct
		 */
		bool unregisterFileCreator(const wxString & file, bool first);
		/** \brief Unregister a project file complement to 'qtPregen'
		 *  @param file : file name  (complement)
		 *  @param creator : it'a creator file
		 *  @param first : the first call to 'unregisterProjectFile(...)'
		 *  @return	true if correct
		 */
		bool unregisterFileComplement(const wxString & file, bool creator, bool first);

		/** \brief Unregister a project file  to 'CB'
		 *  @param complement : file name
		 *  @return	true if correct
		 */
		bool unregisterComplementToCB(const wxString & file);

	private:

		/** \brief Startup banner pre-building
         */
		void beginMesBuildCreate();

		/** \brief Banner End pre-building
         */
		void endMesBuildCreate();

		/** \brief Startup banner pre-compiling file
         */
		void beginMesFileCreate();

		/** \brief Banner End pre-compiling file
         */
		void endMesFileCreate();

		/**	\brief Search the eligible files (which requires additional files)
		 * @return  the number of eligible files
         */
		uint16_t findGoodfiles();

		/** \brief Give if file  is elegible
		 *	 @param file : file name
		 *   @return true if it's
		 */
		bool isElegible(const wxString& file);

		/** \brief Record one file elegible necessary complement
		 * @param fcreator  creator file
		 * @param fout  complement file
		 * @return true if building is correct
		 */
		bool addOneFile(const wxString& fcreator, const wxString& fout) ;

		/**	\brief Records in an internal table to build additional files
		 * @return	the number of recorded files
         */
		uint16_t addAllFiles();

		/**	\brief Searches for files to be created
		 * @param allrebuild  : true -> rebuild everything, false -> reconstruct
		 *						only files whose creators (eligible) have changed
		 * @return	the number of files to be created
         */
		uint16_t filesTocreate(bool allrebuild) ;

        /** \brief Creating complements files
         * @return	true if creating is correct
         */
		bool createFiles() ;

		/**  \brief Checking complements files
         * @return	true if checking is correct
         */
		bool validCreated() ;

		/**  \brief Reference string of libQt by target: "" or "4r" or "4d" or "5r" or "5d"
		 *	 @param buildtarget : 'ProjectBuildTarget*'
		 *   @return reference string
		 */
		wxString refTargetQt(const ProjectBuildTarget * buildtarget) ;

		/**  \brief Find container path
		 *	 @param container : 'cbProject*' or 'ProjectBuildTarget*'
		 * 	 	both inherited of  'CompileTargetBase'
		 *   @return path
		 */
		wxString pathlibQt(CompileTargetBase * container) ;

		/**  \brief Is whether the QT executables exist
		 *	 @param buildtarget : 'CompileTargetBase*' of used target
		 *   @return true if they exist all
		 */
		bool findTargetQtexe(CompileTargetBase * buildtarget) ;

		/**  \brief Read contents file
		 *	 @param filename : file name
		 *   @return contents file
		*/
		wxString ReadFileContents(const wxString& filename) ;

		/**  \brief Write a file
		 *	 @param filename : file name
		 *	 @param contents : str contents
		 *   @return true if it's correct
		*/
		bool WriteFileContents(const wxString& filename, const wxString& contents);

		/** \brief Search a Qt macro inside file
		 *	 @param filename : file name
		 *	 @param qt_macro : the macro (Q_OBJECT, Q_GADGET ...)
		 */
		int q_object(const wxString& filename, const wxString& qt_macro) ;

		/** \brief Search a text inside a string
		 *	 @param tmp : string
		 *	 @param qt_text : the text
		 *	 @return  occurency number
		 */
		int  qt_find (wxString tmp, const wxString& qt_text) ;

		/** \brief Indicates whether the file is already registered in the CB project
		 *  @param file : file name
		 *  @return if it's
		 */
		bool inProjectFileCB(const wxString& file) ;

		/** \brief Indicates whether the file is already included in another
		 *	 @param fcreator : file name
		 *   @return true if it's
		 */
		bool hasIncluded(const wxString& fcreator) ;
		bool verifyIncluded(wxString& source, wxString& txt) ;

		/** \brief Create a directory
		 *	@param dircomplement : dir name
		 *  @return true if it's correct
		 */
		bool createDir (const wxString& dircomplement) ;

		/** \brief Remove a directory
		 *	@param dirgen : directory name
		 *  @return true if it's correct
		 */
		bool removeDir (const wxString&  dirgen ) ;

		/**  \brief Compare both date files
		 *	@param  fileref : file name reference
		 * 	@param  filetarget : file name target
		 *  @return true if identical dates
		 */
		bool compareDate(const wxString&  fileref, const wxString&  filetarget) ;

		/** \brief  Modify date file target
		 *	@param  fileref : file name reference
		 * 	@param  filetarget : file name target
		 *  @return true if changing it's correct
		 */
		bool modifyDate(const wxString& fileref, const wxString& filetarget)  ;

		/** \brief Indicates whether a table is empty
		 *  @param arraystr : a table
		 *  @return if it's
		 */
		bool noFile(const wxArrayString& arraystr) ;

		/** \brief Search the number files to create
		 *  @param arraystr : a table
		 *  @return the number
		 */
		uint16_t nfilesToCreate(const wxArrayString& arraystr) ;

        /** \brief Execute commands 'moc', 'uic', 'rcc'
         *	 @param qexe  : executable name
         *	 @param index : position creator file inside 'm_Filecreator'
         *   @return  _T("") if file created else error string
         */
		wxString createComplement(const wxString& qexe, const uint16_t index) ;

		/** \brief Execute commands 'moc', 'uic', 'rcc'
         *	 @param qexe : executable name
         *	 @param fcreator : file name creator
         *	 @param fout : file name to create
         *   @return _T("") if file created else error string
         */
		wxString createFileComplement(const wxString& qexe, const wxString& fcreator,
									  const wxString& fout) ;

		/** \brief Retrieve path include table for executable
		 * @return string path include
		 */
		wxString pathIncludeMoc(CompileTargetBase * container);

		/** \brief Retrieve all defines table for executables
		 * @return string all defines
		 */
		wxString definesMoc(CompileTargetBase * container) ;

        /**  \brief Launch a command line
         *	 @param command : command line
         *	 @param prepend_error : prepend error to return
         *   @return  empty string if it's correct else error message
         */
		wxString executeAndGetOutputAndError(const wxString& command, bool prepend_error = true) ;

	private:

		/** \brief Indicate if identical dates
		 */
		bool m_Identical = false;

		wxString
		/** \brief Contains all defines Qt for executables
		 */
			m_DefinesQt,
		/** Contains all path include Qt for executables
		 */
			m_IncPathQt,
		/** \brief Contains file name
		 */
			m_filename
			;
};

#endif // _QTPREBUILD_H
