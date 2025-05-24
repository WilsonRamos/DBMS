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
    
    Block(int id, int max_rec) 
        : block_id(id), max_records(max_rec), is_dirty(false) {}
    
    bool hasSpace() const {
        return records.size() < max_records;
    }
    
    bool addRecord(const Record& record) {
        if (!hasSpace()) {
            return false;
        }
        records.push_back(record);
        is_dirty = true;
        return true;
    }
    
    bool removeRecord(int record_id) {
        for (auto& record : records) {
            if (record.record_id == record_id) {
                record.is_deleted = true;
                is_dirty = true;
                return true;
            }
        }
        return false;
    }
    
    Record* findRecord(int record_id) {
        for (auto& record : records) {
            if (record.record_id == record_id && !record.is_deleted) {
                return &record;
            }
        }
        return nullptr;
    }
    
    std::vector<Record*> findRecordsByAttribute(const std::string& attribute, 
                                               const std::string& value, 
                                               const std::string& operator_type) {
        std::vector<Record*> results;
        
        for (auto& record : records) {
            if (record.is_deleted) continue;
            
            if (record.data.find(attribute) != record.data.end()) {
                std::string record_value = record.data.at(attribute);
                bool matches = false;
                
                if (operator_type == "=") {
                    matches = (record_value == value);
                } else if (operator_type == ">=") {
                    matches = (record_value >= value);
                } else if (operator_type == "<=") {
                    matches = (record_value <= value);
                } else if (operator_type == ">") {
                    matches = (record_value > value);
                } else if (operator_type == "<") {
                    matches = (record_value < value);
                }
                
                if (matches) {
                    results.push_back(&record);
                }
            }
        }
        
        return results;
    }
    
    void print() const {
        std::cout << "\n=== Block " << block_id << " ===\n";
        std::cout << "Location: ";
        location.print();
        std::cout << "Records: " << records.size() << "/" << max_records << "\n";
        std::cout << "Dirty: " << (is_dirty ? "Yes" : "No") << "\n";
        
        for (const auto& record : records) {
            if (!record.is_deleted) {
                record.print();
                std::cout << "---\n";
            }
        }
    }
};

// Buffer Manager - Gestiona bloques en memoria
class BufferManager {
private:
    std::unordered_map<int, Block*> buffer_pool;
    std::queue<int> lru_queue;  // Para política LRU simple
    int max_buffer_size;
    
public:
    BufferManager(int max_size) : max_buffer_size(max_size) {}
    
    ~BufferManager() {
        // Escribir todos los bloques sucios antes de destruir
        flushAllBlocks();
        
        // Liberar memoria
        for (auto& pair : buffer_pool) {
            delete pair.second;
        }
    }
    
    Block* getBlock(int block_id) {
        auto it = buffer_pool.find(block_id);
        if (it != buffer_pool.end()) {
            return it->second;
        }
        return nullptr;
    }
    
    bool addBlock(Block* block) {
        if (buffer_pool.size() >= max_buffer_size) {
            // Implementar política LRU - remover el bloque menos usado
            evictLRU();
        }
        
        buffer_pool[block->block_id] = block;
        lru_queue.push(block->block_id);
        return true;
    }
    
    void evictLRU() {
        if (lru_queue.empty()) return;
        
        int block_to_evict = lru_queue.front();
        lru_queue.pop();
        
        auto it = buffer_pool.find(block_to_evict);
        if (it != buffer_pool.end()) {
            Block* block = it->second;
            if (block->is_dirty) {
                // Escribir el bloque al disco antes de eliminarlo
                writeBlockToDisk(block);
            }
            delete block;
            buffer_pool.erase(it);
        }
    }
    
    void flushAllBlocks() {
        for (auto& pair : buffer_pool) {
            if (pair.second->is_dirty) {
                writeBlockToDisk(pair.second);
            }
        }
    }
    
    void writeBlockToDisk(Block* block) {
        // Simulación de escritura al disco
        std::cout << "Writing Block " << block->block_id << " to disk at location: ";
        block->location.print();
        block->is_dirty = false;
    }
    
