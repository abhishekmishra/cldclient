/*
 * clibdocker: cld_command.c
 * Created on: 06-Feb-2019
 *
 * clibdocker
 * Copyright (C) 2018 Abhishek Mishra <abhishekmishra3@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <string.h>
#include "cld_command.h"

 /**
  * Create a new value object of given type.
  *
  * \param val object to create
  * \param type
  * \return error code
  */
cld_cmd_err make_cld_val(cld_val** val, cld_type type)
{
	(*val) = (cld_val*)calloc(1, sizeof(cld_val));
	if ((*val) == NULL)
	{
		return CLD_COMMAND_ERR_ALLOC_FAILED;
	}
	(*val)->type = type;
	clear_cld_val(*val);
	return CLD_COMMAND_SUCCESS;
}

/**
 * Free the created value
 */
void free_cld_val(cld_val* val)
{
	if (val->str_value)
	{
		free(val->str_value);
	}
	free(val);
}

/**
 * Reset values to system defaults.
 */
void clear_cld_val(cld_val* val)
{
	val->bool_value = 0;
	val->int_value = 0;
	val->dbl_value = 0.0;
	val->str_value = NULL;
}

/**
 * Copy values from 'from' to 'to'.
 * Can be used to reset to defaults.
 * (description and type are not copied)
 * str_value should be freed by caller.
 *
 * \param to val to set
 * \param from val to read from
 */
void copy_cld_val(cld_val* to, cld_val* from)
{
	to->bool_value = from->bool_value;
	to->int_value = from->int_value;
	to->dbl_value = from->dbl_value;
	to->str_value = from->str_value;
}

/**
 * Parse the input and read the value of the type of the val object.
 * (Should not be called when the values is a flag.)
 * The value should be set as soon as the argument/option is seen
 *
 * \param val object whose value will be set
 * \param input string input
 * \return error code
 */
cld_cmd_err parse_cld_val(cld_val* val, char* input)
{
	if (input == NULL)
	{
		return CLD_COMMAND_ERR_UNKNOWN;
	}
	else
	{
		int v;
		double d;
		switch (val->type)
		{
		case CLD_TYPE_BOOLEAN:
			sscanf(input, "%d", &v);
			if (v > 0)
			{
				val->bool_value = 1;
			}
			else
			{
				val->bool_value = 0;
			}
			break;
		case CLD_TYPE_INT:
			sscanf(input, "%d", &v);
			val->int_value = v;
			break;
		case CLD_TYPE_DOUBLE:
			sscanf(input, "%lf", &d);
			val->dbl_value = d;
			break;
		case CLD_TYPE_STRING:
			val->str_value = input;
			break;
		default:
			return CLD_COMMAND_ERR_UNKNOWN;
		}
		return CLD_COMMAND_SUCCESS;
	}
}

/**
 * Create a new option given a name and type.
 *
 * \param option object to create
 * \param name
 * \param short_name
 * \param type
 * \param description
 * \return error code
 */
cld_cmd_err make_option(cld_option** option, char* name, char* short_name,
	cld_type type, char* description)
{
	(*option) = (cld_option*)calloc(1, sizeof(cld_option));
	if ((*option) == NULL)
	{
		return CLD_COMMAND_ERR_ALLOC_FAILED;
	}
	(*option)->name = name;
	(*option)->short_name = short_name;
	(*option)->description = description;
	make_cld_val(&((*option)->val), type);
	make_cld_val(&((*option)->default_val), type);
	return CLD_COMMAND_SUCCESS;
}

/**
 * Free resources used by option
 */
void free_option(cld_option* option)
{
	if (option->short_name)
	{
		free(option->short_name);
	}
	if (option->description)
	{
		free(option->description);
	}

	free(option->val);
	free(option->name);
	free(option);
}

/**
 * Create a new argument given a name and type.
 *
 * \param argument object to create
 * \param name
 * \param type
 * \param description
 * \return error code
 */
cld_cmd_err make_argument(cld_argument** arg, char* name, cld_type type,
	char* description)
{
	(*arg) = (cld_argument*)calloc(1, sizeof(cld_argument));
	if ((*arg) == NULL)
	{
		return CLD_COMMAND_ERR_ALLOC_FAILED;
	}
	(*arg)->name = name;
	(*arg)->description = description;
	(*arg)->optional = 0;
	make_cld_val(&((*arg)->val), type);
	make_cld_val(&((*arg)->default_val), type);
	return CLD_COMMAND_SUCCESS;
}

