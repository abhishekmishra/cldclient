#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "cld_img.h"
#include "cld_table.h"
#include "docker_all.h"
#include "cld_progress.h"

typedef struct
{
	cld_command_output_handler success_handler;
	cld_multi_progress* multi_progress;
} docker_image_update_args;

void log_pull_message(docker_image_create_status* status, void* client_cbargs)
{
	docker_image_update_args* upd_args = (docker_image_update_args*) client_cbargs;
	if (status)
	{
		if (status->id)
		{
			int len = array_list_length(upd_args->multi_progress->progress_ls);
			int new_len = len;
			int found = 0;
			int loc = -1;
			for (int i = 0; i < len; i++)
			{
				cld_progress* p = (cld_progress*) array_list_get_idx(
						upd_args->multi_progress->progress_ls, i);
				if (strcmp(status->id, p->name) == 0)
				{
					found = 1;
					loc = i;
				}
			}
			if (found == 0)
			{
				cld_progress* p;
				if (create_cld_progress(&p, status->id, 0, 0) == 0)
				{
					array_list_add(upd_args->multi_progress->progress_ls, p);
					upd_args->multi_progress->old_count = array_list_length(
							upd_args->multi_progress->progress_ls) - 1;
					new_len += 1;
					p->message = status->status;
					if (status->progress != NULL)
					{
						p->extra = status->progress;
						p->current = status->progress_detail->current;
						p->total = status->progress_detail->total;
					}
					else
					{
						p->extra = NULL;
					}
				}
			}
			else
			{
				cld_progress* p = (cld_progress*) array_list_get_idx(
						upd_args->multi_progress->progress_ls, loc);
				upd_args->multi_progress->old_count = array_list_length(
						upd_args->multi_progress->progress_ls);
				p->message = status->status;
				if (status->progress != NULL)
				{
					p->extra = status->progress;
					p->current = status->progress_detail->current;
					p->total = status->progress_detail->total;
				}
				else
				{
					p->extra = NULL;
				}
			}
			upd_args->success_handler(CLD_COMMAND_IS_RUNNING,
					CLD_RESULT_PROGRESS, upd_args->multi_progress);
		}
		else
		{
			upd_args->success_handler(CLD_COMMAND_IS_RUNNING, CLD_RESULT_STRING,
					status->status);
		}
	}
}

cld_cmd_err img_pl_cmd_handler(void *handler_args, struct array_list *options,
		struct array_list *args, cld_command_output_handler success_handler,
		cld_command_output_handler error_handler)
{
	int quiet = 0;
	docker_result *res;
	docker_context *ctx = get_docker_context(handler_args);

	docker_image_update_args* upd_args = (docker_image_update_args*) calloc(1,
			sizeof(docker_image_update_args));
	upd_args->success_handler = success_handler;
	create_cld_multi_progress(&(upd_args->multi_progress));

	int len = array_list_length(args);
	if (len != 1)
	{
		error_handler(CLD_COMMAND_ERR_UNKNOWN, CLD_RESULT_STRING,
				"Image name not provided.");
		return CLD_COMMAND_ERR_UNKNOWN;
	}
	else
	{
		cld_argument* image_name_arg = (cld_argument*) array_list_get_idx(args,
				0);
		char* image_name = image_name_arg->val->str_value;
		d_err_t docker_error = docker_image_create_from_image_cb(ctx, &res,
				&log_pull_message, upd_args, image_name, NULL, NULL);
		handle_docker_error(res, success_handler, error_handler);
		if (docker_error == E_SUCCESS)
		{
			char* res_str = (char*) calloc(1024, sizeof(char));
			sprintf(res_str, "Image pull successful -> %s", image_name);
			success_handler(CLD_COMMAND_SUCCESS, CLD_RESULT_STRING, res_str);
			free(res_str);
			return CLD_COMMAND_SUCCESS;
		}
		else
		{
			return CLD_COMMAND_ERR_UNKNOWN;
		}
	}
	free_cld_multi_progress(upd_args->multi_progress);
}

char* concat_tags(array_list* tags_ls)
{
	char* tags = NULL;
	if (tags_ls)
	{
		int len_tags = array_list_length(tags_ls);
		int tag_strlen = 0;
		for (int i = 0; i < len_tags; i++)
		{
			tag_strlen += strlen((char*) array_list_get_idx(tags_ls, i));
			tag_strlen += 1; //for newline
		}
		tag_strlen += 1; //for null terminator
		tags = (char*) calloc(tag_strlen, sizeof(char));
		if (tags != NULL)
		{
			tags[0] = '\0';
			for (int i = 0; i < len_tags; i++)
			{
				strcat(tags, (char*) array_list_get_idx(tags_ls, i));
				if (i != (len_tags - 1))
				{
					strcat(tags, "\n");
				}
			}
		}
	}
	return tags;
}

char* get_image_tags_concat(docker_image* img)
{
	array_list* tags_ls = img->repo_tags;
	char* tags = concat_tags(tags_ls);
	return tags;
}

