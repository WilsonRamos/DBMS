#ifndef SGBD_H
#define SGBD_H

#include "disk_manager.h"
#include <algorithm>

// Sistema Gestor de Base de Datos Principal
class SGBD {
private:
    DiskManager disk_manager;
    std::unordered_map<int, Block*> all_blocks;
    int next_record_id;
    
public:
    SGBD(int platters, int surfaces, int tracks, int sectors, 
         int sector_cap, int rec_per_block, int buffer_size);
    
    // Cargar datos desde archivo CSV
    bool loadFromCSV(const std::string& filename);
    
    // Añadir un registro individual
    bool addRecord(const Record& record);
    
    // Consultar un registro por ID
    Record* findRecord(int record_id);
    
    // Consultar registros por atributo
    std::vector<Record*> findRecordsByAttribute(const std::string& attribute, 
                                               const std::string& value, 
                                               const std::string& operator_type = "=");
    
    // Obtener todos los registros (SELECT * FROM table)
    std::vector<Record*> getAllRecords();
    
    // Eliminar un registro
    bool deleteRecord(int record_id);
    
    // Mostrar contenido de un bloque específico
    void showBlockContent(int block_id);
    
    // Mostrar todos los bloques
    void showAllBlocks();
    
    // Mostrar estadísticas del sistema
    void showSystemStats();
    
    // Simular bloque sin espacio
    void simulateFullBlock();
    
    // Simular sectores llenos
    void simulateFullSectors();
};

// Funciones auxiliares para crear datos de ejemplo
void createTitanicSample();
void createHousingSample();

#endif // SGBD_H