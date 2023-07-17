#include "fileManager.h"
#include "storage_devManager.h"
#include "list.h"
#include "fat.h"
#include "task.h"
#include "paging.h"
#include "string.h"
#include "math.h"

void file_init(){
    storage_devManager_init();
}

char* file_squashPath(char* filepath){
    char delimiter[] = "/\\";
    #define FILE_MAX_PATH_FILES 16
    #define FILE_MAX_FNAME_LEN 34
    char spl_path[FILE_MAX_PATH_FILES][FILE_MAX_FNAME_LEN];
    uint16_t ret_arr_size = strlen(filepath)+1;
    uint32_t i, j;
    for(i = 0; i < FILE_MAX_PATH_FILES; ++i){
        for(j = 0; j < FILE_MAX_FNAME_LEN; ++j){
            spl_path[i][j] = 0;
        }
    }
    uint32_t path_size = 0;
    char* cur_file = strtok(filepath, delimiter);
    while(cur_file != 0 && path_size < FILE_MAX_PATH_FILES){
        if(strlen(cur_file) > FILE_MAX_FNAME_LEN-1) break; //make sure current filename is not to long
        strcpy(spl_path[path_size], cur_file);
        cur_file = strtok(0, delimiter);
        ++path_size;
    }
    for(i = 0; i < path_size; ++i){
        if(strcmp(spl_path[i], ".") == 0){ //if spl_path[i] equals "."
            strcpy(spl_path[i], "");
            continue;
        }
        if(strcmp(spl_path[i], "..") == 0){ //if spl_path[i] equals ".."
            if(i > 0){
                for(j = i-1; j >= 0; --j){
                    if(strlen(spl_path[j]) != 0){
                        strcpy(spl_path[j], "");
                        break;
                    }
                }
            }
            strcpy(spl_path[i], "");
            continue;
        }
    }

    for(i = 0; i < ret_arr_size; ++i){
        filepath[i] = 0;
    }
    for(i = 0; i < path_size; ++i){
        if(strcmp(spl_path[i], "")){ //if spl_path[i] does NOT equal to ""
            if(strcmp(filepath, "") == 0){ //if filepath.Length is 0
                strcpy(filepath, spl_path[i]);
            }
            else{
                strcat(filepath, "/");
                strcat(filepath, spl_path[i]);
            }
        }
    }
    return filepath;
}

static Partition_t* fileManage_getPartition(char* filepath){
    // /a0/folder/file.txt
    file_squashPath(filepath);
    uint8_t ndevice;
    uint8_t npartition;
    if((filepath[0] == '~') && (filepath[1] == '~')) {
        ndevice = ODA.bootDev;
        npartition = ODA.bootPart;
    } else {
        ndevice = filepath[0] - 'a';
        npartition = filepath[1] - '1';
    }

    listHead_t* devices = storage_getDevList();
    if(devices == 0) return 0;
    storage_dev_t* device = list_getElement(devices, ndevice+1);
    if(device == 0) return 0;
    if(device->partitions == 0) return 0;
    return list_getElement(device->partitions, npartition+1);
}

uint8_t file_create(file_t* file_inst, char* filepath){
    Partition_t* partition = fileManage_getPartition(filepath);
    if(partition == 0) return 0;

    uint8_t tmp_filepath_len = MAX(strlen(filepath), 3) - 3;
    char tmp_filepath[tmp_filepath_len + 1];
    for(uint8_t i = 0; i < tmp_filepath_len; ++i)
        tmp_filepath[i] = filepath[i + 3];
    tmp_filepath[tmp_filepath_len] = 0;
    //file_t* file_inst = malloc(sizeof(file_t), 0);
    if(partition->createFile(partition->inst, tmp_filepath, file_inst) == 0){
        //free(file_inst);
        return 0; // Failed
    }
    file_inst->partition = partition;
    return 1; // Succeeded
}

uint8_t file_find(file_t* file_inst, char* filepath){
    Partition_t* partition = fileManage_getPartition(filepath);
    if(partition == 0) return 0;

    uint8_t tmp_filepath_len = MAX(strlen(filepath), 3) - 3;
    char tmp_filepath[tmp_filepath_len + 1];
    for(uint8_t i = 0; i < tmp_filepath_len; ++i)
        tmp_filepath[i] = filepath[i + 3];
    tmp_filepath[tmp_filepath_len] = 0;

    //file_t* file_inst = malloc(sizeof(file_t), 0);
    if(partition->findFile(partition->inst, tmp_filepath, file_inst) == 0){
        //free(file_inst);
        return 0; // Failed
    }
    file_inst->partition = partition;
    return 1; // Succeeded
}

void file_readContents(file_t* file_inst, uint8_t* buffer, uint32_t start, uint32_t len){
    file_inst->partition->readFileContents(file_inst->partition->inst, file_inst, buffer, start, len);
}

