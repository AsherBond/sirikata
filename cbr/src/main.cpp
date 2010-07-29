/*  Sirikata
 *  main.cpp
 *
 *  Copyright (c) 2009, Ewen Cheslack-Postava
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of Sirikata nor the names of its contributors may
 *    be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */



#include <sirikata/core/util/Timer.hpp>
#include <sirikata/core/network/NTPTimeSync.hpp>

#include <sirikata/core/network/IOServiceFactory.hpp>

#include "SpaceNetwork.hpp"

#include "Forwarder.hpp"

#include "LocationService.hpp"

#include "Proximity.hpp"
#include "Server.hpp"

#include "Options.hpp"
#include <sirikata/core/options/CommonOptions.hpp>
#include <sirikata/core/util/PluginManager.hpp>
#include <sirikata/core/trace/Trace.hpp>
#include "StandardLocationService.hpp"
#include "TCPSpaceNetwork.hpp"
#include "FairServerMessageReceiver.hpp"
#include "FairServerMessageQueue.hpp"
#include <sirikata/core/network/ServerIDMap.hpp>
#include "UniformCoordinateSegmentation.hpp"
#include "CoordinateSegmentationClient.hpp"
#include "LoadMonitor.hpp"
#include "CraqObjectSegmentation.hpp"

#include "SpaceContext.hpp"


