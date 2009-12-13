#include "ByteTransferScenario.hpp"
#include "ScenarioFactory.hpp"
#include "ObjectHost.hpp"
#include "Object.hpp"
#include "Options.hpp"
namespace CBR{
void BTSInitOptions(ByteTransferScenario *thus) {

    Sirikata::InitializeClassOptions ico("DistributedPingScenario",thus,
                                         new OptionValue("num-pings-per-second","1000",Sirikata::OptionValueType<size_t>(),"Number of pings launched per simulation second"),
                                         new OptionValue("allow-same-object-host","false",Sirikata::OptionValueType<bool>(),"allow pings to occasionally hit the same object host you are on"),
                                         new OptionValue("force-same-object-host","false",Sirikata::OptionValueType<bool>(),"force pings to only go through 1 spaec server hop"),
                                         new OptionValue("packet-size","1024",Sirikata::OptionValueType<size_t>(),"Transfer packet size"),
        NULL);
}
ByteTransferScenario::ByteTransferScenario(const String &options):mStartTime(Time::epoch()){
    mNumTotalPings=0;
    mContext=NULL;

    BTSInitOptions(this);
    OptionSet* optionsSet = OptionSet::getOptions("DistributedPingScenario",this);
    optionsSet->parse(options);
    mSameObjectHostPings=optionsSet->referenceOption("allow-same-object-host")->as<bool>();
    mForceSameObjectHostPings=optionsSet->referenceOption("force-same-object-host")->as<bool>();
    mPacketSize=optionsSet->referenceOption("packet-size")->as<size_t>();

    mPort=888;
    mGeneratePings=std::tr1::bind(&ByteTransferScenario::generatePings,this);
}
ByteTransferScenario::~ByteTransferScenario(){
    mContext->objectHost->unregisterService(mPort);
    delete mPingProfiler;
}

ByteTransferScenario*ByteTransferScenario::create(const String&options){
    return new ByteTransferScenario(options);
}
void ByteTransferScenario::addConstructorToFactory(ScenarioFactory*thus){
    thus->registerConstructor("ping",&ByteTransferScenario::create);
}

void ByteTransferScenario::initialize(ObjectHostContext*ctx) {
    mContext=ctx;
    mPingProfiler = mContext->profiler->addStage("Object Host Generate TransferBytes ");
    mContext->objectHost->registerService(mPort,std::tr1::bind(&ByteTransferScenario::pingReturn,this,_1));
    mReturned=false;
}

void ByteTransferScenario::start() {
    mSourceObject=UUID::null();
    mDestinationObject=UUID::null();
    mOutstandingPackets.clear();
    mReturned=false;
    generatePings();
}
void ByteTransferScenario::stop() {
    if( mOutstandingPackets.size()) {
        Time start=mOutstandingPackets[0].start;
        Time finish=start;
        int64 count=0;
        int64 i=0;
        int64 skip=0;
        int64 potentialskip=0;
        for (;i<(int64)mOutstandingPackets.size();++i) {
            if (mOutstandingPackets[i].received==false) {
                ++potentialskip;
            }else {
                skip+=potentialskip;
                potentialskip=0;
                ++count;
                if (mOutstandingPackets[i].finish>finish) {
                    finish=mOutstandingPackets[i].finish;
                }
            }
        }
        if (i>0) {
            SILOG(oh,warning,count*mPacketSize<< " Bytes transfered in "<<(finish-start)<<" seconds: " <<count*mPacketSize/(finish-start).toSeconds()<<"bytes/s"<<" lost "<<skip<<" packets (not counted in total)");
        }
    }
}
void ByteTransferScenario::pingReturn(const CBR::Protocol::Object::ObjectMessage&msg){
    if (msg.payload().size()>=8){
        if (msg.payload().size()!=mPacketSize&&mPacketSize>=8) {
            SILOG(oh,error,"Packet size does not match received packet size "<<msg.payload().size()<< " != "<<mPacketSize);            
        }else {
            int64 pingNumber=0;
            mReturned=true;
            for (int i=8;i-->0;) {
                unsigned char val=msg.payload()[i];
                pingNumber*=256;
                pingNumber+=val;
            }
            if (pingNumber>=(int64)mOutstandingPackets.size()) {
                SILOG(oh,error,"Packet received that hasn't been sent yet "<<pingNumber);                
            }else {
                mOutstandingPackets[pingNumber].update(mContext->simTime());
            }
        }
    }else {
        SILOG(oh,error,"Runt transfer packet sized "<<msg.payload().size()); 
    }
    mContext->mainStrand->post(mGeneratePings);
}
void ByteTransferScenario::generatePings() {
    if (mSourceObject==UUID::null()||mDestinationObject==UUID::null()) {
        mPingProfiler->started();
        unsigned int maxDistance=mContext->objectHost->getObjectConnections()->numServerIDs();
        unsigned int distance=0;
        if (maxDistance&&((!mSameObjectHostPings)&&(!mForceSameObjectHostPings)))
            maxDistance-=1;
        if (maxDistance>1&&!mForceSameObjectHostPings) {
            distance=(rand()%maxDistance);//uniform distance at random
            if (!mSameObjectHostPings)
                distance+=1;
        }
        unsigned int minServer=(rand()%(maxDistance-distance+1))+1;

        Object * objA=mContext->objectHost->getObjectConnections()->randomObject((ServerID)minServer,
                                                                                 false);
        Object * objB=mContext->objectHost->getObjectConnections()->randomObject((ServerID)(minServer+distance),
                                                                                 false);
        
        
        if (rand()<RAND_MAX/2) {
            Object * tmp=objA;
            objA=objB;
            objB=tmp;
        }
        if (objA!=NULL&&objB!=NULL) {
            mSourceObject=objA->uuid();
            mDestinationObject=objB->uuid();
        }
        mContext->mainStrand->post(Duration::milliseconds(10.),mGeneratePings);
    }else {
        std::string buffer;
        if (mPacketSize>8) {
            buffer.resize(mPacketSize);
        }else {
            buffer.resize(8);
        }
        bool pushed=false;
        Time stime(mContext->simTime());
        do {
            int64 pingNumber=mOutstandingPackets.size();
            mOutstandingPackets.push_back(TransferTimeData(stime));
            buffer[0]=pingNumber%256;
            buffer[1]=pingNumber/256%256;
            buffer[2]=pingNumber/256/256%256;
            buffer[3]=pingNumber/256/256/256%256;
            buffer[4]=pingNumber/256/256/256/256%256;
            buffer[5]=pingNumber/256/256/256/256/256%256;
            buffer[6]=pingNumber/256/256/256/256/256/256%256;
            buffer[7]=pingNumber/256/256/256/256/256/256/256%256;
            pushed=mContext->objectHost->send(mContext->objectHost->getObjectConnections()->object(mSourceObject),
                                       mPort,
                                       mDestinationObject,
                                       mPort,
                                       buffer);
        }while (pushed);
        mOutstandingPackets.pop_back();//did not push the last guy
        mPingProfiler->finished();
        if (!mReturned) {
            mContext->mainStrand->post(Duration::microseconds(100),mGeneratePings);
        }else {
            mContext->mainStrand->post(Duration::milliseconds(100),mGeneratePings);
        }

    }
}
}
