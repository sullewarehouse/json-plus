
//
// json-plus.cpp
// 
// Author:
//     Brian Sullender
//     SULLE WAREHOUSE LLC
// 
// Description:
//     The source file for the json_plus namespace.
//     https://github.com/sullewarehouse/json-plus
//

#include <stdio.h>
#include <setjmp.h>
#include <stdlib.h>
#include <memory.h>
#include <cstring>

#include "json-plus.h"
using namespace json_plus;

// The number of 'char' units to add to a string buffer size when the buffer is too small
// Increasing this number may result in faster parsing but will use more memory
#define JSON_PARSER_BUFFER_INCREASE 32

// The number of 'char' units to add to the generator buffer size when the buffer is too small
// Increasing this number may result in faster encoding but will use more memory
#define JSON_GENERATOR_BUFFER_INCREASE 32

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
	LITERAL,
	JSON_END,
	UNRECOGNIZED_TOKEN
};

// JSON error strings
static const char* JSON_ERROR_STRINGS[] =
{
	"none",

	// general errors:

	"invalid paramter.",
	"out of memory.",

	// parse errors:

	"unrecognized token.",
	"unexpected token, json must start with an object or array; '{' or '[' tokens.",

	// parse object errors:

	"object syntax error, expected a ':' token before value.",
	"object syntax error, key already defined, expected a ':' token and value.",
	"object syntax error, key not defined.",
	"unexpected closing square bracket ']' token, use closing curly bracket '}' instead to close the object.",
	"expected object closing curly bracket '}' token, encountered end of json instead.",
	"expected key-value pair, encountered end of object instead.",

	// parse array errors:

	"unexpected value in array, use the comma ',' token to separate values.",
	"unexpected key-value colon ':' token, key-value pair illegal in array, use key-value pairs in object.",
	"unexpected closing curly bracket '}' token, use closing square bracket ']' instead to close the array.",
	"expected array closing square bracket ']' token, encountered end of json instead.",
	"expected array value, encountered end of array instead.",

	// parse string errors:

	"quotation mark, reverse solidus, and the control characters(0x00 - 0x1F) must be escaped.",
	"use (n, r, t, f, b) for control characters (line feed, carriage return, tab, form feed, backspace) respectively.",
	"quotation mark, reverse solidus, or control character must follow a reverse solidus character.",
	"expected string closing double quotes \" token, encountered end of json instead.",

	// parse literal name errors:

	"invalid literal name, only 'false', 'null' and 'true' are valid (lowercase only)."
};

// JSON generator context
struct JSON_GENERATOR_CONTEXT
{
	bool error;
	char* buffer;
	size_t bufferLength;
	size_t index;
	bool visualEscape;
	const char* format;
	long indentation;
	jmp_buf env;
};

// JSON parser context initializer
JSON_PARSER_CONTEXT::JSON_PARSER_CONTEXT()
{
	this->errorCode = JSON_ERROR_CODE::NONE;
	this->errorDescription = JSON_ERROR_STRINGS[(int)JSON_ERROR_CODE::NONE];
	this->visualEscapeOnly = false;
	this->charNumber = 0;
	this->lineNumber = 0;
	this->beginIndex = 0;
	this->errorLength = 0;
}

// ---------------------------- //
// **   _JSON_NODE methods   ** //
// ---------------------------- //

const char* _JSON_NODE::String()
{
	if (this->type == JSON_TYPE::STRING) {
		return (const char*)this->value;
	}

	return NULL;
}

bool _JSON_NODE::Boolean()
{
	if (this->type == JSON_TYPE::BOOLEAN) {
		return (bool)this->value;
	}

	return false;
}

double _JSON_NODE::Double()
{
	if (this->type == JSON_TYPE::NUMBER) {
		return atof((char*)this->value);
	}

	return 0.0f;
}

int _JSON_NODE::Int()
{
	if (this->type == JSON_TYPE::NUMBER) {
		return atoi((char*)this->value);
	}

	return 0;
}

long _JSON_NODE::Long()
{
	if (this->type == JSON_TYPE::NUMBER) {
		return atol((char*)this->value);
	}

	return 0;
}

long long _JSON_NODE::Int64()
{
	if (this->type == JSON_TYPE::NUMBER) {
		return atoll((char*)this->value);
	}

	return 0;
}

void json_free_node(JSON_NODE* node)
{
	if (node->type == JSON_TYPE::OBJECT) {
		JSON_Free((JSON_NODE*)node->value);
	}
	else if (node->type == JSON_TYPE::ARRAY) {
		JSON_Free((JSON_NODE*)node->value);
	}
	else
	{
		if (node->type != JSON_TYPE::BOOLEAN) {
			if (node->value) {
				free(node->value);
			}
		}
	}

	if (node->key) {
		free(node->key);
	}

	if (node->format) {
		free((void*)node->format);
	}

	free(node);
}

// ------------------------ //
// **   JSON generator   ** //
// ------------------------ //

// Forward declaration of json_GeneratorIndentation
// Output tabs for a line, this is automatically enabled when a format string is provided
void json_GeneratorIndentation(JSON_GENERATOR_CONTEXT* context);

// ---------------------------------- //
// **   JSON generator functions   ** //
// ---------------------------------- //

// Append a new character to the JSON string buffer
// If `bFormat` is true then the function checks formatting parameters
void json_GeneratorAppend(JSON_GENERATOR_CONTEXT* context, unsigned long CodePoint, bool bFormat)
{
	unsigned char CharUnits;
	unsigned long newLines;
	const char* pFormat = NULL;

	if ((bFormat) && (context->format))
	{
		if (CodePoint == '}') {
			context->indentation--;
		}

		newLines = 0;

		pFormat = context->format;
		while (*pFormat != '\0')
		{
			if (*pFormat == '\n') {
				newLines++;
			}
			else if (*pFormat == CodePoint) {
				for (unsigned long i = 0; i < newLines; i++) {
					json_GeneratorAppend(context, '\n', false);
					json_GeneratorIndentation(context);
				}
				pFormat++;
				break;
			}
			else {
				newLines = 0;
			}
			pFormat++;
		}
	}

	CharUnits = UTF8_Encoding::EncodeUnsafe(NULL, CodePoint);

	// buffer big enough for CodePoint + NULL character ?
	if ((context->index + CharUnits + 1) >= context->bufferLength)
	{
		context->bufferLength += JSON_GENERATOR_BUFFER_INCREASE;
		char* pNewBuffer = (char*)realloc(context->buffer, context->bufferLength);
		if (pNewBuffer != NULL) {
			context->buffer = pNewBuffer;
		}
		else {
			context->error = true;
			longjmp(context->env, 1);
		}
	}

	context->index += UTF8_Encoding::EncodeUnsafe(&context->buffer[context->index], CodePoint);

	if ((bFormat) && (pFormat != NULL))
	{
		if (CodePoint == '{') {
			context->indentation++;
		}

		while ((*pFormat != '\0') && (*pFormat != 'e'))
		{
			json_GeneratorAppend(context, *pFormat, false);
			if (*pFormat == '\n') {
				json_GeneratorIndentation(context);
			}
			pFormat++;
		}
	}
}

void json_GeneratorIndentation(JSON_GENERATOR_CONTEXT* context)
{
	for (long i = 0; i < context->indentation; i++)
	{
		json_GeneratorAppend(context, '\t', false);
	}
}

