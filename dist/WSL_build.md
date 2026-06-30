# Build OpenTRIM on Windows using WSL

## Enable WSL
- Open "Turn windows features on or off":
- Enable Virtual Platform on Windows Features, enable Windows Subsystem for Linux

#### To run WSL 2 on a virtual machine:
If installing Windows on Hyper-V, run the following command on the host Powershell ("Win10" is the name of the virtual machine)

```
set-VMProcessor -VMName "Win10" -ExposeVirtualizationExtensions $true`
```

### To install WSL2:
Run these commands on the **windows command line**: 

```
wsl --install
wsl --update
wsl --install -d Ubuntu-24.04
```

Update the kernel if needed by installing `wsl_update_x64.msi`

Set wsl 2 as default:

```
wsl --set-default-version 2
wsl --set-version Ubuntu-24.04 2
```

### Integration with VS Code

- Install VS Code.
- Connect to WSL inside of VS Code (lower corner-left icon) and select "Connect to WSL" on the top menu.
- Install the VS Code extension "cmake tools"

## Build OpenTRIM

Run these commands on the **WSL bash terminal** to install dependencies:

```
sudo apt install cmake
sudo apt install g++
sudo apt install -y libqt5svg5-dev
```

Follow the instructions for [building OpenTRIM on Linux](./linux_build.md).

Make the path to the packages persistent:
```
echo 'export PATH="$HOME/.local/bin:$PATH"' >> ~/.bashrc
```

### X-server setup for running WSL's `opentrim-gui`

Unless you have WSLg on Windows 11, you need to install the X server `vcxsrv` (it's a Windows program). This is needed in order to display the opentrim-gui graphical window.

Run the `xlauncher` and create a configuration with the following parameters (enter the following in the text input box at the end of the wizard):

```
"C:\Program Files\VcXsrv\vcxsrv.exe" :0 -ac -terminate -lesspointer -multiwindow -clipboard -wgl -listen tcp
```

Run these commands from the **WSL bash terminal**:

```
export DISPLAY=localhost:0
echo 'export DISPLAY=$(cat /etc/resolv.conf | grep nameserver | awk "{print \$2}"):0.0' >> ~/.bashrc
source ~/.bashrc
```

In order to check if the X server display is working try:
```
sudo apt install x11-apps
xclock
```

If there is an error running xclock try restarting the PC.