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

#define NORTH		128
#define NORTHEAST	64
#define EAST		32
#define SOUTHEAST	16
#define SOUTH		8
#define SOUTHWEST	4
#define WEST		2
#define NORTHWEST	1

in vec2 UV;
layout(pixel_center_integer) in vec4 gl_FragCoord;

uniform sampler2D similarityGraph;

layout(location = 0) out vec4 color0;

int voteA;
int voteB;

int debugA;
int debugB;

int componentSizeA = 2;
int componentSizeB = 2;


void voteIslands() {
	if( int(texelFetch(similarityGraph, ivec2(gl_FragCoord.x-1,gl_FragCoord.y+1), 0).x*255) == 1) {
		voteA = voteA + 5;
		return;
	}
	if( int(texelFetch(similarityGraph, ivec2(gl_FragCoord.x+1,gl_FragCoord.y-1), 0).x*255) == 1) {
		voteA = voteA + 5;
		return;
	}
	if( int(texelFetch(similarityGraph, ivec2(gl_FragCoord.x-1,gl_FragCoord.y-1), 0).x*255) == 1) {
		voteB = voteB + 5;
		return;
	}
	if( int(texelFetch(similarityGraph, ivec2(gl_FragCoord.x+1,gl_FragCoord.y+1), 0).x*255) == 1) {
		voteB = voteB + 5;
		return;
	}
}

void countForComponent(int c) {
	switch(c) {
		case 1:
			componentSizeA++;
			break;
		case 2:
			componentSizeB++;
			break;
		default:
			break;
	}
}

