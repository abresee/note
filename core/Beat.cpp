#include "Beat.h"
#include "Time.h"
#include "Offset.h"
#include "global.h"
Beat::Beat(boost::rational<int64_t> beat_initializer):
    beat_(beat_initializer) {
}

Beat::Beat(int64_t num, int64_t denom):
    beat_(num,denom) {
}

//position(beats) / tempo(beats/sec) * 1e9 ns/second = time
//position(beats)   |     (sec)     | 1e9 (ns)
//                  | tempo(beats)  | 1 (sec)
Time Beat::to_time(Beat tempo) const {
    return Time(boost::rational_cast<underlying>(beat_/tempo.beat_)*Config::count_ns_in_sec);
}

//position (beats) * (1/tempo) (sec/beat) sample rate (sample / second)
//position (beats)  |       (sec)   | sample rate (samples)
//                  | tempo (beats) |             (second)
Offset Beat::to_offset(Beat tempo, Offset sample_rate) const {
    return Offset(boost::rational_cast<underlying>(beat_/tempo.beat_)*sample_rate);
}

bool Beat::operator==(const Beat& other) const {
    return beat_ == other.beat_;
}

bool Beat::operator<(const Beat& other) const {
    return beat_ < other.beat_;
}

bool Beat::operator>(const Beat& other) const {
    return beat_ > other.beat_;
}

Beat Beat::operator/(const Beat& other) const {
    return Beat(beat_/other.beat_);
}

Beat Beat::operator-(const Beat& other) const {
    return Beat(beat_-other.beat_);
}

Beat Beat::operator+(const Beat& other) const {
    return Beat(beat_+other.beat_);
}

Beat Beat::operator*(const underlying& other) const {
    return Beat(beat_*other);
}

Beat Beat::operator/(const underlying& other) const {
    return Beat(beat_/other);
}

double Beat::as_double() const {
    return boost::rational_cast<double>(beat_);
}

Beat operator*(const Beat::underlying& lhs, const Beat& rhs) {
    return rhs*lhs;
}
