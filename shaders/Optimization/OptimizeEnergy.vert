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

//SMOOTHNESS PARAMETERS
#define POSITIONAL_ENERGY_SCALING 2.5

//Golden section search
#define LIMIT_SEARCH_ITERATIONS 20.0 //20
#define R  						0.61803399
#define C  						1 - R
#define TOL 					0.0001
#define BRACKET_SEARCH_A 		0.1 //0.1
#define BRACKET_SEARCH_B  		-0.1 //-0.1
#define GOLD 					1.618034
#define GLIMIT 					10.0 //10
#define TINY 					0.000000001 // prevents division by zero

// Input vertex data
layout(location = 0) in vec2 pos;
layout(location = 1) in ivec4 neighbors;
layout(location = 2) in int flags;

//output vertex data
out vec2 optimizedPos;

uniform ivec2 pixelArtDimensions;
uniform samplerBuffer indexedCellPositions;
uniform isamplerBuffer CPflags;
uniform isamplerBuffer knotNeighbors;
//uniform float offsetAmount;
//uniform int positionalPenalty;
uniform int passNum;

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

float calcPositionalEnergy(vec2 pNew, vec2 pOld) {
	//float dist = distance(pNew,pOld) * POSITIONAL_ENERGY_SCALING;
	//float dist = pow(distance(pNew,pOld), positionalPenalty);
	//float dist = distance(pNew,pOld) * positionalPenalty;
	float dist = 2.5*distance(pNew,pOld);
	return dist*dist*dist*dist;
}

vec2 calcGradient(vec2 node1, vec2 node2, vec2 node3) {
	 return 8 * node2 - 4 * node1 - 4 * node3;
}					

float calcSegmentCurveEnergy(vec2 node1, vec2 node2, vec2 node3) {
	vec2 tmp = node1 - 2 * node2 + node3;
	return tmp.x*tmp.x + tmp.y*tmp.y;
}

vec3 findBracket(vec2 splineNeighbors[2], vec2 gradient) {
	float ulim,u,r,q,fu,dum,qr;
	float ax = BRACKET_SEARCH_A;
	float bx = BRACKET_SEARCH_B;
	vec2 pOpt = pos - gradient * ax;
	float fa = calcSegmentCurveEnergy(splineNeighbors[0],pOpt,splineNeighbors[1]) + calcPositionalEnergy(pOpt, pos);
	pOpt = pos - gradient * bx;
	float fb = calcSegmentCurveEnergy(splineNeighbors[0],pOpt,splineNeighbors[1]) + calcPositionalEnergy(pOpt, pos);
	if(fb > fa) {
		//switch roles of a and b so we can go downhill from a to b
		dum = ax;
		ax = bx;
		bx = dum;
		dum = fb;
		fb = fa;
		fa = dum;
	}
	//first guess for c
	float cx = bx + GOLD * (bx - ax);
	pOpt = pos - gradient * cx;
	float fc = calcSegmentCurveEnergy(splineNeighbors[0],pOpt,splineNeighbors[1]) + calcPositionalEnergy(pOpt, pos);
	//find bracket
	while (fb > fc) {
		r = ( bx - ax ) * ( fb - fc );
		q = ( bx - cx ) * ( fb - fa );
		qr = q-r;
		u = bx - (( bx - cx ) * q - (bx - ax ) * r) / ( 2.0 * sign(qr) * max(abs(qr), TINY) );
		ulim = bx + GLIMIT * ( cx - bx );
		if ( (bx-u) * (u-cx) > 0.0) {
			pOpt = pos - gradient * u;
			fu = calcSegmentCurveEnergy(splineNeighbors[0],pOpt,splineNeighbors[1]) + calcPositionalEnergy(pOpt, pos);
			if(fu < fc) {
				return vec3(bx,u,cx);
			} else if( fu > fb) {
				return vec3(ax,bx,u);
			}
			u = cx + GOLD * (cx - bx);
			pOpt = pos - gradient * u;
			fu = calcSegmentCurveEnergy(splineNeighbors[0],pOpt,splineNeighbors[1]) + calcPositionalEnergy(pOpt, pos);
		} else if ( (cx - u) * (u - ulim) > 0.0 ) {
			pOpt = pos - gradient * u;
			fu = calcSegmentCurveEnergy(splineNeighbors[0],pOpt,splineNeighbors[1]) + calcPositionalEnergy(pOpt, pos);
			if(fu < fc) {
				dum = cx + GOLD * (cx - bx);
				bx = cx;
				cx = u;
				u = dum;
				fb = fc;
				fc = fu;
				pOpt = pos - gradient * u;
				fu = calcSegmentCurveEnergy(splineNeighbors[0],pOpt,splineNeighbors[1]) + calcPositionalEnergy(pOpt, pos);
			}
		} else if ( (u-ulim) * (ulim - cx) >= 0.0 ) {
			u = ulim;
			pOpt = pos - gradient * u;
			fu = calcSegmentCurveEnergy(splineNeighbors[0],pOpt,splineNeighbors[1]) + calcPositionalEnergy(pOpt, pos);
		} else {
			u = cx + GOLD * (cx - bx);
			pOpt = pos - gradient * u;
			fu = calcSegmentCurveEnergy(splineNeighbors[0],pOpt,splineNeighbors[1]) + calcPositionalEnergy(pOpt, pos);
		}
		ax = bx;
		bx = cx;
		cx = u;
		fa = fb;
		fb = fc;
		fc = fu;
	}
	return vec3(ax,bx,cx);
}

