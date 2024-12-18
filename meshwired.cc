#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/animation-interface.h"
#include <fstream>
#include <iostream>
#include <sstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("WiredExample");

// Declaring these variables outside of main() for use in trace sinks
uint32_t g_udpTxCount = 0; //!< Tx packet counter.
uint32_t g_udpRxCount = 0; //!< Rx packet counter.

/**
 * Transmission trace sink.
 *
 * \param p The sent packet.
 */
void
TxTrace(Ptr<const Packet> p)
{
    NS_LOG_DEBUG("Sent " << p->GetSize() << " bytes");
    g_udpTxCount++;
}

/**
 * Reception trace sink.
 *
 * \param p The received packet.
 */
void
RxTrace(Ptr<const Packet> p)
{
    NS_LOG_DEBUG("Received " << p->GetSize() << " bytes");
    g_udpRxCount++;
}

/**
 * \ingroup wired
 * \brief WiredTest class
 */
class WiredTest
{
  public:
    /// Init test
    WiredTest();
    /**
     * Configure test from command line arguments
     *
     * \param argc command line argument count
     * \param argv command line arguments
     */
    void Configure(int argc, char** argv);
    /**
     * Run test
     * \returns the test status
     */
    int Run();

  private:
    double m_totalTime; ///< total time
    double m_packetInterval; ///< packet interval
    uint16_t m_packetSize; ///< packet size
    uint32_t m_nNodes; ///< number of nodes
    bool m_pcap; ///< PCAP

    /// List of network nodes
    NodeContainer nodes;
    /// List of CSMA devices
    NetDeviceContainer csmaDevices;
    /// Addresses of interfaces:
    Ipv4InterfaceContainer interfaces;

  private:
    /// Create nodes and setup their connections
    void CreateNodes();
    /// Install internet stack on nodes
    void InstallInternetStack();
    /// Install applications
    void InstallApplication();
};

WiredTest::WiredTest()
    : m_totalTime(100.0),
      m_packetInterval(1),
      m_packetSize(1024),
      m_nNodes(5),
      m_pcap(false)
{
}

void
WiredTest::Configure(int argc, char* argv[])
{
    CommandLine cmd(__FILE__);
    cmd.AddValue("time", "Simulation time (sec)", m_totalTime);
    cmd.AddValue("packet-interval", "Interval between packets in UDP ping (sec)", m_packetInterval);
    cmd.AddValue("packet-size", "Size of packets in UDP ping (bytes)", m_packetSize);
    cmd.AddValue("n-nodes", "Number of nodes in the wired network", m_nNodes);
    cmd.AddValue("pcap", "Enable PCAP traces on interfaces", m_pcap);

    cmd.Parse(argc, argv);
    NS_LOG_DEBUG("Number of nodes: " << m_nNodes);
    NS_LOG_DEBUG("Simulation time: " << m_totalTime << " s");
}

void
WiredTest::CreateNodes()
{
    // Create nodes
    nodes.Create(m_nNodes);

    // Create CSMA helper
    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));

    // Install CSMA devices
    csmaDevices = csma.Install(nodes);

    if (m_pcap)
    {
        csma.EnablePcapAll("wired");
    }
}

void
WiredTest::InstallInternetStack()
{
    InternetStackHelper internetStack;
    internetStack.Install(nodes);
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    interfaces = address.Assign(csmaDevices);
}

void
WiredTest::InstallApplication()
{
    uint16_t portNumber = 9;

    // Server setup
    UdpEchoServerHelper echoServer(portNumber);
    ApplicationContainer serverApps = echoServer.Install(nodes.Get(m_nNodes - 1));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(m_totalTime + 1));

    // Client setup
    UdpEchoClientHelper echoClient(interfaces.GetAddress(m_nNodes - 1), portNumber);
    echoClient.SetAttribute("MaxPackets", UintegerValue((uint32_t)(m_totalTime * (1 / m_packetInterval))));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(m_packetInterval)));
    echoClient.SetAttribute("PacketSize", UintegerValue(m_packetSize));
    ApplicationContainer clientApps = echoClient.Install(nodes.Get(0));

    Ptr<UdpEchoClient> app = clientApps.Get(0)->GetObject<UdpEchoClient>();
    app->TraceConnectWithoutContext("Tx", MakeCallback(&TxTrace));
    app->TraceConnectWithoutContext("Rx", MakeCallback(&RxTrace));

    clientApps.Start(Seconds(1.0));
    clientApps.Stop(Seconds(m_totalTime + 1.5));
}

int
WiredTest::Run()
{
    CreateNodes();
    InstallInternetStack();
    InstallApplication();
    AnimationInterface anim("wired-animation.xml");
    Simulator::Stop(Seconds(m_totalTime + 2));
    Simulator::Run();
    Simulator::Destroy();
    std::cout << "UDP echo packets sent: " << g_udpTxCount << " received: " << g_udpRxCount
              << std::endl;
    return 0;
}

int
main(int argc, char* argv[])
{
    WiredTest t;
    t.Configure(argc, argv);
    return t.Run();
}