/**
 * Free resources used by argument
 */
void free_argument(cld_argument* arg)
{
	if (arg->description)
	{
		free(arg->description);
	}
	free(arg->val);
	free(arg->name);
	free(arg);
}

/**
 * Create a new command with the given name and handler
 * Options and sub-commands need to be added after creation.
 * The sub-commands and options lists will be initialized,
 * so one just needs to add items using the arraylist add function.
 *
 * \param command obj to be created
 * \param name
 * \param short_name
 * \param description
 * \param handler function ptr to handler
 * \return error code
 */
cld_cmd_err make_command(cld_command** command, char* name, char* short_name,
	char* description, cld_command_handler handler)
{
	(*command) = (cld_command*)calloc(1, sizeof(cld_command));
	if ((*command) == NULL)
	{
		return CLD_COMMAND_ERR_ALLOC_FAILED;
	}
	(*command)->name = name;
	(*command)->short_name = short_name;
	(*command)->description = description;
	(*command)->handler = handler;
	arraylist_new(&((*command)->options), (void (*)(void*)) & free_option);
	arraylist_new(&((*command)->sub_commands), (void (*)(void*)) & free_command);
	arraylist_new(&((*command)->args), (void (*)(void*)) & free_argument);
	return CLD_COMMAND_SUCCESS;
}

/**
 * Free a command object
 */
void free_command(cld_command* command)
{
	if (command->short_name)
	{
		free(command->short_name);
	}
	if (command->description)
	{
		free(command->description);
	}
	free(command->name);
	arraylist_free(command->options);
	arraylist_free(command->sub_commands);
	arraylist_free(command->args);
	free(command);
}

int gobble(int argc, char** argv, int at_pos)
{
	if (at_pos > (argc - 1))
	{
		return argc;
	}
	else
	{
		for (int i = at_pos; i < argc; i++)
		{
			if (i == argc)
			{
				argv[i] = NULL;
			}
			else
			{
				argv[i] = argv[i + 1];
			}
		}
		return argc - 1;
	}
}


arraylist* get_command_to_exec(arraylist* commands, int* argc,
	char** argv)
{
	//First read all commands
	arraylist* cmd_names;
	arraylist_new(&cmd_names, &free);

	arraylist* cmds_to_exec = NULL;
	arraylist_new(&cmds_to_exec, NULL);

	cld_command* cmd_to_exec = NULL;

	arraylist* cmd_list = commands;
	for (int i = 0; i < *argc; i++)
	{
		char* cmd_name = argv[i];
		int found = 0;
		if (cmd_list != NULL)
		{
			for (int j = 0; j < arraylist_length(cmd_list); j++)
			{
				cld_command* cmd = (cld_command*)arraylist_get(cmd_list,
					j);
				if (strcmp(cmd_name, cmd->name) == 0
					|| strcmp(cmd_name, cmd->short_name) == 0)
				{
					found = 1;
					cmd_list = cmd->sub_commands;
					arraylist_add(cmd_names, cmd->name);
					printf("found command %s\n", cmd->name);
					cmd_to_exec = cmd;
					arraylist_add(cmds_to_exec, cmd);
					break;
				}
			}
		}
		//if current name is not a command break
		if (found == 0)
		{
			break;
		}
	}
	for (int i = 0; i < arraylist_length(cmd_names); i++)
	{
		*argc = gobble(*argc, argv, 0);
	}
	return cmds_to_exec;
}

/**
 * Run the help command for all commands or single command
 *
 * \param commands the list of commands registered (this is a list of cld_command*)
 * \param handler_args an args value to be passed to the command handler
 * \param argc the number of tokens in the line
 * \param argv args as an array of strings
 * \param success_handler handle success results
 * \param error_handler handler error results
 */
