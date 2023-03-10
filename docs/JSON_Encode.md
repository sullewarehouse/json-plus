# JSON_Encode

**json_plus::JSON_Encode(json_root, format)**

Create a JSON node.

***json_root***  
The root JSON node. This can be an object or array node.

***format***  
The format of the JSON string. This parameter is a string that specifies when a new-line (`\n`) character is added to the JSON string. For example, if you want a new-line after `{`, then your format string would be `"{\n"`. If you also want a new-line before `{`, then your format string would be `"\n{\n"`. You can seperate new-line character formatting for different characters using a space. You can have any number of new-line characters you need. For example, `"\n\n{\n"` results in 2 new-line characters before the `{` character. When a format string is used tab indentation is automatically turned on for `{` and `}` characters.

**Return Value**  
A `CHAR*` JSON string, or `NULL` if the function fails.
