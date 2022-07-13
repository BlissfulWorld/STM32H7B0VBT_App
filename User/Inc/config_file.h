#ifndef _CONFIG_FILE_H
#define _CONFIG_FILE_H

#include "stdint.h"

typedef struct config_file_s
{
  uint8_t md5[16];
  struct
  {
    char type[4];
    uint32_t size;
    uint8_t reserve[8];

    uint8_t data[0];
  }file;
}config_file_st;

typedef struct common_config_info_s
{
  char name[32];
  char desc[256];
  uint32_t version;
}common_config_info_st;

#include "test_board.h"

extern const unsigned char config_file_xp7_example[1696];
extern const unsigned char config_file_xp7plus_example[1700];
extern const unsigned char config_file_xp8plus_example[1700];

extern const char config_file_update_path[];

const config_file_st* config_file_load_from_board(board_header_st* header);
const config_file_st* config_file_load_from_disk(const char *path);
int config_file_unload(const config_file_st* config_file);
int config_file_update_to_board_from_disk(board_header_st* header,const char *path);
int config_file_data_write_to_board(board_header_st* header,const void* pdata, uint32_t size);
int common_config_info_phase(const config_file_st *config_file,common_config_info_st *info);

#endif
