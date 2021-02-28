#include <Uefi.h>
#include <Library/PcdLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/MemoryAllocationLib.h>
#include <Protocol/LoadedImage.h>
#include <Guid/FileInfo.h>

#define DESIRED_HREZ            640
#define DESIRED_VREZ            480
#define DESIRED_PIXEL_FORMAT    PixelBlueGreenRedReserved8BitPerColor

EFI_GUID gEfiLoadedImageProtocolGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
EFI_GUID gEfiFileInfoGuid = EFI_FILE_INFO_ID;

typedef struct {
    UINT32 baseLow;
    UINT32 baseHigh;
    UINT32 sizeLow;
    UINT32 sizeHigh;
    UINT32 type;
    UINT32 ext;
} E820Entry;

EFI_STATUS SwitchVideoMode(EFI_SYSTEM_TABLE *SystemTable) {
    EFI_STATUS status;
    EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;
    EFI_HANDLE* handle_buffer;
    UINTN handle_count = 0;
    
    status = SystemTable->BootServices->LocateHandleBuffer( ByProtocol,
                                        &gEfiGraphicsOutputProtocolGuid,
                                        NULL,
                                        &handle_count,
                                        &handle_buffer );

    status = SystemTable->BootServices->HandleProtocol( handle_buffer[0],
                                  &gEfiGraphicsOutputProtocolGuid,
                                  (void**)&gop );

    if(status != EFI_SUCCESS) return status;

    UINTN mode_num;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* gop_mode_info;
    UINTN size_of_info;

    for (mode_num = 0;
         (status = gop->QueryMode( gop, mode_num, &size_of_info, &gop_mode_info )) == EFI_SUCCESS;
         ++mode_num) {
        
        Print(L"mode_num: %u\n", mode_num);
        Print(L"\nHorizontalResolution: %u\n", gop_mode_info->HorizontalResolution);
        Print(L"\nHorizontalResolution: %u\n", gop_mode_info->VerticalResolution);
        Print(L"\nPixelFormat: %u\n", gop_mode_info->PixelFormat);

        if (gop_mode_info->HorizontalResolution == DESIRED_HREZ &&
            gop_mode_info->VerticalResolution == DESIRED_VREZ &&
            gop_mode_info->PixelFormat        == DESIRED_PIXEL_FORMAT)
            break;
    }

    status = gop->SetMode(gop, mode_num);
    if(status != EFI_SUCCESS) return status;

    EFI_PHYSICAL_ADDRESS frameBuffBase = gop->Mode->FrameBufferBase;

    Print(L"FrameBufferBase: 0x%X\n", frameBuffBase);

    UINT32* at = (UINT32*)((UINTN)frameBuffBase);
    *at = 0x00FF99FF;

    return EFI_SUCCESS;
}

