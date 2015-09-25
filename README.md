# OVCLI
This is a project that has **slightly** spiralled out of control, however i'm hoping that it should hopefully assist when used in conjunciton with Data Centre automation tools such as [ansible](http://www.ansible.com) or [chef](https://www.chef.io). 

![a](https://raw.githubusercontent.com/thebsdbox/OVCLI/master/OVCLI.jpg "OVCLI example")

So, **OVCLI** spawned from a number of projects and tools that all wrapped around the use of [HP OneView](http://www.hp.com/go/oneview) and it's APIs. This tool serves as both a tool to learn the OneView [API](http://h17007.www1.hp.com/docs/enterprise/servers/oneview1.2/cic-api/en/api-docs/current/index.html) and a tool to assist in scripting and automating some tasks in the data centre. 

**Building/Compilng**

I've left the object archive files that are inside ./Library/{OS}/ directory so that static compilation will produce an executable that should pretty much run in the majority of places. The two object archives are from the latest builds of both jansson and rabbitMQ (as of SEP/15) and have been compiled on both OS X and Linux. 
So to install, grab everthing by clicking on the Download Zip button on the right and download the archive of source files and in a terminal window make your way into OVCLI-master/OVCLI and you'll find the build_ovcli.sh shell script. Running this script will result in detecting the Operating System and creating the binary for you. I will produce some pre-compiled binaries for people that don't have a development environment to hand.

```
| => ./build_ovcli.sh 
Detected Operating System: Darwin
Building for OSX
Compilation Complete, please enjoy
| => ls -la ./ovcli 
-rwxr-xr-x  1 dan  staff  163404 25 Sep 12:44 ./ovcli
``` 

**USAGE** **EXAMPLES**:

Logging into HP OneView

`OVCLI 10.0.0.1 LOGIN Administrator password`

Listing all server profiles
```
OVCLI 10.0.0.1 SHOW SERVER-PROFILES URI
/rest/server-profiles/5fdaf0cb-b7a8-40b1-b576-8a91e5d5acbf  TEST
/rest/server-profiles/dd87433e-c564-4381-8542-7e9cf521b8c0  DEV
/rest/server-profiles/d75a1d9e-8bc4-4ee3-9fa8-3246ba71f5db  PROD
```
Create a Network

```
OVCLI 10.0.0.1 CREATE NETWORKS WEB 101 General
OVCLI 10.0.0.1 CREATE NETWORKS APP 102 General
OVCLI 10.0.0.1 CREATE NETWORKS DB 103 General
```

List Networks
```
OVCLI 10.0.0.1 SHOW NETWORKS FIELDS name
WEB
APP
DB
```

Clone a Server Profile 50 times
```
OVCLI 10.0.0.1 CLONE SERVER-PROFILES /rest/server-profiles/dd87433e-c564-4381-8542-7e9cf521b8c0 50
DEV_0
DEV_1
...
DEV_50
```

More info to follow
...