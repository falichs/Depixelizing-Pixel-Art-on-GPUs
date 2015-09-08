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

#define EDGE_HORVERT 16
#define EDGE_DIAGONAL_ULLR 32
#define EDGE_DIAGONAL_LLUR 64

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

//Similaritygraph per-node Directions 
#define NORTH		128
#define NORTHEAST	64
#define EAST		32
#define SOUTHEAST	16
#define SOUTH		8
#define SOUTHWEST	4
#define WEST		2
#define NORTHWEST	1

//Constants
#define STEP 0.2
#define GAUSS_MULTIPLIER 2.5
#define LINEAR_SEGMENT_LENGTH 1.0

layout(pixel_center_integer) in vec4 gl_FragCoord;
layout(location = 0) out vec4 color0;

uniform sampler2D pixelArt;
uniform sampler2D similarityGraph;
uniform samplerBuffer indexedCellPositions;
uniform isamplerBuffer neighborIndices;
uniform isamplerBuffer CPflags;
uniform ivec2 viewportDimensions;
uniform float uf_zoomFactor;
uniform vec2 uv2_zoomWindowCenter;

vec2 ULCoords;
vec2 URCoords;
vec2 LLCoords;
vec2 LRCoords;
bvec4 influencingPixels;
vec2 cellSpaceCoords;

//----------------------------------------------------------------
// simple method for fetching cell-graph neighbor indices
//----------------------------------------------------------------
int getNeighborIndex(int sourceIndex, int dir) {
	int index = -1;

	if( (dir & NORTH) == NORTH) {
		index = texelFetch(neighborIndices,sourceIndex).x;
	} else if( (dir & EAST) == EAST) {
		index = texelFetch(neighborIndices,sourceIndex).y;
	} else if( (dir & SOUTH) == SOUTH) {
		index = texelFetch(neighborIndices,sourceIndex).z;
	} else if( (dir & WEST) == WEST) {
		index = texelFetch(neighborIndices,sourceIndex).w;
	}
	return index;
}
//----------------------------------------------------------------

//----------------------------------------------------------------
// calulates the spline point at parametric position t
//----------------------------------------------------------------
vec2 calcSplinePoint(vec2 p0, vec2 p1, vec2 p2, float t) {
	float t2 = 0.5*t*t;
	float a = t2 - t + 0.5;
	float b = -2.0 * t2 + t + 0.5;
	return a*p0 + b*p1 + t2*p2;
}
//----------------------------------------------------------------

//----------------------------------------------------------------
// calulates the spline endpoint at parametric position t
//----------------------------------------------------------------
vec2 calcSplineEndPoint(vec2 p0, vec2 p1, float t) {
	float t2 = 0.5*t*t;
	return (-t2+1)*p0 + t2*p1;
}
//----------------------------------------------------------------

//----------------------------------------------------------------
// test two lines for intersection
// source: http://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect
//----------------------------------------------------------------
bool intersects(vec2 lineApointA, vec2 lineApointB, vec2 lineBpointA, vec2 lineBpointB) {
	vec2 r = lineApointB - lineApointA;
	vec2 s = lineBpointB - lineBpointA;
	float rXs = r.x * s.y - r.y * s.x;
	if (rXs == 0.0) {
		return false;
	}
	vec2 ba = lineBpointA - lineApointA;
	float t = (ba.x * s.y - ba.y * s.x)/rXs;
	if( (t < 0.0) || (t > 1.0) ) {
		return false;
	}
	float u = (ba.x * r.y - ba.y * r.x)/rXs;
	if( (u < 0.0) || (u > 1.0) ) {
		return false;
	}
	return true;
}

int computeValence(int flags) {
	int valence = 0;
	if((flags & HAS_NORTHERN_NEIGHBOR) == HAS_NORTHERN_NEIGHBOR) 
		valence++;
	if((flags & HAS_EASTERN_NEIGHBOR) == HAS_EASTERN_NEIGHBOR) 
		valence++;
	if((flags & HAS_SOUTHERN_NEIGHBOR) == HAS_SOUTHERN_NEIGHBOR) 
		valence++;
	if((flags & HAS_WESTERN_NEIGHBOR) == HAS_WESTERN_NEIGHBOR) 
		valence++;
	return valence;
}

