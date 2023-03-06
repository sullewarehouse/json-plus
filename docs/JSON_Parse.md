# JSON_Parse

**json_plus::JSON_Parse(json, context)**

Parses a JSON string.

***json***  
The JSON string to parse. This is a `const CHAR*` string type that can be UTF8 encoded.

***context***  
Pointer to a `JSON_PARSER_CONTEXT` structure that receives the parsing info.

**Return Value**  
A `JSON_NODE` pointer that is the 1st node in the JSON node tree.

**Remarks**  
This function will return the JSON node tree built if an error is encountered, so its important to free the node tree even if parsing fails. A simple example would look like this:
```
JSON_PARSER_CONTEXT context;
const CHAR* json_string = "{ \"password\":1234\" }";
JSON_OBJECT json_file = JSON_Parse(json_string, &context);
if (context.errorCode != JSON_ERROR_CODE::NONE) {
	JSON_Free(json_file);
	printf("%s\n", context.errorDescription);
	return -1;
}
```