#include "sgbd.h"
#include <iostream>

// Función principal de demostración
int main() {
    std::cout << "=== SGBD Implementation Demo ===\n\n";
    
    // Crear archivos de ejemplo
    createTitanicSample();
    createHousingSample();
    
    // Configuración del disco:
    // 2 platos, 2 superficies por plato, 10 pistas por superficie
    // 8 sectores por pista, 512 bytes por sector, 5 registros por bloque
    // Buffer de 10 bloques
    SGBD system(2, 2, 10, 8, 512, 5, 10);
    
    std::cout << "\n=== Loading Titanic Data ===\n";
    system.loadFromCSV("titanic_sample.csv");
    
    std::cout << "\n=== Loading Housing Data ===\n";
    system.loadFromCSV("housing_sample.csv");
    
    std::cout << "\n=== Adding Individual Record ===\n";
    std::map<std::string, std::string> individual_record = {
        {"name", "John Doe"},
        {"age", "30"},
        {"city", "New York"}
    };
    Record new_record(individual_record, 999);
    system.addRecord(new_record);
    
    std::cout << "\n=== Querying Single Record ===\n";
    Record* found = system.findRecord(1);
    if (found) {
        found->print();
    }
    
    std::cout << "\n=== Querying Records by Attribute ===\n";
    auto results = system.findRecordsByAttribute("Sex", "female", "=");
    std::cout << "Female passengers:\n";
    for (Record* record : results) {
        record->print();
        std::cout << "---\n";
    }
    
    std::cout << "\n=== Querying All Records ===\n";
    auto all_records = system.getAllRecords();
    std::cout << "Total active records: " << all_records.size() << "\n";
    
    std::cout << "\n=== Deleting a Record ===\n";
    system.deleteRecord(2);
    
    std::cout << "\n=== Showing Block Content ===\n";
    system.showBlockContent(1);
    
    std::cout << "\n=== System Statistics ===\n";
    system.showSystemStats();
    
    std::cout << "\n=== Simulation Tests ===\n";
    system.simulateFullBlock();
    system.simulateFullSectors();
    
    std::cout << "\n=== Final System State ===\n";
    system.showSystemStats();
    
    std::cout << "\n=== Demo Completed ===\n";
    
    return 0;
}