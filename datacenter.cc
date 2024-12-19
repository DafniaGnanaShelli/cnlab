
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

int main(int argc, char *argv[]) {
    CommandLine cmd;
    cmd.Parse(argc, argv);

    // Create nodes
    NodeContainer nodes;
    nodes.Create(4); // Simple 2-host, 1-switch topology

    // Create links
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("10Gbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));

    // Install links
    NetDeviceContainer devices1 = p2p.Install(nodes.Get(0), nodes.Get(2));
    NetDeviceContainer devices2 = p2p.Install(nodes.Get(1), nodes.Get(2));

    // Install the internet stack
    InternetStackHelper stack;
    stack.Install(nodes);

    // Assign IP addresses
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces1 = address.Assign(devices1);
    Ipv4InterfaceContainer interfaces2 = address.Assign(devices2);

    // Create and configure applications (e.g., bulk senders, receivers)
    uint16_t port = 9; // Port for receiver

    // Create a receiver on Node 1
    Address sinkAddress(InetSocketAddress(interfaces2.GetAddress(0), port));
    PacketSinkHelper packetSinkHelper("ns3::TcpSocketFactory", sinkAddress);
    ApplicationContainer sinkApp = packetSinkHelper.Install(nodes.Get(1));
    sinkApp.Start(Seconds(0.0));
    sinkApp.Stop(Seconds(10.0));

    // Create a sender on Node 0
    OnOffHelper onOffHelper("ns3::TcpSocketFactory", sinkAddress);
    onOffHelper.SetAttribute("DataRate", StringValue("1Gbps"));
    onOffHelper.SetAttribute("PacketSize", UintegerValue(1024));
    ApplicationContainer clientApp = onOffHelper.Install(nodes.Get(0));
    clientApp.Start(Seconds(1.0));
    clientApp.Stop(Seconds(10.0));

    // Enable DC-TCP if implemented
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(TcpDctcp::GetTypeId()));

    // Enable flow monitor
    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll();

    // Set up animation
    AnimationInterface anim("network-animation.xml");
    anim.SetConstantPosition(nodes.Get(0), 0.0, 0.0); // Node 0
    anim.SetConstantPosition(nodes.Get(1), 2.0, 0.0); // Node 1
    anim.SetConstantPosition(nodes.Get(2), 1.0, 1.0); // Node 2 (Switch)
    anim.UpdateNodeDescription(nodes.Get(0), "Sender");
    anim.UpdateNodeDescription(nodes.Get(1), "Receiver");
    anim.UpdateNodeDescription(nodes.Get(2), "Switch");

    // Enable protocol counters with start time, stop time, and polling interval
    anim.EnableIpv4L3ProtocolCounters(Seconds(0.0), Seconds(10.0), Seconds(1.0));

    // Enable packet metadata
    anim.EnablePacketMetadata(true);

    Simulator::Stop(Seconds(10.0));
    Simulator::Run();

    // Print flow statistics
    monitor->CheckForLostPackets();
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowmon.GetClassifier());
    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();

    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin(); i != stats.end(); ++i) {
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(i->first);
        std::cout << "Flow " << i->first << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
        std::cout << " Tx Packets: " << i->second.txPackets << "\n";
        std::cout << " Rx Packets: " << i->second.rxPackets << "\n";
        std::cout << " Throughput: " << i->second.rxBytes * 8.0 / 10 / 1e6 << " Mbps\n";
    }

    Simulator::Destroy();
    return 0;
}