void voteSparsePixels() {
	//INFO on border treatment
	// border treatment currently relies on the similiaritygraph texture wrap setting beeing GL_CLAMP_TO_BORDER
	// addidtionally GL_TEXTURE_BORDER_COLOR needs to be set to black(0,0,0,0), which is the default value btw.
	
	//label-array 8x8
	//let component A be 1 and B be 2
	int lArray[64] = int[64] (0,0,0,0,0,0,0,0,
							  0,0,0,0,0,0,0,0,
							  0,0,0,0,0,0,0,0,
							  0,0,0,1,2,0,0,0,
							  0,0,0,2,1,0,0,0,
							  0,0,0,0,0,0,0,0,
							  0,0,0,0,0,0,0,0,
							  0,0,0,0,0,0,0,0);
	
	// neigborhood indices
	int nNW=0;
	int nW=0;
	int nSW=0;
	int nS=0;
	int nSE=0;
	int nE=0;
	int nNE=0;
	int nN=0;
	//this loop iterates trough 3 levels starting from the middle of the 8x8 label array
	for (int level = 0 ; level < 2 ; level++) {
		int xOFFSET = -(1+2*level);
		int yOFFSET = 1+(2*level);
		//NW corner-node
		//nhood ... stores this nodes neighboring information taken from the similarity graph
		int nhood = int(texelFetch(similarityGraph, ivec2(gl_FragCoord.x+xOFFSET,gl_FragCoord.y+yOFFSET), 0).y*255);
		//int nhood = similarityGraph[FragCoordX-(1+2*level) + (FragCoordY+1+(2*level))*17];
		int currentComponentIndex = 8*(3-level) + 3-level;
		//current value in the label-array
		int currentComponent = lArray[currentComponentIndex];
		nS  = 8*(4-level)+(3-level);//TODO: OPtimieren
		nSW = 8*(4-level)+(2-level);
		nW  = 8*(3-level)+(2-level);
		nNW = 8*(2-level)+(2-level);
		nN  = 8*(2-level)+(3-level);
		nNE = 8*(2-level)+(4-level);
		nE  = 8*(3-level)+(4-level);

		if (currentComponent == 0) {
			//this block scans neighborhood for connected components, in case this node has not yet been labeled
			if( ((nhood & SOUTH) == SOUTH) && (lArray[nS] !=0) )		{currentComponent = lArray[nS]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
			else if( ((nhood & SOUTHWEST) == SOUTHWEST) && (lArray[nSW] !=0) )		{currentComponent = lArray[nSW]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
			else if( ((nhood & WEST) == WEST) && (lArray[nW] !=0) )				{currentComponent = lArray[nW];  lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
			else if( ((nhood & NORTHWEST) == NORTHWEST) && (lArray[nNW] !=0) )	{currentComponent = lArray[nNW]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
			else if( ((nhood & NORTH) == NORTH) && (lArray[nN] !=0) )			{currentComponent = lArray[nN];  lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
			else if( ((nhood & NORTHEAST) == NORTHEAST) && (lArray[nNE] !=0) )	{currentComponent = lArray[nNE]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
			else if( ((nhood & EAST) == EAST) && (lArray[nE] !=0) )	{currentComponent = lArray[nE]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
		}
		if (currentComponent !=0) {
			//check SW W NW N NE
			if( (nhood & SOUTHWEST) == SOUTHWEST ) {
				//SW
				//check if node is already labeled
				if( lArray[nSW] == 0) {
					lArray[nSW] = currentComponent;
					countForComponent(currentComponent);
				}
			}
			if( (nhood & WEST) == WEST) {
				//W
				if(lArray[nW] == 0) {
					lArray[nW] = currentComponent;
					countForComponent(currentComponent);
				}
			}
			if( (nhood & NORTHWEST) == NORTHWEST) {
				if( lArray[nNW] == 0) {
					lArray[nNW] = currentComponent;
					countForComponent(currentComponent);
				}
			}
			if( (nhood & NORTH) == NORTH) {
				//N
				if(lArray[nN] == 0) {
					lArray[nN] = currentComponent;
					countForComponent(currentComponent);
				}
			}
			if( (nhood & NORTHEAST) == NORTHEAST) {
				//NE
				if(lArray[nNE] == 0) {
					lArray[nNE] = currentComponent;
					countForComponent(currentComponent);
				}
			}
		}
		//N nodes
		if(level>0) {
			for(int i = 0 ; i < level*2; i++) {
				xOFFSET = -(2*level-1)+2*i;
				yOFFSET = +1+2*level;
				nhood = int(texelFetch(similarityGraph, ivec2(gl_FragCoord.x+xOFFSET,gl_FragCoord.y+yOFFSET), 0).y*255);
				//nhood = similarityGraph[FragCoordX-(2*level-1)+2*i + (FragCoordY+1+2*level)*17];
				currentComponentIndex = 8*(3-level) + (i+4-level);
				currentComponent = lArray[currentComponentIndex];
				nW  = 8*(3-level) + (i+3-level);
				nNW = 8*(2-level) + (i+3-level);
				nN  = 8*(2-level) + (i+4-level);
				nNE = 8*(2-level) + (i+5-level);
				nE  = 8*(3-level) + (i+5-level);
				if (currentComponent == 0) {
					//this block scans neighborhood for connected components, in case this node has not yet been labeled
					if( ((nhood & WEST) == WEST) && (lArray[nW] !=0) )		{currentComponent = lArray[nW]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
					else if( ((nhood & NORTHWEST) == NORTHWEST) && (lArray[nNW] !=0) )		{currentComponent = lArray[nNW]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
					else if( ((nhood & NORTH) == NORTH) && (lArray[nN] !=0) )			{currentComponent = lArray[nN];	 lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
					else if( ((nhood & NORTHEAST) == NORTHEAST) && (lArray[nNE] !=0) )	{currentComponent = lArray[nNE]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
					else if( ((nhood & EAST) == EAST) && (lArray[nE] !=0) )	{currentComponent = lArray[nE]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
				}
				//check NW,N,NE neighbors
				if(currentComponent != 0) {
					if( (nhood & NORTHWEST) == NORTHWEST) {
						if(lArray[nNW] == 0) {
							// label the NW neighbor in the label-array
							lArray[nNW] = currentComponent;
							countForComponent(currentComponent);
						}
					}
					if( (nhood & NORTH) == NORTH) {
						if(lArray[nN] == 0) {
							// label the N neighbor in the label-array
							lArray[nN] = currentComponent;
							countForComponent(currentComponent);
						}
					}
					if( (nhood & NORTHEAST) == NORTHEAST) {
						if(lArray[nNE] == 0) {
							// label the NE neighbor in the label-array
							lArray[nNE] = currentComponent;
							countForComponent(currentComponent);
						}
					}
				}
			}
		}
		
		//NE corner-node
		xOFFSET = (1+2*level);
		yOFFSET = 1+(2*level);
		nhood = int(texelFetch(similarityGraph, ivec2(gl_FragCoord.x+xOFFSET,gl_FragCoord.y+yOFFSET), 0).y*255);
		
		//nhood = similarityGraph[FragCoordX + xOFFSET + (FragCoordY + yOFFSET)*17];
		//current value in the label-array
		currentComponentIndex = 8*(3-level) + (4+level);
		currentComponent = lArray[currentComponentIndex];
		nW  = 8*(3-level)+(3+level);
		nNW	= 8*(2-level)+(3+level);
		nN  = 8*(2-level)+(4+level);
		nNE = 8*(2-level)+(5+level);
		nE  = 8*(3-level)+(5+level);
		nSE = 8*(4-level)+(5+level);
		nS  = 8*(4-level)+(4+level);
		if (currentComponent == 0) {
			//this block scans neighborhood for connected components, in case this node has not yet been labeled
			if(((nhood & WEST) == WEST) && (lArray[nNW] !=0)){currentComponent = lArray[nW]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
			else if(((nhood & NORTHWEST) == NORTHWEST) && (lArray[nNW] !=0)){currentComponent = lArray[nNW]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
			else if(((nhood & NORTH) == NORTH)		  && (lArray[nN] !=0 )){currentComponent = lArray[nN];  lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
			else if(((nhood & NORTHEAST) == NORTHEAST) && (lArray[nNE] !=0)){currentComponent = lArray[nNE]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
			else if(((nhood & EAST) == EAST)		      && (lArray[nE] !=0 )){currentComponent = lArray[nE];  lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
			else if(((nhood & SOUTHEAST) == SOUTHEAST) && (lArray[nSE] !=0)){currentComponent = lArray[nSE]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
			else if(((nhood & SOUTH) == SOUTH) && (lArray[nS] !=0)){currentComponent = lArray[nS]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
		}
		if (currentComponent != 0) {
			//check NW N NE E SE
			if( (nhood & NORTHWEST) == NORTHWEST) {
				if (lArray[nNW] == 0) {
					lArray[nNW] = currentComponent;
					countForComponent(currentComponent);
				}
			}
			if( (nhood & NORTH) == NORTH) {
				if (lArray[nN] == 0) {
					lArray[nN] = currentComponent;
					countForComponent(currentComponent);
				}
			}
			if( (nhood & NORTHEAST) == NORTHEAST) {
				if (lArray[nNE] == 0) {
					lArray[nNE] = currentComponent;
					countForComponent(currentComponent);
				}
			}
			if( (nhood & EAST) == EAST) {
				if (lArray[nE] == 0) {
					lArray[nE] = currentComponent;
					countForComponent(currentComponent);
				}
			}
			if( (nhood & SOUTHEAST) == SOUTHEAST) {
				if (lArray[nSE] == 0) {
					lArray[nSE] = currentComponent;
					countForComponent(currentComponent);
				}
			}
		}
		//E nodes
		if(level>0) {
			for(int i = 0 ; i < level*2; i++) {
				xOFFSET = 1+2*level;
				yOFFSET = 2*level-1-2*i;
				nhood = int(texelFetch(similarityGraph, ivec2(gl_FragCoord.x+xOFFSET,gl_FragCoord.y+yOFFSET), 0).y*255);
				//nhood = similarityGraph[FragCoordX + xOFFSET + (FragCoordY + yOFFSET)*17];
				currentComponentIndex = 8*(i+4-level) + (4+level);
				currentComponent = lArray[currentComponentIndex];
				nN = 8*(i+3-level) + (4+level);
				nNE= 8*(i+3-level) + (5+level);
				nE = 8*(i+4-level) + (5+level);
				nSE= 8*(i+5-level) + (5+level);
				nS = 8*(i+5-level) + (4+level);
				if (currentComponent == 0) {
					//this block scans neighborhood for connected components, in case this node has not yet been labeled
					if( ((nhood & NORTH) == NORTH) && (lArray[nN] !=0)){currentComponent = lArray[nN]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
					else if( ((nhood & NORTHEAST) == NORTHEAST) && (lArray[nNE] !=0)){currentComponent = lArray[nNE]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
					else if( ((nhood & EAST) == EAST) && (lArray[nE] !=0))			{currentComponent = lArray[nE];	 lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
					else if( ((nhood & SOUTHEAST) == SOUTHEAST) && (lArray[nSE] !=0)){currentComponent = lArray[nSE]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
					else if( ((nhood & SOUTH) == SOUTH) && (lArray[nS] !=0)){currentComponent = lArray[nS]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
				}
				//check NE,E,SE neighbors
				if(currentComponent != 0) {
					if( (nhood & NORTHEAST) == NORTHEAST) {
						if( lArray[nNE] == 0) {
							// label the NW neighbor in the label-array
							lArray[nNE] = currentComponent;
							countForComponent(currentComponent);
						}
					}
					if( (nhood & EAST) == EAST) {
						if( lArray[nE] == 0) {
							// label the N neighbor in the label-array
							lArray[nE] = currentComponent;
							countForComponent(currentComponent);
						}
					}
					if( (nhood & SOUTHEAST) == SOUTHEAST) {
						if( lArray[nSE] == 0) {
							// label the NE neighbor in the label-array
							lArray[nSE] = currentComponent;
							countForComponent(currentComponent);
						}
					}
				}
			}
		}
		
		//SE corner-node
		xOFFSET = (1+2*level);
		yOFFSET = -(1+2*level);
		nhood = int(texelFetch(similarityGraph, ivec2(gl_FragCoord.x+xOFFSET,gl_FragCoord.y+yOFFSET), 0).y*255);
		//nhood = similarityGraph[FragCoordX + xOFFSET + (FragCoordY + yOFFSET)*17];
		currentComponentIndex = 8*(4+level) + (4+level);
		currentComponent = lArray[currentComponentIndex];
		nN =8*(3+level)+(4+level);
		nNE=8*(3+level)+(5+level);
		nE= 8*(4+level)+(5+level);
		nSE=8*(5+level)+(5+level);
		nS= 8*(5+level)+(4+level);
		nSW=8*(5+level)+(3+level);
		nW =8*(4+level)+(3+level);
		if (currentComponent == 0) {
			//this block scans neighborhood for connected components, in case this node has not yet been labeled
			if( ((nhood & NORTH) == NORTH) && (lArray[nN] !=0))	 {currentComponent = lArray[nN]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
			else if( ((nhood & NORTHEAST) == NORTHEAST) && (lArray[nNE] !=0))	 {currentComponent = lArray[nNE]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
			else if( ((nhood & EAST) == EAST) && (lArray[nE] !=0))			 {currentComponent = lArray[nE];  lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
			else if( ((nhood & SOUTHEAST) == SOUTHEAST) && (lArray[nSE] !=0)){currentComponent = lArray[nSE]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
			else if( ((nhood & SOUTH) == SOUTH) && (lArray[nS] !=0))		 {currentComponent = lArray[nS];  lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
			else if( ((nhood & SOUTHWEST) == SOUTHWEST) && (lArray[nSW] !=0)){currentComponent = lArray[nSW]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
			else if( ((nhood & WEST) == WEST) && (lArray[nW] !=0)){currentComponent = lArray[nW]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
		}
		if (currentComponent !=0) {
			//check NE E SE S SW
			if( (nhood & NORTHEAST) == NORTHEAST) {
				if (lArray[nNE] == 0){
					lArray[nNE] = currentComponent;
					countForComponent(currentComponent);
				}
			}
			if( (nhood & EAST) == EAST) {
				if (lArray[nE] == 0){
					lArray[nE] = currentComponent;
					countForComponent(currentComponent);
				}
			}
			if( (nhood & SOUTHEAST) == SOUTHEAST) {
				if (lArray[nSE] == 0){
					lArray[nSE] = currentComponent;
					countForComponent(currentComponent);
				}
			}
			if( (nhood & SOUTH) == SOUTH) {
				if (lArray[nS] == 0){
					lArray[nS] = currentComponent;
					countForComponent(currentComponent);
				}
			}
			if( (nhood & SOUTHWEST) == SOUTHWEST) {
				if (lArray[nSW] == 0){
					lArray[nSW] = currentComponent;
					countForComponent(currentComponent);
				}
			}
		}
		
		//S nodes
		if(level>0) {
			for(int i = 0 ; i < level*2; i++) {
				xOFFSET = -(2*level-1)+2*i;
				yOFFSET = -(1+2*level);
				nhood = int(texelFetch(similarityGraph, ivec2(gl_FragCoord.x+xOFFSET,gl_FragCoord.y+yOFFSET), 0).y*255);
				//nhood = similarityGraph[FragCoordX + xOFFSET + (FragCoordY + yOFFSET)*17];
				currentComponentIndex = 8*(4+level) + (i+4-level);
				currentComponent = lArray[currentComponentIndex];
				nE = 8*(4+level) + (i+5-level);
				nSE= 8*(5+level) + (i+5-level);
				nS = 8*(5+level) + (i+4-level);
				nSW= 8*(5+level) + (i+3-level);
				nW = 8*(4+level) + (i+3-level);
				if (currentComponent == 0) {
					//this block scans neighborhood for connected components, in case this node has not yet been labeled
					if( ((nhood & EAST) == EAST) && (lArray[nE] !=0)){currentComponent = lArray[nE]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
					else if( ((nhood & SOUTHEAST) == SOUTHEAST) && (lArray[nSE] !=0)){currentComponent = lArray[nSE]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
					else if( ((nhood & SOUTH) == SOUTH) && (lArray[nS] !=0))			{currentComponent = lArray[nS];	 lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
					else if( ((nhood & SOUTHWEST) == SOUTHWEST) && (lArray[nSW] !=0)){currentComponent = lArray[nSW]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
					else if( ((nhood & WEST) == WEST) && (lArray[nW] !=0)){currentComponent = lArray[nW]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
				}
				//check SW,S,SE neighbors
				if(currentComponent != 0) {
					if( (nhood & SOUTHEAST) == SOUTHEAST) {
						if(lArray[nSE] == 0) {
							lArray[nSE] = currentComponent;
							countForComponent(currentComponent);
						}
					}
					if( (nhood & SOUTH) == SOUTH) {
						if(lArray[nS] == 0) {
							lArray[nS] = currentComponent;
							countForComponent(currentComponent);
						}
					}
					if( (nhood & SOUTHWEST) == SOUTHWEST) {
						if(lArray[nSW] == 0) {
							lArray[nSW] = currentComponent;
							countForComponent(currentComponent);
						}
					}
				}
			}
		}
		
		//SW corner-node
		xOFFSET = -(1+2*level);
		yOFFSET = -(1+2*level);
		nhood = int(texelFetch(similarityGraph, ivec2(gl_FragCoord.x+xOFFSET,gl_FragCoord.y+yOFFSET), 0).y*255);
		//nhood = similarityGraph[FragCoordX + xOFFSET + (FragCoordY + yOFFSET)*17];
		currentComponentIndex = 8*(4+level) + (3-level);
		currentComponent = lArray[currentComponentIndex];
		nE =8*(4+level)+(4-level);
		nSE=8*(5+level)+(4-level);
		nS= 8*(5+level)+(3-level);
		nSW=8*(5+level)+(2-level);
		nW= 8*(4+level)+(2-level);
		nNW=8*(3+level)+(2-level);
		nN =8*(3+level)+(3-level);
		if (currentComponent == 0) {
			//this block scans neighborhood for connected components, in case this node has not yet been labeled
			if( ((nhood & EAST) == EAST) && (lArray[nE] !=0)){currentComponent = lArray[nE]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
			else if( ((nhood & SOUTHEAST) == SOUTHEAST) && (lArray[nSE] !=0)){currentComponent = lArray[nSE]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
			else if( ((nhood & SOUTH) == SOUTH) && (lArray[nS] !=0))			{currentComponent = lArray[nS];  lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
			else if( ((nhood & SOUTHWEST) == SOUTHWEST) && (lArray[nSW] !=0)){currentComponent = lArray[nSW]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
			else if( ((nhood & WEST) == WEST) && (lArray[nW] !=0))			{currentComponent = lArray[nW];  lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
			else if( ((nhood & NORTHWEST) == NORTHWEST) && (lArray[nNW] !=0)){currentComponent = lArray[nNW]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
			else if( ((nhood & NORTH) == NORTH) && (lArray[nN] !=0)){currentComponent = lArray[nN]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
		}
		if (currentComponent !=0) {
			//check SE S SW W NW
			if( (nhood & SOUTHEAST) == SOUTHEAST) {
				if (lArray[nSE] == 0){
					lArray[nSE] = currentComponent;
					countForComponent(currentComponent);
				}
			}
			if( (nhood & SOUTH) == SOUTH) {
				if (lArray[nS] == 0){
					lArray[nS] = currentComponent;
					countForComponent(currentComponent);
				}
			}
			if( (nhood & SOUTHWEST) == SOUTHWEST) {
				if (lArray[nSW] == 0){
					lArray[nSW] = currentComponent;
					countForComponent(currentComponent);
				}
			}
			if( (nhood & WEST) == WEST) {
				if (lArray[nW] == 0){
					lArray[nW] = currentComponent;
					countForComponent(currentComponent);
				}
			}
			if( (nhood & NORTHWEST) == NORTHWEST) {
				if (lArray[nNW] == 0){
					lArray[nNW] = currentComponent;
					countForComponent(currentComponent);
				}
			}
		}
		
		//W nodes
		if(level>0) {
			for(int i = 0 ; i < level*2; i++) {
				xOFFSET = -(1+2*level);
				yOFFSET = -(2*level-1)+2*i;
				nhood = int(texelFetch(similarityGraph, ivec2(gl_FragCoord.x+xOFFSET,gl_FragCoord.y+yOFFSET), 0).y*255);
				//nhood = similarityGraph[FragCoordX + xOFFSET + (FragCoordY + yOFFSET)*17];
				currentComponentIndex = 8*(3+level-i) + (3-level);
				currentComponent = lArray[currentComponentIndex];
				nN= 8*(2+level-i) + (3-level);
				nNW=8*(2+level-i) + (2-level);
				nW =8*(3+level-i) + (2-level);
				nSW=8*(4+level-i) + (2-level);
				nS =8*(4+level-i) + (3-level);
				if (currentComponent == 0) {
					//this block scans neighborhood for connected components, in case this node has not yet been labeled
					if( ((nhood & SOUTH) == SOUTH) && (lArray[nS] !=0))	{currentComponent = lArray[nS]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
					else if( ((nhood & SOUTHWEST) == SOUTHWEST) && (lArray[nSW] !=0))	{currentComponent = lArray[nSW]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
					else if( ((nhood & WEST) == WEST) && (lArray[nW] !=0))			{currentComponent = lArray[nW];	 lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
					else if( ((nhood & NORTHWEST) == NORTHWEST) && (lArray[nNW] !=0))	{currentComponent = lArray[nNW]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
					else if( ((nhood & NORTH) == NORTH) && (lArray[nN] !=0))	{currentComponent = lArray[nN]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
				}
				//check SW W NW neighbors
				if(currentComponent != 0) {
					if( (nhood & SOUTHWEST) == SOUTHWEST) {
						if( lArray[nSW] == 0) {
							lArray[nSW] = currentComponent;
							countForComponent(currentComponent);
						}
					}
					if( (nhood & WEST) == WEST) {
						if( lArray[nW] == 0) {
							lArray[nW] = currentComponent;
							countForComponent(currentComponent);
						}
					}
					if( (nhood & NORTHWEST) == NORTHWEST) {
						if( lArray[nNW] == 0) {
							lArray[nNW] = currentComponent;
							countForComponent(currentComponent);
						}
					}
				}
			}
		}
	}
	//Level 4 treatment
	//northen border
	for(int i = 0 ; i < 8; i++) {
		int nhood = int(texelFetch(similarityGraph, ivec2(gl_FragCoord.x-7+2*i,gl_FragCoord.y+7), 0).y*255.0);
		
	}
	debugA = componentSizeA;
	debugB = componentSizeB;
	//now that the connected component sizes are computed we vote for the smaller component
	if(componentSizeA < componentSizeB) {
		//vote for A ... weight is difference between the sizes of the components
		voteA = voteA + (componentSizeB - componentSizeA);
	} else if(componentSizeA > componentSizeB) {
		//vote for B ... weight is difference between the sizes of the components
		voteB = voteB + (componentSizeA - componentSizeB);
	}
}


int traceNodes(ivec2 nodeCoords, int predecessorNodeDirection) {
	//codes the edge directions
			// N  ... 0x10000000 = 128
			// NE ... 0x01000000 = 64
			// E  ... 0x00100000 = 32
			// SE ... 0x00010000 = 16
			// S  ... 0x00001000 = 8
			// SW ... 0x00000100 = 4
			// W  ... 0x00000010 = 2
			// NW ... 0x00000001 = 1 
	int totalLength = 0; // initial total length
	
	ivec2 currentNodeCoords = nodeCoords;
	//check node valence
	ivec4 currentNodeValue = ivec4(texelFetch(similarityGraph, currentNodeCoords, 0)*255);
	
	
	ivec2 nextNodeCoords = ivec2(0,0);
	int directionToCurrentNode = 0;
	
	while(currentNodeValue.x == 2) {
		//get next neighbor
		int nextNodeDirection = currentNodeValue.y ^ predecessorNodeDirection;
		
		switch(nextNodeDirection) {
			case 1:
				//NW
				nextNodeCoords = ivec2( currentNodeCoords.x-1, currentNodeCoords.y+1);
				directionToCurrentNode = 16;
				break;
			case 2:
				//W
				nextNodeCoords = ivec2( currentNodeCoords.x-1, currentNodeCoords.y );
				directionToCurrentNode = 32;
				break;
			case 4:
				//SW
				nextNodeCoords = ivec2( currentNodeCoords.x-1, currentNodeCoords.y-1 );
				directionToCurrentNode = 64;
				break;
			case 8:
				//S
				nextNodeCoords = ivec2( currentNodeCoords.x, currentNodeCoords.y-1 );
				directionToCurrentNode = 128;
				break;
			case 16:
				//SE
				nextNodeCoords = ivec2( currentNodeCoords.x+1, currentNodeCoords.y-1 );
				directionToCurrentNode = 1;
				break;
			case 32:
				//E
				nextNodeCoords = ivec2( currentNodeCoords.x+1, currentNodeCoords.y );
				directionToCurrentNode = 2;
				break;
			case 64:
				//NE
				nextNodeCoords = ivec2( currentNodeCoords.x+1, currentNodeCoords.y+1 );
				directionToCurrentNode = 4;
				break;
			case 128:
				//N
				directionToCurrentNode = 8;
				nextNodeCoords = ivec2( currentNodeCoords.x, currentNodeCoords.y+1 );
				break;
			default:
				//this should not be the case, but just in case ;)
				directionToCurrentNode = predecessorNodeDirection;
				nextNodeCoords = currentNodeCoords;
				return 0;
		}
		//get next node
		currentNodeCoords= nextNodeCoords;
		predecessorNodeDirection=directionToCurrentNode;
		currentNodeValue = ivec4(texelFetch(similarityGraph, currentNodeCoords, 0)*255);
		totalLength++;
	}
	return totalLength;
}


void voteCurves() {
	int lengthA = 1;
	int lengthB = 1;
	//coordinates for nodes A1,A2,B1,B2
	//A1 B2
	//B1 A2
	ivec2 A1 = ivec2(gl_FragCoord.x-1,gl_FragCoord.y+1);
	ivec2 A2 = ivec2(gl_FragCoord.x+1,gl_FragCoord.y-1);
	ivec2 B1 = ivec2(gl_FragCoord.x-1,gl_FragCoord.y-1);
	ivec2 B2 = ivec2(gl_FragCoord.x+1,gl_FragCoord.y+1);
	lengthA = lengthA + traceNodes(A1,16);
	lengthA = lengthA + traceNodes(A2,1);
	lengthB = lengthB + traceNodes(B1,64);
	lengthB = lengthB + traceNodes(B2,4);
	//evaluate lengths and vote
	if (lengthA==lengthB) {
		//no one wins
		return;
	} else if(lengthA > lengthB) {
		//A wins
		voteA = voteA + lengthA - lengthB;
	} else {
		//B wins
		voteB = voteB + lengthB - lengthA;
	}
}

/*
returns true if the 2x2 block of nodes around the crossing diagonals is fully connected
*/
bool isFullyConnectedCD() {
	//we only have to look at one neighboring edge to determine whether a 2x2 block is fully connected or not
	if (int(texelFetch(similarityGraph, ivec2(gl_FragCoord.x,gl_FragCoord.y+1), 0).x*255) == 0 ) {
		return false;
	} else return true;
	
}

bool isFullyConnectedD() {
	//examine upper edge
	if ( int(texelFetch(similarityGraph, ivec2(gl_FragCoord.x,gl_FragCoord.y+1), 0).x*255) == 16) {
		//examine right edge
		if ( int(texelFetch(similarityGraph, ivec2(gl_FragCoord.x+1,gl_FragCoord.y), 0).x*255) == 16) {
			//examine lower edge
			if ( int(texelFetch(similarityGraph, ivec2(gl_FragCoord.x,gl_FragCoord.y-1), 0).x*255) == 16) {
				//we could skip the last edgecheck -> this one is fully connected for shure
				if ( int(texelFetch(similarityGraph, ivec2(gl_FragCoord.x-1,gl_FragCoord.y), 0).x*255) == 16) {
					return true;
				} else return false;
			} else return false;
		} else return false;
	} else return false;
}

void main() {
	vec4 fragmentXColor = texelFetch(similarityGraph, ivec2(gl_FragCoord.xy), 0);
	//int fragmentValue = int(texelFetch(similarityGraph, ivec2(gl_FragCoord.xy), 0).x*255);
	int fragmentValue = int(fragmentXColor.x*255);
	//check if fragment hit is a crossing diagonal
	if(fragmentValue == 96) {
		// we are looking at a crossing diagonal
		// 1. check if 2x2 block is fully connected
		if (isFullyConnectedCD()) {
			color0 = vec4(0.0, 0.0, 0.0, 0.0);
			return;
		}
		voteA = 0;
		voteB = 0;
		debugA = 0;
		debugB = 0;
		voteCurves();
		voteIslands();
		voteSparsePixels();
		
		//eliminate loser
		if (voteA == voteB) {
			color0 = vec4(0.0, debugA/255.0, debugB/255.0, 0.0);
		} else if (voteA > voteB) {
			color0 = vec4(32.0/255.0, debugA/255.0, debugB/255.0, 0.0);
		} else {
			color0 = vec4(64.0/255.0, debugA/255.0, debugB/255.0, 0.0);
		}
	} else if (fragmentValue == 32 || fragmentValue == 64) {
		//we just hit a Diagonal ... it might be fully connected anyways
		if (isFullyConnectedD()) {
			color0 = vec4(0.0, 0.0, 0.0, 0.0);
		} else {
			color0 = fragmentXColor;
		}
	} else {
		//copy texel
		color0 = texelFetch(similarityGraph, ivec2(gl_FragCoord.xy), 0);
	}
}