// Recursive JSON text generator (from node)
char* json_GenerateText(JSON_NODE* json_node, JSON_GENERATOR_CONTEXT* context)
{
	unsigned char CharUnits;
	unsigned long CodePoint;
	JSON_NODE* node;
	const char* format;
	long p = -1, n = 0;
	const char* extraChars;

	format = NULL;

	// Look for special 'p' formatting character
	if (context->format != NULL)
	{
		format = context->format;

		while (*format != '\0')
		{
			if (*format == 'p') {
				p = 0;
				format++;

				// Get number of key-value pairs for a single line
				while ((*format >= '0') && (*format <= '9')) {
					p *= 10;
					p += (*format - '0');
					format++;
				}
				break;
			}
			format++;
		}
	}

	node = json_node;
	while (node != NULL)
	{
		if (node->key != NULL)
		{
			json_GeneratorAppend(context, '"', true);

			const char* pKey = node->key;
			CharUnits = UTF8_Encoding::GetCharacterUnits(*pKey);
			CodePoint = UTF8_Encoding::Decode(CharUnits, pKey);

			while (CodePoint != '\0')
			{
				// Escape special characters
				switch (CodePoint)
				{
				case 0x22: // " quotation mark
				case 0x5C: // \ reverse solidus
					json_GeneratorAppend(context, '\\', false);
					json_GeneratorAppend(context, CodePoint, false);
					break;
				case 0x08: // b backspace
					json_GeneratorAppend(context, '\\', false);
					if (context->visualEscape) {
						json_GeneratorAppend(context, 'b', false);
					}
					else {
						json_GeneratorAppend(context, CodePoint, false);
					}
					break;
				case 0x0C: // f form feed
					json_GeneratorAppend(context, '\\', false);
					if (context->visualEscape) {
						json_GeneratorAppend(context, 'f', false);
					}
					else {
						json_GeneratorAppend(context, CodePoint, false);
					}
					break;
				case 0x0A: // n line feed
					json_GeneratorAppend(context, '\\', false);
					if (context->visualEscape) {
						json_GeneratorAppend(context, 'n', false);
					}
					else {
						json_GeneratorAppend(context, CodePoint, false);
					}
					break;
				case 0x0D: // r carriage return
					json_GeneratorAppend(context, '\\', false);
					if (context->visualEscape) {
						json_GeneratorAppend(context, 'r', false);
					}
					else {
						json_GeneratorAppend(context, CodePoint, false);
					}
					break;
				case 0x09: // t tab
					json_GeneratorAppend(context, '\\', false);
					if (context->visualEscape) {
						json_GeneratorAppend(context, 't', false);
					}
					else {
						json_GeneratorAppend(context, CodePoint, false);
					}
					break;
				default:
					json_GeneratorAppend(context, CodePoint, false);
					break;
				}

				pKey += CharUnits;

				CharUnits = UTF8_Encoding::GetCharacterUnits(*pKey);
				CodePoint = UTF8_Encoding::Decode(CharUnits, pKey);
			}

			json_GeneratorAppend(context, '"', true);
			json_GeneratorAppend(context, ':', true);
		}

		if (node->type == JSON_TYPE::OBJECT)
		{
			format = NULL;
			if (node->format) {
				format = context->format;
				context->format = node->format;
			}

			json_GeneratorAppend(context, '{', true);
			json_GenerateText((JSON_NODE*)node->value, context);
			json_GeneratorAppend(context, '}', true);

			if (format) {
				context->format = format;
			}
		}
		else if (node->type == JSON_TYPE::ARRAY)
		{
			format = NULL;
			if (node->format) {
				format = context->format;
				context->format = node->format;
			}

			json_GeneratorAppend(context, '[', true);
			json_GenerateText((JSON_NODE*)node->value, context);
			json_GeneratorAppend(context, ']', true);

			if (format) {
				context->format = format;
			}
		}
		else if (node->type == JSON_TYPE::STRING)
		{
			json_GeneratorAppend(context, '"', true);

			const char* pValue = (const char*)node->value;
			CharUnits = UTF8_Encoding::GetCharacterUnits(*pValue);
			CodePoint = UTF8_Encoding::Decode(CharUnits, pValue);

			while (CodePoint != '\0')
			{
				// Escape special characters
				switch (CodePoint)
				{
				case 0x22: // " quotation mark
				case 0x5C: // \ reverse solidus
					json_GeneratorAppend(context, '\\', false);
					json_GeneratorAppend(context, CodePoint, false);
					break;
				case 0x08: // b backspace
					json_GeneratorAppend(context, '\\', false);
					if (context->visualEscape) {
						json_GeneratorAppend(context, 'b', false);
					}
					else {
						json_GeneratorAppend(context, CodePoint, false);
					}
					break;
				case 0x0C: // f form feed
					json_GeneratorAppend(context, '\\', false);
					if (context->visualEscape) {
						json_GeneratorAppend(context, 'f', false);
					}
					else {
						json_GeneratorAppend(context, CodePoint, false);
					}
					break;
				case 0x0A: // n line feed
					json_GeneratorAppend(context, '\\', false);
					if (context->visualEscape) {
						json_GeneratorAppend(context, 'n', false);
					}
					else {
						json_GeneratorAppend(context, CodePoint, false);
					}
					break;
				case 0x0D: // r carriage return
					json_GeneratorAppend(context, '\\', false);
					if (context->visualEscape) {
						json_GeneratorAppend(context, 'r', false);
					}
					else {
						json_GeneratorAppend(context, CodePoint, false);
					}
					break;
				case 0x09: // t tab
					json_GeneratorAppend(context, '\\', false);
					if (context->visualEscape) {
						json_GeneratorAppend(context, 't', false);
					}
					else {
						json_GeneratorAppend(context, CodePoint, false);
					}
					break;
				default:
					json_GeneratorAppend(context, CodePoint, false);
					break;
				}

				pValue += CharUnits;

				CharUnits = UTF8_Encoding::GetCharacterUnits(*pValue);
				CodePoint = UTF8_Encoding::Decode(CharUnits, pValue);
			}

			json_GeneratorAppend(context, '"', true);
		}
		else if (node->type == JSON_TYPE::NUMBER)
		{
			const char* pValue = (const char*)node->value;
			CharUnits = UTF8_Encoding::GetCharacterUnits(*pValue);
			CodePoint = UTF8_Encoding::Decode(CharUnits, pValue);

			while (CodePoint != '\0')
			{
				json_GeneratorAppend(context, CodePoint, false);

				pValue += CharUnits;

				CharUnits = UTF8_Encoding::GetCharacterUnits(*pValue);
				CodePoint = UTF8_Encoding::Decode(CharUnits, pValue);
			}
		}
		else if (node->type == JSON_TYPE::BOOLEAN)
		{
			bool bValue = (bool)node->value;

			if (bValue) {
				json_GeneratorAppend(context, 't', false);
				json_GeneratorAppend(context, 'r', false);
				json_GeneratorAppend(context, 'u', false);
				json_GeneratorAppend(context, 'e', false);
			}
			else {
				json_GeneratorAppend(context, 'f', false);
				json_GeneratorAppend(context, 'a', false);
				json_GeneratorAppend(context, 'l', false);
				json_GeneratorAppend(context, 's', false);
				json_GeneratorAppend(context, 'e', false);
			}
		}

		node = node->next;
		if (node != NULL)
		{
			json_GeneratorAppend(context, ',', true);

			if (p != -1) {
				n++;
				if (n >= p) {
					json_GeneratorAppend(context, '\n', true);
					json_GeneratorIndentation(context);

					extraChars = format;
					while ((*extraChars != '\0') && (*extraChars != 'e')) {
						json_GeneratorAppend(context, *extraChars, true);
						extraChars++;
					}
					n = 0;
				}
			}
		}
	}

	return NULL;
}

// ------------------------------ //
// **   Forward declarations   ** //
// ------------------------------ //

// Parse JSON string
char* json_ParseString(char** pp_json, JSON_PARSER_CONTEXT* context);

// Parse JSON number
char* json_ParseNumber(char** pp_json, JSON_PARSER_CONTEXT* context);

// Parse JSON literal
bool json_ParseLiteral(char** pp_json, JSON_PARSER_CONTEXT* context, JSON_TYPE* pType);

// Parse JSON array
JSON_NODE* json_ParseArray(char** pp_json, JSON_PARSER_CONTEXT* context);

// Parse JSON object
JSON_NODE* json_ParseObject(char** pp_json, JSON_PARSER_CONTEXT* context);

// --------------------------------------- //
// **   Internal JSON parse functions   ** //
// --------------------------------------- //

// Get the next token in the JSON string
JSON_TOKEN json_GetToken(char** pp_json, JSON_PARSER_CONTEXT* context)
{
	const char* pJson;
	unsigned char CharUnits;
	unsigned long CodePoint;
	JSON_TOKEN token;

	pJson = *pp_json;
	token = JSON_TOKEN::UNRECOGNIZED_TOKEN;

getCodePoint:

	CharUnits = UTF8_Encoding::GetCharacterUnits(*pJson);
	CodePoint = UTF8_Encoding::Decode(CharUnits, pJson);

	if (CodePoint == ' ')
	{
		pJson += CharUnits;
		context->charNumber++;
		goto getCodePoint;
	}

	if (CodePoint == '\t')
	{
		pJson += CharUnits;
		context->charNumber++;
		goto getCodePoint;
	}

	if (CodePoint == '\n')
	{
		pJson += CharUnits;
		context->lineNumber++;
		context->charNumber++;
		goto getCodePoint;
	}

	if (CodePoint == '\r')
	{
		pJson += CharUnits;
		context->charNumber++;
		goto getCodePoint;
	}

	if (CodePoint == '\"')
	{
		token = JSON_TOKEN::STRING;
		pJson += CharUnits;
		context->charNumber++;
	}
	else if (CodePoint == '{')
	{
		token = JSON_TOKEN::CURLY_OPEN;
		pJson += CharUnits;
		context->charNumber++;
	}
	else if (CodePoint == '}')
	{
		token = JSON_TOKEN::CURLY_CLOSE;
		pJson += CharUnits;
		context->charNumber++;
	}
	else if ((CodePoint == '-') || ((CodePoint >= '0') && (CodePoint <= '9')))
	{
		token = JSON_TOKEN::NUMBER;
	}
	else if (((CodePoint >= 'A') && (CodePoint <= 'Z')) || ((CodePoint >= 'a') && (CodePoint <= 'z')))
	{
		token = JSON_TOKEN::LITERAL;
	}
	else if (CodePoint == '[')
	{
		token = JSON_TOKEN::ARRAY_OPEN;
		pJson += CharUnits;
		context->charNumber++;
	}
	else if (CodePoint == ']')
	{
		token = JSON_TOKEN::ARRAY_CLOSE;
		pJson += CharUnits;
		context->charNumber++;
	}
	else if (CodePoint == ':')
	{
		token = JSON_TOKEN::COLON;
		pJson += CharUnits;
		context->charNumber++;
	}
	else if (CodePoint == ',')
	{
		token = JSON_TOKEN::COMMA;
		pJson += CharUnits;
		context->charNumber++;
	}
	else if (CodePoint == 0)
	{
		token = JSON_TOKEN::JSON_END;
	}
	else
	{
		context->charNumber++;
	}

	*pp_json = (char*)pJson;

	return token;
}

