/*
 * Copyright (c) 2017, University Corporation for Atmospheric Research
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, 
 * this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * 
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>

#include "inception.h"


void __attribute__((__noreturn__)) exec_shell(image_config_t* image)
{
	char* args[] = {image->shell, NULL, NULL, NULL};
	if(image->usercmd)
	{
		args[1] = "-c";
		args[2] = image->usercmd;
	}
	if(image->cwd)
	{
		if(chdir(image->cwd)) perror("Setting Working Directory Failed: ");
	}
	environ = image->environ;
	execv(image->shell_full_path, args);
}

static void usage()
{
	printf("-c [image_name]\n");
	printf("-x #copy environment\n");
}

int main(int argc, char** argv)
{
	int ch;
	char* config_name = NULL;
	char* tmpptr;
	int i;
	char** clean_environ = {NULL};
	char restore_environ=0;
	char** old_environ = environ;
	environ = clean_environ;
	//clearenv()?
	image_config_t image;
	static struct option longopts[] = {
		{ "config", optional_argument, NULL, 'c' },
		{ "new_namespace", no_argument, NULL, 'n'},
		{ "export_environment", no_argument, NULL, 'x'},
		{ "cwd", optional_argument, NULL, 'p'},
		{ "help", no_argument, NULL, 'h'},
		{ NULL, 0, NULL, 0 }	
	};
	memset(&image, 0, sizeof(image_config_t));
	while((ch = getopt_long(argc, argv, "c:p:nxh", longopts, NULL))!= -1)
	{
		switch(ch) {
			case 'c':
				asprintf(&config_name, "%s", optarg);
				break;
			case 'n':
				break;
			case 'x':
				restore_environ = 1;
				break;
			case 'p':
				asprintf(&(image.cwd), "%s", optarg);
				break;
			case 'h':
				usage();
				return(0);
			default:
				fprintf(stderr, "Getopt Error\n");
				return(1);
			}
	}
	if(restore_environ)
		image.environ = old_environ;
	else
	{
		image.environ = clean_environ;
		build_default_environ(&image);
	}

	if(optind < argc)
	{
		for(i=optind;i<argc;i++)
		{
			if(image.usercmd)
			{
				tmpptr = image.usercmd;
				asprintf(&(image.usercmd), "%s %s", image.usercmd, argv[i]);
				free(tmpptr);
			}
			else
			{
				asprintf(&(image.usercmd), "%s", argv[i]);
			}

		}
	}

	parse_config(INCEPTION_CONFIG_PATH, config_name, &image);

	setup_namespace(&image);
	find_shell(&image);
	exec_shell(&image);

	if(config_name) free(config_name);
}
