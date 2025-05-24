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
    void start() {
        start_time = std::chrono::high_resolution_clock::now();
    }
    
    double getElapsedTime() {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        return duration.count() / 1000.0; // Retorna en milisegundos
    }
};

// Estructura para representar un registro
class Record {
public:
    std::map<std::string, std::string> data;
    bool is_deleted;
    int record_id;
    
    Record() : is_deleted(false), record_id(-1) {}
    
    Record(const std::map<std::string, std::string>& record_data, int id) 
        : data(record_data), is_deleted(false), record_id(id) {}
    
    // Serializar registro para almacenamiento
    std::string serialize() const {
        std::string result = std::to_string(record_id) + "|";
        result += (is_deleted ? "1" : "0") + "|";
        
        for (const auto& pair : data) {
            result += pair.first + ":" + pair.second + ";";
        }
        return result;
    }
    
    // Deserializar registro desde string
    static Record deserialize(const std::string& serialized_data) {
        Record record;
        std::istringstream iss(serialized_data);
        std::string token;
        
        // Leer ID
        std::getline(iss, token, '|');
        record.record_id = std::stoi(token);
        
        // Leer estado de eliminación
        std::getline(iss, token, '|');
        record.is_deleted = (token == "1");
        
        // Leer datos
        std::getline(iss, token, '|');
        std::istringstream data_stream(token);
        std::string pair;
        
        while (std::getline(data_stream, pair, ';') && !pair.empty()) {
            size_t colon_pos = pair.find(':');
            if (colon_pos != std::string::npos) {
                std::string key = pair.substr(0, colon_pos);
                std::string value = pair.substr(colon_pos + 1);
                record.data[key] = value;
            }
        }
        
        return record;
    }
    
    // Obtener tamaño del registro en bytes
    int getSize() const {
        return serialize().length();
    }
    
    void print() const {
        std::cout << "Record ID: " << record_id << " (Deleted: " << is_deleted << ")\n";
        for (const auto& pair : data) {
            std::cout << "  " << pair.first << ": " << pair.second << "\n";
        }
    }
};

// Estructura física del disco - Sector
class Sector {
public:
    int sector_id;
    int capacity;
    std::vector<char> data;
    int used_space;
    
    Sector(int id, int cap) : sector_id(id), capacity(cap), used_space(0) {
        data.resize(capacity, '\0');
    }
    
    bool hasSpace(int required_space) const {
        return (used_space + required_space) <= capacity;
    }
    
    bool writeData(const std::string& content, int& position) {
        if (!hasSpace(content.length())) {
            return false;
        }
        
        position = used_space;
        for (size_t i = 0; i < content.length(); ++i) {
            data[used_space + i] = content[i];
        }
        used_space += content.length();
        return true;
    }
    
    std::string readData(int position, int length) const {
        if (position + length > used_space) {
            return "";
        }
        
        return std::string(data.begin() + position, data.begin() + position + length);
    }
    
    void print() const {
        std::cout << "Sector " << sector_id << " - Used: " << used_space 
                  << "/" << capacity << " bytes\n";
    }
};

// Estructura física del disco - Pista (Track)
class Track {
public:
    int track_id;
    std::vector<Sector> sectors;
    int sectors_per_track;
    
    Track(int id, int num_sectors, int sector_capacity) 
        : track_id(id), sectors_per_track(num_sectors) {
        for (int i = 0; i < num_sectors; ++i) {
            sectors.emplace_back(i, sector_capacity);
        }
    }
    
    Sector* findSectorWithSpace(int required_space) {
        for (auto& sector : sectors) {
            if (sector.hasSpace(required_space)) {
                return &sector;
            }
        }
        return nullptr;
    }
    
    void print() const {
        std::cout << "Track " << track_id << " with " << sectors.size() << " sectors:\n";
        for (const auto& sector : sectors) {
            std::cout << "  ";
            sector.print();
        }
    }
};

// Estructura física del disco - Superficie
class Surface {
public:
    int surface_id;
    std::vector<Track> tracks;
    int tracks_per_surface;
    
    Surface(int id, int num_tracks, int sectors_per_track, int sector_capacity)
        : surface_id(id), tracks_per_surface(num_tracks) {
        for (int i = 0; i < num_tracks; ++i) {
            tracks.emplace_back(i, sectors_per_track, sector_capacity);
        }
    }
    
    Track* findTrackWithSpace(int required_space) {
        for (auto& track : tracks) {
            if (track.findSectorWithSpace(required_space) != nullptr) {
                return &track;
            }
        }
        return nullptr;
    }
    
    void print() const {
        std::cout << "Surface " << surface_id << " with " << tracks.size() << " tracks:\n";
        for (const auto& track : tracks) {
            std::cout << "  ";
            track.print();
        }
    }
};

// Estructura física del disco - Plato
class Platter {
public:
    int platter_id;
    std::vector<Surface> surfaces;
    int surfaces_per_platter;
    
    Platter(int id, int num_surfaces, int tracks_per_surface, 
            int sectors_per_track, int sector_capacity)
        : platter_id(id), surfaces_per_platter(num_surfaces) {
        for (int i = 0; i < num_surfaces; ++i) {
            surfaces.emplace_back(i, tracks_per_surface, sectors_per_track, sector_capacity);
        }
    }
    
    Surface* findSurfaceWithSpace(int required_space) {
        for (auto& surface : surfaces) {
            if (surface.findTrackWithSpace(required_space) != nullptr) {
                return &surface;
            }
        }
        return nullptr;
    }
    
    void print() const {
        std::cout << "Platter " << platter_id << " with " << surfaces.size() << " surfaces:\n";
        for (const auto& surface : surfaces) {
            std::cout << "  ";
            surface.print();
        }
    }
};

// Estructura para ubicación física de un registro
struct PhysicalLocation {
    int platter_id;
    int surface_id;
    int track_id;
    int sector_id;
    int position;
    
    PhysicalLocation(int p = -1, int s = -1, int t = -1, int sec = -1, int pos = -1)
        : platter_id(p), surface_id(s), track_id(t), sector_id(sec), position(pos) {}
    
    void print() const {
        std::cout << "Location - Platter: " << platter_id 
                  << ", Surface: " << surface_id 
                  << ", Track: " << track_id 
                  << ", Sector: " << sector_id 
                  << ", Position: " << position << std::endl;
    }
};

#endif // SGBD_BASIC_H