cld_cmd_err help_cmd_handler(arraylist* commands, void* handler_args,
	int argc, char** argv, cld_command_output_handler success_handler,
	cld_command_output_handler error_handler)
{
	//First read all commands
	arraylist* cmds_to_exec = get_command_to_exec(commands, &argc, argv);
	if (arraylist_length(cmds_to_exec) > 0) {
		cld_command* cmd_to_exec = arraylist_get(cmds_to_exec, arraylist_length(cmds_to_exec) - 1);
		if (cmd_to_exec == NULL)
		{
			error_handler(CLD_COMMAND_ERR_COMMAND_NOT_FOUND, CLD_RESULT_STRING,
				"No valid command found. Type help to get more help\n");
			return CLD_COMMAND_ERR_COMMAND_NOT_FOUND;
		}

		char* help_str = cmd_to_exec->description;
		success_handler(CLD_COMMAND_SUCCESS, CLD_RESULT_STRING, help_str);
	}
	return CLD_COMMAND_SUCCESS;
}

/**
 * Get the help string for the arg_commands from the registered commands list.
 * \param help_str the help string to return
 * \param commands is the configured list of commands
 * \param arg_commands is a list of string
 * \return error code
 */
cld_cmd_err get_help_for(char** help_str, arraylist* commands,
	arraylist* arg_commands)
{
	arraylist* cmd_list = commands;
	for (int i = 0; i < arraylist_length(arg_commands); i++)
	{
		if (cmd_list != NULL)
		{
			char* cmd_name = arraylist_get(arg_commands, i);
			int found_cmd = 0;
			for (int j = 0; j < arraylist_length(cmd_list); j++)
			{
				cld_command* cmd = arraylist_get(cmd_list, j);
				if (strcmp(cmd_name, cmd->name) == 0)
				{
					found_cmd = 1;
					cmd_list = cmd->sub_commands;
					(*help_str) = cmd->description;
					break;
				}
			}
			if (found_cmd == 0)
			{
				return CLD_COMMAND_ERR_COMMAND_NOT_FOUND;
			}
		}
		else
		{
			return CLD_COMMAND_ERR_COMMAND_NOT_FOUND;
		}
	}
	return CLD_COMMAND_ERR_UNKNOWN;
}

cld_cmd_err parse_options(arraylist* options, int* argc, char*** argv)
{
	int ac = (*argc);
	char** av = (*argv);
	int skip_count = 0;

	for (int i = 0; i < ac; i++)
	{
		char* option = av[i];
		char* long_option_name = NULL;
		char* short_option_name = NULL;
		if (strlen(option) > 1 && option[0] == '-')
		{
			if (option[1] == '-')
			{
				//long option
				long_option_name = option + 2;
			}
			else
			{
				//short option
				short_option_name = option + 1;
			}
			int options_len = arraylist_length(options);
			cld_option* found = NULL;
			for (int j = 0; j < options_len; j++)
			{
				cld_option* opt = arraylist_get(options, j);
				if (long_option_name && opt->name != NULL)
				{
					if (strcmp(long_option_name, opt->name) == 0)
					{
						found = opt;
					}
				}
				if (short_option_name && opt->short_name != NULL)
				{
					if (strcmp(short_option_name, opt->short_name) == 0)
					{
						found = opt;
					}
				}
			}
			if (found == 0)
			{
				printf("Unknown option %s\n.", option);
				return CLD_COMMAND_ERR_OPTION_NOT_FOUND;
			}
			else
			{
				skip_count += 1;
				//read option value if it is not a flag
				if (found->val->type == CLD_TYPE_FLAG) {
					found->val->bool_value = true;
				}
				else
				{
					if (i == (ac - 1))
					{
						printf("Value missing for option %s.\n", option);
						return CLD_COMMAND_ERR_OPTION_NOT_FOUND;
					}
					else
					{
						skip_count += 1;
						i = i + 1;
						char* value = av[i];
						parse_cld_val(found->val, value);
					}
				}
			}
		}
		else
		{
			//not an option, break
			break;
		}
	}

	for (int i = 0; i < skip_count; i++)
	{
		*argc = gobble(*argc, argv, 0);
	}

	return CLD_COMMAND_SUCCESS;
}

