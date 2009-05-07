/*  cbr
 *  LBCoordinateSegmentation.cpp
 *
 *  Copyright (c) 2009, Tahir Azim
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
 *  * Neither the name of cbr nor the names of its contributors may
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

#include "LBCoordinateSegmentation.hpp"

#include <algorithm>
#include <boost/tokenizer.hpp>
#include "Options.hpp"
#include "Time.hpp"
#include "Network.hpp"
#include "Message.hpp"


namespace CBR {

template<typename T>
T clamp(T val, T minval, T maxval) {
    if (val < minval) return minval;
    if (val > maxval) return maxval;
    return val;
}

  LBCoordinateSegmentation::LBCoordinateSegmentation(const ServerID server_id, const BoundingBox3f& region, const Vector3ui32& perdim, ServerMessageQueue* smq) : mServerID(server_id), mServerMessageQueue(smq)
{
  
  mTopLevelRegion.mBoundingBox = region;
 
  int numServers = perdim.x * perdim.y * perdim.z;
  mTopLevelRegion.mChildrenCount = numServers;

  mTopLevelRegion.mChildren = new SegmentedRegion[numServers];

  for (int i=0; i<numServers; i++) {
    BoundingBox3f ith_region = initRegion(i+1, perdim);
    mTopLevelRegion.mChildren[i].mBoundingBox = ith_region;
    mTopLevelRegion.mChildren[i].mServer = i+1;
  }
}

LBCoordinateSegmentation::~LBCoordinateSegmentation() {

  //delete all the SegmentedRegion objects created with 'new'
}

ServerID LBCoordinateSegmentation::lookup(const Vector3f& pos) const {
  Vector3f searchVec = pos;
  BoundingBox3f region = mTopLevelRegion.mBoundingBox;
  
  int i=0;
  (searchVec.z < region.min().z) ? searchVec.z = region.min().z : (i=0);
  (searchVec.x < region.min().x) ? searchVec.x = region.min().x : (i=0);
  (searchVec.y < region.min().y) ? searchVec.y = region.min().y : (i=0);

  (searchVec.z > region.max().z) ? searchVec.z = region.max().z : (i=0);
  (searchVec.x > region.max().x) ? searchVec.x = region.max().x : (i=0);
  (searchVec.y > region.max().y) ? searchVec.y = region.max().y : (i=0);
  
  
  return mTopLevelRegion.lookup(searchVec);
}

BoundingBox3f LBCoordinateSegmentation::serverRegion(const ServerID& server) const 
{
  return mTopLevelRegion.serverRegion(server);
}

BoundingBox3f LBCoordinateSegmentation::region() const {
  return mTopLevelRegion.mBoundingBox;
}

uint32 LBCoordinateSegmentation::numServers() const {
  int count = mTopLevelRegion.countServers();
 
  return count;
} 

void LBCoordinateSegmentation::tick(const Time& t) {
  static bool lbed=false;
  
  if (mServerID == 1) {

    //printf("At id %d time is %d\n", mServerID, t.raw());

    if (!lbed && t.raw() >= 10000000u) {
      lbed=true;printf("Splitting\n");
      
      SegmentedRegion* segRegion = mTopLevelRegion.lookupSegmentedRegion(mServerID);
 
      segRegion->mChildrenCount = 2;
      segRegion->mChildren = new SegmentedRegion[2];
      segRegion->mServer = -1;
      

      BoundingBox3f origBox = segRegion->mBoundingBox;      
      BoundingBox3f newBox1(origBox.min(), Vector3f( (origBox.min().x+origBox.max().x)/2, origBox.max().y, origBox.max().z));
      BoundingBox3f newBox2(Vector3f( (origBox.min().x+origBox.max().x)/2, origBox.min().y, origBox.min().z), origBox.max());

      printf("newBox1= %s, %s\n", newBox1.min().toString().c_str(), newBox1.max().toString().c_str());
      printf("newBox2= %s, %s\n", newBox2.min().toString().c_str(), newBox2.max().toString().c_str());

      segRegion->mChildren[0].mBoundingBox = newBox1;
      segRegion->mChildren[1].mBoundingBox = newBox2;

      segRegion->mChildren[0].mServer = mServerID;
      segRegion->mChildren[1].mServer = 10;
      
      OriginID origin;
      origin.id = mServerID;

      int total_servers = numServers();
      std::vector<Listener::SegmentationInfo> segInfoVector;
      for (int j = 1; j <= total_servers; j++) {
        Listener::SegmentationInfo segInfo;
        segInfo.server = j;
        segInfo.region = serverRegion(j);
        segInfoVector.push_back( segInfo );
      }

      printf("notifying listeners\n");

      notifyListeners(segInfoVector);

      printf("notified listeners\n");            

      for (int i=1 ; i <= total_servers; i++) {
	if (i != mServerID) {
	  printf("Sending server_split to %d\n", i);
	  
	  Network::Chunk msg_serialized;
	  CSegChangeMessage msg(origin, 10);
	  msg.serialize(msg_serialized, 0);
	  
	  mServerMessageQueue->addMessage(i,msg_serialized);	  
	 
	}
      }
    }
  }
}

BoundingBox3f LBCoordinateSegmentation::initRegion(const ServerID& server, const Vector3ui32& perdim) const {
    if ( server > perdim.x * perdim.y * perdim.z ) {
      return BoundingBox3f( Vector3f(0,0,0), Vector3f(0,0,0));
    }

    BoundingBox3f mRegion = mTopLevelRegion.mBoundingBox;

    ServerID sid = server - 1;
    Vector3i32 server_dim_indices(sid%perdim.x,
                                  (sid/perdim.x)%perdim.y,
                                  (sid/perdim.x/perdim.y)%perdim.z);
    double xsize=mRegion.extents().x/perdim.x;
    double ysize=mRegion.extents().y/perdim.y;
    double zsize=mRegion.extents().z/perdim.z;
    Vector3f region_min(server_dim_indices.x*xsize+mRegion.min().x,
                        server_dim_indices.y*ysize+mRegion.min().y,
                        server_dim_indices.z*zsize+mRegion.min().z);
    Vector3f region_max((1+server_dim_indices.x)*xsize+mRegion.min().x,
                        (1+server_dim_indices.y)*ysize+mRegion.min().y,
                        (1+server_dim_indices.z)*zsize+mRegion.min().z);
    return BoundingBox3f(region_min, region_max);
}

void LBCoordinateSegmentation::csegChangeMessage(CSegChangeMessage* ccMsg) {
  
  OriginID id = GetUniqueIDOriginID(ccMsg->id());
  ServerID originID = id.id;

  //printf("%d received server_split_msg from %d for new server %d\n", mServerID, originID,                ccMsg->newServerID());
 
  SegmentedRegion* segRegion = mTopLevelRegion.lookupSegmentedRegion(originID);
  
  segRegion->mChildrenCount = 2;
  segRegion->mChildren = new SegmentedRegion[2];
  segRegion->mServer = -1;
  
  
  BoundingBox3f origBox = segRegion->mBoundingBox;      
  BoundingBox3f newBox1(origBox.min(), Vector3f( (origBox.min().x+origBox.max().x)/2, origBox.max().y, origBox.max().z));
  BoundingBox3f newBox2(Vector3f( (origBox.min().x+origBox.max().x)/2, origBox.min().y, origBox.min().z), origBox.max());
  
  segRegion->mChildren[0].mBoundingBox = newBox1;
  segRegion->mChildren[1].mBoundingBox = newBox2;
  
  segRegion->mChildren[0].mServer = originID;
  segRegion->mChildren[1].mServer = ccMsg->newServerID();

  int total_servers = numServers();
  std::vector<Listener::SegmentationInfo> segInfoVector;
  for (int j = 1; j <= total_servers; j++) {
    Listener::SegmentationInfo segInfo;
    segInfo.server = j;
    segInfo.region = serverRegion(j);
    segInfoVector.push_back( segInfo );
  }
  
  notifyListeners(segInfoVector);
}


} // namespace CBR
