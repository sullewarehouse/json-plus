# JSON_CreateNode

**json_plus::JSON_CreateNode(type, key, value)**

Create a JSON node.

***type***  
The type of JSON node. This can be 1 of the following:
- **`JSON_TYPE::OBJECT`**
- **`JSON_TYPE::ARRAY`**
- **`JSON_TYPE::STRING`**
- **`JSON_TYPE::NUMBER`**
- **`JSON_TYPE::BOOLEAN`**
- **`JSON_TYPE::NULL_TYPE`**

***key***  
The key of the Key-Value pair to create.

***value***  
The value of the Key-Value pair to create.

**Return Value**  
A `JSON_NODE` pointer that contains the Key-Value pair passed to the function, or `NULL` if the function fails.
