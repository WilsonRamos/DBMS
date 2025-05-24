#include "sgbd.h"

// ==================== SGBD IMPLEMENTATION ====================
SGBD::SGBD(int platters, int surfaces, int tracks, int sectors, 
     int sector_cap, int rec_per_block, int buffer_size)
    : disk_manager(platters, surfaces, tracks, sectors, sector_cap, rec_per_block, buffer_size),
      next_record_id(1) {
    
    std::cout << "\n=== SGBD System Initialized ===\n";
    disk_manager.printDiskStatus();
}

bool SGBD::loadFromCSV(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "Error: Cannot open file " << filename << std::endl;
        return false;
    }
    
    Timer timer;
    timer.start();
    
    std::string line;
    std::vector<std::string> headers;
    bool first_line = true;
    int records_loaded = 0;
    
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string token;
        std::vector<std::string> tokens;
        
        // Parsear línea CSV
        while (std::getline(iss, token, ',')) {
            // Remover espacios y comillas
            token.erase(std::remove(token.begin(), token.end(), '\"'), token.end());
            token.erase(std::remove(token.begin(), token.end(), ' '), token.end());
            tokens.push_back(token);
        }
        
        if (first_line) {
            headers = tokens;
            first_line = false;
            continue;
        }
        
        // Crear registro
        std::map<std::string, std::string> record_data;
        for (size_t i = 0; i < headers.size() && i < tokens.size(); ++i) {
            record_data[headers[i]] = tokens[i];
        }
        
        Record new_record(record_data, next_record_id++);
        if (addRecord(new_record)) {
            records_loaded++;
        }
    }
    
    double elapsed_time = timer.getElapsedTime();
    std::cout << "Loaded " << records_loaded << " records from " << filename 
              << " in " << elapsed_time << " ms\n";
    
    file.close();
    return true;
}

bool SGBD::addRecord(const Record& record) {
    Timer timer;
    timer.start();
    
    // Buscar un bloque con espacio disponible
    Block* target_block = nullptr;
    
    for (auto& pair : all_blocks) {
        if (pair.second->hasSpace()) {
            target_block = pair.second;
            break;
        }
    }
    
    // Si no hay bloque disponible, crear uno nuevo
    if (target_block == nullptr) {
        target_block = new Block(static_cast<int>(all_blocks.size()) + 1, 5); // 5 registros por bloque
        all_blocks[target_block->block_id] = target_block;
        
        // Almacenar el bloque en el disco
        if (!disk_manager.storeBlock(target_block)) {
            delete target_block;
            all_blocks.erase(target_block->block_id);
            return false;
        }
        
        // Añadir al buffer manager
        disk_manager.getBufferManager().addBlock(target_block);
    }
    
    bool success = target_block->addRecord(record);
    
    if (success) {
        double elapsed_time = timer.getElapsedTime();
        std::cout << "Record " << record.record_id << " added successfully in " 
                  << elapsed_time << " ms\n";
        std::cout << "Location: ";
        target_block->location.print();
    }
    
    return success;
}

Record* SGBD::findRecord(int record_id) {
    Timer timer;
    timer.start();
    
    for (auto& pair : all_blocks) {
        Record* record = pair.second->findRecord(record_id);
        if (record != nullptr) {
            double elapsed_time = timer.getElapsedTime();
            std::cout << "Record found in " << elapsed_time << " ms\n";
            std::cout << "Location: ";
            pair.second->location.print();
            return record;
        }
    }
    
    std::cout << "Record not found\n";
    return nullptr;
}

std::vector<Record*> SGBD::findRecordsByAttribute(const std::string& attribute, 
                                           const std::string& value, 
                                           const std::string& operator_type) {
    Timer timer;
    timer.start();
    
    std::vector<Record*> results;
    
    for (auto& pair : all_blocks) {
        std::vector<Record*> block_results = pair.second->findRecordsByAttribute(
            attribute, value, operator_type);
        
        for (Record* record : block_results) {
            results.push_back(record);
        }
    }
    
    double elapsed_time = timer.getElapsedTime();
    std::cout << "Query completed in " << elapsed_time << " ms\n";
    std::cout << "Found " << results.size() << " records\n";
    
    return results;
}

std::vector<Record*> SGBD::getAllRecords() {
    Timer timer;
    timer.start();
    
    std::vector<Record*> results;
    
    for (auto& pair : all_blocks) {
        for (auto& record : pair.second->records) {
            if (!record.is_deleted) {
                results.push_back(&record);
            }
        }
    }
    
    double elapsed_time = timer.getElapsedTime();
    std::cout << "Retrieved all " << results.size() << " records in " 
              << elapsed_time << " ms\n";
    
    return results;
}

bool SGBD::deleteRecord(int record_id) {
    Timer timer;
    timer.start();
    
    for (auto& pair : all_blocks) {
        if (pair.second->removeRecord(record_id)) {
            double elapsed_time = timer.getElapsedTime();
            std::cout << "Record " << record_id << " deleted in " 
                      << elapsed_time << " ms\n";
            std::cout << "Location: ";
            pair.second->location.print();
            return true;
        }
    }
    
    std::cout << "Record not found for deletion\n";
    return false;
}

