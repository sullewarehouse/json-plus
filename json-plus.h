
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

#ifndef JSON_PLUS_H
#define JSON_PLUS_H

namespace json_plus
{
	// UTF8 functions
	namespace UTF8_Encoding
	{
		// Get the number of 'char' units for a UTF8 encoded Unicode character
		unsigned char GetCharacterUnits(char Code);

		// Encode a Unicode code point (character) as UTF8
		unsigned char Encode(char* pBuffer, size_t Length, unsigned long CodePoint);

		// Encode a Unicode code point (character) as UTF8 (unsafe version)
		unsigned char EncodeUnsafe(char* pBuffer, unsigned long CodePoint);

		// Get a Unicode code point (character) from a UTF8 encoded string
		unsigned long Decode(unsigned char Units, const char* String);

		// Get the number of 'char' units for a UTF8 encoded string
		size_t GetStringUnits(const char* String);

		// Copy a UTF8 encoded string
		size_t StringCopy(char* Destination, size_t Length, const char* Source);

		// Compare 2 UTF8 encoded strings
		long CompareStrings(const char* String1, const char* String2);

		// Compare 2 UTF8 encoded strings (insensitive)
		long CompareStringsInsensitive(const char* String1, const char* String2);
	}

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
		char* key;
		// Value for the node
		void* value;
		// JSON node type
		JSON_TYPE type;
		// Format override for the node
		const char* format;
		// Get value as a char* string
		const char* String();
		// Get value as a double
		double Double();
		// Get value as a int
		int Int();
		// Get value as a long
		long Long();
		// Get value as a long long (64-bit int)
		long long Int64();
		// Get value as a boolean
		bool Boolean();
	};

	// JSON error codes
	typedef enum class _JSON_ERROR_CODE {
		NONE,

		// general errors:

		INVALID_PARAMETER,
		OUT_OF_MEMORY,

		// parse errors:

		UNRECOGNIZED_TOKEN,
		UNEXPECTED_START_TOKEN,

		// parse object errors:

		OBJECT_SYNTAX_ERROR_EXPECTED_COLON,
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

		STRING_CHARACTERS_MUST_BE_ESCAPED,
		STRING_FORCED_STRICT_ESCAPING,
		STRING_UNUSED_ESCAPE_CHARACTER,
		EXPECTED_DOUBLE_QUOTES_ENCOUNTERED_JSON_END,

		// parse literal name errors:

		INVALID_LITERAL_NAME,
	} JSON_ERROR_CODE;

	// JSON parsing context
	class JSON_PARSER_CONTEXT
	{
	public:
		// Default initializer
		JSON_PARSER_CONTEXT();
		// Error after JSON_Parse call
		JSON_ERROR_CODE errorCode;
		// Error description after JSON_Parse call
		const char* errorDescription;
		// Force strict string escaping for code editors
		bool visualEscapeOnly;
		// Number of characters parsed
		unsigned long charNumber;
		// Number of lines parsed
		unsigned long lineNumber;
		// Begin index of the error
		unsigned long beginIndex;
		// Number of characters from beginIndex
		unsigned long errorLength;
	};

	// Create JSON string from node tree
	char* JSON_Generate(JSON_NODE* json_root, const char* format);

	// Parse a JSON string and create a node tree
	JSON_NODE* JSON_Parse(const char* json, JSON_PARSER_CONTEXT* context);

	// Free a JSON node tree
	void JSON_Free(JSON_NODE* json_root);

	// Get a JSON object from an object
	JSON_NODE* JSON_GetObject(JSON_NODE* object, const char* key);

	// Get a JSON array from an object
	JSON_NODE* JSON_GetArray(JSON_NODE* object, const char* key);

	// Get a JSON string from an object
	char* JSON_GetString(JSON_NODE* object, const char* key);

	// Get a JSON number from an object
	char* JSON_GetNumber(JSON_NODE* object, const char* key);

	// Get a JSON bool value from an object
	bool JSON_GetBoolean(JSON_NODE* object, const char* key);

	// Create a JSON node
	JSON_NODE* JSON_CreateNode(JSON_TYPE type, const char* key, void* value);

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
	public:
		// Default initializer
		JSON_OBJECT();
		// Standard initializer
		JSON_OBJECT(JSON_NODE* root);
		// Parse a JSON string and create a node tree
		JSON_OBJECT(const char* json, JSON_PARSER_CONTEXT* context);
		// Assignment operator overload (to object)
		void operator=(JSON_NODE* root);
		// Assignment operator overload (from object)
		operator JSON_NODE* () const;
		// Create a root object node and assign it to this object
		JSON_NODE* MakeRoot();
		// Free JSON node tree
		void Free();
		// Check if the root object exists
		bool Empty();
		// Get the number of items in the object
		unsigned long Count();
		// Get the first node item in the object
		JSON_NODE* First();
		// Get an object from the object using a key
		JSON_OBJECT Object(const char* key);
		// Get an array from the object using a key
		JSON_ARRAY Array(const char* key);
		// Get a string from the object using a key
		const char* String(const char* key);
		// Get a boolean from the object using a key
		bool Boolean(const char* key);
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
			double Double(const char* key);
			// Get a int from the object using a key
			int Int(const char* key);
			// Get a long from the object using a key
			long Long(const char* key);
			// Get a 64-bit int from the object using a key
			long long Int64(const char* key);
			// Get a number from the object as a string using a key
			const char* String(const char* key);
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
			JSON_OBJECT Object(const char* key);
			// Insert an array with a key
			JSON_ARRAY Array(const char* key);
			// Insert a string with a key
			JSON_NODE* String(const char* key, const char* value);
			// Insert a boolean with a key
			JSON_NODE* Boolean(const char* key, bool value);
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
				JSON_NODE* Double(const char* key, double value);
				// Insert a int with a key
				JSON_NODE* Int(const char* key, int value);
				// Insert a long with a key
				JSON_NODE* Long(const char* key, long value);
				// Insert a 64-bit int with a key
				JSON_NODE* Int64(const char* key, long long value);
				// Insert a number as a string with a key
				JSON_NODE* String(const char* key, const char* value);
			};
			// Insert a number into the object
			Number Number{ *this };
		};
		// Insert a JSON object, array or key-value pair
		Insert Insert{ *this };
		// Delete a key-value pair using a key
		bool Delete(const char* key);
		// Delete a key-value pair using a reference
		bool Delete(JSON_NODE* reference);
		// Create JSON from object
		char* Generate(const char* format);
		// Format for the object, this will override the format parameter passed to JSON_Generate
		bool Format(const char* format);
		// Parse a JSON string and create a node tree
		JSON_NODE* Parse(const char* json, JSON_PARSER_CONTEXT* context);
	};

	// JSON array
	class JSON_ARRAY
	{
	private:
		// Array root node
		JSON_NODE* json_root;
	public:
		// Default initializer
		JSON_ARRAY();
		// Standard initializer
		JSON_ARRAY(JSON_NODE* root);
		// Parse a JSON string and create a node tree
		JSON_ARRAY(const char* json, JSON_PARSER_CONTEXT* context);
		// Assignment operator overload (to array)
		void operator=(JSON_NODE* root);
		// Assignment operator overload (from array)
		operator JSON_NODE* () const;
		// Create a root array node and assign it to this array
		JSON_NODE* MakeRoot();
		// Free JSON node tree
		void Free();
		// Check if the root array exists
		bool Empty();
		// Get the number of items in the array
		unsigned long Count();
		// Get the first node item in the array
		JSON_NODE* First();
		// Get an object from the array using an index
		JSON_OBJECT Object(unsigned long i);
		// Get an array from the array using an index
		JSON_ARRAY Array(unsigned long i);
		// Get a string from the array using an index
		const char* String(unsigned long i);
		// Get a boolean from the array using an index
		bool Boolean(unsigned long i);
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
			double Double(unsigned long i);
			// Get a int from the array using an index
			int Int(unsigned long i);
			// Get a long from the array using an index
			long Long(unsigned long i);
			// Get a 64-bit int from the array using an index
			long long Int64(unsigned long i);
			// Get a double from the array using an index
			const char* String(unsigned long i);
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
			JSON_NODE* String(const char* value);
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
				JSON_NODE* String(const char* value);
			};
			// Insert a number into the array
			Number Number{ *this };
		};
		// Insert a JSON object, array or key-value pair
		Insert Insert{ *this };
		// Delete a key-value pair using a index
		bool Delete(unsigned long i);
		// Delete a key-value pair using a reference
		bool Delete(JSON_NODE* reference);
		// Create JSON from array
		char* Generate(const char* format);
		// Format for the array, this will override the format parameter passed to JSON_Generate
		bool Format(const char* format);
		// Parse a JSON string and create a node tree
		JSON_NODE* Parse(const char* json, JSON_PARSER_CONTEXT* context);
	};
}

#endif // !JSON_PLUS_H
