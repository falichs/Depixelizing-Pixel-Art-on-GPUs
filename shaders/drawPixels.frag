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

in vec2 UV;

uniform sampler2D pixelArt;
uniform float uf_zoomFactor;
uniform vec2 uv2_zoomWindowCenter;

layout(location = 0) out vec4 color0;

void main() {
	//Y = 0.299*R + 0.587*G + 0.114*B
	//U = (B-Y)*0.493
	//V = (R-Y)*0.877
	//color0 = (texture(pixelArt,UV) + 0.2f )/ 1.2f; // debug ... brightens image, so graph connections are visible on dark pixels.
	color0 = texture(pixelArt,UV * uf_zoomFactor + (uv2_zoomWindowCenter - uf_zoomFactor*0.5));

}