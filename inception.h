#ifndef __INCEPTION_H__
#define __INCEPTION_H__

#include <sys/types.h>

#include <jansson.h>

#ifndef INCEPTION_CONFIG_PATH
#define INCEPTION_CONFIG_PATH "./inception.json"
#endif

typedef struct image_config
{
	size_t num_mounts;
	char** mount_from;
	char** mount_to;
	char** mount_type;
	char* imgroot;
	char* usercmd;
	char* shell_full_path;
	char* shell;
	char** environ;
	char* cwd;
} image_config_t;

void drop_permissions(uid_t real_uid, gid_t real_gid, char* real_name);

void do_bind_mounts(image_config_t* image);

int systemd_workaround(image_config_t* image);

void find_shell(image_config_t* image);

void setup_namespace(image_config_t* image);

int load_image(const json_t* const config_root, image_config_t* const image);

int parse_config(const char* const filename, const char* const key, image_config_t* const imagestru);

void build_default_environ(image_config_t* image);

char** load_insecure_environ(pid_t pid);

#endif
