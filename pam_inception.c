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

#define PAM_SM_SESSION
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <security/pam_modules.h>

#include "inception.h"

#define close_syslog() \
		closelog(); \
		openlog(NULL, 0, 0); \
		closelog() 


extern char** environ;
void print_env(char** env)
{
	int i = 0;
	for(i=0;env[i] != NULL; i++)
	{
		syslog(LOG_WARNING, "%s", env[i]);
	}
}

PAM_EXTERN int pam_sm_open_session(pam_handle_t* pamh, int flags,
				int argc, const char** argv)
{
	image_config_t image;
	char* config_name = NULL;
	int ret = 0;
	char* user = NULL;
	//FIXME (maybe): we're dropping a const here rather than moving everything 
	//to a modern C dialect 
	openlog("pam_inception", LOG_PID|LOG_NDELAY|LOG_NOWAIT, LOG_AUTH);
	config_name = (char*) pam_getenv(pamh, "PBS_INCEPTION_IMAGE");
	print_env(environ);
	if(!config_name)
		config_name = (char*) pam_getenv(pamh, "INCEPTION_IMAGE");
	if(!config_name)
	{
		syslog(LOG_WARNING, "Running uncontained ;-(");
		close_syslog();
		return(PAM_SUCCESS);
	}
	ret = pam_get_item(pamh, PAM_USER, (const void**) &user);
	if(ret != PAM_SUCCESS)
	{
		syslog(LOG_ERR, "Error getting user: %s\n Inception FAILURE",
		 		pam_strerror(pamh, ret));
		close_syslog();
		return(PAM_SUCCESS);
	}
	memset(&image, 0, sizeof(image_config_t));
	ret = parse_config(INCEPTION_CONFIG_PATH, config_name, &image);
	if(ret != 0) 
	{
		syslog(LOG_WARNING, 
			"inception requested, but unable to find user: %s image: %s",
			 user,
			 config_name);
		close_syslog();
		return(PAM_SUCCESS);
	}
	else
	{
		syslog(LOG_WARNING, "containerizing user: %s image: %s", 
			user,
			config_name);
	}
	setup_namespace(&image);
	close_syslog();
	if(0)
		return(PAM_SESSION_ERR);
	return(PAM_SUCCESS);
}

PAM_EXTERN int pam_sm_close_session(pam_handle_t* pamh, int flags,
									int argc, const char** argv)
{
	return(PAM_SUCCESS);
}

