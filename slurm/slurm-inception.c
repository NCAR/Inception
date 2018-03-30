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

#include "inception.h"

#include <slurm/spank.h>

SPANK_PLUGIN(inception, 1);

int validate_opts(int val, const char* optarg, int remote)
{
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


int slurm_spank_task_init_privileged(spank_t sp, int ac, char** av)
{
	char* image = NULL;
	image_config_t iimage;
	memset(&iimage, 0, sizeof(image_config_t));
	spank_err_t err = spank_option_getopt(sp, &image_opt, &image);
	if(err != 0)
		slurm_error("spank error: %s", spank_strerror(err));
	if(image)
	{
		slurm_debug("image is: %s\n", image);
		parse_config(INCEPTION_CONFIG_PATH, image, &iimage);
		setup_namespace(&iimage);
	}
	return(0);
}
int slurm_spank_exit(spank_t sp, int ac, char** av)
{
	return(0);
}
