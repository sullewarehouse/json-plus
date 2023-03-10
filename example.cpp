
//
// example.cpp
// 
// Author:
//     Brian Sullender
//     SULLE WAREHOUSE LLC
// 
// Description:
//     The example program for using json-plus.
//

#include <iostream>

#include "json-plus.h"
using namespace json_plus;

// This function is an example of creating a json string from a json node tree
char* create_json_string()
{
	JSON_OBJECT json_file;
	char* json_string;

	json_string = NULL;

	// Create the root object node
	json_file = JSON_CreateNode(JSON_TYPE::OBJECT, NULL, NULL);
	if (!json_file.Empty())
	{
		// Create user object array
		JSON_ARRAY users = json_file.Insert.Array("users");

		// Add user "Kim"
		JSON_OBJECT user_kim = users.Insert.Object();
		user_kim.Insert.Number.Double("balance", 23.1f);
		user_kim.Insert.String("username", "Kim");

		// Add user "Tom"
		JSON_OBJECT user_tom = users.Insert.Object();
		user_tom.Insert.Number.Double("balance", 30.5f);
		user_tom.Insert.String("username", "Tom");

		// Add random number array
		JSON_ARRAY random_numbers = json_file.Insert.Array("random_numbers");
		random_numbers.Insert.Number.Int(rand());
		random_numbers.Insert.Number.Int(rand());
		random_numbers.Insert.Number.Int(rand());
		random_numbers.Insert.Number.Int(rand());

		// Override the format so the "random_numbers" array is on a single line
		random_numbers.FormatOverride("");

		// Add settings object
		JSON_OBJECT settings = json_file.Insert.Object("settings");
		settings.Insert.String("path", "C:\\json_files\\console\\users.txt");

		// Add basic info
		json_file.Insert.Boolean("encrypted", true);
		json_file.Insert.Number.Int("count", 2);
		json_file.Insert.String("type", "accounts");

		// Create the json string from the object
		json_string = json_file.Generate(",\n {\n}");
	}

	// Free json resources
	JSON_Free(json_file);

	// Return the json string
	return json_string;
}

// This example calls the create_json_string() function then walks the json data
int wmain()
{
	// Create a json string
	char* json_string = create_json_string();

	// Parser context
	JSON_PARSER_CONTEXT context;

	// Parse the json string
	JSON_OBJECT json_file = JSON_Parse(json_string, &context);
	if (context.errorCode != JSON_ERROR_CODE::NONE) {
		JSON_Free(json_file);
		printf("%s\n", context.errorDescription);
		return -1;
	}

	// Check if the object exists
	if (!json_file.Empty())
	{
		// Get json members
		const char* type = json_file.String("type");
		long count = json_file.Number.Long("count");
		bool encrypted = json_file.Boolean("encrypted");

		const char* path = NULL;
		JSON_OBJECT settings = json_file.Object("settings");
		if (!settings.Empty()) {
			path = settings.String("path");
		}

		// Print members
		printf("type: %s\ncount: %d\nencrypted: %s\npath: %s\n", type, count, encrypted ? "true" : "false", path);

		// Get users array from the object
		JSON_ARRAY users = json_file.Array("users");
		if (!users.Empty())
		{
			// Walk the array
			for (unsigned long i = 0; i < users.Count(); i++)
			{
				// Get user object
				JSON_OBJECT account = users.Object(i);

				if (!account.Empty()) {
					// Print user info
					printf("username: %s balance: %f\n", account.String("username"), account.Number.Double("balance"));
				}
			}
		}
	}

	// Free json resources
	JSON_Free(json_file);

	// Free json string
	free(json_string);

	// Exit
	return 0;
}