uint8_t file_isDirectory(char* filepath){
    Partition_t* partition = fileManage_getPartition(filepath);
    if(partition == 0) return 0;

    uint8_t tmp_filepath_len = MAX(strlen(filepath), 3) - 3;
    char tmp_filepath[tmp_filepath_len + 1];
    for(uint8_t i = 0; i < tmp_filepath_len; ++i)
        tmp_filepath[i] = filepath[i + 3];
    tmp_filepath[tmp_filepath_len] = 0;
    return partition->isDirectory(partition->inst, tmp_filepath);
}

uint8_t file_createDirectory(char* filepath){
    Partition_t* partition = fileManage_getPartition(filepath);
    if(partition == 0) return 0;

    uint8_t tmp_filepath_len = MAX(strlen(filepath), 3) - 3;
    char tmp_filepath[tmp_filepath_len + 1];
    for(uint8_t i = 0; i < tmp_filepath_len; ++i)
        tmp_filepath[i] = filepath[i + 3];
    tmp_filepath[tmp_filepath_len] = 0;
    //return 0; // just for debugging
    return partition->createDirectory(partition->inst, tmp_filepath);
}

uint8_t file_findByIndex(file_t* file_inst, char* dirpath, uint32_t index){
    Partition_t* partition = fileManage_getPartition(dirpath);
    if(partition == 0) return 0;
    
    uint8_t tmp_filepath_len = MAX(strlen(dirpath), 3) - 3;
    char tmp_filepath[tmp_filepath_len + 1];
    for(uint8_t i = 0; i < tmp_filepath_len; ++i)
        tmp_filepath[i] = dirpath[i + 3];
    tmp_filepath[tmp_filepath_len] = 0;
    if(!partition->findFileByIndex(partition->inst, file_inst, tmp_filepath, index)){
        return 0; // Failed
    }
    file_inst->partition = partition;
    return 1; // Succeeded
}

uint8_t file_execute(char* filepath){
    file_t file_inst = {{0}, {0}, 0, 0, 0};

    if(file_find(&file_inst, filepath)){
        uint8_t buffer[MIN(file_inst.size, PAGESIZE)];
        file_readContents(&file_inst, buffer, 0, MIN(file_inst.size, PAGESIZE));
        if((buffer[1] != 'E') || (buffer[2] != 'L') || (buffer[3] != 'F')) return 0;
        // TODO: add some checks to prevent buffer overflows when file is too small
        uint32_t elf_vaddr     = *( (uint32_t*)( buffer + 0x3C ) );
        uint32_t elf_offset    = *( (uint32_t*)( buffer + 0x38 ) );
        uint32_t elf_filesize  = *( (uint32_t*)( buffer + 0x44 ) );
        uint32_t elf_memsz     = *( (uint32_t*)( buffer + 0x48 ) );
        if( (elf_vaddr>0x5FF000) || (elf_vaddr<0x400000) || (elf_filesize>0xF0000) ) return 0;
        if( (elf_offset + elf_filesize) > file_inst.size ) return 0;

        page_directory_t* pd = paging_createPageDirectory();

        if(!paging_allocVirt(pd, (void*)elf_vaddr, alignUp(elf_memsz, PAGESIZE), MEM_USER | MEM_WRITABLE))
            return 0;

        page_directory_t* active_pagedir = paging_getActivePageDirectory();

        ODA.ts_flag = false;
        paging_switch(pd);
        ODA.ts_flag = true;
        file_readContents(&file_inst, (uint8_t*)elf_vaddr, elf_offset, elf_filesize);
        memset((void*)elf_vaddr + elf_filesize, 0, elf_memsz - elf_filesize); // fill BSS with zeros
        ODA.ts_flag = false;
        paging_switch(active_pagedir);

        create_task(pd, (void*)elf_vaddr, 3); // program in user space (ring 3) takes over
        ODA.ts_flag = true;
        sti();
        switch_context();
        return 1;
    }

    return 0;
}

uint8_t file_delete(char* filepath){
    Partition_t* partition = fileManage_getPartition(filepath);
    if(partition == 0) return 0;

    uint8_t tmp_filepath_len = MAX(strlen(filepath), 3) - 3;
    char tmp_filepath[tmp_filepath_len + 1];
    for(uint8_t i = 0; i < tmp_filepath_len; ++i)
        tmp_filepath[i] = filepath[i + 3];
    tmp_filepath[tmp_filepath_len] = 0;

    partition->deleteFile(partition->inst, tmp_filepath);
    return 1;
}

void fileManage_getBootPath(char* filepath) {
    filepath[0] = ODA.bootDev + 'a';
    filepath[1] = ODA.bootPart + '1';
    filepath[2] = 0;
}
