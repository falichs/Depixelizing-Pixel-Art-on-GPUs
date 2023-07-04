#version 330 core

// Copyright (c) 2015 Felix Kreuzer
//
// This source is subject to the MIT License.
// Please see the LICENSE.txt file for more information.
// All other rights reserved.
//
// THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
// KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.

//Neighborhood Flags
#define HAS_NORTHERN_SPLINE 16
#define HAS_EASTERN_SPLINE 32
#define HAS_SOUTHERN_SPLINE 64
#define HAS_WESTERN_SPLINE 128

// Input vertex data
layout(location = 0) in vec2 pos;
layout(location = 1) in int flags;

//output vertex data
out vec2 optimizedPos;

uniform samplerBuffer indexedCellPositions;
uniform isamplerBuffer CPflags;
uniform isamplerBuffer knotNeighbors;

vec2 calcAdjustedPoint(vec2 p0, vec2 p1, vec2 p2) {
	return 0.125*p0 + 0.75*p1 + 0.125*p2;
	//return vec2(0.0, 0.0);
}

void main() {

if(flags == -1) {
	//get position, flags and neighborhood indices from parent vertex
	int id = gl_VertexID-1;
	vec2 parentPosition = texelFetch(indexedCellPositions, id).rg;
	int parentFlags = texelFetch(CPflags, id).r;
	ivec4 parentNeighborIndices = texelFetch(knotNeighbors, id);
	vec2 splinePoints[2] = vec2[2](vec2(0.0,0.0),vec2(0.0,0.0));
	
	int count = 0;
	if( (parentFlags & HAS_NORTHERN_SPLINE) == HAS_NORTHERN_SPLINE) {
		splinePoints[count] = texelFetch(indexedCellPositions, parentNeighborIndices.x).rg;
		count++;
	}
	if( (parentFlags & HAS_EASTERN_SPLINE) == HAS_EASTERN_SPLINE) {
		splinePoints[count] = texelFetch(indexedCellPositions, parentNeighborIndices.y).rg;
		count++;
	}
	if( (parentFlags & HAS_SOUTHERN_SPLINE) == HAS_SOUTHERN_SPLINE) {
		splinePoints[count] = texelFetch(indexedCellPositions, parentNeighborIndices.z).rg;
		count++;
	}
	if( (parentFlags & HAS_WESTERN_SPLINE) == HAS_WESTERN_SPLINE) {
		splinePoints[count] = texelFetch(indexedCellPositions, parentNeighborIndices.w).rg;
		count++;
	}
	//optimizedPos = calcAdjustedPoint(texelFetch(indexedCellPositions, parentNeighborIndices.z).rg, parentPosition, texelFetch(indexedCellPositions, parentNeighborIndices.y).rg);
	if(count == 2) {
	optimizedPos = calcAdjustedPoint(splinePoints[0], parentPosition, splinePoints[1]);
	} else {
	optimizedPos = parentPosition;
	}
} else {
	optimizedPos = pos;
}

}