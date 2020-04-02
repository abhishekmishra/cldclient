// Copyright (c) 2020 Abhishek Mishra
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef SRC_CLD_LUA_H_
#define SRC_CLD_LUA_H_
#ifdef __cplusplus  
extern "C" {
#endif

#include "lua.h"
#include "lauxlib.h"

#include "cld_common.h"

cld_cmd_err start_lua_interpreter();

cld_cmd_err stop_lua_interpreter();

/**
 * Execute a lua function representing a docker command.
 * The command is passed arguments identical to the C command handlers.
 */
cld_cmd_err execute_lua_command(const char* module_name, const char* command_name, void* handler_args, 
    arraylist* options,	arraylist* args, cld_command_output_handler success_handler,
    cld_command_output_handler error_handler);

#ifdef __cplusplus 
}
#endif

#endif //SRC_CLD_LUA_H_