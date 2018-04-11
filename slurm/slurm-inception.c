/*
 * Copyright (c) 2018, University Corporation for Atmospheric Research
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include "inception.h"

#include <slurm/spank.h>
#define PATH_CWD_MAX 2048

SPANK_PLUGIN(inception, 1);

char* image;
char* image_cwd;

int validate_opts(int val, const char* optarg, int remote)
{
	//S_CTX_REMOTE is slurmstepd, so we pretend it's ok to leak memory
	//since this executable context only exists for the life of the job step
	//we can't validate in the allocator context since it could run on from
	//a completely different platform
	//see slurm:src/common/plugstack.c
	if(spank_context() == S_CTX_REMOTE)
	{
		if(image) free(image);
		if(image_cwd) free(image_cwd);
		image = strdup(optarg);
    image_cwd = NULL;
	}
	return(0);
}

struct spank_option image_opt = 
{
	"inception-image", "[image]", "", 1, 0,
	(spank_opt_cb_f) validate_opts
};	

int slurm_spank_init(spank_t sp, int ac, char** av)
{
	spank_err_t err;
	image = NULL;
	image_cwd = NULL;
	err = spank_option_register(sp, &image_opt);
	if(spank_context() == S_CTX_ALLOCATOR)
	{
		//sbatch/salloc
		return(0);
	}
	return(0);
}

//int slurm_spank_task_post_fork(spank_t sp, int ac, char** av)
//{
//	return(0);
//}

static inline void silog(const char* format, va_list ap)
{
	//log_msg(6, format, ap);
	//6 -> LOG_LEVEL_DEBUG (this isn't part of the public api)
	char* tagged_fmt;
	asprintf(&tagged_fmt, "slurm-inception: %s", format);
	vsyslog(LOG_WARNING, tagged_fmt, ap);
	free(tagged_fmt);
}

int slurm_spank_task_init_privileged(spank_t sp, int ac, char** av)
{
	image_config_t iimage;
	memset(&iimage, 0, sizeof(image_config_t));
	if(image)
	{
		set_inception_log(&silog);
		slurm_debug("image is: \"%s\" from config \"%s\"\n", image, INCEPTION_CONFIG_PATH);
		if(parse_config(INCEPTION_CONFIG_PATH, image, &iimage) < 0)
		{
			slurm_error("Error loading inception image. Check the name. Your job may fail");
			free(image);
			return(-1);
		}
		free(image);
		image = NULL;
		image_cwd = NULL;
		slurm_debug("done parsing config");

    //allocate buffer for user requested cwd inside of inception image (or default)
	  image_cwd = (char*) malloc(sizeof(char*)*PATH_CWD_MAX);
    if(!image_cwd) 
    {
      free(image_cwd);
      return(1);
    }
    const spank_err_t cwd_result = envspank_getenv(sp, "SLURM_REMOTE_CWD", image_cwd, PATH_CWD_MAX - 1);
    if(cwd_result != ESPANK_SUCCESS)
    {
			slurm_error("Unable to extract SLURM_REMOTE_CWD from environment.");
      //return(1);
    }
    else iimage.cwd = image_cwd; 
		setup_namespace(&iimage);
	}
	return(0);
}
int slurm_spank_exit(spank_t sp, int ac, char** av)
{
	if(image)
		free(image);

  if(image_cwd)
    free(image_cwd);

	return(0);
}
