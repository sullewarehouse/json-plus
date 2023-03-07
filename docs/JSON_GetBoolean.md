# JSON_GetBoolean

**json_plus::JSON_GetBoolean(object, key)**

Get a JSON boolean from an object node.

***object***  
The JSON object node to search.

***key***  
The key of the Key-Value pair to find.

**Return Value**  
A `bool` boolean value. `false` is returned if the Key-Value pair is not found.

**Remarks**  
You can use the `JSON_OBJECT` class instead to find a boolean using a key.
```
JSON_OBJECT json_file; // Parsed JSON file
bool IsTrue = json_file.Boolean("IsTrue");
```
