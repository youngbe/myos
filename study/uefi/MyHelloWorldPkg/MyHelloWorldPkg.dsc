[Defines]
  PLATFORM_NAME                  = MyHelloWorld
  PLATFORM_GUID                  = D0099C60-538E-420C-FB40-DF6CA5DCDA77
  PLATFORM_VERSION               = 0.0
  DSC_SPECIFICATION              = 0x00010005
  SUPPORTED_ARCHITECTURES        = X64
  BUILD_TARGETS                  = RELEASE

!include MdePkg/MdeLibs.dsc.inc

[LibraryClasses]
UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf

[LibraryClasses.common.UEFI_APPLICATION]
MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf

[Components]
  MyHelloWorldPkg/main.inf
