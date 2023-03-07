# JSON_GetString

**json_plus::JSON_GetString(object, key)**

Get a JSON string from an object node.

***object***  
The JSON object node to search.

***key***  
The key of the Key-Value pair to find.

**Return Value**  
A `CHAR*` string, or `NULL` if not found.

**Remarks**  
You can use the `JSON_OBJECT` class instead to find a string using a key.
```
JSON_OBJECT json_file; // Parsed JSON file
const CHAR* first_name = json_file.String("first_name");
```
