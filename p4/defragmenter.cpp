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
  int storeSize = 19500;
  int storeMax = storeSize / 2;
  
  
  //creating diskStore
  QuadraticHashTable<DiskBlock *> * diskStore = new QuadraticHashTable<DiskBlock *> (NULL, storeSize);

  //creating diskManage
  QuadraticHashTable<int> * diskManage = new QuadraticHashTable<int> (0, 200000);
  
  //creating nullList
  int maxNull = capacity / 2; 
  BinaryHeap<int> * nullList = new BinaryHeap<int> (maxNull);
  int a = capacity;
  int nullCount = 0;
  int cutOff = (maxNull / 3) * 2;
  while (nullCount <= cutOff && a >= 2) {
    if (!diskDrive->FAT[a]) {
      nullList->insert(a);
      nullCount++;
    }
    a--;
  }
  int minNull = (capacity / 5) * 3;
  
  //hash in 
  int block;
  int index = 2;
  int nullMin;
  
  DiskBlock * currentBlock;
  DiskBlock * writeBlock;
  DiskBlock * tempBlock;
  int returnBlock;
  bool recheck = false;
  
  for (int b = 0; b < numFiles; b++) {
    block = diskDrive->directory[b].getFirstBlockID();
    diskDrive->directory[b].setFirstBlockID(index);
    do {
      currentBlock = diskDrive->readDiskBlock(block);
      if (block != index) {
        if (block < index) {
          do { //replace if lower than index
            tempBlock = diskStore->find(block);
            
            if (tempBlock != NULL) { //DiskBlock is in Store
              diskDrive->writeDiskBlock(tempBlock, block);
              delete tempBlock;
              diskStore->remove(block);
              recheck = false;
            }
            
            else { //DiskBlock is in Manage
              returnBlock = diskManage->find(block);
              writeBlock = diskDrive->readDiskBlock(returnBlock);
              diskDrive->writeDiskBlock(writeBlock, block);
              delete writeBlock;
              diskManage->remove(block);
              
              if (returnBlock >= index) {
                diskDrive->FAT[returnBlock] = false;
                if (returnBlock > minNull && !nullList->isFull()) {
                  nullList->insert(returnBlock);
                }
                recheck = false;
              }
              else {
                //remanaging block for loop
                block = returnBlock;
                recheck = true;
              }
            }
          } while (recheck);
                    
          if (!diskDrive->FAT[index]) { //insert new read into index if open
            diskDrive->writeDiskBlock(currentBlock, index);
            block = currentBlock->getNext();
            diskDrive->FAT[index] = true;
            delete currentBlock;
          }
          else { //store new read into store/manage
            if (diskStore->getCurrentSize() < storeMax) { //insert into store
              diskStore->insert(currentBlock, index);
              block = currentBlock->getNext();
            }
            else { //insert into manage
              do {
                nullList->deleteMin(nullMin);
              } while (diskDrive->FAT[nullMin]);
              
              diskManage->insert(nullMin, index);
              diskDrive->writeDiskBlock(currentBlock, nullMin);
              diskDrive->FAT[nullMin] = true;
              block = currentBlock->getNext();
              delete currentBlock;
            }
          }
        }
        
        else { //block > index
          if (!diskDrive->FAT[index]) {
            diskDrive->writeDiskBlock(currentBlock, index);
            diskDrive->FAT[index] = true;
            diskDrive->FAT[block] = false;
            if (block > minNull && !nullList->isFull()) {
              nullList->insert(block);
            }
            block = currentBlock->getNext();
            delete currentBlock;
          }
          else { //store new read into store/manage
            if (diskStore->getCurrentSize() < storeMax) { //insert into store
              diskStore->insert(currentBlock, index);
              diskDrive->FAT[block] = false;
              if (block > minNull && !nullList->isFull()) {
                nullList->insert(block);
              }
              block = currentBlock->getNext();
            }
            else { //insert into manage
              do {
                nullList->deleteMin(nullMin);
              } while (diskDrive->FAT[nullMin]);
              
              diskDrive->FAT[block] = false;
              if (block > minNull && !nullList->isFull()) {
                nullList->insert(block);
              }
              diskManage->insert(nullMin, index);
              diskDrive->writeDiskBlock(currentBlock, nullMin);
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
