#! /usr/bin/env sh

# Usage
#   mingw64-deploy.sh install_folder distname
#
# input:
#   install_folder : the folder with the project install files.
#                    This is typically [build_folder]/OpenTRIM
#   distname : e.g., opentrim-0.1.6

# the script does the following
#   1. Copy exe and dll to a folder named "distname"
#   2. find all /ucrt64 dlls that exe and dll depend on
#   3. copy these to distname folder
#   4. create a zip archive named distname

INSTALLPATH=$1
DISTNAME=$2
CURFLDR=$PWD

EXE_NAME="opentrim.exe"
LIB_NAME="libopentrim.dll"
GUI_NAME="opentrim-gui.exe"

mkdir $DISTNAME
cp $INSTALLPATH/bin/*.* $DISTNAME
cp $INSTALLPATH/lib/*.dll $DISTNAME
cp $HOME/.local/bin/libdedx.dll $DISTNAME

cd $DISTNAME 

# create a list with all required dlls
printf "ldd "$EXE_NAME"\n"
list=$(ldd "./"$EXE_NAME | sed 's/[^\/]*\(\/[^ ]*\)/\1\n/' | grep ucrt64)
for dll in $list;
do
  dll_lst="$dll_lst $dll"
done

printf "ldd "$GUI_NAME"\n"
list=$(ldd "./"$GUI_NAME | sed 's/[^\/]*\(\/[^ ]*\)/\1\n/' | grep ucrt64)
for dll in $list;
do
  dll_lst="$dll_lst $dll"
done

printf "ldd ./"$LIB_NAME"\n"
list=$(ldd "./"$LIB_NAME | sed 's/[^\/]*\(\/[^ ]*\)/\1\n/' | grep ucrt64)
for dll in $list;
do
  dll_lst="$dll_lst $dll"
done

# remove duplicates
dll_lst=`echo $dll_lst | tr ' ' '\n' | sort | uniq`

for dll in $dll_lst;
do
  cp $dll .
done

windeployqt .

cd $CURFLDR

tar -a -cf $DISTNAME.zip $DISTNAME

