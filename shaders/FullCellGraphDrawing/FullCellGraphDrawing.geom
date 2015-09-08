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
#define HAS_NORTHERN_NEIGHBOR 1
#define HAS_EASTERN_NEIGHBOR 2
#define HAS_SOUTHERN_NEIGHBOR 4
#define HAS_WESTERN_NEIGHBOR 8
#define HAS_NORTHERN_SPLINE 16
#define HAS_EASTERN_SPLINE 32
#define HAS_SOUTHERN_SPLINE 64
#define HAS_WESTERN_SPLINE 128
#define HAS_CORRECTED_POSITION 256
#define DONT_OPTIMIZE_N 512
#define DONT_OPTIMIZE_E 1024
#define DONT_OPTIMIZE_S 2048
#define DONT_OPTIMIZE_W 4096

//Directions
#define NORTH		1
#define EAST		2
#define SOUTH		4
#define WEST		8

#define STEP 0.2
#define NUM_INTEGRATE_DX 0.2

layout (points) in;

in VertexData
{
	vec2 pos;
	ivec4 neighbors;
	int flags;
} perVertexIn[];

//in int gl_PrimitiveIDIn;

layout (line_strip, max_vertices = 36) out;
out vec4 perVertexColor;

uniform ivec2 pixelArtDimensions;
uniform samplerBuffer indexedCellPositions;
uniform isamplerBuffer CPflags;
uniform isamplerBuffer knotNeighbors;
uniform float uf_zoomFactor;
uniform vec2 uv2_zoomWindowCenter;

vec4 mapVertex(vec2 vIn) {
	return vec4((2.0 * vIn/(pixelArtDimensions-1.0) - 1.0) * uf_zoomFactor + uv2_zoomWindowCenter, 0.0, 1.0);//NDC
}

vec2 calcSplinePoint(vec2 p0, vec2 p1, vec2 p2, float t) {
	float t2 = 0.5*t*t;
	float a = t2 - t + 0.5;
	float b = -2.0 * t2 + t + 0.5;
	return a*p0 + b*p1 + t2*p2;
}

float calcCurveEnergy(vec2 p0, vec2 p1, vec2 p2, vec2 p3, vec2 p4, bool useP0, bool useP4) {
	float energy = 0.0;
	vec2 k = p1 - 2.0*p2 + p3;
	
	// for (float r = 0.0 ; r <= 1.0 ; r+=NUM_INTEGRATE_DX) {
	for (int i = 0 ; i <= 1 ; i++) {
		float r = i/10.0;
		float a = r-1.0;
		float b = 1.0-2.0*r;
		vec2 v = a*p1 + b*p2 + r*p3;
		float curvature = abs(v.x*k.y - v.y*k.x) / pow(v.x*v.x + v.y*v.y,3.0/2.0);
		
		if( (r == 0.0) || (r == 1.0) ) {
			curvature = curvature * 0.5;
		}
		energy += curvature;
		// energy += curvature*curvature;
		if(isinf(energy)) {
			// return 4611686018427387900.0;
			return -1.0;
		}
	}
	
	if(useP0) {
		k = p0 - 2.0*p1 + p2;
	
		// for (float r = 0.0 ; r <= 1.0 ; r+=NUM_INTEGRATE_DX) {
		for (int i = 0 ; i <= 1 ; i++) {
			float r = i/10.0;
			float a = r-1.0;
			float b = 1.0-2.0*r;
			vec2 v = a*p0 + b*p1 + r*p2;
			float curvature = abs(v.x*k.y - v.y*k.x) / pow(v.x*v.x + v.y*v.y,3.0/2.0);
			if( (r == 0.0) || (r == 1.0) ) {
				curvature = curvature * 0.5;
			}
			energy += curvature;
			// energy += curvature*curvature;
			if(isinf(energy)) {
				// return 4611686018427387900.0;
				return -1.0;
			}
		}
	}
	if(useP4) {
		k = p2 - 2.0*p3 + p4;
	
		// for (float r = 0.0 ; r <= 1.0 ; r+=NUM_INTEGRATE_DX) {
		for (int i = 0 ; i <= 1 ; i++) {
			float r = i/10.0;
			float a = r-1.0;
			float b = 1.0-2.0*r;
			vec2 v = a*p2 + b*p3 + r*p4;
			float curvature = abs(v.x*k.y - v.y*k.x) / pow(v.x*v.x + v.y*v.y,3.0/2.0);
			if( (r == 0.0) || (r == 1.0) ) {
				curvature = curvature * 0.5;
			}
			energy += curvature;
			// energy += curvature*curvature;
			if(isinf(energy)) {
				//return 4611686018427387900.0;
				return -1.0;
			}
		}
	}
	
	energy = energy * NUM_INTEGRATE_DX;
	
	return energy;
}

