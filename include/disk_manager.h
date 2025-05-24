#ifndef DISK_MANAGER_H
#define DISK_MANAGER_H

#include "sgbd_basic.h"
#include <unordered_map>
#include <queue>

// Clase para representar un bloque de datos
class Block {
public:
    int block_id;
    std::vector<Record> records;
    int max_records;
    PhysicalLocation location;
    bool is_dirty;  // Indica si el bloque ha sido modificado
    
    Block(int id, int max_rec);
    bool hasSpace() const;
    bool addRecord(const Record& record);
    bool removeRecord(int record_id);
    Record* findRecord(int record_id);
    std::vector<Record*> findRecordsByAttribute(const std::string& attribute, 
                                               const std::string& value, 
                                               const std::string& operator_type);
    void print() const;
};

// Buffer Manager - Gestiona bloques en memoria
class BufferManager {
private:
    std::unordered_map<int, Block*> buffer_pool;
    std::queue<int> lru_queue;  // Para política LRU simple
    int max_buffer_size;
    
public:
    BufferManager(int max_size);
    ~BufferManager();
    
    Block* getBlock(int block_id);
    bool addBlock(Block* block);
    void evictLRU();
    void flushAllBlocks();
    void writeBlockToDisk(Block* block);
    void printBufferStatus();
};

// Disk Manager - Gestiona la estructura física del disco
class DiskManager {
private:
    std::vector<Platter> platters;
    int total_platters;
    int surfaces_per_platter;
    int tracks_per_surface;
    int sectors_per_track;
    int sector_capacity;
    int records_per_block;
    
    int next_record_id;
    int next_block_id;
    
    std::unordered_map<int, PhysicalLocation> record_locations;
    BufferManager buffer_manager;
    
public:
    DiskManager(int num_platters, int surfaces, int tracks, int sectors, 
                int sec_capacity, int rec_per_block, int buffer_size);
    
    // Calcular capacidades del disco
    long long getTotalCapacity() const;
    long long getUsedCapacity() const;
    long long getFreeCapacity() const;
    
    // Encontrar ubicación para almacenar un bloque
    PhysicalLocation findLocationForBlock(int required_space);
    
    // Almacenar un bloque en el disco
    bool storeBlock(Block* block);
    
    void printDiskStatus();
    BufferManager& getBufferManager();
};

#endif // DISK_MANAGER_H