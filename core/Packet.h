#ifndef PACKET_H
#define PACKET_H 
#include <vector>
#include <tuple>
#include "Config.h"
class Packet {
    
    typedef std::vector<Sample> Packet_;
    Packet_ packet_;
public: 
    typedef Packet_::size_type size_type;
    typedef Packet_::iterator iterator;
    typedef Packet_::const_iterator const_iterator;
    Packet() : packet_() {}
    Packet(const size_type size);   
    Packet(const size_type size, Sample sample);

    size_type size() const; 
    void reserve(size_type size);

    Sample& operator[](size_type pos);
    const Sample& operator[](size_type pos) const;

    iterator begin();
    const_iterator begin() const;
    iterator end();
    const_iterator end() const;

    Sample* data();

    Packet operator+(const Packet& other) const;
    Packet operator+=(const Packet& other);
};

#endif /* PACKET_H */
