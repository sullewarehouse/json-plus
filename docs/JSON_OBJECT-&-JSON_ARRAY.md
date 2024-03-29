# JSON_OBJECT and JSON_ARRAY

Class Functions:
- [MakeRoot](#MakeRoot)
- [Free](#Free)
- [Empty](#Empty)
- [Count](#Count)
- [First](#First)
- [Object](#Object)
- [Array](#Array)
- [String](#String)
- [Boolean](#Boolean)
- [Number Class](#Number)
  - [Double](#Number)
  - [Int](#Number)
  - [Long](#Number)
  - [Int64](#Number)
  - [String](#Number)
- [Insert Class](#Insert)
  - [Object](#Insert)
  - [Array](#Insert)
  - [String](#Insert)
  - [Boolean](#Insert)
  - [Number Class](#Insert-Number-Class)
    - [Double](#Insert-Number-Class)
    - [Int](#Insert-Number-Class)
    - [Long](#Insert-Number-Class)
    - [Int64](#Insert-Number-Class)
    - [String](#Insert-Number-Class)
- [Delete](#Delete)
- [Generate](#Generate)
- [Format](#Format)
- [Parse](#Parse)

## MakeRoot
Create a root object or array node and assign it to the class.

## Free
Free the JSON node tree in the object or array class.

## Empty
Returns false if the class does not have a JSON root node, true otherwise.

## Count
Returns the number of items in the object or array.

## First
Returns the 1st item (`JSON_NODE*`) in the object or array.

## Object
Get an object using a key (`JSON_OBJECT`) or index (`JSON_ARRAY`).

## Array
Get an array using a key (`JSON_OBJECT`) or index (`JSON_ARRAY`).

## String
Get a string using a key (`JSON_OBJECT`) or index (`JSON_ARRAY`).

## Boolean
Get a boolean using a key (`JSON_OBJECT`) or index (`JSON_ARRAY`).

## Number
Get a number using a key (`JSON_OBJECT`) or index (`JSON_ARRAY`).

| Function | Description |
| --- | --- |
| Double | Returns the number as a double |
| Int | Returns the number as a int |
| Long | Returns the number as a long |
| Int64 | Returns the number as a long long (64-bit int) |
| String | Returns the number as a string |

## Insert
Insert an item. `JSON_OBJECT` takes a key as a parameter, `JSON_ARRAY` does not.

| Function | Description |
| --- | --- |
| Object | Returns a `JSON_OBJECT` class |
| Array | Returns a `JSON_ARRAY` class |
| String | Returns the created `JSON_NODE*` pointer that contains the key-value pair |
| Boolean | Returns the created `JSON_NODE*` pointer that contains the key-value pair |

### Insert Number Class

| Function | Description |
| --- | --- |
| Double | Returns the created `JSON_NODE*` pointer that contains the key-value pair |
| Int | Returns the created `JSON_NODE*` pointer that contains the key-value pair |
| Long | Returns the created `JSON_NODE*` pointer that contains the key-value pair |
| Int64 | Returns the created `JSON_NODE*` pointer that contains the key-value pair |
| String | Returns the created `JSON_NODE*` pointer that contains the key-value pair |

## Delete
Delete an item from a JSON_OBJECT or JSON_ARRAY using a key, index number or reference.

## Generate
Calls the [JSON_Generate](JSON_Generate.md) function.

## Format
Sets the format for the object or array, this will override the format parameter passed to JSON_Generate. See [JSON_Generate](JSON_Generate.md) for details about JSON formatting.

## Parse
Calls the [JSON_Parse](JSON_Parse.md) function.