int main(int argc, char** argv) {

    using namespace Sirikata;

    InitOptions();
    Trace::Trace::InitOptions();
    SpaceTrace::InitOptions();
    InitSpaceOptions();
    ParseOptions(argc, argv);

    PluginManager plugins;
    plugins.loadList( GetOptionValue<String>(OPT_PLUGINS) );
    plugins.loadList( GetOptionValue<String>(OPT_SPACE_PLUGINS) );

    std::string time_server=GetOptionValue<String>("time-server");
    NTPTimeSync sync;
    if (time_server.size() > 0)
        sync.start(time_server);


    ServerID server_id = GetOptionValue<ServerID>("id");
    String trace_file = GetPerServerFile(STATS_TRACE_FILE, server_id);
    Sirikata::Trace::Trace* gTrace = new Trace::Trace(trace_file);

    // Compute the starting date/time
    String start_time_str = GetOptionValue<String>("wait-until");
    Time start_time = start_time_str.empty() ? Timer::now() : Timer::getSpecifiedDate( start_time_str );
    start_time += GetOptionValue<Duration>("wait-additional");

    Duration duration = GetOptionValue<Duration>("duration");

    Network::IOService* ios = Network::IOServiceFactory::makeIOService();
    Network::IOStrand* mainStrand = ios->createStrand();


    SpaceContext* space_context = new SpaceContext(server_id, ios, mainStrand, start_time, gTrace, duration);

    Sirikata::SpaceNetwork* gNetwork = NULL;
    String network_type = GetOptionValue<String>(NETWORK_TYPE);
    if (network_type == "tcp")
      gNetwork = new TCPSpaceNetwork(space_context);

    /*
    String test_mode = GetOptionValue<String>("test");
    if (test_mode != "none") {
        String server_port = GetOptionValue<String>("server-port");
        String client_port = GetOptionValue<String>("client-port");
        String host = GetOptionValue<String>("host");
        if (test_mode == "server")
            CBR::testServer(server_port.c_str(), host.c_str(), client_port.c_str());
        else if (test_mode == "client")
            CBR::testClient(client_port.c_str(), host.c_str(), server_port.c_str());
        return 0;
    }
    */

    BoundingBox3f region = GetOptionValue<BoundingBox3f>("region");
    Vector3ui32 layout = GetOptionValue<Vector3ui32>("layout");


    srand( GetOptionValue<uint32>("rand-seed") );


    String servermap_type = GetOptionValue<String>("servermap");
    String servermap_options = GetOptionValue<String>("servermap-options");
    ServerIDMap * server_id_map =
        ServerIDMapFactory::getSingleton().getConstructor(servermap_type)(servermap_options);

    gNetwork->setServerIDMap(server_id_map);


    Forwarder* forwarder = new Forwarder(space_context);


    String cseg_type = GetOptionValue<String>(CSEG);
    CoordinateSegmentation* cseg = NULL;
    if (cseg_type == "uniform")
        cseg = new UniformCoordinateSegmentation(space_context, region, layout);
    else if (cseg_type == "client") {
      cseg = new CoordinateSegmentationClient(space_context, region, layout, server_id_map);
    }
    else {
        assert(false);
        exit(-1);
    }



    LocationService* loc_service = NULL;
    String loc_service_type = GetOptionValue<String>(LOC);
    if (loc_service_type == "standard")
        loc_service = new StandardLocationService(space_context);
    else
        assert(false);


    ServerMessageQueue* sq = NULL;
    String server_queue_type = GetOptionValue<String>(SERVER_QUEUE);
    if (server_queue_type == "fair") {
        sq = new FairServerMessageQueue(
            space_context, gNetwork,
            (ServerMessageQueue::Sender*)forwarder);
    }
    else {
        assert(false);
        exit(-1);
    }

    ServerMessageReceiver* server_message_receiver = NULL;
    String server_receiver_type = GetOptionValue<String>(SERVER_RECEIVER);
    if (server_queue_type == "fair")
        server_message_receiver =
                new FairServerMessageReceiver(space_context, gNetwork, (ServerMessageReceiver::Listener*)forwarder);
    else {
        assert(false);
        exit(-1);
    }



    LoadMonitor* loadMonitor = new LoadMonitor(space_context, sq, cseg);


    //Create OSeg
    std::string oseg_type=GetOptionValue<String>(OSEG);
    Network::IOStrand* osegStrand = space_context->ioService->createStrand();
    ObjectSegmentation* oseg = NULL;
    if (oseg_type == OSEG_OPTION_CRAQ)
    {
      //using craq approach
      std::vector<UUID> initServObjVec;

      std::vector<CraqInitializeArgs> craqArgsGet;
      CraqInitializeArgs cInitArgs1;

      cInitArgs1.ipAdd = "localhost";
      cInitArgs1.port  =     "10498"; //craq version 2
      craqArgsGet.push_back(cInitArgs1);

      std::vector<CraqInitializeArgs> craqArgsSet;
      CraqInitializeArgs cInitArgs2;
      cInitArgs2.ipAdd = "localhost";
      cInitArgs2.port  =     "10499";
      craqArgsSet.push_back(cInitArgs2);


      std::string oseg_craq_prefix=GetOptionValue<String>(OSEG_UNIQUE_CRAQ_PREFIX);

      if (oseg_type.size() ==0)
      {
        std::cout<<"\n\nERROR: Incorrect craq prefix for oseg.  String must be at least one letter long.  (And be between G and Z.)  Please try again.\n\n";
        assert(false);
      }

      oseg = new CraqObjectSegmentation (space_context, cseg, initServObjVec, craqArgsGet, craqArgsSet, oseg_craq_prefix[0],osegStrand,space_context->mainStrand);

    }      //end craq approach

    //end create oseg


    // We have all the info to initialize the forwarder now
    forwarder->initialize(oseg, sq, server_message_receiver, loc_service);


    Proximity* prox = new Proximity(space_context, loc_service);


    Server* server = new Server(space_context, forwarder, loc_service, cseg, prox, oseg, server_id_map->lookupExternal(space_context->id()));

      prox->initialize(cseg);

    // If we're one of the initial nodes, we'll have to wait until we hit the start time
    {
        Time now_time = Timer::now();
        if (start_time > now_time) {
            Duration sleep_time = start_time - now_time;
            printf("Waiting %f seconds\n", sleep_time.toSeconds() ); fflush(stdout);
#if SIRIKATA_PLATFORM == PLATFORM_WINDOWS
            Sleep( sleep_time.toMilliseconds() );
#else
            usleep( sleep_time.toMicroseconds() );
#endif
        }
    }

    ///////////Go go go!! start of simulation/////////////////////
    SSTConnectionManager* sstConnMgr = new SSTConnectionManager(space_context);

    space_context->add(space_context);
    space_context->add(gNetwork);
    space_context->add(cseg);
    space_context->add(loc_service);
    space_context->add(prox);
    space_context->add(server);
    space_context->add(oseg);
    space_context->add(loadMonitor);
    space_context->add(sstConnMgr);


    space_context->run(2);

    space_context->cleanup();

    if (GetOptionValue<bool>(PROFILE)) {
        space_context->profiler->report();
    }

    gTrace->prepareShutdown();
    prox->shutdown();

    delete server;
    delete sq;
    delete server_message_receiver;
    delete prox;
    delete server_id_map;


    delete cseg;
    delete oseg;
    delete loc_service;
    delete sstConnMgr;
    delete forwarder;

    delete gNetwork;
    gNetwork=NULL;

    gTrace->shutdown();
    delete gTrace;
    gTrace = NULL;


    delete space_context;
    space_context = NULL;

    delete mainStrand;
    delete osegStrand;

    Network::IOServiceFactory::destroyIOService(ios);


    sync.stop();

    plugins.gc();

    return 0;
}