vec3 searchOffset( vec2 splineNeighbors[2] ) {
	
	vec2 gradient = calcGradient(splineNeighbors[0],pos,splineNeighbors[1]);
	if(length(gradient) > 0.0) {
		gradient = normalize(gradient);
	} else return vec3(0.0,0.0,0.0);
	
	vec3 bracket = findBracket(splineNeighbors, gradient);
	
	//float R = 0.61803399;
	//float C = 1 - R;
	
	//float tol = 10^-4;
	
	float x0 = bracket.x;
	float x1 = 0;
	float x2 = 0;
	float x3 = bracket.z;
	
	
	if ( abs(bracket.z - bracket.y) > abs(bracket.y - bracket.x) ) {
		x1 = bracket.y;
		x2 = bracket.y + C * (bracket.z - bracket.y);
	} else {
		x1 = bracket.y - C * (bracket.y - bracket.x);
		x2 = bracket.y;
	}
	vec2 pOpt = vec2(0.0,0.0);
	pOpt = pos - gradient * x1;
	float f1 = calcSegmentCurveEnergy(splineNeighbors[0],pOpt,splineNeighbors[1]) + calcPositionalEnergy(pOpt, pos);
	
	pOpt = pos - gradient * x2;
	float f2 = calcSegmentCurveEnergy(splineNeighbors[0],pOpt,splineNeighbors[1]) + calcPositionalEnergy(pOpt, pos);
	int counter = 0;
	float fx;
	while ( abs(x3 - x0) > TOL * ( abs(x1) + abs(x2) ) && (counter < LIMIT_SEARCH_ITERATIONS) ) {
		counter = counter + 1;
		if (f2 < f1) {
			x0 = x1;
			x1 = x2;
			x2 = R * x1 + C * x3;
			pOpt = pos - gradient * x2;
			fx = calcSegmentCurveEnergy(splineNeighbors[0],pOpt,splineNeighbors[1]) + calcPositionalEnergy(pOpt, pos);
			f1 = f2;
			f2 = fx;
		}
		else {
			x3 = x2;
			x2 = x1;
			x1 = R * x2 + C * x0;
			pOpt = pos - gradient * x1;
			fx = calcSegmentCurveEnergy(splineNeighbors[0],pOpt,splineNeighbors[1]) + calcPositionalEnergy(pOpt, pos);
			f2 = f1;
			f1 = fx;
		}
	}
	float offset = 0;
	if( f1 < f2 ) {
		offset = x1;
	} else {
		offset = x2;
	}
	return vec3(gradient,offset);
}

