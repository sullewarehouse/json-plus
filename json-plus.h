
//
// json-plus.h
// 
// Author:
//     Brian Sullender
//     SULLE WAREHOUSE LLC
// 
// Description:
//     The header file for the json_plus namespace.
//

#include <Windows.h>

#ifndef JSON_PLUS_H
#define JSON_PLUS_H

namespace json_plus
{
	// UTF8 functions
	namespace UTF8_Encoding
	{
		// Get the number of 'CHAR' units for a UTF8 encoded Unicode character
		UCHAR GetCharacterUnits(CHAR Code);

		// Encode a Unicode code point (character) as UTF8
		UCHAR Encode(CHAR* pBuffer, size_t Length, ULONG CodePoint);

		// Encode a Unicode code point (character) as UTF8 (unsafe version)
		UCHAR EncodeUnsafe(CHAR* pBuffer, ULONG CodePoint);

		// Get a Unicode code point (character) from a UTF8 encoded string
		ULONG Decode(UCHAR Units, const CHAR* String);

		// Get the number of 'CHAR' units for a UTF8 encoded string
		size_t GetStringUnits(const CHAR* String);

		// Copy a UTF8 encoded string
		size_t StringCopy(CHAR* Destination, size_t Length, const CHAR* Source);

		// Compare 2 UTF8 encoded strings
		LONG CompareStrings(const CHAR* String1, const CHAR* String2);

		// Compare 2 UTF8 encoded strings (insensitive)
		LONG CompareStringsInsensitive(const CHAR* String1, const CHAR* String2);
	}

	// JSON token types
	enum class JSON_TOKEN
	{
		CURLY_OPEN,
		CURLY_CLOSE,
		COLON,
		STRING,
		NUMBER,
		ARRAY_OPEN,
		ARRAY_CLOSE,
		COMMA,
		BOOLEAN,
		NULL_TYPE,
		JSON_END,
		UNRECOGNIZED_TOKEN
	};

	// JSON element types
	enum class JSON_TYPE
	{
		OBJECT,
		ARRAY,
		STRING,
		NUMBER,
		BOOLEAN,
		NULL_TYPE
	};

	// JSON node, for the parsed tree
	typedef struct _JSON_NODE JSON_NODE;
	struct _JSON_NODE
	{
		// Next node in the linked list
		JSON_NODE* next;
		// Key for the node
		CHAR* key;
		// Value for the node
		void* value;
		// JSON node type
		JSON_TYPE type;
		// Format override for the node
		const CHAR* format;
	};

	// JSON error codes
	typedef enum class _JSON_ERROR_CODE {
		NONE,
		INVALID_PARAMETER,
		OUT_OF_MEMORY,
		UNRECOGNIZED_TOKEN,

		// parse errors:

		UNEXPECTED_START_TOKEN,

		// parse object errors:

		OBJECT_SYNTAX_ERROR_KEY_ALREADY_DEFINED,
		OBJECT_SYNTAX_ERROR_KEY_NOT_DEFINED,
		UNEXPECTED_CLOSING_SQUARE_BRACKET,
		EXPECTED_CURLY_BRACKET_ENCOUNTERED_JSON_END,
		EXPECTED_PAIR_ENCOUNTERED_OBJECT_END,

		// parse array errors:

		UNEXPECTED_ARRAY_VALUE,
		UNEXPECTED_PAIR_COLON_TOKEN,
		UNEXPECTED_CLOSING_CURLY_BRACKET,
		EXPECTED_SQUARE_BRACKET_ENCOUNTERED_JSON_END,
		EXPECTED_ARRAY_VALUE,

		// parse string errors:

		EXPECTED_DOUBLE_QUOTES_ENCOUNTERED_JSON_END,
		STRING_SYNTAX_ERROR,

		// parse boolean errors:

		SYNTAX_ERROR_EXPECTED_BOOLEAN_VALUE,

		// parse null errors:

		SYNTAX_ERROR_EXPECTED_NULL_VALUE
	} JSON_ERROR_CODE;

	// JSON parsing context
	struct JSON_PARSER_CONTEXT
	{
		JSON_ERROR_CODE errorCode;
		const CHAR* errorDescription;
		ULONG lineNumber;
		ULONG charNumber;
		const CHAR* errorTokens;
	};

	// Parse a JSON string
	JSON_NODE* JSON_Parse(const CHAR* json, JSON_PARSER_CONTEXT* context);

	// Free a JSON node tree
	void JSON_Free(JSON_NODE* json_root);

	// Get a JSON object from an object
	JSON_NODE* JSON_GetObject(JSON_NODE* object, const CHAR* key);

	// Get a JSON array from an object
	JSON_NODE* JSON_GetArray(JSON_NODE* object, const CHAR* key);

	// Get a JSON string from an object
	CHAR* JSON_GetString(JSON_NODE* object, const CHAR* key);

	// Get a JSON number from an object
	CHAR* JSON_GetNumber(JSON_NODE* object, const CHAR* key);

	// Get a JSON bool value from an object
	bool JSON_GetBoolean(JSON_NODE* object, const CHAR* key);

	// Create a JSON node
	JSON_NODE* JSON_CreateNode(JSON_TYPE type, const CHAR* key, void* value);

	// Forward declaration of JSON_OBJECT
	class JSON_OBJECT;

	// Forward declaration of JSON_ARRAY
	class JSON_ARRAY;