    void printBufferStatus() {
        std::cout << "\n=== Buffer Manager Status ===\n";
        std::cout << "Blocks in buffer: " << buffer_pool.size() << "/" << max_buffer_size << "\n";
        
        for (const auto& pair : buffer_pool) {
            std::cout << "Block " << pair.first << " (Dirty: " 
                      << (pair.second->is_dirty ? "Yes" : "No") << ")\n";
        }
    }
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
                int sec_capacity, int rec_per_block, int buffer_size)
        : total_platters(num_platters), surfaces_per_platter(surfaces),
          tracks_per_surface(tracks), sectors_per_track(sectors),
          sector_capacity(sec_capacity), records_per_block(rec_per_block),
          next_record_id(1), next_block_id(1), buffer_manager(buffer_size) {
        
        // Inicializar estructura física del disco
        for (int p = 0; p < num_platters; ++p) {
            platters.emplace_back(p, surfaces, tracks, sectors, sec_capacity);
        }
        
        std::cout << "Disk initialized with:\n";
        std::cout << "- Platters: " << num_platters << "\n";
        std::cout << "- Surfaces per platter: " << surfaces << "\n";
        std::cout << "- Tracks per surface: " << tracks << "\n";
        std::cout << "- Sectors per track: " << sectors << "\n";
        std::cout << "- Sector capacity: " << sec_capacity << " bytes\n";
        std::cout << "- Records per block: " << rec_per_block << "\n";
    }
    
    // Calcular capacidades del disco
    long long getTotalCapacity() const {
        return (long long)total_platters * surfaces_per_platter * 
               tracks_per_surface * sectors_per_track * sector_capacity;
    }
    
    long long getUsedCapacity() const {
        long long used = 0;
        for (const auto& platter : platters) {
            for (const auto& surface : platter.surfaces) {
                for (const auto& track : surface.tracks) {
                    for (const auto& sector : track.sectors) {
                        used += sector.used_space;
                    }
                }
            }
        }
        return used;
    }
    
    long long getFreeCapacity() const {
        return getTotalCapacity() - getUsedCapacity();
    }
    
    // Encontrar ubicación para almacenar un bloque
    PhysicalLocation findLocationForBlock(int required_space) {
        for (int p = 0; p < platters.size(); ++p) {
            Surface* surface = platters[p].findSurfaceWithSpace(required_space);
            if (surface != nullptr) {
                Track* track = surface->findTrackWithSpace(required_space);
                if (track != nullptr) {
                    Sector* sector = track->findSectorWithSpace(required_space);
                    if (sector != nullptr) {
                        return PhysicalLocation(p, surface->surface_id, 
                                              track->track_id, sector->sector_id, 
                                              sector->used_space);
                    }
                }
            }
        }
        return PhysicalLocation(); // Ubicación inválida
    }
    
    // Almacenar un bloque en el disco
    bool storeBlock(Block* block) {
        Timer timer;
        timer.start();
        
        // Serializar el bloque para calcular el espacio requerido
        std::string block_data = "";
        for (const auto& record : block->records) {
            block_data += record.serialize() + "\n";
        }
        
        int required_space = block_data.length();
        PhysicalLocation location = findLocationForBlock(required_space);
        
        if (location.platter_id == -1) {
            std::cout << "Error: No space available for block\n";
            return false;
        }
        
        // Escribir en el sector correspondiente
        Sector& sector = platters[location.platter_id]
                        .surfaces[location.surface_id]
                        .tracks[location.track_id]
                        .sectors[location.sector_id];
        
        int position;
        if (sector.writeData(block_data, position)) {
            block->location = PhysicalLocation(location.platter_id, location.surface_id,
                                             location.track_id, location.sector_id, position);
            
            double elapsed_time = timer.getElapsedTime();
            std::cout << "Block " << block->block_id << " stored successfully in ";
            std::cout << elapsed_time << " ms at location: ";
            block->location.print();
            
            return true;
        }
        
        return false;
    }
    
    void printDiskStatus() {
        std::cout << "\n=== Disk Status ===\n";
        std::cout << "Total Capacity: " << getTotalCapacity() << " bytes\n";
        std::cout << "Used Capacity: " << getUsedCapacity() << " bytes\n";
        std::cout << "Free Capacity: " << getFreeCapacity() << " bytes\n";
        std::cout << "Usage: " << (double)getUsedCapacity() / getTotalCapacity() * 100 << "%\n";
    }
    
    BufferManager& getBufferManager() {
        return buffer_manager;
    }
};

#endif // DISK_MANAGER_H