cld_cmd_err img_ls_cmd_handler(void *handler_args, struct array_list *options,
		struct array_list *args, cld_command_output_handler success_handler,
		cld_command_output_handler error_handler)
{
	int quiet = 0;
	docker_result *res;
	docker_context *ctx = get_docker_context(handler_args);
	array_list* images;

	d_err_t docker_error = docker_images_list(ctx, &res, &images, 1, 1, NULL, 0,
	NULL, NULL, NULL);
	int success = is_ok(res);
	handle_docker_error(res, success_handler, error_handler);
	if (success)
	{
		char res_str[1024];
		sprintf(res_str, "Listing images");
		success_handler(CLD_COMMAND_SUCCESS, CLD_RESULT_STRING, res_str);

		int col_num = 0;
		int len_images = array_list_length(images);
		cld_table* img_tbl;
		if (create_cld_table(&img_tbl, len_images, 6) == 0)
		{
			cld_table_set_header(img_tbl, 0, "REPOSITORY");
			cld_table_set_header(img_tbl, 1, "TAG");
			cld_table_set_header(img_tbl, 2, "DIGEST");
			cld_table_set_header(img_tbl, 3, "IMAGE ID");
			cld_table_set_header(img_tbl, 4, "CREATED");
			cld_table_set_header(img_tbl, 5, "SIZE");
			for (int i = 0; i < len_images; i++)
			{
				docker_image* img = (docker_image*) array_list_get_idx(images,
						i);
				col_num = 0;
				char cstr[1024];
				sprintf(cstr, "%ld", img->created);
				char sstr[1024];
				sprintf(sstr, "%ld", img->size);
				cld_table_set_row_val(img_tbl, i, 0,
						concat_tags(img->repo_tags));
				cld_table_set_row_val(img_tbl, i, 1,
						get_image_tags_concat(img));
				cld_table_set_row_val(img_tbl, i, 2,
						concat_tags(img->repo_digests));
				cld_table_set_row_val(img_tbl, i, 3, img->id);
				cld_table_set_row_val(img_tbl, i, 4, cstr);
				cld_table_set_row_val(img_tbl, i, 5, sstr);
			}
			success_handler(CLD_COMMAND_SUCCESS, CLD_RESULT_TABLE, img_tbl);
		}
	}
	else
	{
		return CLD_COMMAND_ERR_UNKNOWN;
	}
	return CLD_COMMAND_SUCCESS;
}

void log_build_message(docker_build_status* status, void* client_cbargs) {
	docker_image_update_args* upd_args = (docker_image_update_args*) client_cbargs;
	if (status)
	{
		if (status->stream)
		{
			upd_args->success_handler(CLD_COMMAND_IS_RUNNING,
					CLD_RESULT_STRING, status->stream);
		}
	}
}

cld_cmd_err img_build_cmd_handler(void *handler_args,
		struct array_list *options, struct array_list *args,
		cld_command_output_handler success_handler,
		cld_command_output_handler error_handler)
{
	int quiet = 0;
	docker_result *res;
	docker_context *ctx = get_docker_context(handler_args);

	docker_image_update_args* upd_args = (docker_image_update_args*) calloc(1,
			sizeof(docker_image_update_args));
	upd_args->success_handler = success_handler;
	create_cld_multi_progress(&(upd_args->multi_progress));

	int len = array_list_length(args);
	if (len != 1)
	{
		error_handler(CLD_COMMAND_ERR_UNKNOWN, CLD_RESULT_STRING,
				"Image name not provided.");
		return CLD_COMMAND_ERR_UNKNOWN;
	}
	else
	{
		cld_argument* folder_url_dash_arg = (cld_argument*) array_list_get_idx(
				args, 0);
		char* folder_url_dash = folder_url_dash_arg->val->str_value;
		d_err_t docker_error = docker_image_build_cb(ctx, &res, folder_url_dash,
		NULL, &log_build_message, upd_args, NULL);
		handle_docker_error(res, success_handler, error_handler);
		if (docker_error == E_SUCCESS)
		{
			char* res_str = (char*) calloc(1024, sizeof(char));
			sprintf(res_str, "Image pull successful -> %s\n", folder_url_dash);
			success_handler(CLD_COMMAND_SUCCESS, CLD_RESULT_STRING, res_str);
			free(res_str);
			return CLD_COMMAND_SUCCESS;
		}
		else
		{
			return CLD_COMMAND_ERR_UNKNOWN;
		}
	}
	free_cld_multi_progress(upd_args->multi_progress);
}

cld_command *img_commands()
{
	cld_command *image_command;
	if (make_command(&image_command, "image", "img", "Docker Image Commands",
	NULL) == CLD_COMMAND_SUCCESS)
	{
		cld_command *imgpl_command, *imgls_command, *imgbuild_command;
		if (make_command(&imgpl_command, "pull", "pl", "Docker Image Pull",
				&img_pl_cmd_handler) == CLD_COMMAND_SUCCESS)
		{
			cld_argument* image_name_arg;
			make_argument(&image_name_arg, "Image Name", CLD_TYPE_STRING,
					"Name of Docker Image to be pulled.");
			array_list_add(imgpl_command->args, image_name_arg);

			array_list_add(image_command->sub_commands, imgpl_command);
		}
		if (make_command(&imgls_command, "list", "ls", "Docker Image List",
				&img_ls_cmd_handler) == CLD_COMMAND_SUCCESS)
		{
			array_list_add(image_command->sub_commands, imgls_command);
		}
		if (make_command(&imgbuild_command, "build", "make",
				"Docker Image Build", &img_build_cmd_handler)
				== CLD_COMMAND_SUCCESS)
		{
			cld_argument* folder_or_url_or_dash;
			make_argument(&folder_or_url_or_dash, "Folder | URL | -",
					CLD_TYPE_STRING,
					"Docker resources to build (folder/url/stdin)");
			array_list_add(imgbuild_command->args, folder_or_url_or_dash);

			array_list_add(image_command->sub_commands, imgbuild_command);
		}
	}
	return image_command;
}