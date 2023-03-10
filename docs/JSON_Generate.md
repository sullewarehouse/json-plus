# JSON_Generate

**json_plus::JSON_Generate(json_root, format)**

Create a JSON string from a node tree.

***json_root***  
The root JSON node. This can be an object or array node.

***format***  
The format of the JSON string. This parameter is a string that specifies when a new-line (`\n`) character is added to the JSON string. For example, if you want a new-line after `{`, then your format string would be `"{\n"`. If you also want a new-line before `{`, then your format string would be `"\n{\n"`. You can seperate new-line character formatting for different characters using a space. You can have any number of new-line characters you need. For example, `"\n\n{\n"` results in 2 new-line characters before the `{` character. When a format string is used tab indentation is automatically turned on and activated by the `{` and `}` characters.

**Return Value**  
A `CHAR*` JSON string, or `NULL` if the function fails.

**Remarks**  
The `JSON_OBJECT` and `JSON_ARRAY` classes have a `Encode` member that calls this function.