ivec2 getCPs(int node0neighborIndex, int dir) {
	ivec2 cpArray = ivec2(node0neighborIndex,-1);
	ivec3 checkFwd = ivec3(0,0,0);
	ivec2 chkdirs = ivec2(0,0);
	int checkBack = 0;
	if( dir  == NORTH) {
		checkFwd.x = HAS_NORTHERN_SPLINE;
		checkFwd.y = HAS_EASTERN_SPLINE;
		checkFwd.z = HAS_WESTERN_SPLINE;
		checkBack = HAS_SOUTHERN_SPLINE;
		chkdirs.x = EAST;
		chkdirs.y = WEST;
	} else if( dir== EAST) {
		checkFwd.x = HAS_EASTERN_SPLINE;
		checkFwd.y = HAS_SOUTHERN_SPLINE;
		checkFwd.z = HAS_NORTHERN_SPLINE;
		checkBack = HAS_WESTERN_SPLINE;
		chkdirs.x = SOUTH;
		chkdirs.y = NORTH;
	} else if( dir == SOUTH) {
		checkFwd.x = HAS_SOUTHERN_SPLINE;
		checkFwd.y = HAS_WESTERN_SPLINE;
		checkFwd.z = HAS_EASTERN_SPLINE;
		checkBack = HAS_NORTHERN_SPLINE;
		chkdirs.x = WEST;
		chkdirs.y = EAST;
	} else if( dir == WEST) {
		checkFwd.x = HAS_WESTERN_SPLINE;
		checkFwd.y = HAS_NORTHERN_SPLINE;
		checkFwd.z = HAS_SOUTHERN_SPLINE;
		checkBack = HAS_EASTERN_SPLINE;
		chkdirs.x = NORTH;
		chkdirs.y = SOUTH;
	}
	
	int node0neighborFlags = texelFetch(CPflags,node0neighborIndex).r;
	//check for t-junktion
	if((node0neighborFlags & checkBack) == checkBack) {
		//the spline continues through the next control point
		//get next spline control point to compute segment extension
		if((node0neighborFlags & checkFwd.x) == checkFwd.x) {
			int neighborsNeighborIndex = getNeighborIndex(node0neighborIndex, dir);
			int neighborsNeighborflags = texelFetch(CPflags,neighborsNeighborIndex).r;
			if ( (neighborsNeighborflags & HAS_CORRECTED_POSITION) == HAS_CORRECTED_POSITION) {
				cpArray.y = neighborsNeighborIndex + 1;
			} else {
				cpArray.y = neighborsNeighborIndex;
			}
		}
		else if((node0neighborFlags & checkFwd.y) == checkFwd.y) {
			int neighborsNeighborIndex = getNeighborIndex(node0neighborIndex, chkdirs.x);
			int neighborsNeighborflags = texelFetch(CPflags,neighborsNeighborIndex).r;
			if ( (neighborsNeighborflags & HAS_CORRECTED_POSITION) == HAS_CORRECTED_POSITION) {
				cpArray.y = neighborsNeighborIndex + 1;
			} else {
				cpArray.y = neighborsNeighborIndex;
			}
		} 
		else if((node0neighborFlags & checkFwd.z) == checkFwd.z) {
			int neighborsNeighborIndex = getNeighborIndex(node0neighborIndex, chkdirs.y);
			int neighborsNeighborflags = texelFetch(CPflags,neighborsNeighborIndex).r;
			if ( (neighborsNeighborflags & HAS_CORRECTED_POSITION) == HAS_CORRECTED_POSITION) {
				cpArray.y = neighborsNeighborIndex + 1;
			} else {
				cpArray.y = neighborsNeighborIndex;
			}
		}
	} else {
		if ( (node0neighborFlags & HAS_CORRECTED_POSITION) == HAS_CORRECTED_POSITION) {
			cpArray.x++;
		} 
	}
	
	return cpArray;
}

bvec4 findSegmentIntersections(vec2 p0, vec2 p1, vec2 p2) {
	vec2 pointA = calcSplinePoint(p0, p1, p2, 0.0);
	for (float t = STEP ; t <  (1.0 + STEP) ; t = t + STEP) {
		vec2 pointB = calcSplinePoint(p0, p1, p2, t);
		//evaluate interections
		if( intersects(cellSpaceCoords, ULCoords, pointA, pointB) ) {
			influencingPixels.x = false;
		}
		if( intersects(cellSpaceCoords, URCoords, pointA, pointB) ) {
			influencingPixels.y = false;
		}
		if( intersects(cellSpaceCoords, LLCoords, pointA, pointB) ) {
			influencingPixels.z = false;
		}
		if( intersects(cellSpaceCoords, LRCoords, pointA, pointB) ) {
			influencingPixels.w = false;
		}
		pointA = pointB;
	}
	return influencingPixels;
}

