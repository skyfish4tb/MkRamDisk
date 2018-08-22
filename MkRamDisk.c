/*
 *  MkRamDisk
 *  Copyright (C) 2018  A1ive
 *
 *  MkRamDisk is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  MkRamDisk is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with MkRamDisk.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/ShellCEntryLib.h>
#include <Library/PrintLib.h>
#include <Library/BaseLib.h>
#include <Library/ShellLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>


#include <Protocol/EfiShell.h>
#include <Protocol/DevicePathToText.h>
#include <Protocol/RamDisk.h>
#include <Protocol/SimpleFileSystem.h>


/***
  Print a welcoming message.

  Establishes the main structure of the application.

  @retval  0         The application exited normally.
  @retval  Other     An error occurred.
***/
INTN
EFIAPI
ShellAppMain (
  IN UINTN Argc,
  IN CHAR16 **Argv
  )
{
    EFI_STATUS               Status;
    EFI_RAM_DISK_PROTOCOL    *MyRamDisk;
    UINT64                   *StartingAddr;
    EFI_DEVICE_PATH_PROTOCOL *DevicePath;
    EFI_FILE_HANDLE          FileHandle;
    EFI_FILE_INFO            *FileInfo;
    UINTN                    ReadSize;
    CHAR16	  		              *CmdLine=L"map -r";
	   EFI_STATUS           	  CmdStatus;
    
    
    if (Argc < 2) {
        Print(L"ERROR!\n");
        return EFI_SUCCESS;
    }
    
    Print(L"File: %s \n", *(Argv+1));
    
  // Look for Ram Disk Protocol
    Status = gBS->LocateProtocol (
                &gEfiRamDiskProtocolGuid,
                NULL,
                (VOID**)&MyRamDisk
             );
    if (EFI_ERROR (Status)) {
        Print(L"Couldn't find RamDiskProtocol\n");
        return EFI_ALREADY_STARTED;
    }

    Status = ShellOpenFileByName(
                *(Argv+1), 
                (SHELL_FILE_HANDLE *)&FileHandle,
                EFI_FILE_MODE_READ, 
                0);
    
    FileInfo = ShellGetFileInfo((SHELL_FILE_HANDLE)FileHandle);
    Status = gBS->AllocatePool (
                EfiReservedMemoryType,
                (UINTN)FileInfo-> FileSize,
                (VOID**)&StartingAddr
                );
    if(EFI_ERROR (Status)) {
        Print(L"Allocate Memory failed!\n");
        return EFI_SUCCESS;
    }

    //Load the whole file to the buffer
    Status = ShellReadFile(FileHandle,&ReadSize,StartingAddr);
    if(EFI_ERROR (Status)) {
            Print(L"Read file failed!\n");
            return EFI_SUCCESS;
        }

        
    //
    // Register the newly created RAM disk.
    //
    Status = MyRamDisk->Register (
                ((UINT64)(UINTN) StartingAddr),
                FileInfo-> FileSize,
                &gEfiVirtualDiskGuid,
                NULL,
                &DevicePath
                );
    if (EFI_ERROR (Status)) {
        Print(L"Can't create RAM Disk!\n");
        return EFI_SUCCESS;
    }

        //Show RamDisk DevicePath
    {
        EFI_DEVICE_PATH_TO_TEXT_PROTOCOL* Device2TextProtocol;
        CHAR16*                           TextDevicePath = 0;
        Status = gBS->LocateProtocol(
                    &gEfiDevicePathToTextProtocolGuid,
                    NULL,
                    (VOID**)&Device2TextProtocol
                    );  
        TextDevicePath = 
            Device2TextProtocol->ConvertDevicePathToText(DevicePath, FALSE, TRUE); 
        Print(L"DevicePath=%s\n", TextDevicePath);
        Print(L"Disk Size =%d Bytes\n", FileInfo-> FileSize);
        if(TextDevicePath)gBS->FreePool(TextDevicePath);
    }      
  
    Print(L"Creat Ram Disk success!\n");
    
    Status = ShellExecute( &gImageHandle, CmdLine, FALSE, NULL, &CmdStatus);

    return EFI_SUCCESS;
}
