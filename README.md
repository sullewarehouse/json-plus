# json-plus
JSON parser and generator with UTF8 support.

You do not need to compile anything to use json-plus, just include **`json-plus.cpp`** and **`json-plus.h`** in your application. Everything is contained in the **`json_plus`** namespace.

See **`example.cpp`** for a detailed example that creates a json string, parses it and prints to the console.

json-plus is cross-platform compatible.

LICENSE TERMS
=============
```
  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.
  
  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:
  
  (1) If any part of the source code for this software is distributed, then this
      README file must be included, with this copyright and no-warranty notice
      unaltered; and any additions, deletions, or changes to the original files
      must be clearly indicated in accompanying documentation.
  (2) Permission for use of this software is granted only if the user accepts
      full responsibility for any undesirable consequences; the authors accept
      NO LIABILITY for damages of any kind.
```

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

[JSON_OBJECT and JSON_ARRAY classes](docs/JSON_OBJECT-&-JSON_ARRAY.md)

## JSON Resources

- [JSON Specification](https://www.rfc-editor.org/rfc/rfc8259)
