#ifndef SGBD_BASIC_H
#define SGBD_BASIC_H

#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <map>
#include <fstream>
#include <sstream>

// Clase para medir tiempo de ejecución
class Timer {
private:
    std::chrono::high_resolution_clock::time_point start_time;
    
public:
    void start();
    double getElapsedTime();
};

// Estructura para ubicación física de un registro
struct PhysicalLocation {
    int platter_id;
    int surface_id;
    int track_id;
    int sector_id;
    int position;
    
    PhysicalLocation(int p = -1, int s = -1, int t = -1, int sec = -1, int pos = -1);
    void print() const;
};

// Estructura para representar un registro
class Record {
public:
    std::map<std::string, std::string> data;
    bool is_deleted;
    int record_id;
    
    Record();
    Record(const std::map<std::string, std::string>& record_data, int id);
    
    // Serializar registro para almacenamiento
    std::string serialize() const;
    
    // Deserializar registro desde string
    static Record deserialize(const std::string& serialized_data);
    
    // Obtener tamaño del registro en bytes
    int getSize() const;
    
    void print() const;
};

// Estructura física del disco - Sector
class Sector {
public:
    int sector_id;
    int capacity;
    std::vector<char> data;
    int used_space;
    
    Sector(int id, int cap);
    bool hasSpace(int required_space) const;
    bool writeData(const std::string& content, int& position);
    std::string readData(int position, int length) const;
    void print() const;
};

// Estructura física del disco - Pista (Track)
class Track {
public:
    int track_id;
    std::vector<Sector> sectors;
    int sectors_per_track;
    
    Track(int id, int num_sectors, int sector_capacity);
    Sector* findSectorWithSpace(int required_space);
    void print() const;
};

// Estructura física del disco - Superficie
class Surface {
public:
    int surface_id;
    std::vector<Track> tracks;
    int tracks_per_surface;
    
    Surface(int id, int num_tracks, int sectors_per_track, int sector_capacity);
    Track* findTrackWithSpace(int required_space);
    void print() const;
};

// Estructura física del disco - Plato
class Platter {
public:
    int platter_id;
    std::vector<Surface> surfaces;
    int surfaces_per_platter;
    
    Platter(int id, int num_surfaces, int tracks_per_surface, 
            int sectors_per_track, int sector_capacity);
    Surface* findSurfaceWithSpace(int required_space);
    void print() const;
};

#endif // SGBD_BASIC_H