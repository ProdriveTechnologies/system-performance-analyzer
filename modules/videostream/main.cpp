#include "linux/shared_memory.h"
#include <iostream>
int main()
{
  std::cout << "Starting application" << std::endl;

  Linux::SharedMemory::Configuration shMemConfig;
  shMemConfig.fileName = "/filename";
  shMemConfig.memorySize = 1000;
  shMemConfig.mode = Linux::SharedMemory::Mode::READ_WRITE;

  Linux::SharedMemory sharedMem{shMemConfig};
  std::cout << "Constructor done" << std::endl;
  auto result = sharedMem.Allocate();
  if (result != Linux::SharedMemory::Error::NONE)
  {
    std::cout << "Error occured during allocation!" << static_cast<int>(result)
              << std::endl;
    perror("mmap error");
    return -1;
  }
  std::cout << "Allocation done" << std::endl;
  struct StoreData
  {
    int cookie = 12;
    int black = 5114;
    char clong = '4';
  };
  StoreData storeData;
  std::cout << "Starting StoreData" << std::endl;
  sharedMem.Write(&storeData, sizeof(StoreData));
  std::cout << "Data written" << std::endl;

  auto readData = sharedMem.Read(sizeof(StoreData));
  std::cout << "Read the data again" << std::endl;
  StoreData *data = reinterpret_cast<StoreData *>(readData.get());
  std::cout << "The data is: " << data->black << " - " << data->clong << " - "
            << data->cookie << std::endl;
}

/*
    DDR uitlezen (kan niet direct de registers uitlezen)

    DDR0 main addressen: 0x004800000000
    DDR1 main addressen: 0x004A00000000
    0xE0000000 (mbit aan data, mag naar geschreven worden)

    selfifg0_h2c_0
*/