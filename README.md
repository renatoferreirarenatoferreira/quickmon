# QuickMon
**Network troubleshooting tool.**

QuickMon is a small monitoring tool that provides instantaneous information regarding the network and application infrastructure. It was made to help us to take quick decisions in our daily tasks. Note that it's not the intention of this tool to store long term information or generate alerts, for this kind of functionality I'm very happy by using Zabbix.

#### Windows binaries

* [QuickMon v0.3.0 32bit](https://github.com/renatoferreirarenatoferreira/quickmon/releases/download/v0.3.0/QuickMon-v0.3.0-32bit.zip)
* [QuickMon v0.3.0 64bit](https://github.com/renatoferreirarenatoferreira/quickmon/releases/download/v0.3.0/QuickMon-v0.3.0-64bit.zip)

#### Basic usage

All the tools can be launched directly from main screen, what is divided in three parts.

![alt tag](https://raw.githubusercontent.com/renatoferreirarenatoferreira/quickmon/master/docs/mainwindow.png)

* **Hosts List**: The host list is located on top of the window and permits to create, edit and remove hosts. Any host to monitored have to be added to host list first, this is needed due to the amount of parameters required for SNMP queries for example.
* **Basic Tools**: Located below the host list, the basic tools can be launched by single clicks over its icons. Multiple instances of each tool can be used simultaneously.
* **SNMP Tools**: SNMP Tools are small groups of data collectors organized within three basic categories: lists, tables and graphs. Due to the nature of the navigation through trees, collapsing and expanding items, the SNMP Tools require double clicks to be launched. As well as the basic tools, multiples instances can be used at same time.

**To initialize the tool simply drag a host from the host list and drop over the tool window. It will start the tool immediately.**

*Some SNMP agents may take a long time to update some OIDs, what can reduce the accuracy of instantaneous measurements and make graphs seem slightly broken. Windows SNMP and Net-SNMP are examples of agents that will take 3 or more seconds to update the ifTable.*

#### Examples of usage

Ping tool against 8.8.8.8 Google DNS:

![alt tag](https://raw.githubusercontent.com/renatoferreirarenatoferreira/quickmon/master/docs/pinggoogledns.png)

Tracerouting path to 8.8.8.8 Google DNS:

![alt tag](https://raw.githubusercontent.com/renatoferreirarenatoferreira/quickmon/master/docs/traceroutegoogledns.png)

Basic system information list:

![alt tag](https://raw.githubusercontent.com/renatoferreirarenatoferreira/quickmon/master/docs/snmpsystemlocalhost.png)

Standard interface table:

![alt tag](https://raw.githubusercontent.com/renatoferreirarenatoferreira/quickmon/master/docs/interfacetablelocalhost.png)

**Right click over a table item shows aditional options for the given object.**

Linux load average graph:

![alt tag](https://raw.githubusercontent.com/renatoferreirarenatoferreira/quickmon/master/docs/linuxnetsnmpload.png)

F5 BIG-IP general information list:

![alt tag](https://raw.githubusercontent.com/renatoferreirarenatoferreira/quickmon/master/docs/bigipgeneral.png)

F5 BIG-IP interface table (from private mib) lauching a interface traffic graph for interface 1.2:

![alt tag](https://raw.githubusercontent.com/renatoferreirarenatoferreira/quickmon/master/docs/bigipinterfaces.png)

![alt tag](https://raw.githubusercontent.com/renatoferreirarenatoferreira/quickmon/master/docs/bigipinterfacetraffic.png)