// Parse a JSON string (key or value)
char* json_ParseString(char** pp_json, JSON_PARSER_CONTEXT* context)
{
	size_t i;
	unsigned char CharUnits;
	unsigned long CodePoint;
	const char* pJson;
	bool bEscape;
	char* buffer;
	size_t bufferLength;
	char* pNewBuffer;

	pJson = *pp_json;
	bEscape = false;

	// bufferLength = 0;
	// buffer = NULL;

	// Allocating here makes the compiler happy but is not actually needed
	bufferLength = JSON_PARSER_BUFFER_INCREASE;
	buffer = (char*)malloc(bufferLength);
	if (buffer == NULL) {
		context->errorCode = JSON_ERROR_CODE::OUT_OF_MEMORY;
		context->errorDescription = JSON_ERROR_STRINGS[(int)context->errorCode];
		return NULL;
	}

	i = 0;

	while (true)
	{
		context->beginIndex = context->charNumber;

		CharUnits = UTF8_Encoding::GetCharacterUnits(*pJson);
		CodePoint = UTF8_Encoding::Decode(CharUnits, pJson);

		if (!bEscape)
		{
			// Escape character?
			if (CodePoint == '\\')
			{
				bEscape = true;
				pJson += CharUnits;
				context->charNumber++;
				continue;
			}

			// End of string?
			if (CodePoint == '\"')
			{
				pJson += CharUnits;
				context->charNumber++;
				break;
			}

			// All Unicode characters may be placed within the
			//	 quotation marks, except for the characters that MUST be escaped :
			// quotation mark, reverse solidus, and the control characters(U + 0000
			//	 through U + 001F).

			if (((CodePoint >= 0x0000) && (CodePoint <= 0x001F)) ||
				(CodePoint == '"') || (CodePoint == '\\'))
			{
				context->errorCode = JSON_ERROR_CODE::STRING_CHARACTERS_MUST_BE_ESCAPED;
				context->charNumber++;
				break;
			}
		}
		else
		{
			// Convert visual representation of control characters
			if (CodePoint == 'n') {
				CodePoint = 0x0A; // line feed
			}
			else if (CodePoint == 'r') {
				CodePoint = 0x0D; // carriage return
			}
			else if (CodePoint == 't') {
				CodePoint = 0x09; // tab
			}
			else if (CodePoint == 'f') {
				CodePoint = 0x0C; // form feed
			}
			else if (CodePoint == 'b') {
				CodePoint = 0x08; // backspace
			}
			else
			{
				// Force strict string escaping for code editor?
				if (context->visualEscapeOnly == true)
				{
					if ((CodePoint == 0x0A) || (CodePoint == 0x0D) ||
						(CodePoint == 0x09) || (CodePoint == 0x0C) || (CodePoint == 0x08)) {
						context->errorCode = JSON_ERROR_CODE::STRING_FORCED_STRICT_ESCAPING;
						context->charNumber++;
						break;
					}
				}
				// Must be a quotation mark, reverse solidus, or control character
				if ((CodePoint > 0x001F) && (CodePoint != '"') && (CodePoint != '\\')) {
					context->errorCode = JSON_ERROR_CODE::STRING_UNUSED_ESCAPE_CHARACTER;
					context->charNumber++;
					break;
				}
			}
			bEscape = false;
		}

		if (CodePoint == '\0') {
			context->errorCode = JSON_ERROR_CODE::EXPECTED_DOUBLE_QUOTES_ENCOUNTERED_JSON_END;
			context->charNumber++;
			break;
		}

		// buffer big enough for CodePoint + NULL character ?
		if ((i + CharUnits + 1) > bufferLength)
		{
			bufferLength += JSON_PARSER_BUFFER_INCREASE;
			pNewBuffer = (char*)realloc(buffer, bufferLength);
			if (pNewBuffer != NULL) {
				buffer = pNewBuffer;
			}
			else {
				context->errorCode = JSON_ERROR_CODE::OUT_OF_MEMORY;
				break;
			}
		}

		UTF8_Encoding::EncodeUnsafe(&buffer[i], CodePoint);

		pJson += CharUnits;
		context->charNumber++;
		i += CharUnits;
	}

	if (context->errorCode != JSON_ERROR_CODE::NONE) {
		if (buffer != NULL) {
			free(buffer);
		}
		return NULL;
	}

	buffer[i] = '\0';
	*pp_json = (char*)pJson;

	return buffer;
}

// Parse a JSON number, we return the number as an individual string to avoid type assumptions
char* json_ParseNumber(char** pp_json, JSON_PARSER_CONTEXT* context)
{
	unsigned char CharUnits;
	unsigned long CodePoint;
	const char* pJson;
	size_t strLen;
	char* result;

	pJson = *pp_json;
	context->beginIndex = context->charNumber;

	CharUnits = UTF8_Encoding::GetCharacterUnits(*pJson);
	CodePoint = UTF8_Encoding::Decode(CharUnits, pJson);

	strLen = 0;
	while ((CodePoint == '-') || ((CodePoint >= '0') && (CodePoint <= '9')) || (CodePoint == '.'))
	{
		pJson += CharUnits;
		CharUnits = UTF8_Encoding::GetCharacterUnits(*pJson);
		CodePoint = UTF8_Encoding::Decode(CharUnits, pJson);
		strLen += CharUnits;
		context->charNumber++;
	}

	result = (char*)malloc(strLen + 1);
	if (result == 0) {
		return 0;
	}

	memcpy(result, *pp_json, strLen);
	result[strLen] = 0;

	*pp_json += strLen;

	return result;
}

// JSON literal names are: "false", "null" and "true"
// The literal names MUST be lowercase.  No other literal names are allowed.
bool json_ParseLiteral(char** pp_json, JSON_PARSER_CONTEXT* context, JSON_TYPE* pType)
{
	const char* pJson;
	unsigned char CharUnits;
	unsigned long CodePoint;
	size_t wordLength;

	pJson = *pp_json;
	context->beginIndex = context->charNumber;

	wordLength = 0;
	bool bValue = false;

	while (true)
	{
		CharUnits = UTF8_Encoding::GetCharacterUnits(*pJson);
		CodePoint = UTF8_Encoding::Decode(CharUnits, pJson);

		if (((CodePoint >= 'A') && (CodePoint <= 'Z')) ||
			((CodePoint >= 'a') && (CodePoint <= 'z')) ||
			((CodePoint >= '0') && (CodePoint <= '9')) ||
			(CodePoint == '_'))
		{
			wordLength += CharUnits;

			context->charNumber++;
			pJson += CharUnits;
		}
		else {
			break;
		}
	}

	if (strncmp(*pp_json, "false", wordLength) == 0) {
		*pType = JSON_TYPE::BOOLEAN;
	}
	else if (strncmp(*pp_json, "true", wordLength) == 0) {
		*pType = JSON_TYPE::BOOLEAN;
		bValue = true;
	}
	else if (strncmp(*pp_json, "null", wordLength) == 0) {
		*pType = JSON_TYPE::NULL_TYPE;
	}
	else {
		context->errorCode = JSON_ERROR_CODE::INVALID_LITERAL_NAME;
	}

	*pp_json = (char*)pJson;

	return bValue;
}

// Parse a JSON array
JSON_NODE* json_ParseArray(char** pp_json, JSON_PARSER_CONTEXT* context)
{
	JSON_TOKEN token;
	JSON_NODE* root, * node, * prev_node;
	const char* pJson;
	bool hasCompleted;

	root = node = prev_node = 0;
	hasCompleted = false;

	pJson = *pp_json;

	while (!hasCompleted)
	{
		context->beginIndex = context->charNumber;
		token = json_GetToken((char**)&pJson, context);

		switch (token)
		{
		case JSON_TOKEN::CURLY_CLOSE:
			context->errorCode = JSON_ERROR_CODE::UNEXPECTED_CLOSING_CURLY_BRACKET;
			break;
		case JSON_TOKEN::COLON:
			context->errorCode = JSON_ERROR_CODE::UNEXPECTED_PAIR_COLON_TOKEN;
			break;
		case JSON_TOKEN::CURLY_OPEN:
			if (node)
			{
				context->errorCode = JSON_ERROR_CODE::UNEXPECTED_ARRAY_VALUE;
				break;
			}
			else
			{
				node = (JSON_NODE*)malloc(sizeof(JSON_NODE));
				if (!node)
				{
					context->errorCode = JSON_ERROR_CODE::OUT_OF_MEMORY;
					break;
				}

				memset(node, 0, sizeof(JSON_NODE));
				node->type = JSON_TYPE::OBJECT;
				node->value = json_ParseObject((char**)&pJson, context);
				node->format = NULL;

				if (prev_node) {
					prev_node->next = node;
				}
			}
			break;
		case JSON_TOKEN::STRING:
			if (node)
			{
				context->errorCode = JSON_ERROR_CODE::UNEXPECTED_ARRAY_VALUE;
				break;
			}
			else
			{
				node = (JSON_NODE*)malloc(sizeof(JSON_NODE));
				if (!node)
				{
					context->errorCode = JSON_ERROR_CODE::OUT_OF_MEMORY;
					break;
				}

				memset(node, 0, sizeof(JSON_NODE));
				node->type = JSON_TYPE::STRING;
				node->value = json_ParseString((char**)&pJson, context);
				node->format = NULL;

				if (prev_node) {
					prev_node->next = node;
				}
			}
			break;
		case JSON_TOKEN::NUMBER:
			if (node)
			{
				context->errorCode = JSON_ERROR_CODE::UNEXPECTED_ARRAY_VALUE;
				break;
			}
			else
			{
				node = (JSON_NODE*)malloc(sizeof(JSON_NODE));
				if (!node)
				{
					context->errorCode = JSON_ERROR_CODE::OUT_OF_MEMORY;
					break;
				}

				memset(node, 0, sizeof(JSON_NODE));
				node->type = JSON_TYPE::NUMBER;
				node->value = json_ParseNumber((char**)&pJson, context);
				node->format = NULL;

				if (prev_node) {
					prev_node->next = node;
				}
			}
			break;
		case JSON_TOKEN::LITERAL:
			if (node)
			{
				context->errorCode = JSON_ERROR_CODE::UNEXPECTED_ARRAY_VALUE;
				break;
			}
			else
			{
				node = (JSON_NODE*)malloc(sizeof(JSON_NODE));
				if (!node)
				{
					context->errorCode = JSON_ERROR_CODE::OUT_OF_MEMORY;
					break;
				}

				memset(node, 0, sizeof(JSON_NODE));
				node->value = (void*)json_ParseLiteral((char**)&pJson, context, &node->type);
				node->format = NULL;

				if (prev_node) {
					prev_node->next = node;
				}
			}
			break;
		case JSON_TOKEN::ARRAY_OPEN:
			if (node)
			{
				context->errorCode = JSON_ERROR_CODE::UNEXPECTED_ARRAY_VALUE;
				break;
			}
			else
			{
				node = (JSON_NODE*)malloc(sizeof(JSON_NODE));
				if (!node)
				{
					context->errorCode = JSON_ERROR_CODE::OUT_OF_MEMORY;
					break;
				}

				memset(node, 0, sizeof(JSON_NODE));
				node->type = JSON_TYPE::ARRAY;
				node->value = json_ParseArray((char**)&pJson, context);
				node->format = NULL;

				if (prev_node) {
					prev_node->next = node;
				}
			}
			break;
		case JSON_TOKEN::ARRAY_CLOSE:
			hasCompleted = true;
			break;
		case JSON_TOKEN::COMMA:
			prev_node = node;
			node = 0;
			break;
		case JSON_TOKEN::JSON_END:
			context->errorCode = JSON_ERROR_CODE::EXPECTED_SQUARE_BRACKET_ENCOUNTERED_JSON_END;
			break;
		case JSON_TOKEN::UNRECOGNIZED_TOKEN:
			context->errorCode = JSON_ERROR_CODE::UNRECOGNIZED_TOKEN;
			break;
		default:
			break;
		}

		if ((!root) && (node)) {
			root = node;
		}

		if (context->errorCode != JSON_ERROR_CODE::NONE) {
			context->errorDescription = JSON_ERROR_STRINGS[(int)context->errorCode];
			context->errorLength = context->charNumber - context->beginIndex;
			break;
		}
	}

	*pp_json = (char*)pJson;

	return root;
}

