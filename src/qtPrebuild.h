/***************************************************************
 * Name:      qtprebuild.h
 * Purpose:   Code::Blocks plugin
 * Author:    LETARTARE
 * Created:   2015-10-17
 * Modified:  2018-20-15
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
         * @param _prj The active project.
         * @param l_ogindex The active log
         * @param _nameplugin The plugin name
         */
		qtPrebuild(cbProject * _prj, int _logindex, wxString & _nameplugin);

		/** \brief Destructor
         */
		virtual ~qtPrebuild();

		/** \brief  Give if target is a Qt target
		 *	 @param _target : a project target
		 *   @return true if it's
		 */
		bool isGoodTargetQt(const ProjectBuildTarget * _target);

		/** \brief Built all files necessary complements
		 * @param _prj The active project.
		 * @param _workspace is true -> no report, no popup messages.
		 * @param _allbuild is true -> rebuild complement files.isGoodTargetQt
		 * @return	true if building is correct
		 */
		bool buildAllFiles(cbProject * _prj, bool _workspace, bool _allbuild);
		/** \brief Built one file elegible necessary complement
		 * @param _prj The active project.
		 * @param _fcreator  creator file
		 * @return	true if building is correct
		 */
		bool buildOneFile(cbProject * _prj, const wxString& _fcreator);


		/** \brief Unregister a project file creator to 'qtPregen'
		 *  @param f_ile : file name  (creator)
		 *  @param _first : the first call to 'unregisterProjectFile(...)'
		 *  @return	true if correct
		 */
		bool unregisterFileCreator(const wxString & _file, bool _first);
		/** \brief Unregister a project file complement to 'qtPregen'
		 *  @param _file : file name  (complement)
		 *  @param _first : the first call to 'unregisterProjectFile(...)'
		 *  @return	true if correct
		 */
		bool unregisterFileComplement(const wxString & _file, bool _first);
		/**	\brief Remove one complement file to disk directory "adding\"
		 *  @param  _filename : complement file name
		 *  @param  _first :  it' first complement to remove
		 * @return	true if correct
         */
		bool removeComplementToDisk(const wxString & _filename, bool _first);
		/** \brief Unregister a project file  to 'CB'
		 *  @param _complement : file name
		 *  @return	true if correct
		 */
		//bool unregisterComplementToCB(const wxString & _file);

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
		 *	 @param _file : file name
		 *   @return true if it's
		 */
		bool isElegible(const wxString& _file);

		/** \brief Record one file elegible necessary complement
		 * @param _fcreator  creator file
		 * @param _fout  complement file
		 * @return true if building is correct
		 */
		bool addOneFile(const wxString& _fcreator, const wxString& _fout) ;

		/**	\brief Records in an internal table to build additional files
		 * @return	the number of recorded files
         */
		uint16_t addAllFiles();

		/**	\brief Searches for files to be created
		 * @param _allrebuild  : true -> rebuild everything, false -> reconstruct
		 *						only files whose creators (eligible) have changed
		 * @return	the number of files to be created
         */
		uint16_t filesTocreate(bool _allrebuild) ;

        /** \brief Creating complements files
         * @return	true if creating is correct
         */
		bool createFiles() ;

		/**  \brief Checking complements files
         * @return	true if checking is correct
         */
		bool validCreated() ;

		/**  \brief Reference string of libQt by target: "" or "4r" or "4d" or "5r" or "5d"
		 *	 @param _buildtarget : 'ProjectBuildTarget*'
		 *   @return reference string
		 */
		wxString refTargetQt(const ProjectBuildTarget * _buildtarget) ;

		/**  \brief Find container path
		 *	 @param _container : 'cbProject*' or 'ProjectBuildTarget*'
		 * 	 	both inherited of  'CompileTargetBase'
		 *   @return path
		 */
		wxString pathlibQt(CompileTargetBase * _container) ;

		/**  \brief Is whether the QT executables exist
		 *	 @param _buildtarget : 'CompileTargetBase*' of used target
		 *   @return true if they exist all
		 */
		bool findTargetQtexe(CompileTargetBase * _buildtarget) ;

		/**  \brief Read contents file
		 *	 @param _filename : file name
		 *   @return contents file
		*/
		wxString ReadFileContents(const wxString& _filename) ;

		/**  \brief Write a file
		 *	 @param _filename : file name
		 *	 @param _contents : str contents
		 *   @return true if it's correct
		*/
		bool WriteFileContents(const wxString& _filename, const wxString& _contents);

		/** \brief Search a Qt macro inside file
		 *	 @param _filename : file name
		 *	 @param _qt_macro : the macro (Q_OBJECT, Q_GADGET ...)
		 */
		int q_object(const wxString& _filename, const wxString& _qt_macro) ;

		/** \brief Search a text inside a string
		 *	 @param _tmp : string
		 *	 @param _qt_text : the text
		 *	 @return  occurency number
		 */
		int  qt_find (wxString _tmp, const wxString& _qt_text) ;

		/** \brief Indicates whether the file is already registered in the CB project
		 *  @param _file : file name
		 *  @return if it's
		 */
		bool inProjectFileCB(const wxString& _file) ;

		/** \brief Indicates whether the file is already included in another
		 *	 @param _fcreator : file name
		 *   @return true if it's
		 */
		bool hasIncluded(const wxString& _fcreator) ;
		bool verifyIncluded(wxString& _source, wxString& _txt) ;

		/** \brief Create a directory
		 *	@param _dircomplement : dir name
		 *  @return true if it's correct
		 */
		bool createDir (const wxString& _dircomplement) ;

		/** \brief Remove a directory
		 *	@param _dirgen : directory name
		 *  @return true if it's correct
		 */
		bool removeDir (const wxString&  _dirgen ) ;

		/**  \brief Compare both date files
		 *	@param  _fileref : file name reference
		 * 	@param  _filetarget : file name target
		 *  @return true if identical dates
		 */
		bool compareDate(const wxString& _fileref, const wxString& _filetarget) ;

		/** \brief  Modify date file target
		 *	@param  _fileref : file name reference
		 * 	@param  _filetarget : file name target
		 *  @return true if changing it's correct
		 */
		bool modifyDate(const wxString& _fileref, const wxString& _filetarget)  ;

		/** \brief Indicates whether a table is empty
		 *  @param _arraystr : a table
		 *  @return if it's
		 */
		bool noFile(const wxArrayString& _arraystr) ;

		/** \brief Search the number files to create
		 *  @param _arraystr : a table
		 *  @return the number
		 */
		uint16_t nfilesToCreate(const wxArrayString& _arraystr) ;

        /** \brief Execute commands 'moc', 'uic', 'rcc'
         *	 @param _qexe  : executable name
         *	 @param _index : position creator file inside 'm_Filecreator'
         *   @return  _T("") if file created else error string
         */
		wxString createComplement(const wxString& _qexe, const uint16_t _index) ;

		/** \brief Execute commands 'moc', 'uic', 'rcc'
         *	 @param _qexe : executable name
         *	 @param _fcreator : file name creator
         *	 @param _fout : file name to create
         *   @return _T("") if file created else error string
         */
		wxString createFileComplement(const wxString& _qexe, const wxString& _fcreator,
									  const wxString& _fout) ;

		/** \brief Retrieve path include table for executable
		 * @param _container : receptacle
		 * @return string path include
		 */
		wxString pathIncludeMoc(CompileTargetBase * _container);

		/** \brief Retrieve all defines table for executables
		 * @param _container : receptacle
		 * @return string all defines
		 */
		wxString definesMoc(CompileTargetBase * _container) ;

        /**  \brief Launch a command line
         *	 @param _command : command line
         *	 @param _prepend_error : prepend error to return
         *   @return  empty string if it's correct else error message
         */
		wxString executeAndGetOutputAndError(const wxString& _command, bool _prepend_error = true) ;

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
