# QtPregenForCB
Plugin to compile QT applications with 'Code::Blocks'

February 12, 2015 -> the patch is in the svn 10115

1- Overview

    It is a plugin to compile C++ Qt4 and Qt5 applications using 'Code::Blocks',
    in a very similar way as projects 'wxWidgets'.
    For now it only works on 'Win32'.

2- Features

    It does not use the 'Makefile'

    This generates additional files the project by the Qt works
	'moc (.exe)', 'uic (.exe)', 'rcc (.exe)'  in a single directory in the
	active target :
        'qtprebuild/targetname'

    1- it detects meta-objects 'Q_OBJECT' and 'QGADGETS' in files
        '*.h, *.hpp, *.cpp' and generates files 'moc_xxxxxx.cpp'

    2- it detected in file '*.cpp' files included by
         '#include "moc_xxxx.cpp' or '#include" yyyyyy.moc'
      to mark those files included 'no compile' and 'no link'

    3- detects forms of files '*.ui' to generate files
         'ui_filename.h'

    4- detects resource files '*.qrc' to generate files
         'qrc_filename.cpp'

    - for now does not recognize in the '*.h' -> 'Q_DECLARE_INTERFACE (...)'


3- 'QtPregenForCB-x.y.z' contains :

	1- a plugin project 'QtPregenForCB.cbp' using the directory 'src'

	2- a directory 'patchs' containing three patches for versions of 'Code::Blocks'

	3- a directory 'CodeBlocks' containing wizards 'cbqt4' and cbqt5'


4- Installation to 'Code::Blocks svn 10091'

    1- first install wizards 'cbqt4' and 'cbqt5',

    2- then the patch 'compilergcc10091ForIdMenu.patch' and recompile the modified
       source of the version 'Code::Blocks svn 10091',

	3- then compile and install the plugin 'qtPregenForCB-xyz'
        - change the local variable 'CB = $(#cb10091)' in agreement with your
          global 'cb10091' which should identify sources of 'Code::Blocks'

5- Using

    1- create a new project by 'cbqt4' or 'cbqt5',

    2- compile and run (if executable).