// Parse a JSON object
JSON_NODE* json_ParseObject(char** pp_json, JSON_PARSER_CONTEXT* context)
{
	JSON_TOKEN token;
	JSON_NODE* root, * node, * prev_node;
	const char* pJson;
	bool isKey;
	bool hasCompleted;

	root = node = prev_node = 0;
	isKey = true;
	hasCompleted = false;

	pJson = *pp_json;

	while (!hasCompleted)
	{
		context->beginIndex = context->charNumber;
		token = json_GetToken((char**)&pJson, context);

		switch (token)
		{
		case JSON_TOKEN::CURLY_CLOSE:
			hasCompleted = true;
			break;
		case JSON_TOKEN::COLON:
			isKey = false;
			break;
		case JSON_TOKEN::CURLY_OPEN:
			if (!node)
			{
				context->errorCode = JSON_ERROR_CODE::OBJECT_SYNTAX_ERROR_KEY_NOT_DEFINED;
				break;
			}
			else
			{
				if (isKey) {
					context->errorCode = JSON_ERROR_CODE::OBJECT_SYNTAX_ERROR_EXPECTED_COLON;
					break;
				}
				node->type = JSON_TYPE::OBJECT;
				node->value = json_ParseObject((char**)&pJson, context);
			}
			break;
		case JSON_TOKEN::STRING:
			if (!isKey)
			{
				node->type = JSON_TYPE::STRING;
				node->value = json_ParseString((char**)&pJson, context);
			}
			else
			{
				if (node)
				{
					context->errorCode = JSON_ERROR_CODE::OBJECT_SYNTAX_ERROR_KEY_ALREADY_DEFINED;
					break;
				}
				else
				{
					node = (JSON_NODE*)malloc(sizeof(JSON_NODE));
					if (!node)
					{
						context->errorCode = JSON_ERROR_CODE::OUT_OF_MEMORY;
						break;
					}

					memset(node, 0, sizeof(JSON_NODE));
					node->key = json_ParseString((char**)&pJson, context);
					node->format = NULL;

					if (prev_node) {
						prev_node->next = node;
					}
				}
			}
			break;
		case JSON_TOKEN::NUMBER:
			if (!node)
			{
				context->errorCode = JSON_ERROR_CODE::OBJECT_SYNTAX_ERROR_KEY_NOT_DEFINED;
				break;
			}
			else
			{
				if (isKey) {
					context->errorCode = JSON_ERROR_CODE::OBJECT_SYNTAX_ERROR_EXPECTED_COLON;
					break;
				}
				node->type = JSON_TYPE::NUMBER;
				node->value = json_ParseNumber((char**)&pJson, context);
			}
			break;
		case JSON_TOKEN::LITERAL:
			if (!node)
			{
				context->errorCode = JSON_ERROR_CODE::OBJECT_SYNTAX_ERROR_KEY_NOT_DEFINED;
				break;
			}
			else
			{
				if (isKey) {
					context->errorCode = JSON_ERROR_CODE::OBJECT_SYNTAX_ERROR_EXPECTED_COLON;
					break;
				}
				node->value = (void*)json_ParseLiteral((char**)&pJson, context, &node->type);
			}
			break;
		case JSON_TOKEN::ARRAY_OPEN:
			if (!node)
			{
				context->errorCode = JSON_ERROR_CODE::OBJECT_SYNTAX_ERROR_KEY_NOT_DEFINED;
				break;
			}
			else
			{
				if (isKey) {
					context->errorCode = JSON_ERROR_CODE::OBJECT_SYNTAX_ERROR_EXPECTED_COLON;
					break;
				}
				node->type = JSON_TYPE::ARRAY;
				node->value = json_ParseArray((char**)&pJson, context);
			}
			break;
		case JSON_TOKEN::ARRAY_CLOSE:
			context->errorCode = JSON_ERROR_CODE::UNEXPECTED_CLOSING_SQUARE_BRACKET;
			break;
		case JSON_TOKEN::COMMA:
			prev_node = node;
			node = 0;
			isKey = true;
			break;
		case JSON_TOKEN::JSON_END:
			context->errorCode = JSON_ERROR_CODE::EXPECTED_CURLY_BRACKET_ENCOUNTERED_JSON_END;
			break;
		case JSON_TOKEN::UNRECOGNIZED_TOKEN:
			context->errorCode = JSON_ERROR_CODE::UNRECOGNIZED_TOKEN;
			break;
		default:
			break;
		}

		if ((!root) && (node)) {
			root = node;
		}

		if (context->errorCode != JSON_ERROR_CODE::NONE) {
			context->errorDescription = JSON_ERROR_STRINGS[(int)context->errorCode];
			context->errorLength = context->charNumber - context->beginIndex;
			break;
		}
	}

	*pp_json = (char*)pJson;

	return root;
}

// ------------------------ //
// **   JSON functions   ** //
// ------------------------ //

char* json_plus::JSON_Generate(JSON_NODE* json_root, const char* format)
{
	JSON_GENERATOR_CONTEXT context = { 0 };

	if (json_root == NULL) {
		return NULL;
	}

	context.error = false;
	context.buffer = NULL;
	context.bufferLength = 0;
	context.index = 0;
	context.visualEscape = false;
	context.format = format;
	context.indentation = 0;

	if (format != NULL) {
		if (*format == 'c') {
			context.visualEscape = true;
		}
	}

	if (setjmp(context.env) == 0)
	{
		if (json_root->type == JSON_TYPE::OBJECT)
		{
			json_GeneratorAppend(&context, '{', true);
			json_GenerateText((JSON_NODE*)json_root->value, &context);
			json_GeneratorAppend(&context, '}', true);
			json_GeneratorAppend(&context, '\0', false);
		}
		else if (json_root->type == JSON_TYPE::ARRAY)
		{
			json_GeneratorAppend(&context, '[', true);
			json_GenerateText((JSON_NODE*)json_root->value, &context);
			json_GeneratorAppend(&context, ']', true);
			json_GeneratorAppend(&context, '\0', false);
		}
	}

	return context.buffer;
}

JSON_NODE* json_plus::JSON_Parse(const char* pJson, JSON_PARSER_CONTEXT* context)
{
	JSON_TOKEN token;
	JSON_NODE* root, * node, * prev_node;
	bool hasCompleted;

	if (context == 0) {
		return 0;
	}

	context->lineNumber = 1;
	context->charNumber = 0;
	context->beginIndex = 0;
	context->errorLength = 0;

	if (pJson == 0)
	{
		context->errorCode = JSON_ERROR_CODE::INVALID_PARAMETER;
		context->errorDescription = JSON_ERROR_STRINGS[(int)JSON_ERROR_CODE::INVALID_PARAMETER];
		return 0;
	}

	context->errorCode = JSON_ERROR_CODE::NONE;
	context->errorDescription = JSON_ERROR_STRINGS[(int)JSON_ERROR_CODE::NONE];

	root = node = prev_node = 0;
	hasCompleted = false;

	while (!hasCompleted)
	{
		context->beginIndex = context->charNumber;
		token = json_GetToken((char**)&pJson, context);

		switch (token)
		{
		case JSON_TOKEN::CURLY_OPEN:
			node = (JSON_NODE*)malloc(sizeof(JSON_NODE));
			if (!node)
			{
				context->errorCode = JSON_ERROR_CODE::OUT_OF_MEMORY;
				break;
			}

			memset(node, 0, sizeof(JSON_NODE));
			node->type = JSON_TYPE::OBJECT;
			node->value = json_ParseObject((char**)&pJson, context);
			node->format = NULL;

			if (prev_node) {
				prev_node->next = node;
			}
			break;
		case JSON_TOKEN::ARRAY_OPEN:
			node = (JSON_NODE*)malloc(sizeof(JSON_NODE));
			if (!node)
			{
				context->errorCode = JSON_ERROR_CODE::OUT_OF_MEMORY;
				break;
			}

			memset(node, 0, sizeof(JSON_NODE));
			node->type = JSON_TYPE::ARRAY;
			node->value = json_ParseArray((char**)&pJson, context);
			node->format = NULL;

			if (prev_node) {
				prev_node->next = node;
			}
			break;
		case JSON_TOKEN::JSON_END:
			hasCompleted = true;
			break;
		case JSON_TOKEN::UNRECOGNIZED_TOKEN:
			context->errorCode = JSON_ERROR_CODE::UNRECOGNIZED_TOKEN;
			break;
		default:
			context->errorCode = JSON_ERROR_CODE::UNEXPECTED_START_TOKEN;
			break;
		}

		if ((!root) && (node)) {
			root = node;
		}

		if (node) {
			prev_node = node;
			node = 0;
		}

		if (context->errorCode != JSON_ERROR_CODE::NONE) {
			context->errorDescription = JSON_ERROR_STRINGS[(int)context->errorCode];
			context->errorLength = context->charNumber - context->beginIndex;
			break;
		}
	}

	return root;
}

void json_plus::JSON_Free(JSON_NODE* json_root)
{
	JSON_NODE* node, * nextNode;

	if (json_root == NULL) {
		return;
	}

	node = json_root;
	while (node)
	{
		nextNode = node->next;

		if (node->type == JSON_TYPE::OBJECT) {
			JSON_Free((JSON_NODE*)node->value);
		}
		else if (node->type == JSON_TYPE::ARRAY) {
			JSON_Free((JSON_NODE*)node->value);
		}
		else
		{
			if (node->type != JSON_TYPE::BOOLEAN) {
				if (node->value) {
					free(node->value);
				}
			}
		}

		if (node->key) {
			free(node->key);
		}

		if (node->format) {
			free((void*)node->format);
		}

		free(node);
		node = nextNode;
	}
}