EFI_STATUS LoadKernel(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    EFI_STATUS status;
    
    EFI_FILE *Kernel;
    
    EFI_HANDLE_PROTOCOL HandleProtocol = SystemTable->BootServices->HandleProtocol;

    EFI_LOADED_IMAGE_PROTOCOL *LoadedImage;
    HandleProtocol(ImageHandle, &gEfiLoadedImageProtocolGuid, (void**)&LoadedImage);

    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem;
    HandleProtocol(LoadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (void**)&FileSystem);

    EFI_FILE *Root;
    FileSystem->OpenVolume(FileSystem, &Root);

    status = Root->Open(Root, &Kernel, L"CHAOSKRN.SYS", EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);
    if (status != EFI_SUCCESS) {
        Print(L"Kernel is missing\n");
        return status;
    }
    Print(L"Kernel found\n");
    

    UINTN FileInfoSize;
    Kernel->GetInfo(Kernel, &gEfiFileInfoGuid, &FileInfoSize, NULL);
    // GetInfo will intentionally error out and provide the correct fileinfosize value
    
    Print(L"Reserving %u Bytes for file info\n", FileInfoSize);
    
    EFI_FILE_INFO *FileInfo;
    FileInfo = AllocatePool(FileInfoSize);
    
    // Actually get the metadata
    status = Kernel->GetInfo(Kernel, &gEfiFileInfoGuid, &FileInfoSize, FileInfo);
    if (status != EFI_SUCCESS) {
        Print(L"GetInfo error.\n");
        return status;
    }
    
    UINT32 fileSize = FileInfo->FileSize;

    Print(L"Filesize: %u\n", fileSize);

    INT32 pages = (fileSize + 0x1000 - 1) / 0x1000;
    void* segment = (void*) 0x40000;
    SystemTable->BootServices->AllocatePages(AllocateAddress, EfiLoaderData, pages, (EFI_PHYSICAL_ADDRESS*)&segment);

    Kernel->SetPosition(Kernel, 0);

    // Read the file in chunks as a workaround for some firmware implementations that have trouble reading files from some storage devices in one go.
    for (UINT32 i = 0; i < fileSize;) {
        UINT32 readSize = ((fileSize - i) > 0x10000) ? 0x10000 : (fileSize - i);
        status = Kernel->Read(Kernel, &readSize, segment + i);
        i += readSize;
    }

    UINTN MemMapSize;
    UINTN MemMapKey;
    UINTN MemMapDescriptorSize;
    UINT32 MemMapDescriptorVersion;
    EFI_MEMORY_DESCRIPTOR* MemMap = NULL;

    Print(L"Getting MemoryMap...\n");

    status = SystemTable->BootServices->GetMemoryMap(&MemMapSize, MemMap, &MemMapKey, &MemMapDescriptorSize, &MemMapDescriptorVersion);
    
    if((status & 0xFF) == (EFI_BUFFER_TOO_SMALL & 0xFF))
    {
        Print(L"EFI_BUFFER_TOO_SMALL\n");
        INT32 pages = (MemMapSize + 0x1000 - 1) / 0x1000;
        MemMap = (EFI_MEMORY_DESCRIPTOR*) 0x30000;
        status = SystemTable->BootServices->AllocatePages(AllocateAddress, EfiLoaderData, pages, (EFI_PHYSICAL_ADDRESS*)&MemMap);
        status = SystemTable->BootServices->GetMemoryMap(&MemMapSize, MemMap, &MemMapKey, &MemMapDescriptorSize, &MemMapDescriptorVersion);
        Print(L"MemMap: %X\n", MemMap);
    }

    Print(L"Exiting BootServices...\n");
    status = SystemTable->BootServices->ExitBootServices(ImageHandle, MemMapKey);
    if(EFI_ERROR(status)) // Error! EFI_INVALID_PARAMETER, MemMapKey is incorrect
    {
        Print(L"ExitBootServices #1 failed, Trying again...\n");

        MemMapSize = 0;
        status = SystemTable->BootServices->GetMemoryMap(&MemMapSize, MemMap, &MemMapKey, &MemMapDescriptorSize, &MemMapDescriptorVersion);
        if(status == EFI_BUFFER_TOO_SMALL)
        {
            status = SystemTable->BootServices->AllocatePool(EfiBootServicesData, MemMapSize, (void **)&MemMap);
            if(EFI_ERROR(status)) // Error! Wouldn't be safe to continue.
            {
                Print(L"MemMap AllocatePool error #2\n");
                return status;
            }
            status = SystemTable->BootServices->GetMemoryMap(&MemMapSize, MemMap, &MemMapKey, &MemMapDescriptorSize, &MemMapDescriptorVersion);
        }
        Print(L"A\n");
        status = SystemTable->BootServices->ExitBootServices(ImageHandle, MemMapKey);

        if(status != EFI_SUCCESS) {
            Print(L"ExitBootServices #2 failed, Quiting...\n");
            return EFI_ABORTED;
        }
    }

    // Convert memory map to E820 format and write it to 0x1000
    
    // EFI Memory Type                  E820 Region Type
    // EfiReservedMemoryType        0   Reserved            2
    // EfiLoaderCode                1   Usable              1
    // EfiLoaderData                2   Usable              1
    // EfiBootServicesCode          3   Usable              1
    // EfiBootServicesData          4   Usable              1
    // EfiRuntimeServicesCode       5   Reserved            2
    // EfiRuntimeServicesData       6   Reserved            2
    // EfiConventionalMemory        7   Usable              1
    // EfiUnusableMemory            8   Bad memory          5
    // EfiACPIReclaimMemory         9   ACPI reclaimable    3
    // EfiACPIMemoryNVS             10  ACPI NVS            4
    // EfiMemoryMappedIO            11  Reserved            2
    // EfiMemoryMappedIOPortSpace   12  Reserved            2
    // EfiPalCode                   13  Reserved            2

    UINT8 E820Conversion[14] = {2, 1, 1, 1, 1, 2, 2, 1, 5, 3, 4, 2, 2, 2};
    UINT8* kernelMemMap = (UINT8*)0x1000;
    UINT32 counter = 0;

    while((counter * (UINT32)MemMapDescriptorSize) < MemMapSize){
        EFI_MEMORY_DESCRIPTOR* desc = (EFI_MEMORY_DESCRIPTOR*) ((UINT32)MemMap + counter * (UINT32)MemMapDescriptorSize);
        E820Entry* kernMapEntry = (E820Entry*) (kernelMemMap + counter * sizeof(E820Entry));
        UINT64 base = desc->PhysicalStart;
        kernMapEntry->baseLow = base;
        kernMapEntry->baseHigh = base >> 32;
        UINT64 size = (desc->NumberOfPages) * 4096;
        kernMapEntry->sizeLow = size;
        kernMapEntry->sizeHigh = size >> 32;
        kernMapEntry->type = E820Conversion[desc->Type];
        kernMapEntry->ext = 0;
        ++counter;
    }
    UINT16* counterTmp = (UINT16*)0x0FFE;
    counterTmp[0] = counter;
    
    void(*entry)() = (void*) segment;
    entry();

    return EFI_ABORTED;
}

/**
     The user Entry Point for Application. The user code starts with this function
    as the real entry point for the application.

    @param[in] ImageHandle    The firmware allocated handle for the EFI image.
    @param[in] SystemTable    A pointer to the EFI System Table.

    @retval EFI_SUCCESS       The entry point is executed successfully.
    @retval other             Some error occurs when executing this entry point.
**/
EFI_STATUS
EFIAPI
UefiMain (
    IN EFI_HANDLE        ImageHandle,
    IN EFI_SYSTEM_TABLE  *SystemTable
    )
{
    //SwitchVideoMode(SystemTable);

    return LoadKernel(ImageHandle, SystemTable);
}
