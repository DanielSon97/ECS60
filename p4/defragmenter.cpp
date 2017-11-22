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
  QuadraticHashTable<DiskBlock *> * diskStore = new QuadraticHashTable<DiskBlock *> (NULL, 30000);

  //creating diskIndex
  QuadraticHashTable<int> * diskManage = new QuadraticHashTable<int> (NULL, 200000);
  
  //creating nullList
  BinaryHeap<int> * nullList = new BinaryHeap<int> (capacity / 4);
  int a = capacity;
  //int nullCount = 0;
  while (/*nullCount <= 40000 && */a >= 2) {
    if (diskDrive->FAT[a] == false) {
      nullList->insert(a);
      //nullCount++;
    }
    a--;
  }
  
  //int nullMin = a;
  
  //hash in 
  int block;
  int index = 2;
  int nullMin;
  int returnBlock;
  int tempBlock;
  
  DiskBlock * currentBlock;
  DiskBlock * writeBlock;
  
  for (int b = 0; b < numFiles; b++) {
    block = diskDrive->directory[b].getFirstBlockID();
    diskDrive->directory[b].setFirstBlockID(index);
    do {
      currentBlock = diskDrive->readDiskBlock(block);
      if (block != index) {
        if (block < index) {
          do { //replace if lower than index
            returnBlock = diskManage->find(block); 
            
            if(returnBlock == 0) { //DiskBlock is in Store
              writeBlock = diskStore->find(block);
              diskDrive->writeDiskBlock(writeBlock, block);
              delete writeBlock;
              diskStore->remove(block);
              diskManage->remove(block);
            }
            else { //DiskBlock is in Manage
              writeBlock = diskDrive->readDiskBlock(returnBlock);
              diskDrive->writeDiskBlock(writeBlock, block);
              delete writeBlock;
              diskManage->remove(block);
              
              if (returnBlock >= index) {
                diskDrive->FAT[returnBlock] = false;
                nullList->insert(returnBlock);
                returnBlock = 0;
              }
              else {
                //remanaging block for loop
                block = returnBlock;
              }
            } 
          } while (returnBlock != 0);
          
          //insert new read into index if open
          if (diskDrive->FAT[index] == false) {
            diskDrive->writeDiskBlock(currentBlock, index);
            block = currentBlock->getNext();
            diskDrive->FAT[index] = true;
            
            delete currentBlock;
          }
          else { //store new read into store/manage
            if (diskStore->getCurrentSize() < 15000) { //insert into store
              diskManage->insert(0, index);
              diskStore->insert(currentBlock, index);
              block = currentBlock->getNext();
            }
            else { //insert into manage
              do {
                nullList->deleteMin(nullMin);
              } while (diskDrive->FAT[nullMin] == true);
              
              diskManage->insert(nullMin, index);
              diskDrive->writeDiskBlock(currentBlock, nullMin);
              diskDrive->FAT[nullMin] = true;
              block = currentBlock->getNext();
              delete currentBlock;
            }
          }
        }
        else { //block > index
          if (diskDrive->FAT[index] == false) {
            diskDrive->writeDiskBlock(currentBlock, index);
            diskDrive->FAT[block] = false;
            nullList->insert(block);
            block = currentBlock->getNext();
            delete currentBlock;
          }
          else { //store new read into store/manage
            if (diskStore->getCurrentSize() < 15000) { //insert into store
              diskManage->insert(0, index);
              diskStore->insert(currentBlock, index);
              block = currentBlock->getNext();
              diskDrive->FAT[block] = false;
              nullList->insert(block);
              block = currentBlock->getNext();
            }
            else { //insert into manage
              do {
                nullList->deleteMin(nullMin);
              } while (diskDrive->FAT[nullMin] == true);
              
              diskDrive->FAT[block] = false;
              diskManage->insert(nullMin, index);
              diskDrive->FAT[nullMin] = true;
              block = currentBlock->getNext();
              delete currentBlock;
            }
          }
        }
      }
      else {
        block = currentBlock->getNext();
      }
      index++;
    } while (block != 1);
  }
} // Defragmenter()