JSON_NODE* json_plus::JSON_GetObject(JSON_NODE* object, const char* key)
{
	JSON_NODE* node;

	if (object == NULL) {
		return NULL;
	}

	node = (JSON_NODE*)object->value;
	while (node != NULL)
	{
		if (node->type == JSON_TYPE::OBJECT)
		{
			if (UTF8_Encoding::CompareStrings(node->key, key) == 0) {
				return node;
			}
		}
		node = node->next;
	}

	return NULL;
}

JSON_NODE* json_plus::JSON_GetArray(JSON_NODE* object, const char* key)
{
	JSON_NODE* node;

	if (object == NULL) {
		return NULL;
	}

	node = (JSON_NODE*)object->value;
	while (node != NULL)
	{
		if (node->type == JSON_TYPE::ARRAY)
		{
			if (UTF8_Encoding::CompareStrings(node->key, key) == 0) {
				return node;
			}
		}
		node = node->next;
	}

	return NULL;
}

char* json_plus::JSON_GetString(JSON_NODE* object, const char* key)
{
	JSON_NODE* node;

	if (object == NULL) {
		return NULL;
	}

	node = (JSON_NODE*)object->value;
	while (node != NULL)
	{
		if (node->type == JSON_TYPE::STRING)
		{
			if (UTF8_Encoding::CompareStrings(node->key, key) == 0) {
				return (char*)node->value;
			}
		}
		node = node->next;
	}

	return NULL;
}

char* json_plus::JSON_GetNumber(JSON_NODE* object, const char* key)
{
	JSON_NODE* node;

	if (object == NULL) {
		return NULL;
	}

	node = (JSON_NODE*)object->value;
	while (node != NULL)
	{
		if (node->type == JSON_TYPE::NUMBER)
		{
			if (UTF8_Encoding::CompareStrings(node->key, key) == 0) {
				return (char*)node->value;
			}
		}
		node = node->next;
	}

	return NULL;
}

bool json_plus::JSON_GetBoolean(JSON_NODE* object, const char* key)
{
	JSON_NODE* node;

	if (object == NULL) {
		return false;
	}

	node = (JSON_NODE*)object->value;
	while (node != NULL)
	{
		if (node->type == JSON_TYPE::BOOLEAN)
		{
			if (UTF8_Encoding::CompareStrings(node->key, key) == 0) {
				return (bool)node->value;
			}
		}
		node = node->next;
	}

	return false;
}

JSON_NODE* json_plus::JSON_CreateNode(JSON_TYPE type, const char* key, void* value)
{
	JSON_NODE* node;

	node = (JSON_NODE*)malloc(sizeof(JSON_NODE));
	if (node != NULL)
	{
		char* node_key;
		void* node_value;

		node_key = NULL;

		if (key != NULL)
		{
			size_t keyLength = UTF8_Encoding::GetStringUnits(key) + 1;
			node_key = (char*)malloc(keyLength);
			if (node_key == NULL) {
				free(node);
				return NULL;
			}

			UTF8_Encoding::StringCopy(node_key, keyLength, key);
		}

		if ((type == JSON_TYPE::STRING) && (value != NULL))
		{
			size_t valueLength = UTF8_Encoding::GetStringUnits((const char*)value) + 1;
			node_value = malloc(valueLength);
			if (node_value == NULL)
			{
				if (node_key != NULL) {
					free(node_key);
				}
				free(node);
				return NULL;
			}

			UTF8_Encoding::StringCopy((char*)node_value, valueLength, (const char*)value);
		}
		else {
			node_value = value;
		}

		node->next = NULL;
		node->key = node_key;
		node->value = node_value;
		node->type = type;
		node->format = NULL;
	}

	return node;
}

// ------------------------------- //
// **   UTF8_Encoding methods   ** //
// ------------------------------- //

unsigned char UTF8_Encoding::GetCharacterUnits(char Code)
{
	if ((Code & 0x80) == 0) {
		return 1;
	}
	else if ((Code & 0xE0) == 0xC0) {
		return 2;
	}
	else if ((Code & 0xF0) == 0xE0) {
		return 3;
	}
	else if ((Code & 0xF8) == 0xF0) {
		return 4;
	}
	else {
		return 0;
	}
}

unsigned char UTF8_Encoding::Encode(char* pBuffer, size_t Length, unsigned long CodePoint)
{
	if (CodePoint < 0x80)
	{
		if ((pBuffer != NULL) && (Length >= 2))
		{
			pBuffer[0] = (char)CodePoint & 0x7F;
			pBuffer[1] = 0;
		}
		return 1;
	}
	else if (CodePoint < 0x800)
	{
		if ((pBuffer != NULL) && (Length >= 3))
		{
			pBuffer[0] = (char)(CodePoint >> 6) | 0xC0;
			pBuffer[1] = (char)(CodePoint & 0x3F) | 0x80;
			pBuffer[2] = 0;
		}
		return 2;
	}
	else if (CodePoint < 0x10000)
	{
		if ((pBuffer != NULL) && (Length >= 4))
		{
			pBuffer[0] = (char)(CodePoint >> 12) | 0xE0;
			pBuffer[1] = (char)((CodePoint >> 6) & 0x3F) | 0x80;
			pBuffer[2] = (char)(CodePoint & 0x3F) | 0x80;
			pBuffer[3] = 0;
		}
		return 3;
	}
	else if (CodePoint < 0x110000)
	{
		if ((pBuffer != NULL) && (Length >= 5))
		{
			pBuffer[0] = (char)(CodePoint >> 18) | 0xF0;
			pBuffer[1] = (char)((CodePoint >> 12) & 0x3F) | 0x80;
			pBuffer[2] = (char)((CodePoint >> 6) & 0x3F) | 0x80;
			pBuffer[3] = (char)(CodePoint & 0x3F) | 0x80;
			pBuffer[4] = 0;
		}
		return 4;
	}

	return 0;
}

unsigned char UTF8_Encoding::EncodeUnsafe(char* pBuffer, unsigned long CodePoint)
{
	if (CodePoint < 0x80)
	{
		if (pBuffer != NULL)
		{
			pBuffer[0] = (char)CodePoint & 0x7F;
		}
		return 1;
	}
	else if (CodePoint < 0x800)
	{
		if (pBuffer != NULL)
		{
			pBuffer[0] = (char)(CodePoint >> 6) | 0xC0;
			pBuffer[1] = (char)(CodePoint & 0x3F) | 0x80;
		}
		return 2;
	}
	else if (CodePoint < 0x10000)
	{
		if (pBuffer != NULL)
		{
			pBuffer[0] = (char)(CodePoint >> 12) | 0xE0;
			pBuffer[1] = (char)((CodePoint >> 6) & 0x3F) | 0x80;
			pBuffer[2] = (char)(CodePoint & 0x3F) | 0x80;
		}
		return 3;
	}
	else if (CodePoint < 0x110000)
	{
		if (pBuffer != NULL)
		{
			pBuffer[0] = (char)(CodePoint >> 18) | 0xF0;
			pBuffer[1] = (char)((CodePoint >> 12) & 0x3F) | 0x80;
			pBuffer[2] = (char)((CodePoint >> 6) & 0x3F) | 0x80;
			pBuffer[3] = (char)(CodePoint & 0x3F) | 0x80;
		}
		return 4;
	}

	return 0;
}

unsigned long UTF8_Encoding::Decode(unsigned char Units, const char* String)
{
	unsigned long CodePoint;

	switch (Units)
	{
	case 1:
		CodePoint = String[0];
		break;
	case 2:
		CodePoint = (String[0] << 6 | (String[1] & 0x3F)) & 0x7FF;
		break;
	case 3:
		CodePoint = (String[0] << 12 | ((String[1] & 0x3F) << 6) | (String[2] & 0x3F)) & 0xFFFF;
		break;
	case 4:
		CodePoint = (String[0] << 18 | ((String[1] & 0x3F) << 12) | ((String[2] & 0x3F) << 6) | (String[3] & 0x3F)) & 0x1FFFFF;
		break;
	default:
		return 0;
	}

	return CodePoint;
}

size_t UTF8_Encoding::GetStringUnits(const char* String)
{
	size_t TotalUnits, CharUnits;

	if (String == NULL) return -1;
	TotalUnits = 0;

	while (*String != 0)
	{
		CharUnits = UTF8_Encoding::GetCharacterUnits(*String);
		if (CharUnits == 0) return TotalUnits;
		String += CharUnits;
		TotalUnits += CharUnits;
	}

	return TotalUnits;
}

size_t UTF8_Encoding::StringCopy(char* Destination, size_t Length, const char* Source)
{
	unsigned char CharUnits;
	size_t Len;

	if ((Destination == NULL) || (Source == NULL) || (Length == 0)) return -1;
	Len = 0;

	while (*Source != 0)
	{
		CharUnits = UTF8_Encoding::GetCharacterUnits(*Source);
		if (CharUnits == 0) break;
		Len += CharUnits;
		if (Len >= Length)
		{
			*Destination = 0;
			return Length;
		}
		switch (CharUnits)
		{
		case 1:
			Destination[0] = Source[0];
			break;
		case 2:
			Destination[0] = Source[0];
			Destination[1] = Source[1];
			break;
		case 3:
			Destination[0] = Source[0];
			Destination[1] = Source[1];
			Destination[2] = Source[2];
			break;
		case 4:
			Destination[0] = Source[0];
			Destination[1] = Source[1];
			Destination[2] = Source[2];
			Destination[3] = Source[3];
			break;
		default:
			break;
		}
		Destination += CharUnits;
		Source += CharUnits;
	}
	*Destination = 0;

	return Len;
}

long UTF8_Encoding::CompareStrings(const char* String1, const char* String2)
{
	unsigned char CharUnits1, CharUnits2;
	unsigned long CodePoint1, CodePoint2;

	if ((String1 == 0) || (String2 == 0)) {
		return 0x7FFFFFFF;
	}

	while ((*String1 != 0) && (*String2 != 0))
	{
		CharUnits1 = UTF8_Encoding::GetCharacterUnits(*String1);
		CharUnits2 = UTF8_Encoding::GetCharacterUnits(*String2);

		if (CharUnits1 == 0) {
			return 0x7FFFFFFF;
		}

		if (CharUnits2 == 0) {
			return 0x7FFFFFFF;
		}

		CodePoint1 = UTF8_Encoding::Decode(CharUnits1, String1);
		CodePoint2 = UTF8_Encoding::Decode(CharUnits2, String2);

		if (CodePoint1 != CodePoint2)
		{
			if (CodePoint1 > CodePoint2) {
				return 1;
			}
			else {
				return -1;
			}
		}
		String1 += CharUnits1;
		String2 += CharUnits2;
	}
	if ((*String1 == 0) && (*String2 == 0)) {
		return 0;
	}
	else
	{
		if (*String1 != 0) {
			return 1;
		}
		else {
			return -1;
		}
	}
}

