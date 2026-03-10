# Enable WSL
Open "Turn windows features on or off":
Enable Virtual Platform on Windows Features, enable Windows Subsystem for Linux

# To run WSL 2 on a virtual machine:
If installing Windows on Hyper-V, run a Powershell command line for the virtual machine (on the host machine, "Win10" is the name of the virtual machine)

 `set-VMProcessor -VMName "Win10" -ExposeVirtualizationExtensions $true`

Qtcreator cannot run on WSL 1.
To install WSL2:
On the windows command line: 
`wsl --install`
`wsl --update`
`wsl --install -d Ubuntu-24.04`
Update the kernel if needed by installing wsl_update_x64.msi
Set wsl 2 as default:
`wsl --set-default-version 2`
`wsl --set-version Ubuntu-24.04 2`

Install VS Code.
Connect to WSL inside of VS Code (lower corner-left icon) and select "Connect to WSL" on the top menu.
Install the vs code extension "cmake tools"

Follow the instructions on this link:
https://software.opensuse.org//download.html?project=home%3Amaxiotis%3Agapost&package=opentrim

Run the following commands to install the needed packages:
`sudo apt update`
`sudo apt install libeigen3-dev libhdf5-dev libhdf5-103-1`

Packages for the GUI component add the following:
`sudo apt install qtbase5-dev libqt5svg5 libqwt-qt5-dev libqwt-qt5-6`

`sudo apt install qtcreator`
`sudo apt install cmake`
`sudo apt install g++`
`sudo apt install -y libqt5svg5-dev`

Install the X server vcxsrv -- It is necessary to have a graphic display on WSL
Run the xlauncher and create configuration with the following parameters
`"C:\Program Files\VcXsrv\vcxsrv.exe" :0 -ac -terminate -lesspointer -multiwindow -clipboard -wgl -listen tcp`

On the WSL bash terminal run these commands:

`export DISPLAY=localhost:0
echo 'export DISPLAY=$(cat /etc/resolv.conf | grep nameserver | awk "{print \$2}"):0.0' >> ~/.bashrc
source ~/.bashrc`

In order to check if the X server display is working try:
`sudo apt install x11-apps
xclock`

If there is an error running xclock try restarting the PC

`cd ~
git clone https://github.com/gapost/qmatplotwidget QMatPlotWidget
cd QMatPlotWidget
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$HOME/.local
cmake --build . -j"$(nproc)"
cmake --install .
cd ..`

`cd ~
git clone https://github.com/ir2-lab/libdedx
cmake -S libdedx -B libdedx/build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$HOME/.local
cmake --build libdedx/build -j"$(nproc)"
cmake --install libdedx/build
cd ..`

`git clone https://github.com/ir2-lab/QtDataBrowser QtDataBrowser
cd QtDataBrowser
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build ./build
cmake --build ./build --target install
cd ..`

`cd ~
git clone https://github.com/ir2-lab/OpenTRIM.git
cd OpenTRIM
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build ./build
cmake --build ./build --target install
cd ..`

Make the path to the packages persistent:
`export PATH="$HOME/.local/bin:$PATH"
echo 'export PATH="$HOME/.local/bin:$PATH"' >> ~/.bashrc`