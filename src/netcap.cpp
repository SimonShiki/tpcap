/**
 * TPCap uses libpcap to capture network packets.
 * Reference: https://yuba.stanford.edu/~casado/pcap/section1.html
 */
#include <arpa/inet.h>
#include <atomic>
#include <chrono>
#include <iomanip>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <pcap.h>

#include "netcap.h"
#include "log.h"

NetCap::NetCap(std::ostream* als) : anotherLogStream(als) {}

NetCap::~NetCap() {
    dispose();
}

void NetCap::capLog (const char* msg) {
  logInfo(msg); 
  if (anotherLogStream != nullptr) {
    logInfo(msg, *anotherLogStream);
  }
}

pcap_if_t* NetCap::getInterfaces () {
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_if_t* alldevs;

    if (pcap_findalldevs(&alldevs, errbuf) == -1) {
        logError("Failed to query interfaces: " + std::string(errbuf));
        throw std::runtime_error("pcap_findalldevs failed");
    }

    if (alldevs == nullptr) {
        logError("No interfaces found.");
        throw std::runtime_error("no interfaces");
    }

    return alldevs;
}

void NetCap::setInterfaceName (std::string ifn) {
    ifName = ifn;
}

std::string NetCap::getInterfaceName () const {
    return ifName;
}

void NetCap::init () {
    // Use first available interface if none specified
    if (ifName.empty()) {
        auto alldevs = getInterfaces();
        ifName = alldevs->name;
        pcap_freealldevs(alldevs);
    }

    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t* handle = pcap_open_live(ifName.c_str(), BUFSIZ, 1, 1000, errbuf);
    if (handle == nullptr) {
        logError("Failed to open interface " + ifName + ": " + std::string(errbuf));
        throw std::runtime_error("pcap_open_live failed");
    }

    // We only care about TCP and UDP, build filter here
    bpf_program fp;
    const char* filter_exp = "tcp or udp";

    if (pcap_compile(handle, &fp, filter_exp, 0, PCAP_NETMASK_UNKNOWN) == -1) {
        logError("Failed to compile filter: " + std::string(pcap_geterr(handle)));
        pcap_close(handle);
        throw std::runtime_error("pcap_compile failed");
    }

    if (pcap_setfilter(handle, &fp) == -1) {
        logError("Failed to set filter: " + std::string(pcap_geterr(handle)));
        pcap_freecode(&fp);
        pcap_close(handle);
        throw std::runtime_error("pcap_setfilter failed");
    }

    pcap_freecode(&fp);

    pcapHandle = handle;
}

std::string NetCap::getCurrentTimestamp() {
  auto now = std::chrono::system_clock::now();
  auto time_t_now = std::chrono::system_clock::to_time_t(now);
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()) %
            1000;

  std::stringstream ss;
  ss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S") << '.'
     << std::setfill('0') << std::setw(3) << ms.count();
  return ss.str();
}

void NetCap::startCapture (const std::atomic<bool>& running) {
  while (running) {
    int result = pcap_dispatch(
        pcapHandle, -1,
        [](u_char* userData, const pcap_pkthdr* pkthdr,
           const u_char* packet) {
          NetCap* self = reinterpret_cast<NetCap*>(userData);
          NetCapPacket info;
          info.timestamp = self->getCurrentTimestamp();
          info.length = pkthdr->len;

          ether_header* ethHeader = (ether_header*)packet;

          // Check if it's an IP packet
          if (ntohs(ethHeader->ether_type) != ETHERTYPE_IP) {
            return;
          }

          ip* ipHeader =
              (ip*)(packet + sizeof(ether_header));

          // Extract IP addresses
          info.srcIP = inet_ntoa(ipHeader->ip_src);
          info.dstIP = inet_ntoa(ipHeader->ip_dst);

          int ipHeaderLen = ipHeader->ip_hl * 4;

          // Determine protocol and extract ports
          if (ipHeader->ip_p == IPPROTO_TCP) {
            info.protocol = "TCP";

            tcphdr* tcpHeader =
                (tcphdr*)((u_char*)ipHeader + ipHeaderLen);

            info.srcPort = ntohs(tcpHeader->th_sport);
            info.dstPort = ntohs(tcpHeader->th_dport);

          } else if (ipHeader->ip_p == IPPROTO_UDP) {
            info.protocol = "UDP";

            udphdr* udpHeader =
                (udphdr*)((u_char*)ipHeader + ipHeaderLen);

            info.srcPort = ntohs(udpHeader->uh_sport);
            info.dstPort = ntohs(udpHeader->uh_dport);

          } else {
            return;
          }

          self->capLog(("[" + info.timestamp + "] " + info.protocol + " " +
                  info.srcIP + ":" + std::to_string(info.srcPort) + " -> " +
                  info.dstIP + ":" + std::to_string(info.dstPort) +
                  " Length: " + std::to_string(info.length))
                     .c_str());
        },
        reinterpret_cast<u_char*>(this));
    if (result == -1) {
      logError("Error capturing packets: " +
               std::string(pcap_geterr(pcapHandle)));
      break;
    }
  }
}

void NetCap::dispose () {
    if (pcapHandle != nullptr) {
        pcap_close(pcapHandle);
        pcapHandle = nullptr;
    }
}
