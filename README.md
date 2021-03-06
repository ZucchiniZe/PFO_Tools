PFO_Tools
=========

Tools to help plan item creation, training, and achievement farming for Pathfinder Online

Goal/Plan
=========

I'll need to consume data from an xls spreadsheet.  I want to do this all in c/c++ but I can't find a
library to do this.  I spent two days trying to get libxls to work before I realized that it only
works on .xls files, not .xlsx files - which is the requirement.

Here I found a python script to convert the xlsx file into a directory of csv files:

http://xmodulo.com/how-to-convert-xlsx-files-to-xls-or-csv.html

$ git clone https://github.com/dilshod/xlsx2csv.git
$ cd xlsx2csv
$ ./xlsx2csv.py -a input.xlsx outputDir

The steps will be
   1. download the xlsx file from google drive (via a web client because I can't see how to get it via wget)
   2. store the xlsx file in my drop box
   3. copy the xlsx file to my working dir
   4. cd to my working dir
   5. run ./xlsx2csv.py -a PFOWiki_OfficialData.xlsx official_data
   6. copy the "Refining Recipes - Please Enter Qty Produced" as "Other file types"
   7. rename and copy into the official_data dir as RecipeYields_Crowdforged.csv
   8. run src/arch_test
   9. ...
  10. Profit


Status
======

The proof of concept tool, arch_test, opens the dir with the Official Data csv files (above) and goes through all the files.
If the file name has a handler defined, it will read the file and store the results in
an OfficialData object.

As of this checkin, we handle a lot of the data files but I think there may still be some bugs.  I'll
need to add a more detailed dumper of the entities to make sure of correctness.

The first solution planner (for items) is in and working - but not yet complete (see TODOs).

I wanted to demo this as a webservice but due to Cross-origin resource sharing (CORS) restrictions, I don't
think that will work so well.  So I will expand my use of the microhttpd to also serve some demo functionality.

Ultimately, this seems like a module for a webserver - but the only gotcha is that I don't want the webserver
to have to read, parse all the data files for every request - it should just keep the data in memory.

For the demo web page, I've imported a download of jquery-ui.  I wasn't sure if I should check it in - but
I'm doing it now because 1) I don't think it breaks any license agreements, and 2) it makes this thing
more self-contained.

TODO
====

- [ ] Add achievement flag to Provides list for crafting recipes
- [ ] add "Provides" nodes to OfficialData digesters
- [ ] figure out how to and add a proper "make test" target
- [ ] make the Bank output more readable
- [ ] make the skipping statements more readable
- [ ] Add verbosity flag to trim down the output
- [ ] Add the Provides nodes to the bank
- [ ] Begin to add http framework

- [x] Clarify the cost output by not showing quantities for things that don't combine - like Feats or "Items"
- [x] Make the OfficialData a singleton and
  - [x] remove the OfficialData arg from the planners
- [x] Work on Supply implementation
- [x] Work in TrackedResources implementation
- [x] Flesh out the Cost classes to enable rich/informative display of the cost of meeting a goal.
- [x] Add handler and howto for consuming the quantiites resulting from crafting - IE, one "craft" of Hemp Twine makes about 20 items
- [x] Skill Processor
- [x] Add detailed Entity dumper to verify correct parsing
- [x] Make the make file build in an obj dir and store the resulting binary in a bin dir
- [x] Fix command line parsing to allow showing plan for "Fighter 8", etc
- [x] undo the GetEntity dummy flag
- [x] rename skills to feats - maybe remove skill from the list of types - per Cheney post in EntityTypeHelper.cc
- [x] Add clear info to cost output when skipping OR nodes

BUGS
====

- [x] the planner bank deposit algo is bugged
   - src/arch_test -p "Skill.Fighter Level 4" # see the multiple costs for "1 of Hit Points", etc.
   - FIXED: I was erroneously ending the planner recursion when I hit an or gate - I needed only to 
     skip the gate, not return from the function

Nice to have:
I want a proper logging framework - like maybe http://logging.apache.org/log4cxx/index.html - but
for now I'll just write stuff to stdout.

Notes on libmicrohttpd
======================

