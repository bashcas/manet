#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/aodv-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include <vector>
#include <fstream>
#include <iomanip>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("HierarchicalManetSimulation");

// Función auxiliar para escribir encabezados CSV
void WriteCSVHeader(std::ofstream& csv) {
  csv << "run,flowId,srcAddr,dstAddr,txPackets,rxPackets,lostPackets,"
      << "throughputMbps,delaySeconds,hopCount,pdr" << std::endl;
}

int
main (int argc, char *argv[])
{
  // Configuration parameters (can be overridden from command line)
  uint32_t membersPerCluster = 5;   // nodes per L1 cluster excluding the cluster-head
  uint32_t run = 1;                 // run number for this simulation
  double simTime = 200.0;           // seconds
  bool tracing = false;
  std::string csvFile = "manet-results.csv";  // archivo de salida por defecto
  bool appendResults = false;                  // append to existing CSV?

  CommandLine cmd (__FILE__);
  cmd.AddValue ("members", "Number of member nodes per first-level cluster", membersPerCluster);
  cmd.AddValue ("time", "Simulation time [s]", simTime);
  cmd.AddValue ("run", "Run number for this simulation", run);
  cmd.AddValue ("tracing", "Enable pcap tracing", tracing);
  cmd.AddValue ("csv", "CSV output file path", csvFile);
  cmd.AddValue ("append", "Append to CSV instead of overwrite", appendResults);
  cmd.Parse (argc, argv);

  // Set random seed based on run number
  RngSeedManager::SetSeed (12345);
  RngSeedManager::SetRun (run);

  // ------------------------------------------------------------
  // 1.  Create the node topology (hierarchical)
  // ------------------------------------------------------------
  //   Level-1 : two clusters (A and B)
  //   Level-2 : one super-cluster that connects the two heads
  // ------------------------------------------------------------

  NodeContainer clusterHeadA, clusterHeadB, superHead;
  clusterHeadA.Create (1);
  clusterHeadB.Create (1);
  superHead.Create (1);

  // Member nodes for each first-level cluster
  NodeContainer membersA, membersB;
  membersA.Create (membersPerCluster);
  membersB.Create (membersPerCluster);

  // Convenience containers
  NodeContainer clusterANodes;
  clusterANodes.Add (clusterHeadA);
  clusterANodes.Add (membersA);

  NodeContainer clusterBNodes;
  clusterBNodes.Add (clusterHeadB);
  clusterBNodes.Add (membersB);

  NodeContainer allWirelessNodes;
  allWirelessNodes.Add (clusterANodes);
  allWirelessNodes.Add (clusterBNodes);

  // CSMA backbone (Level-2)
  NodeContainer backboneNodes;
  backboneNodes.Add (clusterHeadA);
  backboneNodes.Add (clusterHeadB);
  backboneNodes.Add (superHead);

  // ------------------------------------------------------------
  // 2.  Configure the wireless (Wi-Fi ad-hoc) for clusters
  // ------------------------------------------------------------
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy; // default constructor
  phy.SetChannel (wifiChannel.Create ());

  WifiHelper wifi; // default constructor
  wifi.SetStandard (WIFI_STANDARD_80211b);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                               "DataMode", StringValue ("DsssRate11Mbps"),
                               "ControlMode", StringValue ("DsssRate11Mbps"));

  WifiMacHelper mac;
  mac.SetType ("ns3::AdhocWifiMac");

  // Install devices per cluster so that we can later assign separate network numbers
  NetDeviceContainer wifiDevicesA = wifi.Install (phy, mac, clusterANodes);
  NetDeviceContainer wifiDevicesB = wifi.Install (phy, mac, clusterBNodes);
  // Combine for convenience if needed
  NetDeviceContainer wifiDevices;
  wifiDevices.Add (wifiDevicesA);
  wifiDevices.Add (wifiDevicesB);

  // ------------------------------------------------------------
  // 3.  Configure the CSMA backbone that interconnects cluster-heads
  // ------------------------------------------------------------
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", DataRateValue (DataRate ("100Mbps")));
  csma.SetChannelAttribute ("Delay", TimeValue (MicroSeconds (6560)));

  NetDeviceContainer csmaDevices = csma.Install (backboneNodes);

  // ------------------------------------------------------------
  // 4.  Install the Internet stack with AODV routing
  // ------------------------------------------------------------
  AodvHelper aodv;
  InternetStackHelper internet;
  internet.SetRoutingHelper (aodv);
  internet.Install (allWirelessNodes);
  internet.Install (superHead); // super-head participates in routing too

  // ------------------------------------------------------------
  // 5.  Assign IP addresses
  // ------------------------------------------------------------
  Ipv4AddressHelper ip;

  // Cluster A Wi-Fi subnet
  ip.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer ifaceClusterA = ip.Assign (wifiDevicesA);

  // Cluster B Wi-Fi subnet
  ip.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer ifaceClusterB = ip.Assign (wifiDevicesB);

  // Backbone subnet
  ip.SetBase ("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer ifaceBackbone = ip.Assign (csmaDevices);

  // Enable static ARP replies on CSMA to speed-up simulation (optional)
  // Ipv4StaticRouting can also be configured if desired

  // ------------------------------------------------------------
  // 6.  Configure mobility
  // ------------------------------------------------------------
  // Cluster A members: RandomWalk2d inside [0,100]x[0,100]
  MobilityHelper mobMembersA;
  mobMembersA.SetPositionAllocator ("ns3::RandomRectanglePositionAllocator",
                                   "X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=100.0]"),
                                   "Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=100.0]"));
  mobMembersA.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                               "Bounds", RectangleValue (Rectangle (0, 100, 0, 100)),
                               "Speed", StringValue ("ns3::UniformRandomVariable[Min=0.2|Max=1.0]"));
  mobMembersA.Install (membersA);

  // Cluster B members: RandomWalk2d inside [200,300]x[0,100]
  MobilityHelper mobMembersB;
  mobMembersB.SetPositionAllocator ("ns3::RandomRectanglePositionAllocator",
                                   "X", StringValue ("ns3::UniformRandomVariable[Min=200.0|Max=300.0]"),
                                   "Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=100.0]"));
  mobMembersB.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                               "Bounds", RectangleValue (Rectangle (200, 300, 0, 100)),
                               "Speed", StringValue ("ns3::UniformRandomVariable[Min=0.2|Max=1.0]"));
  mobMembersB.Install (membersB);

  // Cluster-level movement: constant velocity for each head
  MobilityHelper mobHead;
  mobHead.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
  Ptr<ListPositionAllocator> headPosAlloc = CreateObject<ListPositionAllocator> ();
  headPosAlloc->Add (Vector (50.0, 50.0, 0.0));  // initial pos head A
  headPosAlloc->Add (Vector (250.0, 50.0, 0.0)); // initial pos head B
  mobHead.SetPositionAllocator (headPosAlloc);

  NodeContainer headNodes;
  headNodes.Add (clusterHeadA);
  headNodes.Add (clusterHeadB);
  mobHead.Install (headNodes);

  // Give the heads a velocity (cluster-level mobility)
  Ptr<ConstantVelocityMobilityModel> headAmm = clusterHeadA.Get (0)->GetObject<ConstantVelocityMobilityModel> ();
  Ptr<ConstantVelocityMobilityModel> headBmm = clusterHeadB.Get (0)->GetObject<ConstantVelocityMobilityModel> ();
  headAmm->SetVelocity (Vector (1.0, 0.3, 0)); //  ~1 m/s diagonal
  headBmm->SetVelocity (Vector (-1.0, 0.3, 0));

  // Super-head: slow RandomWaypoint in the middle area [100,200]x[120,220]
  MobilityHelper mobSuper;
  Ptr<RandomBoxPositionAllocator> superAlloc = CreateObject<RandomBoxPositionAllocator> ();
  superAlloc->SetAttribute ("X", StringValue ("ns3::UniformRandomVariable[Min=100.0|Max=200.0]"));
  superAlloc->SetAttribute ("Y", StringValue ("ns3::UniformRandomVariable[Min=120.0|Max=220.0]"));
  superAlloc->SetAttribute ("Z", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=0.0]"));
  mobSuper.SetPositionAllocator (superAlloc);
  mobSuper.SetMobilityModel ("ns3::RandomWaypointMobilityModel",
                            "Speed", StringValue ("ns3::UniformRandomVariable[Min=0.3|Max=0.8]"),
                            "Pause", StringValue ("ns3::ConstantRandomVariable[Constant=2.0]"),
                            "PositionAllocator", PointerValue (superAlloc));
  mobSuper.Install (superHead);

  // ------------------------------------------------------------
  // 7.  Configure traffic generators
  // ------------------------------------------------------------
  uint16_t udpPort = 9;

  // Application: cluster A member[0] -> cluster B member[0]
  Ptr<Node> srcNode = membersA.Get (0);
  Ptr<Node> dstNode = membersB.Get (0);
  Ipv4Address dstAddr = dstNode->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ();

  OnOffHelper onoff ("ns3::UdpSocketFactory", Address (InetSocketAddress (dstAddr, udpPort)));
  onoff.SetAttribute ("DataRate", DataRateValue (DataRate ("2Mbps")));
  onoff.SetAttribute ("PacketSize", UintegerValue (1024));
  onoff.SetAttribute ("StartTime", TimeValue (Seconds (20.0)));
  onoff.SetAttribute ("StopTime", TimeValue (Seconds (simTime - 1)));
  onoff.Install (srcNode);

  // Sink on destination
  PacketSinkHelper sink ("ns3::UdpSocketFactory", Address (InetSocketAddress (Ipv4Address::GetAny (), udpPort)));
  sink.Install (dstNode);

  // Optional reverse traffic
  OnOffHelper onoff2 ("ns3::UdpSocketFactory", Address (InetSocketAddress (srcNode->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal (), udpPort)));
  onoff2.SetAttribute ("DataRate", DataRateValue (DataRate ("2Mbps")));
  onoff2.SetAttribute ("PacketSize", UintegerValue (1024));
  onoff2.SetAttribute ("StartTime", TimeValue (Seconds (30.0)));
  onoff2.SetAttribute ("StopTime", TimeValue (Seconds (simTime - 1)));
  onoff2.Install (dstNode);

  sink.Install (srcNode);

  // ------------------------------------------------------------
  // 8.  Enable tracing / pcap (optional)
  // ------------------------------------------------------------
  if (tracing)
    {
      phy.EnablePcap ("clusterA", wifiDevicesA.Get (0));
      csma.EnablePcap ("backbone", csmaDevices, false);
    }

  // ------------------------------------------------------------
  // 9.  Flow monitor to collect metrics
  // ------------------------------------------------------------
  FlowMonitorHelper flowmonHelper;
  Ptr<FlowMonitor> flowmon = flowmonHelper.InstallAll ();

  // ------------------------------------------------------------
  // 10.  Run simulation
  // ------------------------------------------------------------
  Simulator::Stop (Seconds (simTime));
  Simulator::Run ();

  // Gather & print flow monitor statistics
  flowmon->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmonHelper.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = flowmon->GetFlowStats ();

  // Abrir archivo CSV (append o overwrite según parámetro)
  std::ofstream csv;
  if (appendResults) {
    csv.open(csvFile, std::ios::app);
  } else {
    csv.open(csvFile);
    WriteCSVHeader(csv);
  }

  // Configurar precisión para números flotantes
  csv << std::fixed << std::setprecision(6);

  std::cout << "\n*** Flow monitor statistics ***" << std::endl;
  for (auto const &flow : stats)
    {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (flow.first);

      // Calculate metrics
      double throughput = 0;
      double meanDelay = 0;
      double meanHopCount = 0;
      double pdr = 0;  // Packet Delivery Ratio

      if (flow.second.rxPackets > 0)
        {
          throughput = (flow.second.rxBytes * 8.0) / (simTime - 20.0) / 1e6; // Mbps
          meanDelay = flow.second.delaySum.GetSeconds () / flow.second.rxPackets;
          meanHopCount = (double) flow.second.timesForwarded / flow.second.rxPackets + 1;
          pdr = (double) flow.second.rxPackets / flow.second.txPackets;
        }

      // Print to console
      std::cout << "Flow " << flow.first << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")" << std::endl;
      std::cout << "  Tx Packets:   " << flow.second.txPackets << std::endl;
      std::cout << "  Rx Packets:   " << flow.second.rxPackets << std::endl;
      std::cout << "  Lost Packets: " << flow.second.lostPackets << std::endl;
      if (flow.second.rxPackets > 0)
        {
          std::cout << "  Throughput:   " << throughput << " Mbps" << std::endl;
          std::cout << "  Mean Delay:   " << meanDelay << " s" << std::endl;
          std::cout << "  Mean Hop Cnt: " << meanHopCount << std::endl;
          std::cout << "  PDR:          " << pdr * 100 << "%" << std::endl;
        }

      // Write to CSV
      csv << run << ","
          << flow.first << ","
          << t.sourceAddress << ","
          << t.destinationAddress << ","
          << flow.second.txPackets << ","
          << flow.second.rxPackets << ","
          << flow.second.lostPackets << ","
          << throughput << ","
          << meanDelay << ","
          << meanHopCount << ","
          << pdr
          << std::endl;
    }

  csv.close();
  Simulator::Destroy ();
  return 0;
}