long UTF8_Encoding::CompareStringsInsensitive(const char* String1, const char* String2)
{
	unsigned char CharUnits1, CharUnits2;
	unsigned long CodePoint1, CodePoint2;

	if ((String1 == 0) || (String2 == 0)) {
		return 0x7FFFFFFF;
	}

	while ((*String1 != 0) && (*String2 != 0))
	{
		CharUnits1 = UTF8_Encoding::GetCharacterUnits(*String1);
		CharUnits2 = UTF8_Encoding::GetCharacterUnits(*String2);

		if (CharUnits1 == 0) {
			return 0x7FFFFFFF;
		}

		if (CharUnits2 == 0) {
			return 0x7FFFFFFF;
		}

		CodePoint1 = UTF8_Encoding::Decode(CharUnits1, String1);
		CodePoint2 = UTF8_Encoding::Decode(CharUnits2, String2);

		if ((CodePoint1 > 0x40) && (CodePoint1 < 0x5B)) {
			CodePoint1 += 0x20;
		}
		else if ((CodePoint1 > 0xFF20) && (CodePoint1 < 0xFF3B)) {
			CodePoint1 -= 0xFEE0;
		}
		else if ((CodePoint1 > 0xFF40) && (CodePoint1 < 0xFF5B)) {
			CodePoint1 -= 0xFF00;
		}

		if ((CodePoint2 > 0x40) && (CodePoint2 < 0x5B)) {
			CodePoint2 += 0x20;
		}
		else if ((CodePoint2 > 0xFF20) && (CodePoint2 < 0xFF3B)) {
			CodePoint2 -= 0xFEE0;
		}
		else if ((CodePoint2 > 0xFF40) && (CodePoint2 < 0xFF5B)) {
			CodePoint2 -= 0xFF00;
		}

		if (CodePoint1 != CodePoint2)
		{
			if (CodePoint1 > CodePoint2) {
				return 1;
			}
			else {
				return -1;
			}
		}
		String1 += CharUnits1;
		String2 += CharUnits2;
	}
	if ((*String1 == 0) && (*String2 == 0)) {
		return 0;
	}
	else
	{
		if (*String1 != 0) {
			return 1;
		}
		else {
			return -1;
		}
	}
}

// ----------------------------- //
// **   JSON_OBJECT methods   ** //
// ----------------------------- //

JSON_OBJECT::JSON_OBJECT() {
	this->json_root = NULL;
}

JSON_OBJECT::JSON_OBJECT(JSON_NODE* root) {
	this->json_root = root;
}

JSON_OBJECT::JSON_OBJECT(const char* json, JSON_PARSER_CONTEXT* context)
{
	this->json_root = JSON_Parse(json, context);
}

JSON_OBJECT& JSON_OBJECT::operator=(const JSON_OBJECT& other)
{
	this->json_root = other.json_root;
	return *this;
}

JSON_OBJECT::operator JSON_NODE* () const
{
	return this->json_root;
}

JSON_NODE* JSON_OBJECT::MakeRoot()
{
	return this->json_root = JSON_CreateNode(JSON_TYPE::OBJECT, NULL, NULL);
}

void JSON_OBJECT::Free()
{
	JSON_Free(this->json_root);
	this->json_root = NULL;
}

bool JSON_OBJECT::Empty() {
	if (this->json_root) {
		return false;
	}
	else {
		return true;
	}
}

unsigned long JSON_OBJECT::Count()
{
	unsigned long count;

	count = 0;
	if (this->json_root != NULL)
	{
		JSON_NODE* node;

		node = (JSON_NODE*)this->json_root->value;
		while (node != NULL)
		{
			node = node->next;
			count++;
		}
	}

	return count;
}

JSON_NODE* JSON_OBJECT::First()
{
	if (this->json_root != NULL) {
		return (JSON_NODE*)this->json_root->value;
	}

	return NULL;
}

JSON_OBJECT JSON_OBJECT::Object(const char* key)
{
	JSON_NODE* node;

	if (this->json_root == NULL) {
		return NULL;
	}

	node = (JSON_NODE*)this->json_root->value;
	while (node != NULL)
	{
		if (node->type == JSON_TYPE::OBJECT)
		{
			if (UTF8_Encoding::CompareStrings(node->key, key) == 0) {
				return node;
			}
		}
		node = node->next;
	}

	return NULL;
}

JSON_ARRAY JSON_OBJECT::Array(const char* key)
{
	JSON_NODE* node;

	if (this->json_root == NULL) {
		return NULL;
	}

	node = (JSON_NODE*)this->json_root->value;
	while (node != NULL)
	{
		if (node->type == JSON_TYPE::ARRAY)
		{
			if (UTF8_Encoding::CompareStrings(node->key, key) == 0) {
				return node;
			}
		}
		node = node->next;
	}

	return NULL;
}

const char* JSON_OBJECT::String(const char* key)
{
	JSON_NODE* node;

	if (this->json_root == NULL) {
		return NULL;
	}

	node = (JSON_NODE*)this->json_root->value;
	while (node != NULL)
	{
		if (node->type == JSON_TYPE::STRING)
		{
			if (UTF8_Encoding::CompareStrings(node->key, key) == 0) {
				return (const char*)node->value;
			}
		}
		node = node->next;
	}

	return NULL;
}

bool JSON_OBJECT::Boolean(const char* key)
{
	JSON_NODE* node;

	if (this->json_root == NULL) {
		return false;
	}

	node = (JSON_NODE*)this->json_root->value;
	while (node != NULL)
	{
		if (node->type == JSON_TYPE::BOOLEAN)
		{
			if (UTF8_Encoding::CompareStrings(node->key, key) == 0) {
				return (bool)node->value;
			}
		}
		node = node->next;
	}

	return false;
}

JSON_OBJECT::Number::Number(JSON_OBJECT& parent) : parent(parent) {}

double JSON_OBJECT::Number::Double(const char* key)
{
	const char* number;

	number = JSON_GetNumber(this->parent.json_root, key);
	if (number != NULL) {
		return atof(number);
	}

	return 0.0f;
}

int JSON_OBJECT::Number::Int(const char* key)
{
	const char* number;

	number = JSON_GetNumber(this->parent.json_root, key);
	if (number != NULL) {
		return atoi(number);
	}

	return 0;
}

long JSON_OBJECT::Number::Long(const char* key)
{
	const char* number;

	number = JSON_GetNumber(this->parent.json_root, key);
	if (number != NULL) {
		return atol(number);
	}

	return 0;
}

long long JSON_OBJECT::Number::Int64(const char* key)
{
	const char* number;

	number = JSON_GetNumber(this->parent.json_root, key);
	if (number != NULL) {
		return atoll(number);
	}

	return 0;
}

const char* JSON_OBJECT::Number::String(const char* key)
{
	return JSON_GetNumber(parent.json_root, key);
}

JSON_OBJECT::Insert::Insert(JSON_OBJECT& parent) : parent(parent) {}

JSON_OBJECT JSON_OBJECT::Insert::Object(const char* key)
{
	JSON_NODE* node;

	node = (JSON_NODE*)malloc(sizeof(JSON_NODE));
	if (node != NULL)
	{
		size_t keyLength = UTF8_Encoding::GetStringUnits(key) + 1;
		char* object_key = (char*)malloc(keyLength);
		if (object_key == NULL) {
			free(node);
			return NULL;
		}

		UTF8_Encoding::StringCopy(object_key, keyLength, key);

		node->key = object_key;
		node->value = 0;
		node->type = JSON_TYPE::OBJECT;
		node->next = (JSON_NODE*)this->parent.json_root->value;
		node->format = NULL;

		this->parent.json_root->value = node;
	}

	return node;
}

JSON_ARRAY JSON_OBJECT::Insert::Array(const char* key)
{
	JSON_NODE* node;

	node = (JSON_NODE*)malloc(sizeof(JSON_NODE));
	if (node != NULL)
	{
		size_t keyLength = UTF8_Encoding::GetStringUnits(key) + 1;
		char* array_key = (char*)malloc(keyLength);
		if (array_key == NULL) {
			free(node);
			return NULL;
		}

		UTF8_Encoding::StringCopy(array_key, keyLength, key);

		node->key = array_key;
		node->value = 0;
		node->type = JSON_TYPE::ARRAY;
		node->next = (JSON_NODE*)this->parent.json_root->value;
		node->format = NULL;

		this->parent.json_root->value = node;
	}

	return node;
}

JSON_NODE* JSON_OBJECT::Insert::String(const char* key, const char* value)
{
	JSON_NODE* node;

	node = (JSON_NODE*)malloc(sizeof(JSON_NODE));
	if (node != NULL)
	{
		size_t keyLength = UTF8_Encoding::GetStringUnits(key) + 1;
		char* string_key = (char*)malloc(keyLength);
		if (string_key == NULL) {
			free(node);
			return NULL;
		}

		size_t valueLength = UTF8_Encoding::GetStringUnits(value) + 1;
		char* string_value = (char*)malloc(valueLength);
		if (string_value == NULL) {
			free(string_key);
			free(node);
			return NULL;
		}

		UTF8_Encoding::StringCopy(string_key, keyLength, key);
		UTF8_Encoding::StringCopy(string_value, valueLength, value);

		node->key = string_key;
		node->value = string_value;
		node->type = JSON_TYPE::STRING;
		node->next = (JSON_NODE*)this->parent.json_root->value;
		node->format = NULL;

		this->parent.json_root->value = node;
	}

	return node;
}

