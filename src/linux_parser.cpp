#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <iostream>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, version, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// DONE: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  string key1, key2, line1, line2;
  float memTotal, memFree, memUtil;

  std::ifstream filestream(kProcDirectory + kMeminfoFilename);

  if(filestream.is_open()){
    std::getline(filestream, line1);
    std::istringstream linestream1(line1);
    linestream1 >> key1 >> memTotal;

    std::getline(filestream, line2);
    std::istringstream linestream2(line2);
    linestream2 >> key2 >> memFree;
  }

  memUtil = (memTotal - memFree) / memTotal;
  return memUtil;
}

// DONE: Read and return the system uptime
long LinuxParser::UpTime() {
  string line;
  long upTime;
  
  std::ifstream filestream(kProcDirectory + kUptimeFilename);

  if(filestream.is_open()){
    std::getline(filestream, line);
    std::istringstream linestream(line);
    linestream >> upTime;
    //return upTime;
  }

  return upTime;
}

// DONE: Read and return the number of jiffies for the system
// https://stackoverflow.com/questions/23367857/accurate-calculation-of-cpu-usage-given-in-percentage-in-linux
long LinuxParser::Jiffies() {
  string cpu, line;
  string user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
  long total_jiffies;

  std::ifstream filestream(kProcDirectory + kStatFilename);
  if(filestream.is_open()) {

    std::getline(filestream, line);
    std::istringstream linestream(line);
    
    linestream >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guest >> guest_nice;
  }

  long total_user_time = std::stol(user) - std::stol(guest);
  long total_nice_time = std::stol(nice) - std::stol(guest_nice);
  long total_idleall_time = std::stol(idle) + std::stol(iowait);
  long total_system_time = std::stol(system) + std::stol(irq) + std::stol(softirq);
  long total_virtall_time = std::stol(guest) + std::stol(guest_nice);
  total_jiffies = total_user_time + total_nice_time + total_idleall_time + total_system_time + total_virtall_time;

  return total_jiffies;

}

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid[[maybe_unused]]) { return 0; }

// DONE: Read and return the number of active jiffies for the system
// https://stackoverflow.com/questions/23367857/accurate-calculation-of-cpu-usage-given-in-percentage-in-linux
long LinuxParser::ActiveJiffies() {
  string cpu, line;
  string user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
  long activeJiffies;

  std::ifstream filestream(kProcDirectory + kStatFilename);
  if(filestream.is_open()) {

    std::getline(filestream, line);
    std::istringstream linestream(line);
    
    linestream >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guest >> guest_nice;
  }

  activeJiffies = std::stol(user) + std::stol(nice) + std::stol(system) + std::stol(irq) + std::stol(softirq) + std::stol(steal) + std::stol(guest) + std::stol(guest_nice);

  return activeJiffies;
}

// DONE: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  long idleJiffies;
  idleJiffies = LinuxParser::Jiffies() - LinuxParser::ActiveJiffies();
  return idleJiffies;
}

// https://github.com/rmorejon65/CppND-System-Monitor/blob/master/src/linux_parser.cpp
vector<LinuxParser::CpuTimes> LinuxParser::CpuUtilPercentage() {
  std::ifstream filestream(kProcDirectory + kStatFilename);
  vector<LinuxParser::CpuTimes> returnVector;

  if (filestream.is_open()) {
      string line;
      while (std::getline(filestream, line)) {
          std::istringstream linestream(line);
          string cpu;
          long user;
          long nice;
          long system;
          long idle;
          long iowait;
          long irq;
          long softirq;
          long steal;
          long guess;
          long guessnice; 
          linestream >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guess >> guessnice;
          if (cpu.substr(0,3) != "cpu")
              return returnVector;
          
          long totalIdleTime = idle + iowait;
          long totalNoIdleTime = user + nice + system + irq + softirq;
          
          LinuxParser::CpuTimes current;
          current.idleTime = totalIdleTime;
          current.totalTime = totalIdleTime + totalNoIdleTime;

          returnVector.emplace_back(current);
      }     
      return returnVector;
  }
}

// DONE: Read and return CPU utilization
// https://stackoverflow.com/questions/4184468/sleep-for-milliseconds
// https://stackoverflow.com/questions/23367857/accurate-calculation-of-cpu-usage-given-in-percentage-in-linux
// https://github.com/rmorejon65/CppND-System-Monitor/blob/master/src/linux_parser.cpp
vector<string> LinuxParser::CpuUtilization() {
  
  std::vector<LinuxParser::CpuTimes> previousVector = LinuxParser::CpuUtilPercentage(); 
  sleep(1);
  std::vector<LinuxParser::CpuTimes> currentVector = LinuxParser::CpuUtilPercentage(); 
  vector<std::string> returnCpu;
  
  for(int i = 0; i < currentVector.size(); i++) {
      std::ostringstream oCpuStream;
      long totalDelta = currentVector[i].totalTime - previousVector[i].totalTime ;
      long idleDelta = currentVector[i].idleTime - previousVector[i].idleTime ;
      oCpuStream << (totalDelta - idleDelta)*1.0/totalDelta*1.0;
      returnCpu.emplace_back(oCpuStream.str());
  }
  return returnCpu;
  
}