void SGBD::showBlockContent(int block_id) {
    Timer timer;
    timer.start();
    
    auto it = all_blocks.find(block_id);
    if (it != all_blocks.end()) {
        it->second->print();
        double elapsed_time = timer.getElapsedTime();
        std::cout << "Block content displayed in " << elapsed_time << " ms\n";
    } else {
        std::cout << "Block " << block_id << " not found\n";
    }
}

void SGBD::showAllBlocks() {
    std::cout << "\n=== All Blocks Information ===\n";
    for (auto& pair : all_blocks) {
        pair.second->print();
    }
}

void SGBD::showSystemStats() {
    std::cout << "\n=== System Statistics ===\n";
    disk_manager.printDiskStatus();
    disk_manager.getBufferManager().printBufferStatus();
    
    std::cout << "\nBlocks Information:\n";
    std::cout << "Total blocks: " << all_blocks.size() << "\n";
    
    int total_records = 0;
    int deleted_records = 0;
    
    for (auto& pair : all_blocks) {
        for (auto& record : pair.second->records) {
            total_records++;
            if (record.is_deleted) {
                deleted_records++;
            }
        }
    }
    
    std::cout << "Total records: " << total_records << "\n";
    std::cout << "Active records: " << (total_records - deleted_records) << "\n";
    std::cout << "Deleted records: " << deleted_records << "\n";
}

void SGBD::simulateFullBlock() {
    std::cout << "\n=== Simulating Full Block Scenario ===\n";
    
    // Crear un bloque pequeño (solo 2 registros)
    Block* small_block = new Block(999, 2);
    
    // Llenar el bloque
    std::map<std::string, std::string> data1 = {{"name", "Test1"}, {"value", "100"}};
    std::map<std::string, std::string> data2 = {{"name", "Test2"}, {"value", "200"}};
    
    Record r1(data1, 9001);
    Record r2(data2, 9002);
    
    small_block->addRecord(r1);
    small_block->addRecord(r2);
    
    std::cout << "Block filled with " << small_block->records.size() << " records\n";
    
    // Intentar añadir otro registro
    std::map<std::string, std::string> data3 = {{"name", "Test3"}, {"value", "300"}};
    Record r3(data3, 9003);
    
    Timer timer;
    timer.start();
    
    if (!small_block->addRecord(r3)) {
        double elapsed_time = timer.getElapsedTime();
        std::cout << "Block is full! Cannot add more records. Time: " 
                  << elapsed_time << " ms\n";
        std::cout << "Creating new block for overflow...\n";
        
        // Crear nuevo bloque para el registro overflow
        Block* new_block = new Block(1000, 5);
        if (new_block->addRecord(r3)) {
            all_blocks[new_block->block_id] = new_block;
            disk_manager.storeBlock(new_block);
            std::cout << "Record added to new block successfully\n";
        }
    }
    
    delete small_block;
}

void SGBD::simulateFullSectors() {
    std::cout << "\n=== Simulating Full Sectors Scenario ===\n";
    
    // Crear muchos bloques para llenar sectores
    for (int i = 0; i < 20; ++i) {
        Block* block = new Block(2000 + i, 3);
        
        // Llenar cada bloque con datos
        for (int j = 0; j < 3; ++j) {
            std::map<std::string, std::string> data = {
                {"id", std::to_string(i * 3 + j)},
                {"data", "Large data string to fill sector space quickly " + std::to_string(i)},
                {"timestamp", "2024-01-01"},
                {"category", "simulation"}
            };
            Record record(data, 8000 + i * 3 + j);
            block->addRecord(record);
        }
        
        Timer timer;
        timer.start();
        
        if (disk_manager.storeBlock(block)) {
            all_blocks[block->block_id] = block;
        } else {
            double elapsed_time = timer.getElapsedTime();
            std::cout << "Sector full! Cannot store block " << block->block_id 
                      << ". Time: " << elapsed_time << " ms\n";
            delete block;
            break;
        }
    }
}

// ==================== AUXILIARY FUNCTIONS ====================
void createTitanicSample() {
    std::ofstream file("titanic_sample.csv");
    file << "PassengerId,Survived,Pclass,Name,Sex,Age,SibSp,Parch,Ticket,Fare,Cabin,Embarked\n";
    file << "1,0,3,Braund Mr. Owen Harris,male,22,1,0,A/5 21171,7.25,,S\n";
    file << "2,1,1,Cumings Mrs. John Bradley,female,38,1,0,PC 17599,71.2833,C85,C\n";
    file << "3,1,3,Heikkinen Miss. Laina,female,26,0,0,STON/O2. 3101282,7.925,,S\n";
    file << "4,1,1,Futrelle Mrs. Jacques Heath,female,35,1,0,113803,53.1,C123,S\n";
    file << "5,0,3,Allen Mr. William Henry,male,35,0,0,373450,8.05,,S\n";
    file.close();
}

void createHousingSample() {
    std::ofstream file("housing_sample.csv");
    file << "price,bedrooms,bathrooms,sqft_living,sqft_lot,floors,waterfront,view\n";
    file << "221900,3,1,1180,5650,1,0,0\n";
    file << "538000,3,2.25,2570,7242,2,0,0\n";
    file << "180000,2,1,770,10000,1,0,0\n";
    file << "604000,4,3,1960,5000,1,0,0\n";
    file << "510000,3,2,1680,8080,1,0,0\n";
    file.close();
}