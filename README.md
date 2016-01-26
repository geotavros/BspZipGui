BspZipGui
=========

BspZipGui is GUI tool for Valve's BSPZIP command line utility. It is used to easily **embed** a folder with all your custom files(textures, models, scripts etc) into a BSP file. With BspZipGui you can also **extract** all files contained inside BSP file. The main feature of BspZipGui is that it automatically generates a file list from the folder you specified and then runs bspzip.exe to embed these files into BSP. The application can be used for **any Source Engine game** including Nuclear Dawn, Left 4 Dead 2, Counter Strike Global Offensive etc. 

More info at 
https://sites.google.com/site/softdevdy/software/bspzipgui

To embed your files into BSP file you need to:
- Specify the path to bspzip.exe of your game. Usually <Path to your Game folder>/bin/bspzip.exe
- Put your BSP file into Game's "maps" folder. Because bspzip.exe can crash otherwise
- Specify path to BSP file
- Put your custom files in a folder. Don't forget about folder hierarchy(materials, models, sound subfolders)
- Specify the path to your custom files folder
- Press embed button. 

Building
--------

Get Qt5 SDK. Run Qt Creator. Open VpkCompare.pro and build.
