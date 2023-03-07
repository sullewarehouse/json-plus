# JSON_GetObject

**json_plus::JSON_GetObject(object, key)**

Get a JSON object node from an object node.

***object***  
The JSON object node to search.

***key***  
The key of the Key-Value pair to find.

**Return Value**  
A `JSON_NODE` pointer that is the object, or `NULL` if not found.

**Remarks**  
You can use the `JSON_OBJECT` class instead to find a object using a key.
```
JSON_OBJECT json_file; // Parsed JSON file
JSON_OBJECT settings = json_file.Object("settings");
```
