
//
// example.cpp
// 
// Author:
//     Brian Sullender
//     SULLE WAREHOUSE LLC
// 
// Description:
//     The example program for using json-plus.
//     https://github.com/sullewarehouse/json-plus
//

#include <iostream>

#include "json-plus.h"
using namespace json_plus;

// This function is an example of creating a JSON node tree using JSON_OBJECT and JSON_ARRAY classes, -
// then generating and returning a JSON string
char* create_json_string()
{
	JSON_OBJECT json_file;
	char* json_string;

	// Delete success
	bool success;

	// Default return value
	json_string = NULL;

	// Create the root object
	json_file.MakeRoot();

	// Check if the object exists
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

		// Add a bool value
		json_file.Insert.Boolean("present", true);

		// Items can be deleted from an object using a key or reference
		success = json_file.Delete("present");

		// Add random number array
		JSON_ARRAY random_numbers = json_file.Insert.Array("random_numbers");
		random_numbers.Insert.Number.Int(463);
		random_numbers.Insert.Number.Int(9273);
		random_numbers.Insert.Number.Int(52);
		random_numbers.Insert.Number.Int(20192);
		random_numbers.Insert.Number.Int(1726);

		// Items can be deleted from an array using a index or reference
		success = random_numbers.Delete(2UL);

		// Override the format passed to JSON_Generate
		// Add a space after each ',' character and put 2 key-value pairs on a single line
		random_numbers.Format(", e p2");

		// Add settings object
		JSON_OBJECT settings = json_file.Insert.Object("settings");
		settings.Insert.String("path", "C:\\json_files\\console\\users.txt");

		// Add basic info
		json_file.Insert.Boolean("encrypted", true);
		json_file.Insert.Number.Int("count", 2);
		json_file.Insert.String("type", "accounts");

		// Create the json string from the object
		json_string = json_file.Generate("c ,\ne {\ne \n}");

		// Free json resources (node tree)
		json_file.Free();
	}

	// Return the json string
	return json_string;
}

// The main entry point calls the create_json_string() function to create a JSON string, -
// then call JSON_Parse to create the node tree. We use JSON_OBJECT and JSON_ARRAY classes to get the JSON values
int main()
{
	// Create a json string
	char* json_string = create_json_string();

	// Parser context
	JSON_PARSER_CONTEXT context;

	// JSON_Parse function call example
	//JSON_OBJECT json_file = JSON_Parse(json_string, &context);

	// Object parsing call example
	//JSON_OBJECT json_file;
	//json_file.Parse(json_string, &context);

	// Object initialize with parsing call
	JSON_OBJECT json_file(json_string, &context);
	if (context.errorCode != JSON_ERROR_CODE::NONE) {
		json_file.Free();
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

		// Get random_numbers array from the object
		JSON_ARRAY random_numbers = json_file.Array("random_numbers");
		if (!random_numbers.Empty())
		{
			// for each number
			for (JSON_NODE* node = random_numbers.First(); node != NULL; node = node->next)
			{
				// Get int from node
				int number = node->Int();
				// Print random number
				printf("%d", number);
				if (node->next != NULL) {
					printf("%s", ", ");
				}
				else {
					printf("%s", "\n");
				}
			}
		}

		// Attempt to print deleted member
		bool present = json_file.Boolean("present");
		printf("present: %s\n", present ? "true" : "false");

		// Get users array from the object
		JSON_ARRAY users = json_file.Array("users");
		if (!users.Empty())
		{
			// for each user object
			for (JSON_NODE* node = users.First(); node != NULL; node = node->next)
			{
				JSON_OBJECT account = node;
				if (!account.Empty()) {
					// Print user info
					printf("username: %s balance: %f\n", account.String("username"), account.Number.Double("balance"));
				}
			}
		}
	}

	// Free json resources
	json_file.Free();

	// Free json string
	free(json_string);

	// Exit
	return 0;
}