cld_cmd_err parse_args(arraylist* args, int* argc, char*** argv)
{
	int ac = (*argc);
	char** av = (*argv);
	int args_len = arraylist_length(args);
	//	printf("args len %d\n", args_len);
	if (args_len == ac)
	{
		for (int i = 0; i < args_len; i++)
		{
			cld_argument* arg = arraylist_get(args, i);
			char* argval = av[i];
			//check if we have
			parse_cld_val(arg->val, argval);
			//			printf("Argval %s\n", argval);
		}
	}
	else
	{
		return CLD_COMMAND_ERR_ARG_NOT_FOUND;
	}
	for (int i = 0; i < args_len; i++)
	{
		(*argc) = gobble((*argc), av, 0);
		//		printf("new argc = %d\n", (*argc));
	}
	return CLD_COMMAND_SUCCESS;
}

void print_options(arraylist* options) {
	if (options != NULL) {
		int options_len = arraylist_length(options);
		for (int i = 0; i < options_len; i++) {
			cld_option* o = arraylist_get(options, i);
			switch (o->val->type) {
			case CLD_TYPE_BOOLEAN:
				printf("%s, %s = %d\n", o->name, o->short_name, o->val->bool_value);
				break;
			case CLD_TYPE_FLAG:
				printf("%s, %s = %d\n", o->name, o->short_name, o->val->bool_value);
				break;
			case CLD_TYPE_STRING:
				printf("%s, %s = %s\n", o->name, o->short_name, o->val->str_value);
				break;
			}
		}
	}
}

/**
 * Execute a single line containing one top-level command.
 * All output is written to stdout, all errors to stderr
 *
 * \param commands the list of commands registered (this is a list of cld_command*)
 * \param handler_args an args value to be passed to the command handler
 * \param argc the number of tokens in the line
 * \param argv args as an array of strings
 * \param success_handler handle success results
 * \param error_handler handler error results
 */
cld_cmd_err exec_command(arraylist* commands, void* handler_args,
	int argc, char** argv, cld_command_output_handler success_handler,
	cld_command_output_handler error_handler)
{
	cld_cmd_err err = CLD_COMMAND_SUCCESS;

	//First read all commands
	arraylist* cmds_to_exec = get_command_to_exec(commands, &argc, argv);
	int len_cmds = arraylist_length(cmds_to_exec);
	arraylist* all_options, * all_args;
	arraylist_new(&all_options, NULL);
	arraylist_new(&all_args, NULL);

	for (int i = 0; i < len_cmds; i++) {
		cld_command* cmd_to_exec = arraylist_get(cmds_to_exec, i);
		int len_options = arraylist_length(cmd_to_exec->options);
		for (int j = 0; j < len_options; j++) {
			arraylist_add(all_options, arraylist_get(cmd_to_exec->options, j));
		}
		for (int j = 0; j < len_options; j++) {
			arraylist_add(all_args, arraylist_get(cmd_to_exec->args, j));
		}
	}

	//Then read all options
	err = parse_options(all_options, &argc, &argv);
	if (err != CLD_COMMAND_SUCCESS)
	{
		return err;
	}

	print_options(all_options);

	////Now read all arguments
	//err = parse_args(all_args, &argc, &argv);
	//if (err != CLD_COMMAND_SUCCESS)
	//{
	//	return err;
	//}

	////anything leftover
	//if (argc > 0)
	//{
	//	printf("%d extra arguments found.\n", argc);
	//	return CLD_COMMAND_ERR_EXTRA_ARGS_FOUND;
	//}

	for (int i = 0; i < len_cmds; i++) {
		cld_command* cmd_to_exec = arraylist_get(cmds_to_exec, i);
		if (cmd_to_exec == NULL)
		{
			printf("No valid command found. Type help to get more help\n");
			return CLD_COMMAND_ERR_COMMAND_NOT_FOUND;
		}

		// for the last command in the chain, it can have args
		if (i == (len_cmds - 1)) {
			//Now read all arguments
			err = parse_args(cmd_to_exec->args, &argc, &argv);
			if (err != CLD_COMMAND_SUCCESS)
			{
				return err;
			}

			//anything leftover
			if (argc > 0)
			{
				printf("%d extra arguments found.\n", argc);
				return CLD_COMMAND_ERR_EXTRA_ARGS_FOUND;
			}
		}

		if (cmd_to_exec->handler != NULL) {
			err = cmd_to_exec->handler(handler_args, all_options,
				all_args, success_handler, error_handler);
		}
	}

	return err;
}
