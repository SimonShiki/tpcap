#pragma once

#include <memory>
#include <optional>
#include <ostream>
#include <pcap.h>

class NetCap {
private:
  std::string ifName;
  std::ostream *anotherLogStream;
  pcap_t *pcapHandle;

  struct NetCapPacket {
    std::string timestamp;
    std::string protocol;
    std::string srcIP;
    uint16_t srcPort;
    std::string dstIP;
    uint16_t dstPort;
    uint32_t length;
  };

  void capLog(const char *msg);
  pcap_if_t *getInterfaces();
  static std::string getCurrentTimestamp();

public:
  NetCap(std::ostream *als);
  ~NetCap();

  void setInterfaceName(std::string ifn);
  std::string getInterfaceName() const;

  void init();
  void startCapture(const std::atomic<bool> &running);
  void dispose();
};
