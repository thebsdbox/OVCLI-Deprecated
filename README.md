# OVCLI
This is a project that has **slightly** spiralled out of control, however i'm hoping that it should hopefully assist when used in conjunciton with Data Centre automation tools such as [ansible](http://www.ansible.com) or [chef](https://www.chef.io). 

![a](https://raw.githubusercontent.com/thebsdbox/OVCLI/master/OVCLI.jpg "OVCLI example")

So, **OVCLI** spawned from a number of projects and tools that all wrapped around the use of [HP OneView](http://www.hp.com/go/oneview) and it's APIs. This tool serves as both a tool to learn the OneView [API](http://h17007.www1.hp.com/docs/enterprise/servers/oneview1.2/cic-api/en/api-docs/current/index.html) and a tool to assist in scripting and automating some tasks in the data centre. 

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


More info to follow
...