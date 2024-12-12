#! /usr/bin/env sh

# Usage
#   mingw64-deploy.sh program_name install_folder distname
#
# input:
#   program_name  : the exe program name, e.g., ions
#   install_folder : the folder with the project install files.
#                    This is typically [build_folder]/iradinapp
#   distname : e.g., iradinapp-0.1.6

# the script does the following
#   1. Copy exe and dll to a folder named "distname"
#   1. find all /ucrt64 dlls that exe and dll depend on
#   2. copy these to distname folder
#   3. create a zip archive named distname 

PROGRAM_NAME=$1
INSTALLPATH=$2
DISTNAME=$3
CURFLDR=$PWD

EXE_NAME=$PROGRAM_NAME".exe"
LIB_NAME="lib"$PROGRAM_NAME".dll"
GUI_NAME=$PROGRAM_NAME"-gui.exe"

mkdir $DISTNAME
cp $INSTALLPATH/bin/*.* $DISTNAME
cp $INSTALLPATH/lib/*.dll $DISTNAME

cd $DISTNAME 

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

printf "ldd ./libdedx.dll\n"
list=$(ldd ./libdedx.dll | sed 's/[^\/]*\(\/[^ ]*\)/\1\n/' | grep ucrt64)
for dll in $list;
do
  dll_lst="$dll_lst $dll"
done

printf "ldd ./libxs_zbl.dll\n"
list=$(ldd ./libxs_zbl.dll | sed 's/[^\/]*\(\/[^ ]*\)/\1\n/' | grep ucrt64)
for dll in $list;
do
  dll_lst="$dll_lst $dll"
done

printf "ldd ./libxs_lj.dll\n"
list=$(ldd ./libxs_lj.dll | sed 's/[^\/]*\(\/[^ ]*\)/\1\n/' | grep ucrt64)
for dll in $list;
do
  dll_lst="$dll_lst $dll"
done

printf "ldd ./libxs_krc.dll\n"
list=$(ldd ./libxs_krc.dll | sed 's/[^\/]*\(\/[^ ]*\)/\1\n/' | grep ucrt64)
for dll in $list;
do
  dll_lst="$dll_lst $dll"
done

printf "ldd ./libxs_moliere.dll\n"
list=$(ldd ./libxs_moliere.dll | sed 's/[^\/]*\(\/[^ ]*\)/\1\n/' | grep ucrt64)
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