void main(){
	optimizedPos = pos;
	//if( gl_VertexID % 2 == passNum){
	//if( passNum == 1){
	if(flags > 16) {
		if(flags < 512) {
			// vec2 currentPosition = pos;
			// // check if this point lies on a T-Junction and get its corrected Position
			// if ( (flags & HAS_CORRECTED_POSITION) == HAS_CORRECTED_POSITION ) {
				// int index; //this points index
				// if(neighbors.w != -1) { //take western neighbor index for this points index calculation
					// if(neighbors.w%2 == 0) {index = neighbors.w+3;}
					// else {index = neighbors.w+2;}
				// } else {
					// //since we are on a t-junction and there is no western neighbor there must be an eastern one
					// if(neighbors.y%2 == 0) {index = neighbors.y-1;}
					// else {index = neighbors.y-2;}
				// }
				// currentPosition = texelFetch(indexedCellPositions,index).rg;
				// // perVertexColor = vec4(1.0,0.0,0.0,.0);
			// }
			//get neighboring Points
			
			vec2 splineNeighbors[2];
			int splineCount = 0;
			bool splineNoOpt = false;
			vec2 splineNeighborsNeighbors[2] = vec2[2](vec2(0.0,0.0),vec2(0.0,0.0));
			bool neighborHasNeighbor[2] = bool[2](false,false);
			
			//spline neighbor indicaors
			bool hasNorthernSpline = (flags & HAS_NORTHERN_SPLINE) == HAS_NORTHERN_SPLINE;
			bool hasEasternSpline = (flags & HAS_EASTERN_SPLINE) == HAS_EASTERN_SPLINE;
			bool hasSouthernSpline = (flags & HAS_SOUTHERN_SPLINE) == HAS_SOUTHERN_SPLINE;
			bool hasWesternSpline = (flags & HAS_WESTERN_SPLINE) == HAS_WESTERN_SPLINE;
			
			if( hasNorthernSpline ) {
				int neighborflags = texelFetch(CPflags,neighbors.x).r;
				if( ((flags & DONT_OPTIMIZE_N) == DONT_OPTIMIZE_N) 
					|| ((neighborflags & DONT_OPTIMIZE_S) == DONT_OPTIMIZE_S)) {
					splineNoOpt = true;
				} 
				if(!splineNoOpt) {
					if( ((neighborflags & HAS_CORRECTED_POSITION) == HAS_CORRECTED_POSITION) 
						&& !( (neighborflags & HAS_SOUTHERN_SPLINE) == HAS_SOUTHERN_SPLINE )) {
						//our neighbour is an endpoint on a t-junction and needs to be adjusted
						splineNeighbors[splineCount] = texelFetch(indexedCellPositions,neighbors.x+1).rg;
						splineCount++;
					} else {
						splineNeighbors[splineCount] = texelFetch(indexedCellPositions,neighbors.x).rg;
						
						//see if the northern point has a neighbor - this one would be influenced by the optimization
						if( (neighborflags & HAS_NORTHERN_SPLINE) == HAS_NORTHERN_SPLINE) {
							//get neighbors neighbors flag
							int neighborsNeighborIndex = getNeighborIndex(neighbors.x, NORTH, 0);
							int neighborsNeighborflags = texelFetch(CPflags,neighborsNeighborIndex).r;
							if( ((neighborsNeighborflags & HAS_SOUTHERN_SPLINE) == HAS_SOUTHERN_SPLINE) 
								&& ((neighborsNeighborflags & DONT_OPTIMIZE_S) == 0) ) {
								//take this one
								splineNeighborsNeighbors[splineCount] = texelFetch(indexedCellPositions,neighborsNeighborIndex).rg;
								neighborHasNeighbor[splineCount]=true;
							} 
							// else {( (neighborsNeighborflags & HAS_CORRECTED_POSITION) == HAS_CORRECTED_POSITION) )
								// //use the corrected position
								// splineNeighborsNeighbors[splineCount] = texelFetch(indexedCellPositions,neighborsNeighborIndex+1).rg;
								// neighborHasNeighbor[splineCount]=true;
							// }
						} else if((neighborflags & HAS_EASTERN_SPLINE) == HAS_EASTERN_SPLINE) {
							int neighborsNeighborIndex = getNeighborIndex(neighbors.x, EAST, 0);
							int neighborsNeighborflags = texelFetch(CPflags,neighborsNeighborIndex).r;
							if( ((neighborsNeighborflags & HAS_WESTERN_SPLINE) == HAS_WESTERN_SPLINE) 
								&& ((neighborsNeighborflags & DONT_OPTIMIZE_W) == 0) )
								{
								//take this one
								splineNeighborsNeighbors[splineCount] = texelFetch(indexedCellPositions,neighborsNeighborIndex).rg;
								neighborHasNeighbor[splineCount]=true;
							} 
							// else {
								// //use the corrected position
								// splineNeighborsNeighbors[splineCount] = texelFetch(indexedCellPositions,neighborsNeighborIndex+1).rg;
								// neighborHasNeighbor[splineCount]=true;
							// }
						} else if((neighborflags & HAS_WESTERN_SPLINE) == HAS_WESTERN_SPLINE) {
							int neighborsNeighborIndex = getNeighborIndex(neighbors.x, WEST, 0);
							int neighborsNeighborflags = texelFetch(CPflags,neighborsNeighborIndex).r;
							if( ((neighborsNeighborflags & HAS_EASTERN_SPLINE) == HAS_EASTERN_SPLINE) 
								&& ((neighborsNeighborflags & DONT_OPTIMIZE_E) == 0) )
								{
								//take this one
								splineNeighborsNeighbors[splineCount] = texelFetch(indexedCellPositions,neighborsNeighborIndex).rg;
								neighborHasNeighbor[splineCount]=true;
							} 
							// else {
								// //use the corrected position
								// splineNeighborsNeighbors[splineCount] = texelFetch(indexedCellPositions,neighborsNeighborIndex+1).rg;
								// neighborHasNeighbor[splineCount]=true;
							// }
						}
						splineCount++;
					}
				}
			}
			if( hasEasternSpline) {
				int neighborflags = texelFetch(CPflags,neighbors.y).r;
				if( ((flags & DONT_OPTIMIZE_E) == DONT_OPTIMIZE_E ) || ((neighborflags & DONT_OPTIMIZE_W) == DONT_OPTIMIZE_W)) {
					splineNoOpt = true;
				} 
				if(!splineNoOpt) {
					if( ((neighborflags & HAS_CORRECTED_POSITION) == HAS_CORRECTED_POSITION)
						&& !( (neighborflags & HAS_WESTERN_SPLINE) == HAS_WESTERN_SPLINE )) {
						//our neighbour is an endpoint on a t-junction and needs to be adjusted
						splineNeighbors[splineCount] = texelFetch(indexedCellPositions,neighbors.y+1).rg;
						splineCount++;
					} else {
						splineNeighbors[splineCount] = texelFetch(indexedCellPositions,neighbors.y).rg;
						
						if( (neighborflags & HAS_NORTHERN_SPLINE) == HAS_NORTHERN_SPLINE) {
							//get neighbors neighbors flag
							int neighborsNeighborIndex = getNeighborIndex(neighbors.y, NORTH, 0);
							int neighborsNeighborflags = texelFetch(CPflags,neighborsNeighborIndex).r;
							if( ((neighborsNeighborflags & HAS_SOUTHERN_SPLINE) == HAS_SOUTHERN_SPLINE) 
								&& ((neighborsNeighborflags & DONT_OPTIMIZE_S) == 0) )
								{
								//take this one
								splineNeighborsNeighbors[splineCount] = texelFetch(indexedCellPositions,neighborsNeighborIndex).rg;
								neighborHasNeighbor[splineCount]=true;
							} 
							// else {
								// //use the corrected position
								// splineNeighborsNeighbors[splineCount] = texelFetch(indexedCellPositions,neighborsNeighborIndex+1).rg;
								// neighborHasNeighbor[splineCount]=true;
							// }
						} else if((neighborflags & HAS_EASTERN_SPLINE) == HAS_EASTERN_SPLINE) {
							int neighborsNeighborIndex = getNeighborIndex(neighbors.y, EAST, 0);
							int neighborsNeighborflags = texelFetch(CPflags,neighborsNeighborIndex).r;
							if( ((neighborsNeighborflags & HAS_WESTERN_SPLINE) == HAS_WESTERN_SPLINE) 
								&& ((neighborsNeighborflags & DONT_OPTIMIZE_W) == 0) )
								{
								//take this one
								splineNeighborsNeighbors[splineCount] = texelFetch(indexedCellPositions,neighborsNeighborIndex).rg;
								neighborHasNeighbor[splineCount]=true;
							} 
							// else {
								// //use the corrected position
								// splineNeighborsNeighbors[splineCount] = texelFetch(indexedCellPositions,neighborsNeighborIndex+1).rg;
								// neighborHasNeighbor[splineCount]=true;
							// }
						} else if((neighborflags & HAS_SOUTHERN_SPLINE) == HAS_SOUTHERN_SPLINE) {
							int neighborsNeighborIndex = getNeighborIndex(neighbors.y, SOUTH, 0);
							int neighborsNeighborflags = texelFetch(CPflags,neighborsNeighborIndex).r;
							if( ((neighborsNeighborflags & HAS_NORTHERN_SPLINE) == HAS_NORTHERN_SPLINE) 
								&& ((neighborsNeighborflags & DONT_OPTIMIZE_N) == 0) )
								{
								//take this one
								splineNeighborsNeighbors[splineCount] = texelFetch(indexedCellPositions,neighborsNeighborIndex).rg;
								neighborHasNeighbor[splineCount]=true;
							} 
							// else {
								// //use the corrected position
								// splineNeighborsNeighbors[splineCount] = texelFetch(indexedCellPositions,neighborsNeighborIndex+1).rg;
								// neighborHasNeighbor[splineCount]=true;
							// }
						}
						splineCount++;
					}
				}
			}
			if( hasSouthernSpline ) {
				int neighborflags = texelFetch(CPflags,neighbors.z).r;
				if( ((flags & DONT_OPTIMIZE_S) == DONT_OPTIMIZE_S ) || ((neighborflags & DONT_OPTIMIZE_N) == DONT_OPTIMIZE_N)) {
					splineNoOpt = true;
				} 
				if(!splineNoOpt) {
					if( ((neighborflags & HAS_CORRECTED_POSITION) == HAS_CORRECTED_POSITION) 
						&& !( (neighborflags & HAS_NORTHERN_SPLINE) == HAS_NORTHERN_SPLINE )) {
						//our neighbour is an endpoint on a t-junction and needs to be adjusted
						splineNeighbors[splineCount] = texelFetch(indexedCellPositions,neighbors.z+1).rg;
						splineCount++;
					} else {
						splineNeighbors[splineCount] = texelFetch(indexedCellPositions,neighbors.z).rg;
						
						if( (neighborflags & HAS_WESTERN_SPLINE) == HAS_WESTERN_SPLINE) {
							//get neighbors neighbors flag
							int neighborsNeighborIndex = getNeighborIndex(neighbors.z, WEST, 0);
							int neighborsNeighborflags = texelFetch(CPflags,neighborsNeighborIndex).r;
							if( ((neighborsNeighborflags & HAS_EASTERN_SPLINE) == HAS_EASTERN_SPLINE) 
								&& ((neighborsNeighborflags & DONT_OPTIMIZE_E) == 0) )
								{
								//take this one
								splineNeighborsNeighbors[splineCount] = texelFetch(indexedCellPositions,neighborsNeighborIndex).rg;
								neighborHasNeighbor[splineCount]=true;
							} 
							// else {
								// //use the corrected position
								// splineNeighborsNeighbors[splineCount] = texelFetch(indexedCellPositions,neighborsNeighborIndex+1).rg;
								// neighborHasNeighbor[splineCount]=true;
							// }
						} else if((neighborflags & HAS_EASTERN_SPLINE) == HAS_EASTERN_SPLINE) {
							int neighborsNeighborIndex = getNeighborIndex(neighbors.z, EAST, 0);
							int neighborsNeighborflags = texelFetch(CPflags,neighborsNeighborIndex).r;
							if( ((neighborsNeighborflags & HAS_WESTERN_SPLINE) == HAS_WESTERN_SPLINE) 
								&& ((neighborsNeighborflags & DONT_OPTIMIZE_W) == 0) ) {
								//take this one
								splineNeighborsNeighbors[splineCount] = texelFetch(indexedCellPositions,neighborsNeighborIndex).rg;
								neighborHasNeighbor[splineCount]=true;
							} 
							// else {
								// //use the corrected position
								// splineNeighborsNeighbors[splineCount] = texelFetch(indexedCellPositions,neighborsNeighborIndex+1).rg;
								// neighborHasNeighbor[splineCount]=true;
							// }
						} else if((neighborflags & HAS_SOUTHERN_SPLINE) == HAS_SOUTHERN_SPLINE) {
							int neighborsNeighborIndex = getNeighborIndex(neighbors.z, SOUTH, 0);
							int neighborsNeighborflags = texelFetch(CPflags,neighborsNeighborIndex).r;
							if (((neighborsNeighborflags & HAS_NORTHERN_SPLINE) == HAS_NORTHERN_SPLINE) 
								&& ((neighborsNeighborflags & DONT_OPTIMIZE_N) == 0) ) {
								//take this one
								splineNeighborsNeighbors[splineCount] = texelFetch(indexedCellPositions,neighborsNeighborIndex).rg;
								neighborHasNeighbor[splineCount]=true;
							} 
								// else {
								// //use the corrected position
								// splineNeighborsNeighbors[splineCount] = texelFetch(indexedCellPositions,neighborsNeighborIndex+1).rg;
								// neighborHasNeighbor[splineCount]=true;
								// }
							
						}
						splineCount++;
					}
				}
			}
			if( hasWesternSpline ) {
				int neighborflags = texelFetch(CPflags,neighbors.w).r;
				if( ((flags & DONT_OPTIMIZE_W) == DONT_OPTIMIZE_W ) || ((neighborflags & DONT_OPTIMIZE_E) == DONT_OPTIMIZE_E)) {
					splineNoOpt = true;
				} 
				if(!splineNoOpt) {
					if( ((neighborflags & HAS_CORRECTED_POSITION) == HAS_CORRECTED_POSITION) 
						&& !( (neighborflags & HAS_EASTERN_SPLINE) == HAS_EASTERN_SPLINE )) {
						//our neighbour is an endpoint on a t-junction and needs to be adjusted
						splineNeighbors[splineCount] = texelFetch(indexedCellPositions,neighbors.w+1).rg;
						splineCount++;
					} else {
						splineNeighbors[splineCount] = texelFetch(indexedCellPositions,neighbors.w).rg;
						
						if( (neighborflags & HAS_NORTHERN_SPLINE) == HAS_NORTHERN_SPLINE) {
							//get neighbors neighbors flag
							int neighborsNeighborIndex = getNeighborIndex(neighbors.w, NORTH, 0);
							int neighborsNeighborflags = texelFetch(CPflags,neighborsNeighborIndex).r;
							if( ((neighborsNeighborflags & HAS_SOUTHERN_SPLINE) == HAS_SOUTHERN_SPLINE) 
								&& ((neighborsNeighborflags & DONT_OPTIMIZE_S) == 0) )
								{
								//take this one
								splineNeighborsNeighbors[splineCount] = texelFetch(indexedCellPositions,neighborsNeighborIndex).rg;
								neighborHasNeighbor[splineCount]=true;
							} 
							// else {
								// //use the corrected position
								// splineNeighborsNeighbors[splineCount] = texelFetch(indexedCellPositions,neighborsNeighborIndex+1).rg;
								// neighborHasNeighbor[splineCount]=true;
							// }
						} else if((neighborflags & HAS_WESTERN_SPLINE) == HAS_WESTERN_SPLINE) {
							int neighborsNeighborIndex = getNeighborIndex(neighbors.w, WEST, 0);
							int neighborsNeighborflags = texelFetch(CPflags,neighborsNeighborIndex).r;
							if( ((neighborsNeighborflags & HAS_EASTERN_SPLINE) == HAS_EASTERN_SPLINE) 
								&& ((neighborsNeighborflags & DONT_OPTIMIZE_E) == 0) )
								{
								//take this one
								splineNeighborsNeighbors[splineCount] = texelFetch(indexedCellPositions,neighborsNeighborIndex).rg;
								neighborHasNeighbor[splineCount]=true;
							} 
							// else {
								// //use the corrected position
								// splineNeighborsNeighbors[splineCount] = texelFetch(indexedCellPositions,neighborsNeighborIndex+1).rg;
								// neighborHasNeighbor[splineCount]=true;
							// }
						} else if((neighborflags & HAS_SOUTHERN_SPLINE) == HAS_SOUTHERN_SPLINE) {
							int neighborsNeighborIndex = getNeighborIndex(neighbors.w, SOUTH, 0);
							int neighborsNeighborflags = texelFetch(CPflags,neighborsNeighborIndex).r;
							if( ((neighborsNeighborflags & HAS_NORTHERN_SPLINE) == HAS_NORTHERN_SPLINE) 
								&& ((neighborsNeighborflags & DONT_OPTIMIZE_N) == 0) )
								{
								//take this one
								splineNeighborsNeighbors[splineCount] = texelFetch(indexedCellPositions,neighborsNeighborIndex).rg;
								neighborHasNeighbor[splineCount]=true;
							} 
							// else {
								// //use the corrected position
								// splineNeighborsNeighbors[splineCount] = texelFetch(indexedCellPositions,neighborsNeighborIndex+1).rg;
								// neighborHasNeighbor[splineCount]=true;
							// }
						}
						splineCount++;
					}
				}
			}
			
			if( (splineCount == 2) && (!splineNoOpt)) {
				
				vec3 shift = searchOffset(splineNeighbors);
				//if( shift.x > 0.0 || shift.y > 0.0) {
					//if( shift.z != 0.0) {
						optimizedPos = pos - shift.xy*shift.z;
					//}
				//}
				//offset positions
				// int offsetCount = 0;
				// vec2 offsetPos[2];
				// if( hasNorthernSpline && hasEasternSpline ) {
					// offsetPos[0] = pos + vec2(offsetAmount,offsetAmount);
					// offsetCount = 1;
				// }
				// else if( hasNorthernSpline && hasSouthernSpline ) {
					// offsetPos[0] = pos + vec2(offsetAmount,0.0);
					// offsetPos[1] = pos + vec2(-offsetAmount,0.0);
					// offsetCount = 2;
				// }
				// else if( hasNorthernSpline && hasWesternSpline ) {
					// offsetPos[0] = pos + vec2(-offsetAmount,offsetAmount);
					// offsetCount = 1;
				// }
				// else if( hasEasternSpline && hasSouthernSpline ) {
					// offsetPos[0] = pos + vec2(offsetAmount,-offsetAmount);
					// offsetCount = 1;
				// }
				// else if( hasEasternSpline && hasWesternSpline ) {
					// offsetPos[0] = pos + vec2(0.0,-offsetAmount);
					// offsetPos[1] = pos + vec2(0.0,offsetAmount);
					// offsetCount = 2;
				// }
				// else if( hasSouthernSpline && hasWesternSpline ) {
					// offsetPos[0] = pos + vec2(-offsetAmount,-offsetAmount);
					// offsetCount = 1;
				// }
				// float eMin = calcCurveEnergy(splineNeighborsNeighbors[0],splineNeighbors[0],pos,splineNeighbors[1],splineNeighborsNeighbors[1], neighborHasNeighbor[0], neighborHasNeighbor[1]);
				
				// for(int i = 0; i < offsetCount ; i++) {
					// float currentEnergy = calcPositionalEnergy(offsetPos[i], pos) + calcCurveEnergy(splineNeighborsNeighbors[0],splineNeighbors[0],offsetPos[i],splineNeighbors[1],splineNeighborsNeighbors[1], neighborHasNeighbor[0], neighborHasNeighbor[1]);
					
					// if(!isinf(currentEnergy)) {
						// if(currentEnergy < eMin) {
							// eMin = currentEnergy;
							// //optimizedPos = p[i];
							// optimizedPos = offsetPos[i];
						// }
					// }
				// }
			}
			//optimizedPos = pos + vec2(-offsetAmount,-offsetAmount);
		}
	}
}
