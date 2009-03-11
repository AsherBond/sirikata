/*  cbr
 *  QuakeMotionPath.hpp
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

#ifndef _CBR_QUAKE_MOTION_PATH_HPP_
#define _CBR_QUAKE_MOTION_PATH_HPP_

#include "Utility.hpp"
#include "MotionPath.hpp"
#include "BoundingBox.hpp"

namespace CBR {

/** Quake motion path. The path is initialized and stored 
    from a data trace generated by Quake III OpenArena.
 */
class QuakeMotionPath : public MotionPath {
public:
    QuakeMotionPath( const char* quakeDataTraceFile, float scaleDownFactor,const BoundingBox3f& region );

    virtual const MotionVector3f initial() const;
    virtual const MotionVector3f* nextUpdate(const Time& curtime) const;
private:
    MotionVector3f parseTraceLines(String firstLine, String secondLine, float scaleDownFactor);

    std::vector<MotionVector3f> mUpdates;
}; // class QuakeMotionPath

} // namespace CBR

#endif //_CBR_QUAKE_MOTION_PATH_HPP_
