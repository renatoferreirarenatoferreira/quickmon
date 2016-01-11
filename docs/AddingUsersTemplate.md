# Adding user's template

Since version v0.4.0 it's possible to add a custom template file to have your own SNMP items. It can be done by simply creating a fale named 'User.json' in the same directory where the QuickMon's excutable is located.

Once the file is created, the QuickMon will show the 'User's Template' under SNMP Tools like in the image below:

![alt tag](https://raw.githubusercontent.com/renatoferreirarenatoferreira/quickmon/master/docs/userstemplate.png)

The template files must be written using the JSON format. We suggest the following resources regarding the JSON format and the QuickMon's SNMP template format:

* [JSON Tutorial](http://www.w3schools.com/json/): JSON Tutorial from w3schools.com;
* [JSON Formatter & Validator](https://jsonformatter.curiousconcept.com/): Online JSON syntax validator;
* [quickmon/MainResources/SNMPTemplates/](https://github.com/renatoferreirarenatoferreira/quickmon/tree/master/MainResources/SNMPTemplates): Template files natively used by QuickMon.

Don't forget to submit your custom template file to us if you think that it could be useful to other users. User contributions will be added natively in future releases.

#### Basic file structure example

```
{
    "Item Name": {
        "Type": "List",
        "Order": 0,
        "Items": {
            "Variable Name": {
                "OID": "1.3.6.1.4.1.3375.2.1.3.5.1.0",
                "Type": "CounterPerSecond",
                "Order": 0,
                "Unit": "B",
                "Interval": 5000,
                "Calculate": "*8",
                "ValueMappings": {
                    "0": "value(0)",
                    "1": "value(1)"
                }
            }
        },
        "ContextMenu": {
            ...
        }
}
```

#### Items' descriptions:

* **/Item Name**

   Name of the item. This key can be changed the represent its real name.

* **/Item Name/Type**

   Type of item.

   Valid values are *Graph*, *List* and *Table*.

* **/Item Name/Order**

   Number value used to sort all items in template's tree.

* **/Item Name/Items**

   List of variables.

* **/Item Name/Items/Variable Name**

   Variable's name. This key can be changed the represent its real name.

* **/Item Name/Items/Variable Name/OID**

   SNMP OID.

   For the types *Graph* and *List* the OID must be complete with their indexes, usually '.0'. For *Table* types only the OID is needed, as all indexes are discovered during table generation.

* **/Item Name/Items/Variable Name/Type**

   Type of variable.

   Valid values are *Counter*, *CounterPerSecond*, *Uint* and *Int*.

* **/Item Name/Items/Variable Name/Order**

   Number value used to sort all variables.

* **/Item Name/Items/Variable Name/Unit**

   Value unit.

   Unit 'B' will be reduced/simplified by 1024, while all the others will be by 1000. Using '%' will make the graph fixed from 0 to 100 and its values are never reduced/simplified. Used only with item type *Graph*.

* **/Item Name/Items/Variable Name/Interval**

   Update interval in miliseconds and have to be greater than 1000. Note that some SNMP agents may take more than 5 or 10 seconds to update values.

   Used only with item type *Graph*.

* **/Item Name/Items/Variable Name/Calculate**

   Calculate the number before being presented supporting the four operations, "+", "-", "*" and "/".

   Used only with item type *Graph*.

* **/Item Name/Items/Variable Name/ValueMappings**

   Map SNMP returns with custom values.

   Used only with item type *List* and *Table*.

* **/Item Name/ContextMenu**

   List of conext menu items. It is used to show graphs from table lines and its subitems syntax are identical as the items type *Graph*.

   Used only with type *Table*.
