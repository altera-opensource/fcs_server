# Build FCS Server

FCS Server source code can be found here: https://github.com/altera-opensource/fcs_server

FCS Server source code now supports build for ARM64 (since HPS is ARM based) and x86 for testing purpose.

## Build 32 bit version

To build x86 version, g++ compiler is needed. Go to FCS Server folder and run:

```
make x86
```

## Build 64 bit version

To build ARM64 executable on the Linux x86, AArch64 toolchain is needed. E.g. on Ubuntu 18, install:

```
apt-get install g++-aarch64-linux-gnu
```

Go to FCS Server folder and run:

```
make aarch64
```

## Build for All architectures

To build for both architectures, run:

```
make all
```

Output files:

1. fcsServer.**x86**
1. fcsServer.**aarch64**
1. fcsServer _where fcsServer is a copy of fcsServer.aarch64_

### Running FCS Server on HPS (Yocto linux)

After building FCS Server, copy following files to HPS running Yocto OS to one folder:

- **fcsServer** - executable
- **fcsServer.service** - service unit file used by systemd
- **install.sh** - install script

To change default log level (Info) or default port (50001), edit fcsServer.service file:

```
ExecStart=/usr/sbin/fcsServer [PORT_NUMBER] [LOG_LEVEL]
```

Possible log levels: Debug, Info, Error, Fatal

To install FCS Server, run install.sh within the folder script is located, with root privileges. FCS Server will
automatically start and will persist after system reboot.

SYSTEMCTL commands:

- Checking FCS Server status
  ```
  systemctl status fcsServer
  ```
- Start/stop FCS Server
  ```
  systemctl start fcsServer
  systemctl stop fcsServer
  ```

- Restart FCS Server
  ```
  systemctl restart fcsServer
  ```
- Changing FCS Server settings after installation: edit service unit file `/etc/systemd/system/fcsServer.service`,
  reload systemd manager configuration
  ```
  systemctl daemon-reload
  ```
  and restart service
    ```
    systemctl restart fcsServer
    ```

### Logs

To view FCS Server logs, run journalctl:

```
journalctl -u fcsServer
```
