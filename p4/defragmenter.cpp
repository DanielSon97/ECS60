#include "defragmenter.h"
#include "DefragRunner.h"
#include "mynew.h"
#include "QuadraticProbing.h"
#include "BinaryHeap.h"

Defragmenter::Defragmenter(DiskDrive *diskDrive)
{
  //copying over constants from diskDrive
  int numFiles = diskDrive->getNumFiles();
  int capacity = diskDrive->getCapacity();
  
  //creating diskSim
  QuadraticHashTable<DiskBlock *> * diskStore;
  diskStore = new QuadraticHashTable<DiskBlock *> (NULL, 30000);
  /*
  //creating diskIndex
  int *diskIndex;
  diskIndex = new int[30];
  
  //creating nullList
  nullList = new StackLi<int>;
  */
  
  
  /*BinaryHeap<int> * nullList;
  nullList = new BinaryHeap<int> (200000);
  */
  //fill in nullList
  /*for (int a = capacity; a >= 0; a--) {
    if (diskDrive->FAT[a] == false) {
      nullList->push(a);
    }
  }*/
  
  //hash in 
  int block;
  int size;
  int fileID;
  int index = 2;
  DiskBlock * currentBlock;
  DiskBlock * writeBlock;
  
  for (int b = 0; b < numFiles; b++) {
    block = diskDrive->directory[b].getFirstBlockID();
    size = diskDrive->directory[b].getSize();
    fileID = diskDrive->directory[b].getFileID();
    do {
      currentBlock = diskDrive->readDiskBlock(block);
      if (block != index) {
        if (block < index) {
          writeBlock = diskStore->find(block);
          diskDrive->writeDiskBlock(writeBlock, block);
          delete writeBlock;
          diskStore->remove(block);
          if (diskDrive->FAT[index] == false) {
            diskDrive->writeDiskBlock(currentBlock, index);
            block = currentBlock->getNext();
            delete currentBlock;
          }
          else {
            diskStore->insert(currentBlock, index);
            block = currentBlock->getNext();
          }
        }
        else {
          diskStore->insert(currentBlock, index);
          diskDrive->FAT[block] = false;
          block = currentBlock->getNext();
          //make a line for null list
        }
      }
      else {
        block = currentBlock->getNext();
      }
      index++;
    } while (block != 1);
  }
} // Defragmenter()