//----------------------------------------------------------------
void main() {

	// create a boolean vector containing information on the influence of surrounding pixels  on this fragment 
	// R G B A ... UL UR LL LR
	influencingPixels = bvec4(true,true,true,true);
	// convert fragment-space coordinates to cell-space coordinates
	cellSpaceCoords = (textureSize(pixelArt,0) - 1) * (gl_FragCoord.xy / vec2(viewportDimensions - 1)  * uf_zoomFactor + (uv2_zoomWindowCenter - uf_zoomFactor*0.5));
	// Knot Buffer lookup Coordinates
	int fragmentBaseKnotIndex = int (2 * floor(cellSpaceCoords.x) + floor(cellSpaceCoords.y) * 2 * (textureSize(pixelArt,0).x - 1) );
	//fetch flags
	int node0flags = texelFetch(CPflags,fragmentBaseKnotIndex).r;
	bool hasCorrectedPosition = false; //TODO!!!
	// surrounding pixel Coordinates 
	ULCoords = vec2(floor(cellSpaceCoords.x),ceil(cellSpaceCoords.y));
	URCoords = vec2(ceil(cellSpaceCoords.x),ceil(cellSpaceCoords.y));
	LLCoords = vec2(floor(cellSpaceCoords.x),floor(cellSpaceCoords.y));
	LRCoords = vec2(ceil(cellSpaceCoords.x),floor(cellSpaceCoords.y));
	
	if(node0flags > 0.0) {
		//gather neighbors
		ivec4 node0neighbors = texelFetch(neighborIndices, fragmentBaseKnotIndex);
		//compute valence
		int node0valence = computeValence(node0flags);
		vec2 node0pos = texelFetch(indexedCellPositions,fragmentBaseKnotIndex).rg;
		if(node0valence == 1) {
			ivec2 cpArray = ivec2(-1,-1); //this array holds indices of the neighboring spline control points we need to interpolate
			if((node0flags & HAS_NORTHERN_NEIGHBOR) == HAS_NORTHERN_NEIGHBOR) {
				cpArray = getCPs(node0neighbors.x, NORTH);
			} else if((node0flags & HAS_EASTERN_NEIGHBOR) == HAS_EASTERN_NEIGHBOR) {
				cpArray = getCPs(node0neighbors.y, EAST);
			} else if((node0flags & HAS_SOUTHERN_NEIGHBOR) == HAS_SOUTHERN_NEIGHBOR) {
				cpArray = getCPs(node0neighbors.z, SOUTH);
			} else if((node0flags & HAS_WESTERN_NEIGHBOR) == HAS_WESTERN_NEIGHBOR) {
				cpArray = getCPs(node0neighbors.w, WEST);
			}
			vec2 p1pos =  texelFetch(indexedCellPositions,cpArray.x).rg;
			findSegmentIntersections(node0pos, node0pos, p1pos);
			if(cpArray.y > -1) {
				vec2 p2pos =  texelFetch(indexedCellPositions,cpArray.y).rg;
				findSegmentIntersections(node0pos, p1pos, p2pos);
			} else {
				findSegmentIntersections(node0pos, p1pos, p1pos);
			}
		} else if(node0valence == 2) {
			ivec4 cpArray = ivec4(-1,-1,-1,-1); //this array holds indices of the neighboring spline control points we need to interpolate
			bool foundFirst = false;
			if((node0flags & HAS_NORTHERN_NEIGHBOR) == HAS_NORTHERN_NEIGHBOR) {
				cpArray.xy = getCPs(node0neighbors.x, NORTH);
				foundFirst = true;
			}
			
			if((node0flags & HAS_EASTERN_NEIGHBOR) == HAS_EASTERN_NEIGHBOR) {
				if(foundFirst) {
					cpArray.zw = getCPs(node0neighbors.y, EAST);
				} else {
					cpArray.xy = getCPs(node0neighbors.y, EAST);
					foundFirst = true;
				}
			}
			if((node0flags & HAS_SOUTHERN_NEIGHBOR) == HAS_SOUTHERN_NEIGHBOR) {
				if(foundFirst) {
					cpArray.zw = getCPs(node0neighbors.z, SOUTH);
				} else {
					cpArray.xy = getCPs(node0neighbors.z, SOUTH);
					foundFirst = true;
				}
			}
			
			if((node0flags & HAS_WESTERN_NEIGHBOR) == HAS_WESTERN_NEIGHBOR) {
				cpArray.zw = getCPs(node0neighbors.w, WEST);
			}
			vec2 pm1pos =  texelFetch(indexedCellPositions,cpArray.x).rg;
			vec2 p1pos =  texelFetch(indexedCellPositions,cpArray.z).rg;
			findSegmentIntersections(pm1pos, node0pos, p1pos);
			if(cpArray.y > -1) {
				vec2 pm2pos =  texelFetch(indexedCellPositions,cpArray.y).rg;
				findSegmentIntersections(node0pos, pm1pos, pm2pos);
			} else {
				findSegmentIntersections(node0pos, pm1pos, pm1pos);
			}
			if(cpArray.w > -1) {
				vec2 p2pos =  texelFetch(indexedCellPositions,cpArray.w).rg;
				findSegmentIntersections(node0pos, p1pos, p2pos);
			} else {
				findSegmentIntersections(node0pos, p1pos, p1pos);
			}
		}
		else if(node0valence == 3) {
			hasCorrectedPosition = true;
			ivec4 cpArray = ivec4(-1,-1,-1,-1); //this array holds indices of the neighboring spline control points we need to interpolate
			bool foundFirst = false;
			int tBaseDir = 0;
			int tBaseNeighborIndex = -1;
			if((node0flags & HAS_NORTHERN_NEIGHBOR) == HAS_NORTHERN_NEIGHBOR) {
				if((node0flags & HAS_NORTHERN_SPLINE) == HAS_NORTHERN_SPLINE) {
					cpArray.xy = getCPs(node0neighbors.x, NORTH);
					foundFirst = true;
				} else {
					tBaseDir = NORTH;
					tBaseNeighborIndex = node0neighbors.x;
				}
			}
			
			if((node0flags & HAS_EASTERN_NEIGHBOR) == HAS_EASTERN_NEIGHBOR) {
				if((node0flags & HAS_EASTERN_SPLINE) == HAS_EASTERN_SPLINE) {
					if(foundFirst) {
						cpArray.zw = getCPs(node0neighbors.y, EAST);
					} else {
						cpArray.xy = getCPs(node0neighbors.y, EAST);
						foundFirst = true;
					}
				} else {
					tBaseDir = EAST;
					tBaseNeighborIndex = node0neighbors.y;
				}
			}
			if((node0flags & HAS_SOUTHERN_NEIGHBOR) == HAS_SOUTHERN_NEIGHBOR) {
				if((node0flags & HAS_SOUTHERN_SPLINE) == HAS_SOUTHERN_SPLINE) {
					if(foundFirst) {
						cpArray.zw = getCPs(node0neighbors.z, SOUTH);
					} else {
						cpArray.xy = getCPs(node0neighbors.z, SOUTH);
						foundFirst = true;
					}
				} else {
					tBaseDir = SOUTH;
					tBaseNeighborIndex = node0neighbors.z;
				}
			}
			
			if((node0flags & HAS_WESTERN_NEIGHBOR) == HAS_WESTERN_NEIGHBOR) {
				if((node0flags & HAS_WESTERN_SPLINE) == HAS_WESTERN_SPLINE) {
					cpArray.zw = getCPs(node0neighbors.w, WEST);
				} else {
					tBaseDir = WEST;
					tBaseNeighborIndex = node0neighbors.w;
				}
			}
			vec2 pm1pos =  texelFetch(indexedCellPositions,cpArray.x).rg;
			vec2 p1pos =  texelFetch(indexedCellPositions,cpArray.z).rg;
			findSegmentIntersections(pm1pos, node0pos, p1pos);
			if(cpArray.y > -1) {
				vec2 pm2pos =  texelFetch(indexedCellPositions,cpArray.y).rg;
				findSegmentIntersections(node0pos, pm1pos, pm2pos);
			} else {
				findSegmentIntersections(node0pos, pm1pos, pm1pos);
			}
			if(cpArray.w > -1) {
				vec2 p2pos =  texelFetch(indexedCellPositions,cpArray.w).rg;
				findSegmentIntersections(node0pos, p1pos, p2pos);
			} else {
				findSegmentIntersections(node0pos, p1pos, p1pos);
			}
			
			//check T-Base
			cpArray.xy = getCPs(tBaseNeighborIndex, tBaseDir);
			node0pos = texelFetch(indexedCellPositions,fragmentBaseKnotIndex+1).rg;
			p1pos =  texelFetch(indexedCellPositions,cpArray.x).rg;
			findSegmentIntersections(node0pos, node0pos, p1pos);
			if(cpArray.y > -1) {
				vec2 p2pos =  texelFetch(indexedCellPositions,cpArray.y).rg;
				findSegmentIntersections(node0pos, p1pos, p2pos);
			} else {
				findSegmentIntersections(node0pos, p1pos, p1pos);
			}
		}
		else { // valence 4
			ivec2 cpArray = ivec2(-1,-1);
			cpArray = getCPs(node0neighbors.x, NORTH);
			vec2 p1pos =  texelFetch(indexedCellPositions,cpArray.x).rg;
			findSegmentIntersections(node0pos, node0pos, p1pos);
			if(cpArray.y > -1) {
				vec2 p2pos =  texelFetch(indexedCellPositions,cpArray.y).rg;
				findSegmentIntersections(node0pos, p1pos, p2pos);
			} else {
				findSegmentIntersections(node0pos, p1pos, p1pos);
			}
			cpArray = getCPs(node0neighbors.y, EAST);
			p1pos =  texelFetch(indexedCellPositions,cpArray.x).rg;
			findSegmentIntersections(node0pos, node0pos, p1pos);
			if(cpArray.y > -1) {
				vec2 p2pos =  texelFetch(indexedCellPositions,cpArray.y).rg;
				findSegmentIntersections(node0pos, p1pos, p2pos);
			} else {
				findSegmentIntersections(node0pos, p1pos, p1pos);
			}
			cpArray = getCPs(node0neighbors.z, SOUTH);
			p1pos =  texelFetch(indexedCellPositions,cpArray.x).rg;
			findSegmentIntersections(node0pos, node0pos, p1pos);
			if(cpArray.y > -1) {
				vec2 p2pos =  texelFetch(indexedCellPositions,cpArray.y).rg;
				findSegmentIntersections(node0pos, p1pos, p2pos);
			} else {
				findSegmentIntersections(node0pos, p1pos, p1pos);
			}
			cpArray = getCPs(node0neighbors.w, WEST);
			p1pos =  texelFetch(indexedCellPositions,cpArray.x).rg;
			findSegmentIntersections(node0pos, node0pos, p1pos);
			if(cpArray.y > -1) {
				vec2 p2pos =  texelFetch(indexedCellPositions,cpArray.y).rg;
				findSegmentIntersections(node0pos, p1pos, p2pos);
			} else {
				findSegmentIntersections(node0pos, p1pos, p1pos);
			}
		}
	}
	if(!hasCorrectedPosition) {
		int node1flags = texelFetch(CPflags,fragmentBaseKnotIndex+1).r;
		if(node1flags > 0.0) {
			//gather neighbors
			ivec4 node1neighbors = texelFetch(neighborIndices, fragmentBaseKnotIndex+1);
			//compute valence
			int node1valence = computeValence(node1flags);
			vec2 node1pos = texelFetch(indexedCellPositions,fragmentBaseKnotIndex+1).rg;
			if(node1valence == 1) {
				ivec2 cpArray = ivec2(-1,-1); //this array holds indices of the neighboring spline control points we need to interpolate
				if((node1flags & HAS_NORTHERN_NEIGHBOR) == HAS_NORTHERN_NEIGHBOR) {
					cpArray = getCPs(node1neighbors.x, NORTH);
				} else if((node1flags & HAS_EASTERN_NEIGHBOR) == HAS_EASTERN_NEIGHBOR) {
					cpArray = getCPs(node1neighbors.y, EAST);
				} else if((node1flags & HAS_SOUTHERN_NEIGHBOR) == HAS_SOUTHERN_NEIGHBOR) {
					cpArray = getCPs(node1neighbors.z, SOUTH);
				} else if((node1flags & HAS_WESTERN_NEIGHBOR) == HAS_WESTERN_NEIGHBOR) {
					cpArray = getCPs(node1neighbors.w, WEST);
				}
				vec2 p1pos =  texelFetch(indexedCellPositions,cpArray.x).rg;
				findSegmentIntersections(node1pos, node1pos, p1pos);
				if(cpArray.y > -1) {
					vec2 p2pos =  texelFetch(indexedCellPositions,cpArray.y).rg;
					findSegmentIntersections(node1pos, p1pos, p2pos);
				} else {
					findSegmentIntersections(node1pos, p1pos, p1pos);
				}
			} else if(node1valence == 2) {
				ivec4 cpArray = ivec4(-1,-1,-1,-1); //this array holds indices of the neighboring spline control points we need to interpolate
				bool foundFirst = false;
				if((node1flags & HAS_NORTHERN_NEIGHBOR) == HAS_NORTHERN_NEIGHBOR) {
					cpArray.xy = getCPs(node1neighbors.x, NORTH);
					foundFirst = true;
				}
				
				if((node1flags & HAS_EASTERN_NEIGHBOR) == HAS_EASTERN_NEIGHBOR) {
					if(foundFirst) {
						cpArray.zw = getCPs(node1neighbors.y, EAST);
					} else {
						cpArray.xy = getCPs(node1neighbors.y, EAST);
						foundFirst = true;
					}
				}
				if((node1flags & HAS_SOUTHERN_NEIGHBOR) == HAS_SOUTHERN_NEIGHBOR) {
					if(foundFirst) {
						cpArray.zw = getCPs(node1neighbors.z, SOUTH);
					} else {
						cpArray.xy = getCPs(node1neighbors.z, SOUTH);
						foundFirst = true;
					}
				}
				
				if((node1flags & HAS_WESTERN_NEIGHBOR) == HAS_WESTERN_NEIGHBOR) {
					cpArray.zw = getCPs(node1neighbors.w, WEST);
				}
				vec2 pm1pos =  texelFetch(indexedCellPositions,cpArray.x).rg;
				vec2 p1pos =  texelFetch(indexedCellPositions,cpArray.z).rg;
				findSegmentIntersections(pm1pos, node1pos, p1pos);
				if(cpArray.y > -1) {
					vec2 pm2pos =  texelFetch(indexedCellPositions,cpArray.y).rg;
					findSegmentIntersections(node1pos, pm1pos, pm2pos);
				} else {
					findSegmentIntersections(node1pos, pm1pos, pm1pos);
				}
				if(cpArray.w > -1) {
					vec2 p2pos =  texelFetch(indexedCellPositions,cpArray.w).rg;
					findSegmentIntersections(node1pos, p1pos, p2pos);
				} else {
					findSegmentIntersections(node1pos, p1pos, p1pos);
				}
			}
		}
	}
	
	vec4 colorSum = vec4(0.0,0.0,0.0,0.0);
	float weightSum = 0.0;
	//influencingPixels order: UL UR LL LR
	
	if(influencingPixels.x) {
		//calculate influence of the Pixel
		vec4 col = texelFetch(pixelArt, ivec2(ULCoords),0);
		float dist = distance(cellSpaceCoords,ULCoords);
		float weight = exp(-(dist*dist)*GAUSS_MULTIPLIER);
		colorSum += col * weight;
		weightSum += weight;
		//checkout this pixels connected neigbors
		ivec2 lookupcoords = ivec2(2 * ULCoords + 1);
		int edges = int(texelFetch(similarityGraph, lookupcoords ,0).g);
		//calculate weights for those pixels
		if( (edges & SOUTHWEST) == SOUTHWEST ) {
			col = texelFetch(pixelArt, ivec2(ULCoords.x-1,ULCoords.y-1),0);
			dist = distance(cellSpaceCoords,vec2(ULCoords.x-1,ULCoords.y-1));
			weight = exp(-(dist*dist)*GAUSS_MULTIPLIER);
			colorSum += col * weight;
			weightSum += weight;
		}
		if( (edges & WEST) == WEST ) {
			col = texelFetch(pixelArt, ivec2(ULCoords.x-1,ULCoords.y),0);
			dist = distance(cellSpaceCoords,vec2(ULCoords.x-1,ULCoords.y));
			weight = exp(-(dist*dist)*GAUSS_MULTIPLIER);
			colorSum += col * weight;
			weightSum += weight;
		}
		if( (edges & NORTHWEST) == NORTHWEST ) {
			col = texelFetch(pixelArt, ivec2(ULCoords.x-1,ULCoords.y+1),0);
			dist = distance(cellSpaceCoords,vec2(ULCoords.x-1,ULCoords.y+1));
			weight = exp(-(dist*dist)*GAUSS_MULTIPLIER);
			colorSum += col * weight;
			weightSum += weight;
		}
		if( (edges & NORTH) == NORTH ) {
			col = texelFetch(pixelArt, ivec2(ULCoords.x,ULCoords.y+1),0);
			dist = distance(cellSpaceCoords,vec2(ULCoords.x,ULCoords.y+1));
			weight = exp(-(dist*dist)*GAUSS_MULTIPLIER);
			colorSum += col * weight;
			weightSum += weight;
		}
		if( (edges & NORTHEAST) == NORTHEAST ) {
			col = texelFetch(pixelArt, ivec2(ULCoords.x+1,ULCoords.y+1),0);
			dist = distance(cellSpaceCoords,vec2(ULCoords.x+1,ULCoords.y+1));
			weight = exp(-(dist*dist)*GAUSS_MULTIPLIER);
			colorSum += col * weight;
			weightSum += weight;
		}
	}
	
	if(influencingPixels.y) {
		//calculate influence of the Pixel
		vec4 col = texelFetch(pixelArt, ivec2(URCoords),0);
		float dist = distance(cellSpaceCoords,URCoords);
		float weight = exp(-(dist*dist)*GAUSS_MULTIPLIER);
		colorSum += col * weight;
		weightSum += weight;
		//checkout this pixels connected neigbors
		ivec2 lookupcoords = ivec2(2 * URCoords + 1);
		int edges = int(texelFetch(similarityGraph, lookupcoords ,0).g);
		//calculate weights for those pixels
		//TODO: THIS ACTUALLY CAUSES ARTIFACTS - NO IDEA WHY
		/*
		if( (edges & NORTHWEST) == NORTHWEST ) {
			col = texelFetch(pixelArt, ivec2(URCoords.x-1,URCoords.y+1),0);
			dist = distance(cellSpaceCoords,vec2(URCoords.x-1,URCoords.y+1));
			weight = exp(-(dist*dist)*GAUSS_MULTIPLIER);
			colorSum += col * weight;
			weightSum += weight;
		}
		*/
		if( (edges & NORTH) == NORTH ) {
			col = texelFetch(pixelArt, ivec2(URCoords.x,URCoords.y+1),0);
			dist = distance(cellSpaceCoords,vec2(URCoords.x,URCoords.y+1));
			weight = exp(-(dist*dist)*GAUSS_MULTIPLIER);
			colorSum += col * weight;
			weightSum += weight;
		}
		if( (edges & NORTHEAST) == NORTHEAST ) {
			col = texelFetch(pixelArt, ivec2(URCoords.x+1,URCoords.y+1),0);
			dist = distance(cellSpaceCoords,vec2(URCoords.x+1,URCoords.y+1));
			weight = exp(-(dist*dist)*GAUSS_MULTIPLIER);
			colorSum += col * weight;
			weightSum += weight;
		}
		if( (edges & EAST) == EAST ) {
			col = texelFetch(pixelArt, ivec2(URCoords.x+1,URCoords.y),0);
			dist = distance(cellSpaceCoords,vec2(URCoords.x+1,URCoords.y));
			weight = exp(-(dist*dist)*GAUSS_MULTIPLIER);
			colorSum += col * weight;
			weightSum += weight;
		}
		if( (edges & SOUTHEAST) == SOUTHEAST ) {
			col = texelFetch(pixelArt, ivec2(URCoords.x+1,URCoords.y-1),0);
			dist = distance(cellSpaceCoords,vec2(URCoords.x+1,URCoords.y-1));
			weight = exp(-(dist*dist)*GAUSS_MULTIPLIER);
			colorSum += col * weight;
			weightSum += weight;
		}
		
	}
	
	
	if(influencingPixels.z) {
		//calculate influence of the Pixel
		vec4 col = texelFetch(pixelArt, ivec2(LLCoords),0);
		float dist = distance(cellSpaceCoords,LLCoords);
		float weight = exp(-(dist*dist)*GAUSS_MULTIPLIER);
		colorSum += col * weight;
		weightSum += weight;
		//checkout this pixels connected neigbors
		ivec2 lookupcoords = ivec2(2 * LLCoords + 1);
		int edges = int(texelFetch(similarityGraph, lookupcoords ,0).g);
		//calculate weights for those pixels
		//TODO: THIS ACTUALLY CAUSES ARTIFACTS - NO IDEA WHY
		/*
		if( (edges & NORTHWEST) == NORTHWEST ) {
			col = texelFetch(pixelArt, ivec2(LLCoords.x-1,LLCoords.y+1),0);
			dist = distance(cellSpaceCoords,vec2(LLCoords.x-1,LLCoords.y+1));
			weight = exp(-(dist*dist)*GAUSS_MULTIPLIER);
			colorSum += col * weight;
			weightSum += weight;
		}
		*/
		if( (edges & WEST) == WEST ) {
			col = texelFetch(pixelArt, ivec2(LLCoords.x-1,LLCoords.y),0);
			dist = distance(cellSpaceCoords,vec2(LLCoords.x-1,LLCoords.y));
			weight = exp(-(dist*dist)*GAUSS_MULTIPLIER);
			colorSum += col * weight;
			weightSum += weight;
		}
		if( (edges & SOUTHWEST) == SOUTHWEST ) {
			col = texelFetch(pixelArt, ivec2(LLCoords.x-1,LLCoords.y-1),0);
			dist = distance(cellSpaceCoords,vec2(LLCoords.x-1,LLCoords.y-1));
			weight = exp(-(dist*dist)*GAUSS_MULTIPLIER);
			colorSum += col * weight;
			weightSum += weight;
		}
		if( (edges & SOUTH) == SOUTH ) {
			col = texelFetch(pixelArt, ivec2(LLCoords.x,LLCoords.y-1),0);
			dist = distance(cellSpaceCoords,vec2(LLCoords.x,LLCoords.y-1));
			weight = exp(-(dist*dist)*GAUSS_MULTIPLIER);
			colorSum += col * weight;
			weightSum += weight;
		}
		if( (edges & SOUTHEAST) == SOUTHEAST ) {
			col = texelFetch(pixelArt, ivec2(LLCoords.x+1,LLCoords.y-1),0);
			dist = distance(cellSpaceCoords,vec2(LLCoords.x+1,LLCoords.y-1));
			weight = exp(-(dist*dist)*GAUSS_MULTIPLIER);
			colorSum += col * weight;
			weightSum += weight;
		}
	}
	if(influencingPixels.w) {
		//calculate influence of the Pixel
		vec4 col = texelFetch(pixelArt, ivec2(LRCoords),0);
		float dist = distance(cellSpaceCoords,LRCoords);
		float weight = exp(-(dist*dist)*GAUSS_MULTIPLIER);
		colorSum += col * weight;
		weightSum += weight;
		//checkout this pixels connected neigbors
		ivec2 lookupcoords = ivec2(2 * LRCoords + 1);
		int edges = int(texelFetch(similarityGraph, lookupcoords ,0).g);
		//calculate weights for those pixels
		if( (edges & NORTHEAST) == NORTHEAST ) {
			col = texelFetch(pixelArt, ivec2(LRCoords.x+1,LRCoords.y+1),0);
			dist = distance(cellSpaceCoords,vec2(LRCoords.x+1,LRCoords.y+1));
			weight = exp(-(dist*dist)*GAUSS_MULTIPLIER);
			colorSum += col * weight;
			weightSum += weight;
		}
		if( (edges & EAST) == EAST ) {
			col = texelFetch(pixelArt, ivec2(LRCoords.x+1,LRCoords.y),0);
			dist = distance(cellSpaceCoords,vec2(LRCoords.x+1,LRCoords.y));
			weight = exp(-(dist*dist)*GAUSS_MULTIPLIER);
			colorSum += col * weight;
			weightSum += weight;
		}
		if( (edges & SOUTHWEST) == SOUTHWEST ) {
			col = texelFetch(pixelArt, ivec2(LRCoords.x-1,LRCoords.y-1),0);
			dist = distance(cellSpaceCoords,vec2(LRCoords.x-1,LRCoords.y-1));
			weight = exp(-(dist*dist)*GAUSS_MULTIPLIER);
			colorSum += col * weight;
			weightSum += weight;
		}
		if( (edges & SOUTH) == SOUTH ) {
			col = texelFetch(pixelArt, ivec2(LRCoords.x,LRCoords.y-1),0);
			dist = distance(cellSpaceCoords,vec2(LRCoords.x,LRCoords.y-1));
			weight = exp(-(dist*dist)*GAUSS_MULTIPLIER);
			colorSum += col * weight;
			weightSum += weight;
		}
		if( (edges & SOUTHEAST) == SOUTHEAST ) {
			col = texelFetch(pixelArt, ivec2(LRCoords.x+1,LRCoords.y-1),0);
			dist = distance(cellSpaceCoords,vec2(LRCoords.x+1,LRCoords.y-1));
			weight = exp(-(dist*dist)*GAUSS_MULTIPLIER);
			colorSum += col * weight;
			weightSum += weight;
		}
	}
	color0 = colorSum/weightSum;
	
}
