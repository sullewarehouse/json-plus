# JSON_GetArray

**json_plus::JSON_GetArray(object, key)**

Get a JSON array node from an object node.

***object***  
The JSON object node to search.

***key***  
The key of the Key-Value pair to find.

**Return Value**  
A `JSON_NODE` pointer that is the array, or `NULL` if not found.

**Remarks**  
You can use the `JSON_OBJECT` class instead to find a array using a key.
```
JSON_OBJECT json_file; // Parsed JSON file
JSON_ARRAY numbers = json_file.Array("numbers");
```