Quick search led me here:
http://www.gnu.org/software/libmicrohttpd/

I picked up the latest version from a mirror:
http://mirrors.syringanetworks.net/gnu/libmicrohttpd/libmicrohttpd-0.9.38.tar.gz

It configured and compiled just fine.  I was hesitant to "sudo make install" - I want folks to be able to use
this tool w/o a bunch of fiddly bits.

The only version of this available via the default package sources was version 5 (sudo apt-get install
libmicrohttpd5) *and* I already had it installed apparently (the apt-get command did nothing) but after
doing that I still couldn't find the header file (ie, locate microhttd returned nothing).

apt-get also had a libmicrohttpd-dev so I tried that (apt-get install libmicrohttpd-dev) and while that let
me find the header file (microhttpd.h) the compiled failed w/ a bunch of errors like:

   /usr/include/microhttpd.h:497:3: error: ‘intptr_t’ does not name a type
   /usr/include/microhttpd.h:830:5: error: ‘uint64_t’ has not been declared
   /usr/include/microhttpd.h:868:46: error: ‘uint64_t’ has not been declared
   /usr/include/microhttpd.h:893:55: error: ‘va_list’ has not been declared

So I uninstalled that (apt-get remove libmicrohttpd-dev) and committed myself to just running sudo make install
on the source I just downloaded.  Wich me luck.

Yay!  It almost compiled.  Now I just need to add the lib to the makefile.


Notes on libxls
===============

The first thing I did after creating this github repo was look for a lib to parse xls.

    cd ~/Downloads
    wget http://downloads.sourceforge.net/project/libxls/libxls-1.4.0.zip
    unzip libxls-1.4.0.zip
    cd libxls
    cat INSTALL
    ./configure
    make
    sudo make install
    
    Libraries have been installed in: /usr/local/libxls/lib

    If you ever happen to want to link against installed libraries
    in a given directory, LIBDIR, you must either use libtool, and
    specify the full pathname of the library, or use the `-LLIBDIR'
    flag during linking and do at least one of the following:
      - add LIBDIR to the `LD_LIBRARY_PATH' environment variable during execution
      - add LIBDIR to the `LD_RUN_PATH' environment variable during linking
      - use the `-Wl,--rpath -Wl,LIBDIR' linker flag
      - have your system administrator add LIBDIR to `/etc/ld.so.conf'

Beginner learnings - it took me a day to figure out that the lib needs to be listed last on the compile line
and that I needed to wrap the include statements in extern "C"{ ... }.  This page states that the reason is:

http://www.cplusplus.com/forum/general/13700/#msg66132

You need to do this because if the C++ compiler doesn't know it's looking at C declarations it'll try to mangle the function names, but the C function definitions won't be mangled, so when the time comes to link everything, the linker won't find the definitions. extern "C" tells the compiler that the declarations inside shouldn't be mangled.

```
dbl@fish... cat PFO_Tools.cc
#include <stdio.h>

extern "C"{
#include <libxls/xls.h>
}

int main() {
  xlsWorkBook* pWB;
  pWB = xls_open("file.xls", "iso-8859-15//TRANSLIT");
  return 0;
}
Sat Oct 04 09:39:48
dbl@fish... g++ -Wno-write-strings -L/usr/local/libxls/lib -I/usr/local/libxls/include PFO_Tools.cc -lxlsreader -o read_spreadsheet_test
```


http://sourceforge.net/projects/libxls/files/latest/download?source=files

http://sourceforge.net/projects/libxls/files/libxls-1.4.0.zip/download


Here are some notes on GitHub
=============================

   * signed up for an account
   * created a new repository named PFO_Tools and let it do the default thing of seeding the repo w/ README.md
   * sudo apt-get install git
   * [did many confusing, wrong things before I figured it out]
   * git config --global user.name "MCMLXXXIV"
   * git config --global user.email "stupid@yahoo.com"
   * git config --global credential.helper cache
   * git config --global credential.helper 'cache --timeout=3600'
   * cd
   * git clone https://github.com/MCMLXXXIV/PFO_Tools

Then modify and:
   1. git commit -m 'learning the ropes'
   2. git push