	// JSON object
	class JSON_OBJECT
	{
	private:
		// Object root node
		JSON_NODE* json_root;
		// Number of items in the object
		ULONG count;
	public:
		// Default initializer
		JSON_OBJECT();
		// Standard initializer
		JSON_OBJECT(JSON_NODE* root);
		// Assignment operator overload (to object)
		void operator=(JSON_NODE* root);
		// Assignment operator overload (from object)
		operator JSON_NODE* () const;
		// Check if the object exists
		bool Empty();
		// Get the number of items in the object
		ULONG Count();
		// Get an object from the object using a key
		JSON_OBJECT Object(const CHAR* key);
		// Get an array from the object using a key
		JSON_ARRAY Array(const CHAR* key);
		// Get a string from the object using a key
		const CHAR* String(const CHAR* key);
		// Get a boolean from the object using a key
		bool Boolean(const CHAR* key);
		// Nested Number class
		class Number
		{
		private:
			// Parent reference
			JSON_OBJECT& parent;
		public:
			// Constructor for the Number class, which is a nested class inside the JSON_OBJECT class.
			Number(JSON_OBJECT& parent);
			// Get a double from the object using a key
			double Double(const CHAR* key);
			// Get a int from the object using a key
			int Int(const CHAR* key);
			// Get a long from the object using a key
			long Long(const CHAR* key);
			// Get a 64-bit int from the object using a key
			long long Int64(const CHAR* key);
			// Get a number from the object as a string using a key
			const CHAR* String(const CHAR* key);
		};
		// Get a number from the object
		Number Number{ *this };
		// Nested Insert class
		class Insert
		{
		private:
			// Parent reference
			JSON_OBJECT& parent;
		public:
			// Constructor for the Insert class, which is a nested class inside the JSON_OBJECT class.
			Insert(JSON_OBJECT& parent);
			// Insert an object with a key
			JSON_OBJECT Object(const CHAR* key);
			// Insert an array with a key
			JSON_ARRAY Array(const CHAR* key);
			// Insert a string with a key
			JSON_NODE* String(const CHAR* key, const CHAR* value);
			// Insert a boolean with a key
			JSON_NODE* Boolean(const CHAR* key, bool value);
			// Nested Number class for Insert
			class Number
			{
			private:
				// Parent reference
				Insert& parent;
			public:
				// Constructor for the Number class, which is a nested class inside the JSON_OBJECT::Insert class.
				Number(Insert& parent);
				// Insert a double with a key
				JSON_NODE* Double(const CHAR* key, double value);
				// Insert a int with a key
				JSON_NODE* Int(const CHAR* key, int value);
				// Insert a long with a key
				JSON_NODE* Long(const CHAR* key, long value);
				// Insert a 64-bit int with a key
				JSON_NODE* Int64(const CHAR* key, long long value);
				// Insert a number as a string with a key
				JSON_NODE* String(const CHAR* key, const CHAR* value);
			};
			// Insert a number into the object
			Number Number{ *this };
		};
		// Insert a JSON object, array or key-value pair
		Insert Insert{ *this };
		// Create JSON from object
		CHAR* Encode(const CHAR* format);
		// Format override
		bool FormatOverride(const CHAR* format);
	};

	// JSON array
	class JSON_ARRAY
	{
	private:
		// Array root node
		JSON_NODE* json_root;
		// Number of items in the array
		ULONG count;
	public:
		// Default initializer
		JSON_ARRAY();
		// Standard initializer
		JSON_ARRAY(JSON_NODE* root);
		// Assignment operator overload (to array)
		void operator=(JSON_NODE* root);
		// Assignment operator overload (from array)
		operator JSON_NODE* () const;
		// Check if the array exists
		bool Empty();
		// Get the number of items in the array
		ULONG Count();
		// Get an object from the array using an index
		JSON_OBJECT Object(ULONG i);
		// Get an array from the array using an index
		JSON_ARRAY Array(ULONG i);
		// Get a string from the array using an index
		const CHAR* String(ULONG i);
		// Get a boolean from the array using an index
		bool Boolean(ULONG i);
		// Nested Number class
		class Number
		{
		private:
			// Parent reference
			JSON_ARRAY& parent;
		public:
			// Constructor for the Number class, which is a nested class inside the JSON_ARRAY class.
			Number(JSON_ARRAY& parent);
			// Get a double from the array using an index
			double Double(ULONG i);
			// Get a int from the array using an index
			int Int(ULONG i);
			// Get a long from the array using an index
			long Long(ULONG i);
			// Get a 64-bit int from the array using an index
			long long Int64(ULONG i);
			// Get a double from the array using an index
			const CHAR* String(ULONG i);
		};
		// Get a number from the array
		Number Number{ *this };
		// Nested Insert class
		class Insert
		{
		private:
			// Parent reference
			JSON_ARRAY& parent;
		public:
			// Constructor for the Insert class, which is a nested class inside the JSON_ARRAY class.
			Insert(JSON_ARRAY& parent);
			// Insert an object
			JSON_OBJECT Object();
			// Insert an array
			JSON_ARRAY Array();
			// Insert a string
			JSON_NODE* String(const CHAR* value);
			// Insert a boolean
			JSON_NODE* Boolean(bool value);
			// Nested Number class for Insert
			class Number
			{
			private:
				// Parent reference
				Insert& parent;
			public:
				// Constructor for the Number class, which is a nested class inside the JSON_ARRAY::Insert class.
				Number(Insert& parent);
				// Insert a double
				JSON_NODE* Double(double value);
				// Insert a int
				JSON_NODE* Int(int value);
				// Insert a long
				JSON_NODE* Long(long value);
				// Insert a 64-bit int
				JSON_NODE* Int64(long long value);
				// Insert a number as a string
				JSON_NODE* String(const CHAR* value);
			};
			// Insert a number into the array
			Number Number{ *this };
		};
		// Insert a JSON object, array or key-value pair
		Insert Insert{ *this };
		// Create JSON from array
		CHAR* Encode(const CHAR* format);
		// Format override
		bool FormatOverride(const CHAR* format);
	};
}

#endif // !JSON_PLUS_H