vec4 evaluateEnergy(float energy){
	vec4 color = vec4(0.0,0.0,0.0,0.0);
	if(energy > 0.0) {
		if(energy > 0.25) {
			if(energy > 0.5) {
				if(energy > 0.75) {
					if(energy > 1.0) {
						color.r = 1.0;
					} else {color.r = 0.8;}
				} else {color.r = 0.6;}
			} else {color.r = 0.4;}
		} else {color.r = 0.2;}
	} else if (energy < 0.0) {color.g = 1.0;}
	return color;
}

int getNeighborIndex(int sourceIndex, int dir, int targetSector) {
	int index = -1;

	if( (dir & NORTH) == NORTH) {
		index = texelFetch(knotNeighbors,sourceIndex).x;
	} else if( (dir & EAST) == EAST) {
		index = texelFetch(knotNeighbors,sourceIndex).y;
	} else if( (dir & SOUTH) == SOUTH) {
		index = texelFetch(knotNeighbors,sourceIndex).z;
	} else if( (dir & WEST) == WEST) {
		index = texelFetch(knotNeighbors,sourceIndex).w;
	}
	return index;
}

void main() {
	vec2 correctedPosition =perVertexIn[0].pos;
	// check if this point lies on a T-Junction and get its corrected Position
	if ( (perVertexIn[0].flags & HAS_CORRECTED_POSITION) == HAS_CORRECTED_POSITION ) {
		int index; //this points index
		if(perVertexIn[0].neighbors.w != -1) { //take western neighbor index for this points index calculation
			if(perVertexIn[0].neighbors.w%2 == 0) {index =perVertexIn[0].neighbors.w+3;}
			else {index = perVertexIn[0].neighbors.w+2;}
		} else {
			//since we are on a t-junction and there is no western neighbor there must be an eastern one
			if(perVertexIn[0].neighbors.y%2 == 0) {index = perVertexIn[0].neighbors.y-1;}
			else {index = perVertexIn[0].neighbors.y-2;}
		}
		correctedPosition = texelFetch(indexedCellPositions,index).rg;
	}
		
	vec2 linearNeighbors[4]; // up to 4
	int linearCount = 0;
	//debug corner pattern detection
	// bool linearNoOpt[4];

	vec2 splineNeighbors[2];
	int splineCount = 0;
	bool splineNoOpt[2];
	vec2 splineNeighborsNeighbors[2] = vec2[2](vec2(0.0,0.0),vec2(0.0,0.0));
	bool neighborHasNeighbor[2] = bool[2](false,false);
	// bool neighborHasNeighbor[2];
	// neighborHasNeighbor[0]= false;
	// neighborHasNeighbor[1]= false;

	//check present connections - this helps to identify image borders
	bool northernConnectionPresent = (perVertexIn[0].neighbors.x != -1);
	bool easternConnectionPresent = (perVertexIn[0].neighbors.y != -1);
	bool southernConnectionPresent = (perVertexIn[0].neighbors.z != -1);
	bool westernConnectionPresent = (perVertexIn[0].neighbors.w != -1);

	if(northernConnectionPresent) {
		//get the neighbours falgs
		int neighborflags = texelFetch(CPflags,perVertexIn[0].neighbors.x).r;
		//check if there's a visible connection to a northern neighbor
		if( (perVertexIn[0].flags & HAS_NORTHERN_NEIGHBOR) == HAS_NORTHERN_NEIGHBOR ) {
			//check if point is part of a Spline
			if( (perVertexIn[0].flags & HAS_NORTHERN_SPLINE) == HAS_NORTHERN_SPLINE ) {
				if( ((perVertexIn[0].flags & DONT_OPTIMIZE_N) == DONT_OPTIMIZE_N) || ((neighborflags & DONT_OPTIMIZE_S) == DONT_OPTIMIZE_S)) {
					splineNoOpt[splineCount] = true;
				} else {
					splineNoOpt[splineCount] = false;
				}
				if(neighborflags >= HAS_NORTHERN_SPLINE && !( (neighborflags & HAS_SOUTHERN_SPLINE) == HAS_SOUTHERN_SPLINE )) {
					//our neighbour is an endpoint on a t-junction and needs to be adjusted
					splineNeighbors[splineCount] = texelFetch(indexedCellPositions,perVertexIn[0].neighbors.x+1).rg;
					splineCount++;
				} else {
					splineNeighbors[splineCount] = texelFetch(indexedCellPositions,perVertexIn[0].neighbors.x).rg;
					
					// BEGIN ENERGY DEBUG INSERT
						//see if the northern point has a neighbor - this one would be influenced by the optimization
						if( (neighborflags & HAS_NORTHERN_SPLINE) == HAS_NORTHERN_SPLINE) {
							//get neighbors neighbors flag
							int neighborsNeighborIndex = getNeighborIndex(perVertexIn[0].neighbors.x, NORTH, 0);
							int neighborsNeighborflags = texelFetch(CPflags,neighborsNeighborIndex).r;
							if( ((neighborsNeighborflags & HAS_SOUTHERN_SPLINE) == HAS_SOUTHERN_SPLINE) 
								|| !( (neighborsNeighborflags & HAS_CORRECTED_POSITION) == HAS_CORRECTED_POSITION) ){
								//take this one
								splineNeighborsNeighbors[splineCount] = texelFetch(indexedCellPositions,neighborsNeighborIndex).rg;
								neighborHasNeighbor[splineCount]=true;
							} else {
								//use the corrected position
								splineNeighborsNeighbors[splineCount] = texelFetch(indexedCellPositions,neighborsNeighborIndex+1).rg;
								neighborHasNeighbor[splineCount]=true;
							}
						} else if((neighborflags & HAS_EASTERN_SPLINE) == HAS_EASTERN_SPLINE) {
							int neighborsNeighborIndex = getNeighborIndex(perVertexIn[0].neighbors.x, EAST, 0);
							int neighborsNeighborflags = texelFetch(CPflags,neighborsNeighborIndex).r;
							if( ((neighborsNeighborflags & HAS_WESTERN_SPLINE) == HAS_WESTERN_SPLINE) 
								|| !( (neighborsNeighborflags & HAS_CORRECTED_POSITION) == HAS_CORRECTED_POSITION) ){
								//take this one
								splineNeighborsNeighbors[splineCount] = texelFetch(indexedCellPositions,neighborsNeighborIndex).rg;
								neighborHasNeighbor[splineCount]=true;
							} else {
								//use the corrected position
								splineNeighborsNeighbors[splineCount] = texelFetch(indexedCellPositions,neighborsNeighborIndex+1).rg;
								neighborHasNeighbor[splineCount]=true;
							}
						} else if((neighborflags & HAS_WESTERN_SPLINE) == HAS_WESTERN_SPLINE) {
							int neighborsNeighborIndex = getNeighborIndex(perVertexIn[0].neighbors.x, WEST, 0);
							int neighborsNeighborflags = texelFetch(CPflags,neighborsNeighborIndex).r;
							if( ((neighborsNeighborflags & HAS_EASTERN_SPLINE) == HAS_EASTERN_SPLINE) 
								|| !( (neighborsNeighborflags & HAS_CORRECTED_POSITION) == HAS_CORRECTED_POSITION) ){
								//take this one
								splineNeighborsNeighbors[splineCount] = texelFetch(indexedCellPositions,neighborsNeighborIndex).rg;
								neighborHasNeighbor[splineCount]=true;
							} else {
								//use the corrected position
								splineNeighborsNeighbors[splineCount] = texelFetch(indexedCellPositions,neighborsNeighborIndex+1).rg;
								neighborHasNeighbor[splineCount]=true;
							}
						}
					// END ENERGY DEBUG INSERT
					splineCount++;
				}
				
			} else if ( (neighborflags & HAS_CORRECTED_POSITION) == HAS_CORRECTED_POSITION) {
				// if( (perVertexIn[0].flags & DONT_OPTIMIZE_N) == DONT_OPTIMIZE_N ) {linearNoOpt[linearCount] = true;} else {linearNoOpt[linearCount] = false;}
				//in this case our northern neighbour is the non spline part of a t-junction
				linearNeighbors[linearCount] = texelFetch(indexedCellPositions, perVertexIn[0].neighbors.x+1).rg;
				linearCount++;
			} else {
				// if( (perVertexIn[0].flags & DONT_OPTIMIZE_N) == DONT_OPTIMIZE_N ) {linearNoOpt[linearCount] = true;} else {linearNoOpt[linearCount] = false;}
				linearNeighbors[linearCount] = texelFetch(indexedCellPositions, perVertexIn[0].neighbors.x).rg;
				linearCount++;
			}
		}
	}
	if(easternConnectionPresent) {
		//get the neighbours falgs
		int neighborflags = texelFetch(CPflags, perVertexIn[0].neighbors.y).r;
		//check if there's a visible connection to a northern neighbor
		if( (perVertexIn[0].flags & HAS_EASTERN_NEIGHBOR) == HAS_EASTERN_NEIGHBOR ) {
			//check if point is part of a Spline
			if( (perVertexIn[0].flags & HAS_EASTERN_SPLINE) == HAS_EASTERN_SPLINE ) {
				if( ((perVertexIn[0].flags & DONT_OPTIMIZE_E) == DONT_OPTIMIZE_E ) || ((neighborflags & DONT_OPTIMIZE_W) == DONT_OPTIMIZE_W)) {
					splineNoOpt[splineCount] = true;
				} else {
					splineNoOpt[splineCount] = false;
				}
				if(neighborflags >= HAS_NORTHERN_SPLINE && !( (neighborflags & HAS_WESTERN_SPLINE) == HAS_WESTERN_SPLINE )) {
					//our neighbour is an endpoint on a t-junction and needs to be adjusted
					splineNeighbors[splineCount] = texelFetch(indexedCellPositions, perVertexIn[0].neighbors.y+1).rg;
					splineCount++;
				} else {
					splineNeighbors[splineCount] = texelFetch(indexedCellPositions, perVertexIn[0].neighbors.y).rg;
					
					// BEGIN ENERGY DEBUG INSERT
						if( (neighborflags & HAS_NORTHERN_SPLINE) == HAS_NORTHERN_SPLINE) {
							//get neighbors neighbors flag
							int neighborsNeighborIndex = getNeighborIndex(perVertexIn[0].neighbors.y, NORTH, 0);
							int neighborsNeighborflags = texelFetch(CPflags,neighborsNeighborIndex).r;
							if( ((neighborsNeighborflags & HAS_SOUTHERN_SPLINE) == HAS_SOUTHERN_SPLINE) 
								|| !( (neighborsNeighborflags & HAS_CORRECTED_POSITION) == HAS_CORRECTED_POSITION) ){
								//take this one
								splineNeighborsNeighbors[splineCount] = texelFetch(indexedCellPositions,neighborsNeighborIndex).rg;
								neighborHasNeighbor[splineCount]=true;
							} else {
								//use the corrected position
								splineNeighborsNeighbors[splineCount] = texelFetch(indexedCellPositions,neighborsNeighborIndex+1).rg;
								neighborHasNeighbor[splineCount]=true;
							}
						} else if((neighborflags & HAS_EASTERN_SPLINE) == HAS_EASTERN_SPLINE) {
							int neighborsNeighborIndex = getNeighborIndex(perVertexIn[0].neighbors.y, EAST, 0);
							int neighborsNeighborflags = texelFetch(CPflags,neighborsNeighborIndex).r;
							if( ((neighborsNeighborflags & HAS_WESTERN_SPLINE) == HAS_WESTERN_SPLINE) 
								|| !( (neighborsNeighborflags & HAS_CORRECTED_POSITION) == HAS_CORRECTED_POSITION) ){
								//take this one
								splineNeighborsNeighbors[splineCount] = texelFetch(indexedCellPositions,neighborsNeighborIndex).rg;
								neighborHasNeighbor[splineCount]=true;
							} else {
								//use the corrected position
								splineNeighborsNeighbors[splineCount] = texelFetch(indexedCellPositions,neighborsNeighborIndex+1).rg;
								neighborHasNeighbor[splineCount]=true;
							}
						} else if((neighborflags & HAS_SOUTHERN_SPLINE) == HAS_SOUTHERN_SPLINE) {
							int neighborsNeighborIndex = getNeighborIndex(perVertexIn[0].neighbors.y, SOUTH, 0);
							int neighborsNeighborflags = texelFetch(CPflags,neighborsNeighborIndex).r;
							if( ((neighborsNeighborflags & HAS_NORTHERN_SPLINE) == HAS_NORTHERN_SPLINE) 
								|| !( (neighborsNeighborflags & HAS_CORRECTED_POSITION) == HAS_CORRECTED_POSITION) ){
								//take this one
								splineNeighborsNeighbors[splineCount] = texelFetch(indexedCellPositions,neighborsNeighborIndex).rg;
								neighborHasNeighbor[splineCount]=true;
							} else {
								//use the corrected position
								splineNeighborsNeighbors[splineCount] = texelFetch(indexedCellPositions,neighborsNeighborIndex+1).rg;
								neighborHasNeighbor[splineCount]=true;
							}
						}
					// END ENERGY DEBUG INSERT
					splineCount++;
				}
			} else if ( (neighborflags & HAS_CORRECTED_POSITION) == HAS_CORRECTED_POSITION) {
				// if( (perVertexIn[0].flags & DONT_OPTIMIZE_E) == DONT_OPTIMIZE_E ) {linearNoOpt[linearCount] = true;} else {linearNoOpt[linearCount] = false;}
				//in this case our northern neighbour is the non spline part of a t-junction
				linearNeighbors[linearCount] = texelFetch(indexedCellPositions, perVertexIn[0].neighbors.y+1).rg;
				linearCount++;
			} else {
				// if( (perVertexIn[0].flags & DONT_OPTIMIZE_E) == DONT_OPTIMIZE_E ) {linearNoOpt[linearCount] = true;} else {linearNoOpt[linearCount] = false;}
				linearNeighbors[linearCount] = texelFetch(indexedCellPositions, perVertexIn[0].neighbors.y).rg;
				linearCount++;
			}
		}
	}

	if(southernConnectionPresent) {
		//get the neighbours falgs
		int neighborflags = texelFetch(CPflags, perVertexIn[0].neighbors.z).r;
		//check if there's a visible connection to a northern neighbor
		if( (perVertexIn[0].flags & HAS_SOUTHERN_NEIGHBOR) == HAS_SOUTHERN_NEIGHBOR ) {
			//check if point is part of a Spline
			if( (perVertexIn[0].flags & HAS_SOUTHERN_SPLINE) == HAS_SOUTHERN_SPLINE ) {
				if( ((perVertexIn[0].flags & DONT_OPTIMIZE_S) == DONT_OPTIMIZE_S ) || ((neighborflags & DONT_OPTIMIZE_N) == DONT_OPTIMIZE_N)) {
					splineNoOpt[splineCount] = true;
				} else {
					splineNoOpt[splineCount] = false;
				}
				if(neighborflags >= HAS_NORTHERN_SPLINE && !( (neighborflags & HAS_NORTHERN_SPLINE) == HAS_NORTHERN_SPLINE )) {
					//our neighbour is an endpoint on a t-junction and needs to be adjusted
					splineNeighbors[splineCount] = texelFetch(indexedCellPositions, perVertexIn[0].neighbors.z+1).rg;
					splineCount++;
				} else {
					splineNeighbors[splineCount] = texelFetch(indexedCellPositions, perVertexIn[0].neighbors.z).rg;
					
					// BEGIN ENERGY DEBUG INSERT
						if( (neighborflags & HAS_WESTERN_SPLINE) == HAS_WESTERN_SPLINE) {
							//get neighbors neighbors flag
							int neighborsNeighborIndex = getNeighborIndex(perVertexIn[0].neighbors.z, WEST, 0);
							int neighborsNeighborflags = texelFetch(CPflags,neighborsNeighborIndex).r;
							if( ((neighborsNeighborflags & HAS_EASTERN_SPLINE) == HAS_EASTERN_SPLINE) 
								|| !( (neighborsNeighborflags & HAS_CORRECTED_POSITION) == HAS_CORRECTED_POSITION) ){
								//take this one
								splineNeighborsNeighbors[splineCount] = texelFetch(indexedCellPositions,neighborsNeighborIndex).rg;
								neighborHasNeighbor[splineCount]=true;
							} else {
								//use the corrected position
								splineNeighborsNeighbors[splineCount] = texelFetch(indexedCellPositions,neighborsNeighborIndex+1).rg;
								neighborHasNeighbor[splineCount]=true;
							}
						} else if((neighborflags & HAS_EASTERN_SPLINE) == HAS_EASTERN_SPLINE) {
							int neighborsNeighborIndex = getNeighborIndex(perVertexIn[0].neighbors.z, EAST, 0);
							int neighborsNeighborflags = texelFetch(CPflags,neighborsNeighborIndex).r;
							if( ((neighborsNeighborflags & HAS_WESTERN_SPLINE) == HAS_WESTERN_SPLINE) 
								|| !( (neighborsNeighborflags & HAS_CORRECTED_POSITION) == HAS_CORRECTED_POSITION) ){
								//take this one
								splineNeighborsNeighbors[splineCount] = texelFetch(indexedCellPositions,neighborsNeighborIndex).rg;
								neighborHasNeighbor[splineCount]=true;
							} else {
								//use the corrected position
								splineNeighborsNeighbors[splineCount] = texelFetch(indexedCellPositions,neighborsNeighborIndex+1).rg;
								neighborHasNeighbor[splineCount]=true;
							}
						} else if((neighborflags & HAS_SOUTHERN_SPLINE) == HAS_SOUTHERN_SPLINE) {
							int neighborsNeighborIndex = getNeighborIndex(perVertexIn[0].neighbors.z, SOUTH, 0);
							int neighborsNeighborflags = texelFetch(CPflags,neighborsNeighborIndex).r;
							if( ((neighborsNeighborflags & HAS_NORTHERN_SPLINE) == HAS_NORTHERN_SPLINE) 
								|| !( (neighborsNeighborflags & HAS_CORRECTED_POSITION) == HAS_CORRECTED_POSITION) ){
								//take this one
								splineNeighborsNeighbors[splineCount] = texelFetch(indexedCellPositions,neighborsNeighborIndex).rg;
								neighborHasNeighbor[splineCount]=true;
							} else {
								//use the corrected position
								splineNeighborsNeighbors[splineCount] = texelFetch(indexedCellPositions,neighborsNeighborIndex+1).rg;
								neighborHasNeighbor[splineCount]=true;
							}
						}
					// END ENERGY DEBUG INSERT
					splineCount++;
				}
			} else if ( (neighborflags & HAS_CORRECTED_POSITION) == HAS_CORRECTED_POSITION) {
				// if( (perVertexIn[0].flags & DONT_OPTIMIZE_S) == DONT_OPTIMIZE_S ) {linearNoOpt[linearCount] = true;} else {linearNoOpt[linearCount] = false;}
				//in this case our northern neighbour is the non spline part of a t-junction
				linearNeighbors[linearCount] = texelFetch(indexedCellPositions, perVertexIn[0].neighbors.z+1).rg;
				linearCount++;
			} else {
				// if( (perVertexIn[0].flags & DONT_OPTIMIZE_S) == DONT_OPTIMIZE_S ) {linearNoOpt[linearCount] = true;} else {linearNoOpt[linearCount] = false;}
				linearNeighbors[linearCount] = texelFetch(indexedCellPositions, perVertexIn[0].neighbors.z).rg;
				linearCount++;
			}
		}
	}

	if(westernConnectionPresent) {
		//get the neighbours falgs
		int neighborflags = texelFetch(CPflags, perVertexIn[0].neighbors.w).r;
		//check if there's a visible connection to a northern neighbor
		if( (perVertexIn[0].flags & HAS_WESTERN_NEIGHBOR) == HAS_WESTERN_NEIGHBOR ) {
			//check if point is part of a Spline
			if( (perVertexIn[0].flags & HAS_WESTERN_SPLINE) == HAS_WESTERN_SPLINE ) {
				if( ((perVertexIn[0].flags & DONT_OPTIMIZE_W) == DONT_OPTIMIZE_W ) || ((neighborflags & DONT_OPTIMIZE_E) == DONT_OPTIMIZE_E)) {
				splineNoOpt[splineCount] = true;
				} else {
				splineNoOpt[splineCount] = false;
				}
				if(neighborflags >= HAS_NORTHERN_SPLINE && !( (neighborflags & HAS_EASTERN_SPLINE) == HAS_EASTERN_SPLINE )) {
					//our neighbour is an endpoint on a t-junction and needs to be adjusted
					splineNeighbors[splineCount] = texelFetch(indexedCellPositions, perVertexIn[0].neighbors.w+1).rg;
					splineCount++;
				} else {
					splineNeighbors[splineCount] = texelFetch(indexedCellPositions, perVertexIn[0].neighbors.w).rg;
					
					// BEGIN ENERGY DEBUG INSERT
						if( (neighborflags & HAS_NORTHERN_SPLINE) == HAS_NORTHERN_SPLINE) {
							//get neighbors neighbors flag
							int neighborsNeighborIndex = getNeighborIndex(perVertexIn[0].neighbors.w, NORTH, 0);
							int neighborsNeighborflags = texelFetch(CPflags,neighborsNeighborIndex).r;
							if( ((neighborsNeighborflags & HAS_SOUTHERN_SPLINE) == HAS_SOUTHERN_SPLINE) 
								|| !( (neighborsNeighborflags & HAS_CORRECTED_POSITION) == HAS_CORRECTED_POSITION) ){
								//take this one
								splineNeighborsNeighbors[splineCount] = texelFetch(indexedCellPositions,neighborsNeighborIndex).rg;
								neighborHasNeighbor[splineCount]=true;
							} else {
								//use the corrected position
								splineNeighborsNeighbors[splineCount] = texelFetch(indexedCellPositions,neighborsNeighborIndex+1).rg;
								neighborHasNeighbor[splineCount]=true;
							}
						} else if((neighborflags & HAS_WESTERN_SPLINE) == HAS_WESTERN_SPLINE) {
							int neighborsNeighborIndex = getNeighborIndex(perVertexIn[0].neighbors.w, WEST, 0);
							int neighborsNeighborflags = texelFetch(CPflags,neighborsNeighborIndex).r;
							if( ((neighborsNeighborflags & HAS_EASTERN_SPLINE) == HAS_EASTERN_SPLINE) 
								|| !( (neighborsNeighborflags & HAS_CORRECTED_POSITION) == HAS_CORRECTED_POSITION) ){
								//take this one
								splineNeighborsNeighbors[splineCount] = texelFetch(indexedCellPositions,neighborsNeighborIndex).rg;
								neighborHasNeighbor[splineCount]=true;
							} else {
								//use the corrected position
								splineNeighborsNeighbors[splineCount] = texelFetch(indexedCellPositions,neighborsNeighborIndex+1).rg;
								neighborHasNeighbor[splineCount]=true;
							}
						} else if((neighborflags & HAS_SOUTHERN_SPLINE) == HAS_SOUTHERN_SPLINE) {
							int neighborsNeighborIndex = getNeighborIndex(perVertexIn[0].neighbors.w, SOUTH, 0);
							int neighborsNeighborflags = texelFetch(CPflags,neighborsNeighborIndex).r;
							if( ((neighborsNeighborflags & HAS_NORTHERN_SPLINE) == HAS_NORTHERN_SPLINE) 
								|| !( (neighborsNeighborflags & HAS_CORRECTED_POSITION) == HAS_CORRECTED_POSITION) ){
								//take this one
								splineNeighborsNeighbors[splineCount] = texelFetch(indexedCellPositions,neighborsNeighborIndex).rg;
								neighborHasNeighbor[splineCount]=true;
							} else {
								//use the corrected position
								splineNeighborsNeighbors[splineCount] = texelFetch(indexedCellPositions,neighborsNeighborIndex+1).rg;
								neighborHasNeighbor[splineCount]=true;
							}
						}
					// END ENERGY DEBUG INSERT
					splineCount++;
				}
			} else if ( (neighborflags & HAS_CORRECTED_POSITION) == HAS_CORRECTED_POSITION) {
				// if( (perVertexIn[0].flags & DONT_OPTIMIZE_W) == DONT_OPTIMIZE_W ) {linearNoOpt[linearCount] = true;} else {linearNoOpt[linearCount] = false;}
				//in this case our northern neighbour is the non spline part of a t-junction
				linearNeighbors[linearCount] = texelFetch(indexedCellPositions, perVertexIn[0].neighbors.w+1).rg;
				linearCount++;
			} else {
				// if( (perVertexIn[0].flags & DONT_OPTIMIZE_W) == DONT_OPTIMIZE_W ) {linearNoOpt[linearCount] = true;} else {linearNoOpt[linearCount] = false;}
				linearNeighbors[linearCount] = texelFetch(indexedCellPositions, perVertexIn[0].neighbors.w).rg;
				linearCount++;
			}
		}
	}

	//draw

	//draw spline segments
	perVertexColor = vec4(0.0,0.0,0.0,0.0);
	if(splineCount == 2) {
		//perVertexColor = vec4(hsv2rgb(calcEnergy(splineNeighbors[0], perVertexIn[0].pos, splineNeighbors[1])*2.0, 1.0,1.0),0.0);
		if (splineNoOpt[0] == true) {perVertexColor = vec4(0.0,0.0,1.0,0.0);} else {
			// float energy = calcCurveEnergy(splineNeighborsNeighbors[0],splineNeighbors[0], perVertexIn[0].pos,splineNeighbors[1],splineNeighborsNeighbors[1], neighborHasNeighbor[0], neighborHasNeighbor[1]);
			float energy = calcCurveEnergy(splineNeighborsNeighbors[0],splineNeighbors[0], perVertexIn[0].pos,splineNeighbors[1],splineNeighborsNeighbors[1], neighborHasNeighbor[0], neighborHasNeighbor[1]);
			perVertexColor = evaluateEnergy(energy);
			
			//if(gl_PrimitiveIDIn == 41 || gl_PrimitiveIDIn == 12) {perVertexColor.b = 0.7;}
		}
		for (float t = 0.0 ; t <  (1.0 + STEP) ; t = t + STEP) {
			if(t >= 0.5){
				if (splineNoOpt[1] == true) {perVertexColor = vec4(0.0,0.0,1.0,0.0);} 
			}
			gl_Position = mapVertex(calcSplinePoint(splineNeighbors[0], perVertexIn[0].pos, splineNeighbors[1], t));
			EmitVertex();
		}
		EndPrimitive();
	}
	//draw the linear segments
	perVertexColor = vec4(0.0,0.0,0.0,0.0);
	for(int i = 0 ; i < linearCount ; i++) {
		// if (linearNoOpt[i] == true) {perVertexColor = vec4(1.0,0.0,0.0,0.0);} else {perVertexColor = vec4(0.0,0.0,0.0,0.0);}
		gl_Position = mapVertex(correctedPosition);
		EmitVertex();
		gl_Position = mapVertex( mix(correctedPosition, linearNeighbors[i], 0.5) );
		EmitVertex();
		EndPrimitive();
	}
	//for(int i = 0 ; i < linearCount ; i++) {
	//	for (float t = 0.0 ; t <= 1.0 ; t = t + STEP) {
	//		gl_Position = mapVertex(calcSplinePoint(correctedPosition, correctedPosition, linearNeighbors[i], t));
	//		EmitVertex();
	//	}
	//	EndPrimitive();
	//}
}