// DONE: Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  
  string line, key;
  long totalProcesses = 0;

  std::ifstream filestream(kProcDirectory + kStatFilename);

  if(filestream.is_open()){
    while(std::getline(filestream, line)){
      std::istringstream linestream(line);
      linestream >> key;

      if(key == "processes"){
        linestream >> totalProcesses;
        return totalProcesses;
      }
    }
  }

  return totalProcesses;
}

// DONE: Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  string line, key;
  long runningProcesses = 0;

  std::ifstream filestream(kProcDirectory + kStatFilename);

  if(filestream.is_open()){
    while(std::getline(filestream, line)){
      std::istringstream linestream(line);
      linestream >> key;

      if(key == "procs_running"){
        linestream >> runningProcesses;
        return runningProcesses;
      }
    }
  }

  return runningProcesses;
}

// https://stackoverflow.com/questions/16726779/how-do-i-get-the-total-cpu-usage-of-an-application-from-proc-pid-stat/16736599#16736599
// https://github.com/rmorejon65/CppND-System-Monitor/blob/master/src/linux_parser.cpp#L223
LinuxParser::CpuProcessInfo LinuxParser::GetProcessCpuInfo(int pid) {
    std::stringstream filename;
    filename << kProcDirectory << "/" << pid << "/" << kStatFilename;
    std::ifstream filestream(filename.str());
    LinuxParser::CpuProcessInfo returnInfo;
    if (filestream.is_open()) {
        std::string line;
        std::getline(filestream, line);
        std::istringstream linestream(line);
        std::string ignore;
        long utime;
        long stime;
        long cutime;
        long cstime;
        long starttime;
        for(int i = 0; i < 13; i++) linestream >> ignore;
        linestream >> utime >> stime >> cutime >> cstime ;
        for(int i = 0; i < 4; i++) linestream >> ignore;
        linestream >> starttime;
        returnInfo.seconds = LinuxParser::UpTime() - (starttime/sysconf(_SC_CLK_TCK));
        returnInfo.totalTime = (utime + stime + cutime + cstime)/sysconf(_SC_CLK_TCK);
    }   
    return returnInfo;
}

float LinuxParser::CpuUtilization(int pid) {
  LinuxParser::CpuProcessInfo current = LinuxParser::GetProcessCpuInfo(pid);
  long seconds = current.seconds;
  long total = current.totalTime;
  return total*1.0/seconds;
}

// DONE: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid) {
  std::stringstream filename;
  filename << kProcDirectory << "/" << pid << "/" << kCmdlineFilename;
  std::ifstream filestream(filename.str());
  std::string cmd ;

  if (filestream.is_open()) {
      std::getline(filestream, cmd);
  }

  return cmd;
}

// DONE: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid) {
  std::stringstream filename;
  filename << kProcDirectory << "/" << pid << "/" << kStatusFilename;
  std::ifstream filestream(filename.str());
  long memory;
  std::string unit;

  if (filestream.is_open()) {
      std::string line;
      bool foundMemory = false;

      while (!foundMemory && std::getline(filestream, line)) {
        std::istringstream linestream(line);
        std::string key;
        linestream >> key;
        if (key == "VmSize:") {
          linestream >> memory >> unit;
          foundMemory = true;
        }
      }
  }

  std::ostringstream ostream;
  ostream << memory/1024 ;
  return ostream.str(); 
}

// DONE: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid) {
  string uid;
  std::stringstream filename;
  filename << kProcDirectory << "/" << pid << "/" << kStatFilename;
  std::ifstream filestream(filename.str());

  if (filestream.is_open()){
    string line;
    bool foundUid = false;
    
    while(!foundUid && std::getline(filestream, line)){
      std::istringstream linestream(line);
      string key;
      linestream >> key;
      if(key == "Uid:"){
        linestream >> uid;
        foundUid = true;
      }
    }
  }

  return uid;
}

// DONE: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
// https://github.com/rdas22/System-Monitor/blob/master/src/linux_parser.cpp
string LinuxParser::User(int pid) {
  std::string uid = Uid(pid);
  std::string userName;
  std::ifstream filestream(kPasswordPath);
  long runningProcesses = 0;
  if (filestream.is_open()) {
      std::string line;
      bool uidFound = false;
      
      while (std::getline(filestream, line) && !uidFound) {
        std::replace(line.begin(), line.end(), ' ', '_');
        std::replace(line.begin(), line.end(), ':', ' ');
        std::istringstream linestream(line);
        std::string pwd;
        std::string currentUid;
        linestream >> userName >> pwd >> currentUid;
        if (currentUid == uid)
        {          
            uidFound = true;
        }
      }
  }
  return userName; 
}

// DONE: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid) {
 string value, line;
  long starttime, uptime;
  vector<string> stat_list;
  std::ifstream filestream(kProcDirectory + std::to_string(pid) + kStatFilename);

  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    while (linestream >> value) {
			stat_list.push_back(value);
    }
  }

  starttime = std::stol(stat_list[21])/sysconf(_SC_CLK_TCK);
  uptime =  LinuxParser::UpTime() - starttime;
  return uptime;

}