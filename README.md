
# Need For Speed: Underground Server

This server was initially developed by [3 PriedeZ](http://3priedez.net)


## Server Configuration

Edit `nfsu.conf` and `news.txt` to suit your needs.

After first start `server.log` should be created. After first user login account database `rusers.dat` and statistic database `stat.dat` will be created.

## Server Installation

### Linux

Download static binary from [Releases](https://github.com/ian-travers/nfsu-server/releases) or build it from sources:

```
apt-get install git make build-essential
git clone https://github.com/nfsu-server/nfsu-server.git
cd nfsuserver/nfsuserver
make
./nfsuserver
```

### Windows

Download `nfsuserver.2.6.win32.zip` from [Releases](https://github.com/ian-travers/nfsu-server/releases) or use Visual Studio to build own executable 

`nfsuserver.exe` is for normal start. 

If you need to create a service you have to build new executable using Visual Studio. 
Uncomment [NT_SERVICE](https://github.com/ian-travers/nfsu-server/blob/main/NFSUServer2/win_nix.h#L5) flag for Windows service mode).
For compilation you have to install [Windows SDK](https://developer.microsoft.com/en-us/windows/downloads/windows-10-sdk/).

To install a server as a Windows Service run:
```
nfsuserver_svc.exe -i
```
To uninstall run:
```
nfsuserver_svc.exe -u
```
