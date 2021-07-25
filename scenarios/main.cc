#include "rl.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/ndnSIM/helper/ndn-link-control-helper.hpp"
#include "string"
#include "random"


namespace ns3 {
    int main(int argc, char* argv[])
    {
        /*
        // setting default parameters for PointToPoint links and channels
        Config::SetDefault("ns3::PointToPointNetDevice::DataRate", StringValue("2Mbps"));
        Config::SetDefault("ns3::PointToPointChannel::Delay", StringValue("10ms"));
        Config::SetDefault("ns3::QueueBase::MaxSize", StringValue("20p"));
        // Creating nodes
        NodeContainer nodes;
        nodes.Create(3);
        // Connecting nodes using two links
        PointToPointHelper p2p;
        p2p.Install(nodes.Get(0), nodes.Get(1));
        p2p.Install(nodes.Get(1), nodes.Get(2));
        */
        // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize

        if (Config::SetGlobalFailSafe("SharedMemoryPoolSize", UintegerValue(4096)) &&
            Config::SetGlobalFailSafe("SharedMemoryKey", UintegerValue(1234)))
            std::cout << "set success" << std::endl;

        std::random_device rdev;
        std::mt19937 reng(rdev());
        std::uniform_int_distribution<> u(1, 10);
        std::uniform_int_distribution<> uuu(0);
        const string str = std::to_string(u(reng));
        ns3::SeedManager smgr;
        smgr.SetSeed(uuu(reng));

        CommandLine cmd;
        cmd.Parse(argc, argv);

        AnnotatedTopologyReader topologyReader("", 15);
        topologyReader.SetFileName("/root/ndn/ourproj/scenarios/topo_baseline.txt");
        topologyReader.Read();
        NodeContainer allNodes = topologyReader.GetNodes();
        Ptr<Node> c0 = allNodes[0];
        Ptr<Node> p0 = allNodes[3];

        // Install NDN stack on all nodes
        ndn::StackHelper ndnHelper;
        //ndnHelper.SetDefaultRoutes(true);
        ndnHelper.InstallAll();

        // Routing strategy
        ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
        ndnGlobalRoutingHelper.InstallAll();
        ndnGlobalRoutingHelper.AddOrigins("/ustc", p0);
        ndnGlobalRoutingHelper.CalculateRoutes();

        // Forwarding strategy
        //ndn::StrategyChoiceHelper::Install(allNodes[3], "/ustc", "/localhost/nfd/strategy/best-route2-conges/%FD%01");
        //ndn::StrategyChoiceHelper::Install(allNodes[4], "/ustc", "/localhost/nfd/strategy/best-route2-conges/%FD%01");
        ndn::StrategyChoiceHelper::Install(allNodes[1], "/ustc", "/localhost/nfd/strategy/best-route2-conges/%FD%01");
        ndn::StrategyChoiceHelper::Install(allNodes[2], "/ustc", "/localhost/nfd/strategy/best-route/%FD%05");
        ndn::StrategyChoiceHelper::Install(c0, "/ustc", "/localhost/nfd/strategy/best-route/%FD%05");
        ndn::StrategyChoiceHelper::Install(p0, "/ustc", "/localhost/nfd/strategy/best-route/%FD%05");
        //ndn::StrategyChoiceHelper::InstallAll("/ustc", "/localhost/nfd/strategy/best-route/%FD%05");

        // Installing Consumer
        ndn::AppHelper consumerHelper("ns3::ndn::ConsumerRL");
        //consumerHelper.SetAttribute("ReactToCongestionMarks", BooleanValue(false));
        //consumerHelper.SetAttribute("UseCwa", BooleanValue(true));
        consumerHelper.SetPrefix("/ustc");
        consumerHelper.SetAttribute("RetxTimer", StringValue("10ms"));
        consumerHelper.SetAttribute("Window", StringValue("4"));
        consumerHelper.SetAttribute("CcAlgorithm", EnumValue(ndn::CcAlgorithm::RL));
        consumerHelper.SetAttribute("InitialWindowOnTimeout", BooleanValue(true));
        consumerHelper.SetAttribute("Frequency", DoubleValue(0));
        consumerHelper.SetAttribute("Randomize", StringValue("none"));
        consumerHelper.SetAttribute("WatchDog", DoubleValue(0.2));
        consumerHelper.Install(c0);

        // Installing Producer
        ndn::AppHelper producerHelper("ns3::ndn::Producer");
        producerHelper.SetPrefix("/ustc");
        producerHelper.SetAttribute("PayloadSize", StringValue("1024"));
        producerHelper.Install(p0);

        //Simulator::Stop(Seconds(60));
        Simulator::Run();
        Simulator::Destroy();
        return 0;
    }

} // namespace ns3

int
main(int argc, char* argv[])
{
    return ns3::main(argc, argv);
}
