# QtPregen-2.7.0 November 09, 2019 (Win-32, Linux-64)

Plugin to compile QT applications with 'Code::Blocks'

**Versions for all 'Code::Block, sdk >= 1.25.0, svn >= 10127'**

**Last: sdk = 1.46.0, svn = 11883 -> ...**

It uses a log 'Prebuild log' for all posts plugin.

1- Overview

    It is a plugin to compile C++ Qt5 applications using 'Code::Blocks', in a 
	very similar way as projects 'wxWidgets'.
    For now it works on 'Win-32 and Linux-64'.

2- Features

    It does not use the 'Makefile'

    This generates additional files the project by the Qt works
	'moc (.exe)', 'uic (.exe)', 'rcc (.exe)'  in a single directory in the 
	active target :'adding/targetname'

    1- it detects meta-objects 'Q_OBJECT' and 'QGADGETS' in files
        '*.h, *.hpp, *.cpp' and generates files 'moc_xxxxxx.cpp'

    2- it detected in file '*.cpp' files included by
         '#include "moc_xxxx.cpp' or '#include" yyyyyy.moc'
      to mark those files included 'no compile' and 'no link'

    3- detects forms of files '*.ui' to generate files 'ui_filename.h'
      to mark those files included 'no compile' and 'no link'

    4- detects resource files '*.qrc' to generate files 'qrc_filename.cpp'

3- 'QtPregen' contains :

	1- a plugin project 'QtPregen.cbp' using the directory 'src'

	2- a directory 'patch' containing patchs for versions of 'Code::Blocks'(sdk_rev)
	(before_sdk130, sdk130_from10735_16.01, sdk131_from11135,  sdk133_from11227_17.12, 
	sdk136_from11350, sdk137_from11543, sdk138_from11656, sdk139_from11761, sdk140_from11768,
	sdk141_from11770, sdk142_from11792, sdk143_from11798), sdk144_from11845, sdk145_from11865,
	sdk146_from11883

	3- a directory 'CodeBlocks' containing wizards and files 'cbqt5'

4- Installation to 'Code::Blocks 1883' (sdk-1.46.0)
    1- first install wizards 'cbqt5',

    2- then the patch 'qtPregenx.x.x-sdk146.patch' and recompile the modified 
	source of the version 'Code::Blocks-11883',

	3- then with 'C::B-11883' compile and install the plugin 'qtPregen' 
        : change the local variable 'cb = $(#sdk146)' in agreement with your 
        global 'sdk146' which should identify sources of 'Code::Blocks-11883'

5- Using

    1- create a new project by 'cbqt5',

    2- compile and run (if executable).



