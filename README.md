# json-plus
JSON parser and encoder with UTF8 support.

You do not need to compile anything to use json-plus, just include **`json-plus.cpp`** and **`json-plus.h`** in your application.
Everything is contained in the **`json_plus`** namespace.

Check **`example.cpp`** for a detailed example that creates a json string, parses it and prints some details to the console.

## Functions

- [JSON_Generate](docs/JSON_Generate.md)
- [JSON_Parse](docs/JSON_Parse.md)
- [JSON_Free](docs/JSON_Free.md)
- [JSON_GetObject](docs/JSON_GetObject.md)
- [JSON_GetArray](docs/JSON_GetArray.md)
- [JSON_GetString](docs/JSON_GetString.md)
- [JSON_GetNumber](docs/JSON_GetNumber.md)
- [JSON_GetBoolean](docs/JSON_GetBoolean.md)
- [JSON_CreateNode](docs/JSON_CreateNode.md)

## JSON_OBJECT and JSON_ARRAY classes

You can use the `JSON_OBJECT` and `JSON_ARRAY` classes instead of using the JSON_Get*** functions.  
Here is a simple example:
```
JSON_PARSER_CONTEXT context;
const CHAR* json_string = "{ \"username\":\"John\", \"password\":\"1234\" }";
JSON_OBJECT json_file = JSON_Parse(json_string, &context);
const CHAR* username = json_file.String("username");
```

Members:
- Empty
- Count
- Object
- Array
- String
- Boolean
- Number
  - Double
  - Int
  - Long
  - Int64
  - String
- Insert
  - Object
  - Array
  - String
  - Boolean
  - Number
    - Double
    - Int
    - Long
    - Int64
    - String
- Generate
- FormatOverride

## JSON resources

- [JSON Specification](https://www.rfc-editor.org/rfc/rfc8259)
