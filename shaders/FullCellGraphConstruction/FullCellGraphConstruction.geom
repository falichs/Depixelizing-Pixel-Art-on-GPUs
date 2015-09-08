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

/** 
 * @file FullCellGraphConstruction.geom
 * Initializes B-Spline control points and decides on which spline or line segments they lie.
 * TODO: more detail, especially on the output array datastructure.
 */

/**
* @def EDGE_HORVERT 
* this macro represents a horizontal or vertical edge connecting two adjacent pixels.
*/
#define EDGE_HORVERT 16

/**
* @def EDGE_DIAGONAL_ULLR
* this macro represents a diagonal edge connecting two diagonally adjacent pixels.
*/
#define EDGE_DIAGONAL_ULLR 32

/**
* @def EDGE_DIAGONAL_LLUR
* this macro represents a diagonal edge connecting two diagonally adjacent pixels.
*/
#define EDGE_DIAGONAL_LLUR 64

//Neighborhood Flags

/**
* @def HAS_NORTHERN_NEIGHBOR
* We use this control-point flag to define neighborhood connectivity. 
*/
#define HAS_NORTHERN_NEIGHBOR 1

/**
* @def HAS_EASTERN_NEIGHBOR
* We use this control-point flag to define neighborhood connectivity.
*/
#define HAS_EASTERN_NEIGHBOR 2

/**
* @def HAS_SOUTHERN_NEIGHBOR
* We use this control-point flag to define neighborhood connectivity.
*/
#define HAS_SOUTHERN_NEIGHBOR 4

/**
* @def HAS_WESTERN_NEIGHBOR
* We use this control-point flag to define neighborhood connectivity.
*/
#define HAS_WESTERN_NEIGHBOR 8

/**
* @def HAS_NORTHERN_SPLINE
* We use this control-point flag to define if it lies on a spline curve connecting a certain neighbor.
*/
#define HAS_NORTHERN_SPLINE 16

/**
* @def HAS_EASTERN_SPLINE
* We use this control-point flag to define if it lies on a spline curve connecting a certain neighbor.
*/
#define HAS_EASTERN_SPLINE 32

/**
* @def HAS_SOUTHERN_SPLINE
* We use this control-point flag to define if it lies on a spline curve connecting a certain neighbor.
*/
#define HAS_SOUTHERN_SPLINE 64

/**
* @def HAS_WESTERN_SPLINE
* We use this control-point flag to define if it lies on a spline curve connecting a certain neighbor.
*/
#define HAS_WESTERN_SPLINE 128

/**
* @def HAS_CORRECTED_POSITION
* We use this control-point flag to define if it has an additional corrected position, because it lies on a T-junction.
*/
#define HAS_CORRECTED_POSITION 256

/**
* @def DONT_OPTIMIZE_N
* We use this control-point flag to tell the optimizer not to optimize curve energy of this segment.
*/
#define DONT_OPTIMIZE_N 512

/**
* @def DONT_OPTIMIZE_E
* We use this control-point flag to tell the optimizer not to optimize curve energy of this segment.
*/
#define DONT_OPTIMIZE_E 1024

/**
* @def DONT_OPTIMIZE_S
* We use this control-point flag to tell the optimizer not to optimize curve energy of this segment.
*/
#define DONT_OPTIMIZE_S 2048

/**
* @def DONT_OPTIMIZE_W
* We use this control-point flag to tell the optimizer not to optimize curve energy of this segment.
*/
#define DONT_OPTIMIZE_W 4096

//Directions

/**
* @def NORTH
* We use this macro to address an adjacent neighbor in the positions array.
*/
#define NORTH		1

/**
* @def EAST
* We use this macro to address an adjacent neighbor in the positions array.
*/
#define EAST		2

/**
* @def SOUTH
* We use this macro to address an adjacent neighbor in the positions array.
*/
#define SOUTH		4

/**
* @def WEST
* We use this macro to address an adjacent neighbor in the positions array.
*/
#define WEST		8

/**
* @def CENTER
* We use this macro to address an adjacent neighbor in the positions array.
*/
#define CENTER		16

//central offsets
#define XOFFSET_CUL	-0.25
#define YOFFSET_CUL	0.25
#define XOFFSET_CUR	0.25
#define YOFFSET_CUR	0.25
#define XOFFSET_CLL	-0.25
#define YOFFSET_CLL	-0.25
#define XOFFSET_CLR	0.25
#define YOFFSET_CLR	-0.25

