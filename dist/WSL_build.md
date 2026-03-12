# Build OpenTRIM on Windows using WSL

## Enable WSL
Open "Turn windows features on or off":
Enable Virtual Platform on Windows Features, enable Windows Subsystem for Linux

### To run WSL 2 on a virtual machine:
If installing Windows on Hyper-V, run a Powershell command line for the virtual machine (on the host machine, "Win10" is the name of the virtual machine)

 `set-VMProcessor -VMName "Win10" -ExposeVirtualizationExtensions $true`

### To install WSL2:
Run these commands on the windows command line: 

```wsl --install```

```wsl --update```

```wsl --install -d Ubuntu-24.04```

Update the kernel if needed by installing wsl_update_x64.msi

Set wsl 2 as default:

```wsl --set-default-version 2```

```wsl --set-version Ubuntu-24.04 2```

### Integration with VS code
Install the Windows application VS Code.
Connect to WSL inside of VS Code (lower corner-left icon) and select "Connect to WSL" on the top menu.
Install the vs code extension "cmake tools"



## Prepare build environment

### Install necessary packages

Run these commands on the WSL bash terminal:

```sudo apt install cmake```

```sudo apt install g++```

```sudo apt install -y libqt5svg5-dev```

```cd ~```

Follow the instructions for building OpenTRIM on Linux on this link:
https://github.com/ir2-lab/OpenTRIM/blob/main/dist/linux_build.md

Make the path to the packages persistent:

```export PATH="$HOME/.local/bin:$PATH"``
```echo 'export PATH="$HOME/.local/bin:$PATH"' >> ~/.bashrc```

## Packages for GUI build

### X-server setup for running WSL-opentrim-gui

Unless you have WSLg on Windows 11, you need to install the X server vcxsrv (it's a Windows program) -- It is necessary to have a graphic display on WSL, needed for opentrim-gui
Run the xlauncher and create configuration with the following parameters:

`"C:\Program Files\VcXsrv\vcxsrv.exe" :0 -ac -terminate -lesspointer -multiwindow -clipboard -wgl -listen tcp`

On the WSL bash terminal run these commands:

`export DISPLAY=localhost:0
echo 'export DISPLAY=$(cat /etc/resolv.conf | grep nameserver | awk "{print \$2}"):0.0' >> ~/.bashrc
source ~/.bashrc`

In order to check if the X server display is working try:

```sudo apt install x11-apps```

```xclock```

If there is an error running xclock try restarting the PC.