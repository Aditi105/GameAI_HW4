#include "DataRecorder.hpp"

DataRecorder::DataRecorder(const std::string& filename)
  : out_(filename)
{
    out_ << "room,dist,aggro,wall,action\n";
}

DataRecorder::~DataRecorder() {
    out_.close();
}

void DataRecorder::record(const Sample& s) {
    out_
      << s.roomId << "," 
      << s.distToPlayer << ","
      << (s.inAggro?1:0) << ","
      << (s.hittingWall?1:0) << ","
      << s.action << "\n";
}