JSON_NODE* JSON_OBJECT::Insert::Boolean(const char* key, bool value)
{
	JSON_NODE* node;

	node = (JSON_NODE*)malloc(sizeof(JSON_NODE));
	if (node != NULL)
	{
		size_t keyLength = UTF8_Encoding::GetStringUnits(key) + 1;
		char* boolean_key = (char*)malloc(keyLength);
		if (boolean_key == NULL) {
			free(node);
			return NULL;
		}

		UTF8_Encoding::StringCopy(boolean_key, keyLength, key);

		node->key = boolean_key;
		node->value = (void*)value;
		node->type = JSON_TYPE::BOOLEAN;
		node->next = (JSON_NODE*)this->parent.json_root->value;
		node->format = NULL;

		this->parent.json_root->value = node;
	}

	return node;
}

JSON_OBJECT::Insert::Number::Number(Insert& parent) : parent(parent) {}

JSON_NODE* JSON_OBJECT::Insert::Number::Double(const char* key, double value)
{
	JSON_NODE* node;

	node = (JSON_NODE*)malloc(sizeof(JSON_NODE));
	if (node != NULL)
	{
		size_t keyLength = UTF8_Encoding::GetStringUnits(key) + 1;
		char* number_key = (char*)malloc(keyLength);
		if (number_key == NULL) {
			free(node);
			return NULL;
		}

		char* number_value = (char*)malloc(128);
		if (number_value == NULL) {
			free(number_key);
			free(node);
			return NULL;
		}

		UTF8_Encoding::StringCopy(number_key, keyLength, key);
		snprintf(number_value, 128, "%f", value);

		node->key = number_key;
		node->value = number_value;
		node->type = JSON_TYPE::NUMBER;
		node->next = (JSON_NODE*)this->parent.parent.json_root->value;
		node->format = NULL;

		this->parent.parent.json_root->value = node;
	}

	return node;
}

JSON_NODE* JSON_OBJECT::Insert::Number::Int(const char* key, int value)
{
	JSON_NODE* node;

	node = (JSON_NODE*)malloc(sizeof(JSON_NODE));
	if (node != NULL)
	{
		size_t keyLength = UTF8_Encoding::GetStringUnits(key) + 1;
		char* number_key = (char*)malloc(keyLength);
		if (number_key == NULL) {
			free(node);
			return NULL;
		}

		char* number_value = (char*)malloc(32);
		if (number_value == NULL) {
			free(number_key);
			free(node);
			return NULL;
		}

		UTF8_Encoding::StringCopy(number_key, keyLength, key);
		snprintf(number_value, 32, "%d", value);

		node->key = number_key;
		node->value = number_value;
		node->type = JSON_TYPE::NUMBER;
		node->next = (JSON_NODE*)this->parent.parent.json_root->value;
		node->format = NULL;

		this->parent.parent.json_root->value = node;
	}

	return node;
}

JSON_NODE* JSON_OBJECT::Insert::Number::Long(const char* key, long value)
{
	JSON_NODE* node;

	node = (JSON_NODE*)malloc(sizeof(JSON_NODE));
	if (node != NULL)
	{
		size_t keyLength = UTF8_Encoding::GetStringUnits(key) + 1;
		char* number_key = (char*)malloc(keyLength);
		if (number_key == NULL) {
			free(node);
			return NULL;
		}

		char* number_value = (char*)malloc(32);
		if (number_value == NULL) {
			free(number_key);
			free(node);
			return NULL;
		}

		UTF8_Encoding::StringCopy(number_key, keyLength, key);
		snprintf(number_value, 32, "%ld", value);

		node->key = number_key;
		node->value = number_value;
		node->type = JSON_TYPE::NUMBER;
		node->next = (JSON_NODE*)this->parent.parent.json_root->value;
		node->format = NULL;

		this->parent.parent.json_root->value = node;
	}

	return node;
}

JSON_NODE* JSON_OBJECT::Insert::Number::Int64(const char* key, long long value)
{
	JSON_NODE* node;

	node = (JSON_NODE*)malloc(sizeof(JSON_NODE));
	if (node != NULL)
	{
		size_t keyLength = UTF8_Encoding::GetStringUnits(key) + 1;
		char* number_key = (char*)malloc(keyLength);
		if (number_key == NULL) {
			free(node);
			return NULL;
		}

		char* number_value = (char*)malloc(64);
		if (number_value == NULL) {
			free(number_key);
			free(node);
			return NULL;
		}

		UTF8_Encoding::StringCopy(number_key, keyLength, key);
		snprintf(number_value, 64, "%lld", value);

		node->key = number_key;
		node->value = number_value;
		node->type = JSON_TYPE::NUMBER;
		node->next = (JSON_NODE*)this->parent.parent.json_root->value;
		node->format = NULL;

		this->parent.parent.json_root->value = node;
	}

	return node;
}

JSON_NODE* JSON_OBJECT::Insert::Number::String(const char* key, const char* value)
{
	JSON_NODE* node;

	node = (JSON_NODE*)malloc(sizeof(JSON_NODE));
	if (node != NULL)
	{
		size_t keyLength = UTF8_Encoding::GetStringUnits(key) + 1;
		char* number_key = (char*)malloc(keyLength);
		if (number_key == NULL) {
			free(node);
			return NULL;
		}

		size_t valueLength = UTF8_Encoding::GetStringUnits(value) + 1;
		char* number_value = (char*)malloc(valueLength);
		if (number_value == NULL) {
			free(number_key);
			free(node);
			return NULL;
		}

		UTF8_Encoding::StringCopy(number_key, keyLength, key);
		UTF8_Encoding::StringCopy(number_value, valueLength, value);

		node->key = number_key;
		node->value = number_value;
		node->type = JSON_TYPE::NUMBER;
		node->next = (JSON_NODE*)this->parent.parent.json_root->value;
		node->format = NULL;

		this->parent.parent.json_root->value = node;
	}

	return node;
}

bool JSON_OBJECT::Delete(const char* key)
{
	JSON_NODE* prev_node, * node;

	prev_node = node = NULL;

	node = (JSON_NODE*)this->json_root->value;
	if (UTF8_Encoding::CompareStrings(node->key, key) == 0)
	{
		this->json_root->value = node->next;
		json_free_node(node);
		return true;
	}
	else
	{
		prev_node = node;
		node = node->next;

		while (node != NULL)
		{
			if (UTF8_Encoding::CompareStrings(node->key, key) == 0)
			{
				prev_node->next = node->next;
				json_free_node(node);
				return true;
			}
			prev_node = node;
			node = node->next;
		}
	}
	return false;
}

bool JSON_OBJECT::Delete(JSON_NODE* reference)
{
	JSON_NODE* prev_node, * node;

	prev_node = node = NULL;

	node = (JSON_NODE*)this->json_root->value;
	if (node == reference)
	{
		this->json_root->value = node->next;
		json_free_node(node);
		return true;
	}
	else
	{
		prev_node = node;
		node = node->next;

		while (node != NULL)
		{
			if (node == reference)
			{
				prev_node->next = node->next;
				json_free_node(node);
				return true;
			}
			prev_node = node;
			node = node->next;
		}
	}
	return false;
}

char* JSON_OBJECT::Generate(const char* format)
{
	return JSON_Generate(this->json_root, format);
}

bool JSON_OBJECT::Format(const char* format)
{
	if (this->json_root) {
		if (json_root->format) {
			free((void*)json_root->format);
		}
		if (format == NULL) {
			this->json_root->format = NULL;
		}
		else
		{
			size_t blockLen = strlen(format) + 1;
			this->json_root->format = (const char*)malloc(blockLen);
			if (this->json_root->format == NULL) {
				return false;
			}
			memcpy((void*)this->json_root->format, format, blockLen);
		}

		return true;
	}

	return false;
}

JSON_NODE* JSON_OBJECT::Parse(const char* json, JSON_PARSER_CONTEXT* context)
{
	if (this->json_root) {
		JSON_Free(this->json_root);
	}

	return this->json_root = JSON_Parse(json, context);
}

// ---------------------------- //
// **   JSON_ARRAY methods   ** //
// ---------------------------- //

JSON_ARRAY::JSON_ARRAY() {
	this->json_root = NULL;
}

JSON_ARRAY::JSON_ARRAY(JSON_NODE* root) {
	this->json_root = root;
}

JSON_ARRAY::JSON_ARRAY(const char* json, JSON_PARSER_CONTEXT* context)
{
	this->json_root = JSON_Parse(json, context);
}

JSON_ARRAY& JSON_ARRAY::operator=(const JSON_ARRAY& other)
{
	this->json_root = other.json_root;
	return *this;
}

JSON_ARRAY::operator JSON_NODE* () const
{
	return this->json_root;
}

JSON_NODE* JSON_ARRAY::MakeRoot()
{
	return this->json_root = JSON_CreateNode(JSON_TYPE::ARRAY, NULL, NULL);
}

void JSON_ARRAY::Free()
{
	JSON_Free(this->json_root);
	this->json_root = NULL;
}

bool JSON_ARRAY::Empty() {
	if (this->json_root) {
		return false;
	}
	else {
		return true;
	}
}

unsigned long JSON_ARRAY::Count()
{
	unsigned long count;

	count = 0;
	if (this->json_root != NULL)
	{
		JSON_NODE* node;

		node = (JSON_NODE*)this->json_root->value;
		while (node != NULL)
		{
			node = node->next;
			count++;
		}
	}

	return count;
}

JSON_NODE* JSON_ARRAY::First()
{
	if (this->json_root != NULL) {
		return (JSON_NODE*)this->json_root->value;
	}

	return NULL;
}

JSON_OBJECT JSON_ARRAY::Object(unsigned long i)
{
	JSON_NODE* node;
	unsigned long ci = 0;

	node = (JSON_NODE*)this->json_root->value;
	while (node != NULL)
	{
		if (ci == i) {
			return node;
		}
		node = node->next;
		ci++;
	}

	return NULL;
}

JSON_ARRAY JSON_ARRAY::Array(unsigned long i)
{
	JSON_NODE* node;
	unsigned long ci = 0;

	node = (JSON_NODE*)this->json_root->value;
	while (node != NULL)
	{
		if (ci == i) {
			return node;
		}
		node = node->next;
		ci++;
	}

	return NULL;
}

const char* JSON_ARRAY::String(unsigned long i)
{
	JSON_NODE* node;
	unsigned long ci = 0;

	node = (JSON_NODE*)this->json_root->value;
	while (node != NULL)
	{
		if (ci == i) {
			return (char*)node->value;
		}
		node = node->next;
		ci++;
	}

	return NULL;
}