layout (points) in;
layout (points, max_vertices = 2) out;
out vec2 pos;
out ivec4 neighbors; // N,E,S,W
out int flags;
out ivec4 nNcolors; // pixel color index coded L.x,L.y,R.x,R.y 
out ivec4 nEcolors;
out ivec4 nScolors;
out ivec4 nWcolors;

uniform sampler2D similarityGraph; /**< texture containing the source similarity graph datastructure. @see dissimilar.frag */
uniform sampler2D pixelArt; /**< texture containing the source image. */

/**
* @fn bool isContour(ivec4 LRcolors)
* checks wheather two colors are similar or not. use for contour spline detection.
* @param pixelA first pixel color
* @return TRUE if the colors are considered to be similar
*/
bool isContour(ivec4 LRcolors) {
	vec4 pL = texelFetch(pixelArt, LRcolors.xy ,0);
	vec4 pR = texelFetch(pixelArt, LRcolors.zw ,0);
	float yA = 0.299*pL.r + 0.587*pL.g + 0.114*pL.b;
	float uA = 0.493*(pL.b-yA);
	float vA = 0.877*(pL.r-yA);
	float yB = 0.299*pR.r + 0.587*pR.g + 0.114*pR.b;
	float uB = 0.493*(pR.b-yB);
	float vB = 0.877*(pR.r-yB);
	
	bool isContour = false;
	// if( abs(yA-yB) > 100.0/255.0) {
		// if( abs(uA-uB) > 100.0/255.0) {
			// if( abs(vA-vB) > 100.0/255.0) {
				// isContour=true;
			// }
		// }
	// }
	if( distance(vec3(yA,uA,vA),vec3(yB,uB,vB)) > 100.0/255.0) {
		isContour=true;
	}
	return isContour;
}

/**
* @fn void emitPoint(vec2 vpos, ivec4 vneighbors, int vflags, ivec4 vnNcolors, ivec4 vnEcolors, ivec4 vnScolors, ivec4 vnWcolors)
* Writes control point to output stream.
* @param vpos CP position
* @param vneighbors CP neighbourhood flag
* @param vflags CP flags
* @param vnNcolors CP norther neighbor color
* @param vnEcolors CP eastern neighbor color
* @param vnScolors CP southern neighbor color
* @param vnWcolors CP western neighbor color
*/
void emitPoint(vec2 vpos, ivec4 vneighbors, int vflags, ivec4 vnNcolors, ivec4 vnEcolors, ivec4 vnScolors, ivec4 vnWcolors) {
	pos = vpos;
	//index = -1;
	neighbors = vneighbors;
	flags = vflags;
	nNcolors = vnNcolors;
	nEcolors = vnEcolors;
	nScolors = vnScolors;
	nWcolors = vnWcolors;
	//valence = vvalence;
	EmitVertex();
	EndPrimitive();
}
/**
* @fn int getNeighborIndex(int dir, int targetSector)
* calculates the index to adress the neighbor.
* @param dir direction of 2x2 cell containing the neighbor
* @param targetSector must be 0 or 1
*/
int getNeighborIndex(int dir, int targetSector) {
	int index = -1;
	ivec2 centralPos = ivec2(gl_in[0].gl_Position.xy); //centralPos ... current 2x2 cell x,y position (from 0,0 to textureSize(pixelArt)-1).
	int dy = textureSize(pixelArt,0).x-1;

	if( (dir & NORTH) == NORTH) {
		index = ( (centralPos.y+1) * dy + centralPos.x ) * 2 + targetSector;
	} else if( (dir & EAST) == EAST) {
		index = ( centralPos.y * dy + centralPos.x + 1 ) * 2 + targetSector;
	} else if( (dir & SOUTH) == SOUTH) {
		index = ( (centralPos.y-1) * dy + centralPos.x ) * 2 + targetSector;
	} else if( (dir & WEST) == WEST) {
		index = ( centralPos.y * dy + centralPos.x - 1 ) * 2 + targetSector;
	} else if( (dir & CENTER) == CENTER) {
		index = ( centralPos.y * dy + centralPos.x) * 2 + targetSector;
	}
	return index;
}
/**
* @fn vec2 calcAdjustedPoint(vec2 p0, vec2 p1, vec2 p2)
* intepolates the central point lying on the segment
* @param p0 1st point on B-Spline
* @param p1 2nd point on B-Spline
* @param p2 3rd point on B-Spline
* @return central point on segment
*/
vec2 calcAdjustedPoint(vec2 p0, vec2 p1, vec2 p2) {
	return 0.125*p0 + 0.75*p1 + 0.125*p2;
}

