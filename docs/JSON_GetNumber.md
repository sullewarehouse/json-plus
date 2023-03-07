# JSON_GetNumber

**json_plus::JSON_GetNumber(object, key)**

Get a JSON number from an object node. The function returns the number as a string to avoid type assumptions and precision loss.

***object***  
The JSON object node to search.

***key***  
The key of the Key-Value pair to find.

**Return Value**  
A `CHAR*` string, or `NULL` if not found.

**Remarks**  
You can use the `JSON_OBJECT` class instead to find a number using a key.
```
JSON_OBJECT json_file; // Parsed JSON file
int file_count = json_file.Number.Int("file_count");
```