bool JSON_ARRAY::Boolean(unsigned long i)
{
	JSON_NODE* node;
	unsigned long ci = 0;

	node = (JSON_NODE*)this->json_root->value;
	while (node != NULL)
	{
		if (ci == i) {
			return (bool)node->value;
		}
		node = node->next;
		ci++;
	}

	return NULL;
}

JSON_ARRAY::Number::Number(JSON_ARRAY& parent) : parent(parent) {}

double JSON_ARRAY::Number::Double(unsigned long i)
{
	JSON_NODE* node;
	unsigned long ci = 0;

	node = (JSON_NODE*)this->parent.json_root->value;
	while (node != NULL)
	{
		if (ci == i) {
			return atof((const char*)node->value);
		}
		node = node->next;
		ci++;
	}

	return 0.0f;
}

int JSON_ARRAY::Number::Int(unsigned long i)
{
	JSON_NODE* node;
	unsigned long ci = 0;

	node = (JSON_NODE*)this->parent.json_root->value;
	while (node != NULL)
	{
		if (ci == i) {
			return atoi((const char*)node->value);
		}
		node = node->next;
		ci++;
	}

	return 0;
}

long JSON_ARRAY::Number::Long(unsigned long i)
{
	JSON_NODE* node;
	unsigned long ci = 0;

	node = (JSON_NODE*)this->parent.json_root->value;
	while (node != NULL)
	{
		if (ci == i) {
			return atol((const char*)node->value);
		}
		node = node->next;
		ci++;
	}

	return 0;
}

long long JSON_ARRAY::Number::Int64(unsigned long i)
{
	JSON_NODE* node;
	unsigned long ci = 0;

	node = (JSON_NODE*)this->parent.json_root->value;
	while (node != NULL)
	{
		if (ci == i) {
			return atoll((const char*)node->value);
		}
		node = node->next;
		ci++;
	}

	return 0;
}

const char* JSON_ARRAY::Number::String(unsigned long i)
{
	JSON_NODE* node;
	unsigned long ci = 0;

	node = (JSON_NODE*)this->parent.json_root->value;
	while (node != NULL)
	{
		if (ci == i) {
			return (const char*)node->value;
		}
		node = node->next;
		ci++;
	}

	return 0;
}

JSON_ARRAY::Insert::Insert(JSON_ARRAY& parent) : parent(parent) {}

JSON_OBJECT JSON_ARRAY::Insert::Object()
{
	JSON_NODE* node;

	node = (JSON_NODE*)malloc(sizeof(JSON_NODE));
	if (node != NULL)
	{
		node->key = NULL;
		node->value = 0;
		node->type = JSON_TYPE::OBJECT;
		node->next = (JSON_NODE*)this->parent.json_root->value;
		node->format = NULL;

		this->parent.json_root->value = node;
	}

	return node;
}

JSON_ARRAY JSON_ARRAY::Insert::Array()
{
	JSON_NODE* node;

	node = (JSON_NODE*)malloc(sizeof(JSON_NODE));
	if (node != NULL)
	{
		node->key = NULL;
		node->value = 0;
		node->type = JSON_TYPE::ARRAY;
		node->next = (JSON_NODE*)this->parent.json_root->value;
		node->format = NULL;

		this->parent.json_root->value = node;
	}

	return node;
}

JSON_NODE* JSON_ARRAY::Insert::String(const char* value)
{
	JSON_NODE* node;

	node = (JSON_NODE*)malloc(sizeof(JSON_NODE));
	if (node != NULL)
	{
		size_t valueLength = UTF8_Encoding::GetStringUnits(value) + 1;
		char* string_value = (char*)malloc(valueLength);
		if (string_value == NULL) {
			free(node);
			return NULL;
		}

		UTF8_Encoding::StringCopy(string_value, valueLength, value);

		node->key = NULL;
		node->value = string_value;
		node->type = JSON_TYPE::STRING;
		node->next = (JSON_NODE*)this->parent.json_root->value;
		node->format = NULL;

		this->parent.json_root->value = node;
	}

	return node;
}

JSON_NODE* JSON_ARRAY::Insert::Boolean(bool value)
{
	JSON_NODE* node;

	node = (JSON_NODE*)malloc(sizeof(JSON_NODE));
	if (node != NULL)
	{
		node->key = NULL;
		node->value = (void*)value;
		node->type = JSON_TYPE::BOOLEAN;
		node->next = (JSON_NODE*)this->parent.json_root->value;
		node->format = NULL;

		this->parent.json_root->value = node;
	}

	return node;
}

JSON_ARRAY::Insert::Number::Number(Insert& parent) : parent(parent) {}

JSON_NODE* JSON_ARRAY::Insert::Number::Double(double value)
{
	JSON_NODE* node;

	node = (JSON_NODE*)malloc(sizeof(JSON_NODE));
	if (node != NULL)
	{
		char* number_value = (char*)malloc(128);
		if (number_value == NULL) {
			free(node);
			return NULL;
		}

		snprintf(number_value, 128, "%f", value);

		node->key = NULL;
		node->value = number_value;
		node->type = JSON_TYPE::NUMBER;
		node->next = (JSON_NODE*)this->parent.parent.json_root->value;
		node->format = NULL;

		this->parent.parent.json_root->value = node;
	}

	return node;
}

JSON_NODE* JSON_ARRAY::Insert::Number::Int(int value)
{
	JSON_NODE* node;

	node = (JSON_NODE*)malloc(sizeof(JSON_NODE));
	if (node != NULL)
	{
		char* number_value = (char*)malloc(32);
		if (number_value == NULL) {
			free(node);
			return NULL;
		}

		snprintf(number_value, 32, "%d", value);

		node->key = NULL;
		node->value = number_value;
		node->type = JSON_TYPE::NUMBER;
		node->next = (JSON_NODE*)this->parent.parent.json_root->value;
		node->format = NULL;

		this->parent.parent.json_root->value = node;
	}

	return node;
}

JSON_NODE* JSON_ARRAY::Insert::Number::Long(long value)
{
	JSON_NODE* node;

	node = (JSON_NODE*)malloc(sizeof(JSON_NODE));
	if (node != NULL)
	{
		char* number_value = (char*)malloc(32);
		if (number_value == NULL) {
			free(node);
			return NULL;
		}

		snprintf(number_value, 32, "%ld", value);

		node->key = NULL;
		node->value = number_value;
		node->type = JSON_TYPE::NUMBER;
		node->next = (JSON_NODE*)this->parent.parent.json_root->value;
		node->format = NULL;

		this->parent.parent.json_root->value = node;
	}

	return node;
}

JSON_NODE* JSON_ARRAY::Insert::Number::Int64(long long value)
{
	JSON_NODE* node;

	node = (JSON_NODE*)malloc(sizeof(JSON_NODE));
	if (node != NULL)
	{
		char* number_value = (char*)malloc(64);
		if (number_value == NULL) {
			free(node);
			return NULL;
		}

		snprintf(number_value, 64, "%lld", value);

		node->key = NULL;
		node->value = number_value;
		node->type = JSON_TYPE::NUMBER;
		node->next = (JSON_NODE*)this->parent.parent.json_root->value;
		node->format = NULL;

		this->parent.parent.json_root->value = node;
	}

	return node;
}

JSON_NODE* JSON_ARRAY::Insert::Number::String(const char* value)
{
	JSON_NODE* node;

	node = (JSON_NODE*)malloc(sizeof(JSON_NODE));
	if (node != NULL)
	{
		size_t valueLength = UTF8_Encoding::GetStringUnits(value) + 1;
		char* number_value = (char*)malloc(valueLength);
		if (number_value == NULL) {
			free(node);
			return NULL;
		}

		UTF8_Encoding::StringCopy(number_value, valueLength, value);

		node->key = NULL;
		node->value = number_value;
		node->type = JSON_TYPE::NUMBER;
		node->next = (JSON_NODE*)this->parent.parent.json_root->value;
		node->format = NULL;

		this->parent.parent.json_root->value = node;
	}

	return node;
}

bool JSON_ARRAY::Delete(unsigned long i)
{
	unsigned long index;
	JSON_NODE* prev_node, * node;

	prev_node = node = NULL;

	node = (JSON_NODE*)this->json_root->value;
	if (i == 0)
	{
		this->json_root->value = node->next;
		json_free_node(node);
		return true;
	}
	else
	{
		index = 1;
		prev_node = node;
		node = node->next;

		while (node != NULL)
		{
			if (index == i)
			{
				prev_node->next = node->next;
				json_free_node(node);
				return true;
			}
			prev_node = node;
			node = node->next;
			index++;
		}
	}
	return false;
}

bool JSON_ARRAY::Delete(JSON_NODE* reference)
{
	JSON_NODE* prev_node, * node;

	prev_node = node = NULL;

	node = (JSON_NODE*)this->json_root->value;
	if (node == reference)
	{
		this->json_root->value = node->next;
		json_free_node(node);
		return true;
	}
	else
	{
		prev_node = node;
		node = node->next;

		while (node != NULL)
		{
			if (node == reference)
			{
				prev_node->next = node->next;
				json_free_node(node);
				return true;
			}
			prev_node = node;
			node = node->next;
		}
	}
	return false;
}

char* JSON_ARRAY::Generate(const char* format)
{
	return JSON_Generate(this->json_root, format);
}

bool JSON_ARRAY::Format(const char* format)
{
	if (this->json_root) {
		if (json_root->format) {
			free((void*)json_root->format);
		}
		if (format == NULL) {
			this->json_root->format = NULL;
		}
		else
		{
			size_t blockLen = strlen(format) + 1;
			this->json_root->format = (const char*)malloc(blockLen);
			if (this->json_root->format == NULL) {
				return false;
			}
			memcpy((void*)this->json_root->format, format, blockLen);
		}

		return true;
	}

	return false;
}

JSON_NODE* JSON_ARRAY::Parse(const char* json, JSON_PARSER_CONTEXT* context)
{
	if (this->json_root) {
		JSON_Free(this->json_root);
	}

	return this->json_root = JSON_Parse(json, context);
}