/**
* @fn bool checkForCorner(vec2 spline1, vec2 spline2)
* decides wheater two adjacent segments should be treated as a corner or a curve.
* @param spline1 1st point on B-Spline
* @param spline2 2nd point on B-Spline
* @return central point on segment
*/
bool checkForCorner(vec2 spline1, vec2 spline2) {
	//calculate inner angle
	float dp = dot( normalize(spline1), normalize(spline2) );
	//there seems to be a numerical problem when using normalize & dot product
	//angles:
	// -0.7071
	// -0.3162
	// 0
	if( dp > -0.7072 && dp < -0.7070 ){
		return true;
	} else if( dp > -0.3163 && dp < -0.3161 ) {
		return true;
	} else if (dp > -0.0001 && dp < 0.0001) {
		return true;
	}
	return false;
}

/**
* @fn void main()
* uses a 3x3 pixel neighborhood to define initial control points.
* before they get written to the buffer each control point gets a set of attributes: POSITION, FLAGS and NEIGHBORHOOD info.
*/
void main() {
	//each time this shader is issued it samples the nodes between 2x2 corresponding nodes in the similaritygraph
	//the double vertex position corresponds to a "diagonal"-pixel in the similarity graph
	//at first we fetch the surrounding edges (N,W,S,E of the pixel) and the diagonal itself from the similaritygraph
	ivec2 simGraphDiagonalCoords = ivec2(gl_in[0].gl_Position.xy*2.0)+2;
	int eCenter = int(texelFetch(similarityGraph,simGraphDiagonalCoords,0).r*255);
	int eNorth = int(texelFetch(similarityGraph,ivec2(simGraphDiagonalCoords.x, simGraphDiagonalCoords.y+1),0).r*255.0);
	int eNorthCenter = int(texelFetch(similarityGraph,ivec2(simGraphDiagonalCoords.x, simGraphDiagonalCoords.y+2),0).r*255);
	int eEast   = int(texelFetch(similarityGraph,ivec2(simGraphDiagonalCoords.x+1, simGraphDiagonalCoords.y),0).r*255.0);
	int eEastCenter = int(texelFetch(similarityGraph,ivec2(simGraphDiagonalCoords.x+2, simGraphDiagonalCoords.y),0).r*255);
	int eSouth  = int(texelFetch(similarityGraph,ivec2(simGraphDiagonalCoords.x, simGraphDiagonalCoords.y-1),0).r*255.0);
	int eSouthCenter = int(texelFetch(similarityGraph,ivec2(simGraphDiagonalCoords.x, simGraphDiagonalCoords.y-2),0).r*255);
	int eWest   = int(texelFetch(similarityGraph,ivec2(simGraphDiagonalCoords.x-1, simGraphDiagonalCoords.y),0).r*255.0);
	int eWestCenter = int(texelFetch(similarityGraph,ivec2(simGraphDiagonalCoords.x-2, simGraphDiagonalCoords.y),0).r*255);
	
	//init vertexdata
		//int v0_valence = 0;
		vec2 v0_pos = vec2(-1.0,-1.0);
		//int v0_index = -1;
		ivec4 v0_neighbors = ivec4(-1,-1,-1,-1);
		int v0_flags = 0;
		ivec4 v0_nNcolors = ivec4(-1,-1,-1,-1);
		ivec4 v0_nEcolors = ivec4(-1,-1,-1,-1);
		ivec4 v0_nScolors = ivec4(-1,-1,-1,-1);
		ivec4 v0_nWcolors = ivec4(-1,-1,-1,-1);
		// bool dontOptimizeN = false;
		// bool dontOptimizeE = false;
		// bool dontOptimizeS = false;
		// bool dontOptimizeW = false;

		int v1_valence = 0;
		vec2 v1_pos = vec2(-1.0,-1.0);
		//int v1_index = -1;
		ivec4 v1_neighbors = ivec4(-1,-1,-1,-1);
		int v1_flags = 0;
		ivec4 v1_nNcolors = ivec4(-1,-1,-1,-1);
		ivec4 v1_nEcolors = ivec4(-1,-1,-1,-1);
		ivec4 v1_nScolors = ivec4(-1,-1,-1,-1);
		ivec4 v1_nWcolors = ivec4(-1,-1,-1,-1);
	//corner detection (TODO: make this more convenient)
		bool ignoreN=false;
		bool ignoreE=false;
		bool ignoreS=false;
		bool ignoreW=false;
		if(gl_in[0].gl_Position.y > textureSize(pixelArt,0).y-3) ignoreN = true;//something is weired about the -3
		if(gl_in[0].gl_Position.x > textureSize(pixelArt,0).x-3) ignoreE = true;//something is weired about the -3
		if(gl_in[0].gl_Position.y < 1.0) ignoreS = true;
		if(gl_in[0].gl_Position.x < 1.0) ignoreW = true;

	bool neighborsFound = false;
	bool nNeighborsFound = false;
	bool wNeighborsFound = false;
	bool sNeighborsFound = false;
	bool eNeighborsFound = false;
	int neighborCount = 0;
	int nNeighborIndex = - 1;
	int wNeighborIndex = - 1;
	int sNeighborIndex = - 1;
	int eNeighborIndex = - 1;

	//we need these for solving the t-junction position adjustment issues
	vec2 nVector = vec2(0.0,0.0);
	vec2 eVector = vec2(0.0,0.0);
	vec2 sVector = vec2(0.0,0.0);
	vec2 wVector = vec2(0.0,0.0);
	
	//browse neighborhood

	//if (!ignoreN ) {
		//if(eNorth == 0) {
	if (!ignoreN && eNorth == 0) {
		nNeighborsFound = true;
		neighborsFound = true;
		neighborCount++;
		//}
		//investigate northern neighborhood
		//the northern neighbor depends on the northern diagonal
		if (eNorthCenter == EDGE_DIAGONAL_ULLR) {
			nNeighborIndex = getNeighborIndex( NORTH, 0);
			nVector = vec2(-0.25,0.75);
		} else if (eNorthCenter == EDGE_DIAGONAL_LLUR) {
			nNeighborIndex = getNeighborIndex( NORTH, 1);
			nVector = vec2(0.25,0.75);
		} else {
			nNeighborIndex = getNeighborIndex( NORTH, 0);
			nVector = vec2(0.0,1.0);
		}
	} 
	//is there a western edge?
	//if (!ignoreW) {
		//if(eWest == 0) {
	if (!ignoreW && eWest == 0) {
		wNeighborsFound = true;
		neighborsFound = true;
		neighborCount++;
		//}
		//investigate western neighborhood
		//the western neighbor depends on the western diagonal
		if (eWestCenter == EDGE_DIAGONAL_ULLR) {
			wNeighborIndex = getNeighborIndex( WEST, 1);
			wVector = vec2(-0.75, 0.25);
		} else if (eWestCenter == EDGE_DIAGONAL_LLUR) {
			wNeighborIndex = getNeighborIndex( WEST, 1);
			wVector = vec2(-0.75, -0.25);
		} else {
			wNeighborIndex = getNeighborIndex( WEST, 0);
			wVector = vec2(-1.0,0.0);
		}
	}
	//is there a southern edge?
	//if (!ignoreS) {
		//if(eSouth == 0) {
	if (!ignoreS && eSouth == 0) {
		sNeighborsFound = true;
		neighborsFound = true;
		neighborCount++;
		//}
		//investigate southern neighborhood
		//the southern neighbor depends on the southern diagonal
		if (eSouthCenter == EDGE_DIAGONAL_ULLR) {
			sNeighborIndex = getNeighborIndex( SOUTH, 1);
			sVector = vec2(0.25,-0.75);
		} else if (eSouthCenter == EDGE_DIAGONAL_LLUR) {
			sNeighborIndex = getNeighborIndex( SOUTH, 0);
			sVector = vec2(-0.25,-0.75);
		} else {
			sNeighborIndex = getNeighborIndex( SOUTH, 0);
			sVector = vec2(0.0,-1.0);
		}
	}
	//is there a eastern edge?
	//if (!ignoreE) {
		//if(eEast == 0) {
	if (!ignoreE && eEast == 0) {
		eNeighborsFound = true;
		neighborsFound = true;
		neighborCount++;
		//}
		//investigate eastern neighborhood
		//the eastern neighbor depends on the eastern diagonal
		if (eEastCenter == EDGE_DIAGONAL_ULLR) {
			eNeighborIndex = getNeighborIndex( EAST, 0);
			eVector = vec2(0.75,-0.25);
		} else if (eEastCenter == EDGE_DIAGONAL_LLUR) {
			eNeighborIndex = getNeighborIndex( EAST, 0);
			eVector = vec2(0.75, 0.25);
		} else {
			eNeighborIndex = getNeighborIndex( EAST, 0);
			eVector = vec2(1.0, 0.0);
		}
	}
	
	

	if (neighborsFound) {

		//calculate the color indices
		ivec2 LLPixelColorIndex = ivec2(gl_in[0].gl_Position.xy);
		ivec2 ULPixelColorIndex = LLPixelColorIndex + ivec2(0,1);
		ivec2 LRPixelColorIndex = LLPixelColorIndex + ivec2(1,0);
		ivec2 URPixelColorIndex = LLPixelColorIndex + 1;

		//gather vertexdata
		vec2 centerPos = gl_in[0].gl_Position.xy + 0.5; //The "world space" position of the 2x2 center 
		// we now check if the central 2x2 block is "split by a diagonal"
		if (eCenter == EDGE_DIAGONAL_ULLR) {
				bool twoNeighbors = true;
				//create the Vertex in the first sector -v0
				v0_pos = centerPos + vec2(XOFFSET_CLL,YOFFSET_CLL);
				//index = getNeighborIndex(CENTER, 0);
				int sIndex = sNeighborIndex; 
				int wIndex = wNeighborIndex;
				v0_nScolors = ivec4(LRPixelColorIndex,LLPixelColorIndex);
				v0_nWcolors = ivec4(LLPixelColorIndex,ULPixelColorIndex);
				if(sNeighborsFound) { v0_flags = HAS_SOUTHERN_NEIGHBOR | HAS_SOUTHERN_SPLINE; 
					// if(dontOptimizeS){v0_flags = v0_flags | DONT_OPTIMIZE_S;} 
				} else twoNeighbors = false;
				if(wNeighborsFound) { v0_flags = v0_flags | HAS_WESTERN_NEIGHBOR | HAS_WESTERN_SPLINE;  
					// if(dontOptimizeW){v0_flags = v0_flags | DONT_OPTIMIZE_W;} 
				} else twoNeighbors = false;
				if(twoNeighbors ) {
					if (checkForCorner(sVector - vec2(XOFFSET_CLL,YOFFSET_CLL), wVector - vec2(XOFFSET_CLL,YOFFSET_CLL))) {
						v0_flags = v0_flags | DONT_OPTIMIZE_S| DONT_OPTIMIZE_W;
					}
				}
				
				v0_neighbors = ivec4(-1,-1,sIndex,wIndex);
				
				twoNeighbors = true;
				//emit the Vertex in the second sector -v1
				v1_pos = centerPos + vec2(XOFFSET_CUR,YOFFSET_CUR);
				//index = getNeighborIndex(CENTER, 1);
				int nIndex = nNeighborIndex;
				int eIndex = eNeighborIndex;
				v1_nNcolors = ivec4(ULPixelColorIndex,URPixelColorIndex);
				v1_nEcolors = ivec4(URPixelColorIndex,LRPixelColorIndex);
				if(nNeighborsFound) { v1_flags = HAS_NORTHERN_NEIGHBOR | HAS_NORTHERN_SPLINE; 
					// if(dontOptimizeN){v1_flags = v1_flags | DONT_OPTIMIZE_N;} 
				} else twoNeighbors = false;
				if(eNeighborsFound) { v1_flags = v1_flags | HAS_EASTERN_NEIGHBOR | HAS_EASTERN_SPLINE; 
					// if(dontOptimizeE){v1_flags = v1_flags | DONT_OPTIMIZE_E;} 
				} else twoNeighbors = false;
				
				if(twoNeighbors ) {
					if (checkForCorner(nVector - vec2(XOFFSET_CUR,YOFFSET_CUR), eVector - vec2(XOFFSET_CUR,YOFFSET_CUR))) {
						v1_flags = v1_flags | DONT_OPTIMIZE_N| DONT_OPTIMIZE_E;
					}
				}
				
				v1_neighbors = ivec4(nIndex,eIndex,-1,-1);
				
		} else if (eCenter == EDGE_DIAGONAL_LLUR) {
				bool twoNeighbors = true;
			//emit the Vertex in the first sector
				v0_pos = centerPos + vec2(XOFFSET_CUL,YOFFSET_CUL);
				//index = getNeighborIndex(CENTER, 0);
				int nIndex = nNeighborIndex;
				int wIndex = wNeighborIndex;
				v0_nNcolors = ivec4(ULPixelColorIndex,URPixelColorIndex);
				v0_nWcolors = ivec4(LLPixelColorIndex,ULPixelColorIndex);
				if(nNeighborsFound) { v0_flags = HAS_NORTHERN_NEIGHBOR | HAS_NORTHERN_SPLINE; 
					// if(dontOptimizeN){v0_flags = v0_flags | DONT_OPTIMIZE_N;} 
				} else twoNeighbors = false;
				if(wNeighborsFound) { v0_flags = v0_flags | HAS_WESTERN_NEIGHBOR | HAS_WESTERN_SPLINE; 
					// if(dontOptimizeW){v0_flags = v0_flags | DONT_OPTIMIZE_W;} 
				} else twoNeighbors = false;
				if(twoNeighbors ) {
					if (checkForCorner(nVector - vec2(XOFFSET_CUL,YOFFSET_CUL), wVector - vec2(XOFFSET_CUL,YOFFSET_CUL))) {
						v0_flags = v0_flags | DONT_OPTIMIZE_N| DONT_OPTIMIZE_W;
					}
				}
				
				v0_neighbors = ivec4(nIndex,-1,-1,wIndex);
			
				twoNeighbors = true;
				//emit the Vertex in the second sector
				v1_pos = centerPos + vec2(XOFFSET_CLR,YOFFSET_CLR);
				//index = getNeighborIndex(CENTER, 1);
				int sIndex = sNeighborIndex;
				int eIndex = eNeighborIndex;
				v1_nScolors = ivec4(LRPixelColorIndex,LLPixelColorIndex);
				v1_nEcolors = ivec4(URPixelColorIndex,LRPixelColorIndex);
				if(sNeighborsFound) { v1_flags = HAS_SOUTHERN_NEIGHBOR | HAS_SOUTHERN_SPLINE; 
					// if(dontOptimizeS){v1_flags = v1_flags | DONT_OPTIMIZE_S;} 
				} else twoNeighbors = false;
				if(eNeighborsFound) { v1_flags = v1_flags | HAS_EASTERN_NEIGHBOR | HAS_EASTERN_SPLINE; 
					// if(dontOptimizeE){v1_flags = v1_flags | DONT_OPTIMIZE_E;} 
				} else twoNeighbors = false;
				if(twoNeighbors ) {
					if (checkForCorner(sVector - vec2(XOFFSET_CLR,YOFFSET_CLR), eVector - vec2(XOFFSET_CLR,YOFFSET_CLR))) {
						v1_flags = v1_flags | DONT_OPTIMIZE_S| DONT_OPTIMIZE_E;
					}
				}
				v1_neighbors = ivec4(-1,eIndex,sIndex,-1);
				
		} else {
			//there is only one Vertex but we have to create a second one- as a dummy, in order to keep our indexing working
			
			v0_pos = centerPos;
			//index = getNeighborIndex(CENTER, 0);
			int nIndex = nNeighborIndex;
			int eIndex = eNeighborIndex;
			int sIndex = sNeighborIndex;
			int wIndex = wNeighborIndex;
			v0_nNcolors = ivec4(ULPixelColorIndex,URPixelColorIndex);
			v0_nEcolors = ivec4(URPixelColorIndex,LRPixelColorIndex);
			v0_nScolors = ivec4(LRPixelColorIndex,LLPixelColorIndex);
			v0_nWcolors = ivec4(LLPixelColorIndex,ULPixelColorIndex);
			if(nNeighborsFound) {  v0_flags = HAS_NORTHERN_NEIGHBOR;  
				// if(dontOptimizeN){v0_flags = v0_flags | DONT_OPTIMIZE_N;} 
			}
			if(eNeighborsFound) {  v0_flags = v0_flags | HAS_EASTERN_NEIGHBOR;  
				// if(dontOptimizeE){v0_flags = v0_flags | DONT_OPTIMIZE_E;} 
			}
			if(sNeighborsFound) {  v0_flags = v0_flags | HAS_SOUTHERN_NEIGHBOR;  
				// if(dontOptimizeS){v0_flags = v0_flags | DONT_OPTIMIZE_S;} 
			}
			if(wNeighborsFound) {  v0_flags = v0_flags | HAS_WESTERN_NEIGHBOR;  
				// if(dontOptimizeW){v0_flags = v0_flags | DONT_OPTIMIZE_W;} 
			}
			
			if (neighborCount == 2) {
				// vec2 spline[2];
				// int c = 0;
				if(nNeighborsFound) {
					v0_flags = v0_flags | HAS_NORTHERN_SPLINE;
					// spline[c] = nVector;c++;
				}
				if(eNeighborsFound) {
					v0_flags = v0_flags | HAS_EASTERN_SPLINE;
					// spline[c] = eVector;c++;
				}
				if(sNeighborsFound) {
					v0_flags = v0_flags | HAS_SOUTHERN_SPLINE;
					// spline[c] = sVector;c++;
				}
				if(wNeighborsFound) {
					v0_flags = v0_flags | HAS_WESTERN_SPLINE;
					// spline[c] = wVector;c++;
				}
			} else if (neighborCount == 3 ) {
				int contours = 0;
				int contourCount = 0;
				vec2 p[3];
				if (nNeighborsFound) {
					if(isContour(v0_nNcolors)) {
						p[0]= nVector;
						contourCount++;
						contours = HAS_NORTHERN_SPLINE;
					}
				}
				if (eNeighborsFound) {
					if(isContour(v0_nEcolors)) {
						p[contourCount]= eVector;
						contourCount++;
						contours = contours | HAS_EASTERN_SPLINE;
					}
				}
				if (sNeighborsFound) {
					if(isContour(v0_nScolors)) {
						p[contourCount]= sVector;
						contourCount++;
						contours = contours | HAS_SOUTHERN_SPLINE;
					}
				}
				if (wNeighborsFound) {
					if(isContour(v0_nWcolors)) {
						p[contourCount]= wVector;
						contourCount++;
						contours = contours | HAS_WESTERN_SPLINE;
					}
				}
				if(contourCount == 2) {
					v0_flags = v0_flags | contours | HAS_CORRECTED_POSITION; 
					v1_pos = calcAdjustedPoint(centerPos + p[0], centerPos, centerPos + p[1]);
					//mark vertex as corrected position vertex by setting its flag to -1
					v1_flags = -1;
				} else {
					//use angles to solve the problem
					if(nNeighborsFound && sNeighborsFound) {
						v0_flags = v0_flags| HAS_NORTHERN_SPLINE | HAS_SOUTHERN_SPLINE | HAS_CORRECTED_POSITION; 
						v1_pos = calcAdjustedPoint(centerPos+nVector, centerPos, centerPos+sVector);
						//mark vertex as corrected position vertex by setting its flag to -1
						v1_flags = -1;
					}
					else {
						v0_flags = v0_flags| HAS_EASTERN_SPLINE | HAS_WESTERN_SPLINE | HAS_CORRECTED_POSITION; 
						v1_pos = calcAdjustedPoint(centerPos+eVector, centerPos, centerPos+wVector);
						//mark vertex as corrected position vertex by setting its flag to -1
						v1_flags = -1;
					}
				}
			}
			v0_neighbors = ivec4(nIndex,eIndex,sIndex,wIndex);
		}

	}
	//vertex emission
	emitPoint(v0_pos, v0_neighbors, v0_flags, v0_nNcolors, v0_nEcolors, v0_nScolors, v0_nWcolors);
	emitPoint(v1_pos, v1_neighbors, v1_flags, v1_nNcolors, v1_nEcolors, v1_nScolors, v1_nWcolors);
}
