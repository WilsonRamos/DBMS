#include "sgbd_basic.h"

// ==================== TIMER ====================
void Timer::start() {
    start_time = std::chrono::high_resolution_clock::now();
}

double Timer::getElapsedTime() {
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    return duration.count() / 1000.0; // Retorna en milisegundos
}

// ==================== PHYSICAL LOCATION ====================
PhysicalLocation::PhysicalLocation(int p, int s, int t, int sec, int pos)
    : platter_id(p), surface_id(s), track_id(t), sector_id(sec), position(pos) {}

void PhysicalLocation::print() const {
    std::cout << "Location - Platter: " << platter_id 
              << ", Surface: " << surface_id 
              << ", Track: " << track_id 
              << ", Sector: " << sector_id 
              << ", Position: " << position << std::endl;
}

// ==================== RECORD ====================
Record::Record() : is_deleted(false), record_id(-1) {}

Record::Record(const std::map<std::string, std::string>& record_data, int id) 
    : data(record_data), is_deleted(false), record_id(id) {}

std::string Record::serialize() const {
    std::string result = std::to_string(record_id) + "|";
    result += (is_deleted ? "1" : "0");
    result += "|";
    
    for (const auto& pair : data) {
        result += pair.first + ":" + pair.second + ";";
    }
    return result;
}

Record Record::deserialize(const std::string& serialized_data) {
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

int Record::getSize() const {
    return static_cast<int>(serialize().length());
}

void Record::print() const {
    std::cout << "Record ID: " << record_id << " (Deleted: " << is_deleted << ")\n";
    for (const auto& pair : data) {
        std::cout << "  " << pair.first << ": " << pair.second << "\n";
    }
}

// ==================== SECTOR ====================
Sector::Sector(int id, int cap) : sector_id(id), capacity(cap), used_space(0) {
    data.resize(capacity, '\0');
}

bool Sector::hasSpace(int required_space) const {
    return (used_space + required_space) <= capacity;
}

bool Sector::writeData(const std::string& content, int& position) {
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

std::string Sector::readData(int position, int length) const {
    if (position + length > used_space) {
        return "";
    }
    
    return std::string(data.begin() + position, data.begin() + position + length);
}

void Sector::print() const {
    std::cout << "Sector " << sector_id << " - Used: " << used_space 
              << "/" << capacity << " bytes\n";
}

// ==================== TRACK ====================
Track::Track(int id, int num_sectors, int sector_capacity) 
    : track_id(id), sectors_per_track(num_sectors) {
    for (int i = 0; i < num_sectors; ++i) {
        sectors.emplace_back(i, sector_capacity);
    }
}

Sector* Track::findSectorWithSpace(int required_space) {
    for (auto& sector : sectors) {
        if (sector.hasSpace(required_space)) {
            return &sector;
        }
    }
    return nullptr;
}

void Track::print() const {
    std::cout << "Track " << track_id << " with " << sectors.size() << " sectors:\n";
    for (const auto& sector : sectors) {
        std::cout << "  ";
        sector.print();
    }
}

// ==================== SURFACE ====================
Surface::Surface(int id, int num_tracks, int sectors_per_track, int sector_capacity)
    : surface_id(id), tracks_per_surface(num_tracks) {
    for (int i = 0; i < num_tracks; ++i) {
        tracks.emplace_back(i, sectors_per_track, sector_capacity);
    }
}

Track* Surface::findTrackWithSpace(int required_space) {
    for (auto& track : tracks) {
        if (track.findSectorWithSpace(required_space) != nullptr) {
            return &track;
        }
    }
    return nullptr;
}

void Surface::print() const {
    std::cout << "Surface " << surface_id << " with " << tracks.size() << " tracks:\n";
    for (const auto& track : tracks) {
        std::cout << "  ";
        track.print();
    }
}

// ==================== PLATTER ====================
Platter::Platter(int id, int num_surfaces, int tracks_per_surface, 
        int sectors_per_track, int sector_capacity)
    : platter_id(id), surfaces_per_platter(num_surfaces) {
    for (int i = 0; i < num_surfaces; ++i) {
        surfaces.emplace_back(i, tracks_per_surface, sectors_per_track, sector_capacity);
    }
}

Surface* Platter::findSurfaceWithSpace(int required_space) {
    for (auto& surface : surfaces) {
        if (surface.findTrackWithSpace(required_space) != nullptr) {
            return &surface;
        }
    }
    return nullptr;
}

void Platter::print() const {
    std::cout << "Platter " << platter_id << " with " << surfaces.size() << " surfaces:\n";
    for (const auto& surface : surfaces) {
        std::cout << "  ";
        surface.print();
    }
}