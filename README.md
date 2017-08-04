# QtPregenForCB-0.9.0 August 4, 2017 (for Win32)

Plugin to compile QT applications with 'Code::Blocks'

**Version for all 'Code::Block, sdk >= 1.25.0, svn >= 10127'**

It uses a log 'Prebuild log' for all posts plugin.

1- Overview

    It is a plugin to compile C++ Qt4 and Qt5 applications using 'Code::Blocks', in a very similar way as projects 'wxWidgets'.
    For now it only works on 'Win32'.

2- Features

    It does not use the 'Makefile'

    This generates additional files the project by the Qt works
	'moc (.exe)', 'uic (.exe)', 'rcc (.exe)'  in a single directory in the active target :'qtprebuild/targetname'

    1- it detects meta-objects 'Q_OBJECT' and 'QGADGETS' in files
        '*.h, *.hpp, *.cpp' and generates files 'moc_xxxxxx.cpp'

    2- it detected in file '*.cpp' files included by
         '#include "moc_xxxx.cpp' or '#include" yyyyyy.moc'
      to mark those files included 'no compile' and 'no link'

    3- detects forms of files '*.ui' to generate files        'ui_filename.h'

    4- detects resource files '*.qrc' to generate files
         'qrc_filename.cpp'

3- 'QtPregenForCB-x.y.z' contains :

	1- a plugin project 'QtPregenForCB.cbp' using the directory 'src'

	2- a directory 'patch' containing patchs for versions of 'Code::Blocks' (10253, 10376, 10474, 10528, 10600, 10703, cb-16.01, 10816, 10856, 10922, 11021, 11112)

	3- a directory 'CodeBlocks' containing wizards 'cbqt4' and cbqt5'


4- Installation to 'Code::Blocks 11112'

    1- first install wizards 'cbqt4' and 'cbqt5',

    2- then the patch 'cb-11112-evPREGEN-0.9.0.patch' and recompile the modified source of the version 'Code::Blocks-11112',

	3- then with 'C::B-11112' compile and install the plugin 'qtPregenForCB-xyz' 
        - change the local variable 'CB = $(#cb11112)' in agreement with your global 'cb11112' which should identify sources of 'Code::Blocks'

5- Using

    1- create a new project by 'cbqt4' or 'cbqt5',

    2- compile and run (if executable).



