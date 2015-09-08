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

// Input vertex data
layout(location = 0) in vec3 vertexPosition_modelspace;
uniform ivec2 positionsRange; 
uniform float uf_zoomFactor;
uniform vec2 uv2_zoomWindowCenter;

void main(){

	gl_Position = vec4((2.0*vertexPosition_modelspace.xy/positionsRange.xy-(1.0-1.0/positionsRange.xy)) * uf_zoomFactor + uv2_zoomWindowCenter, -1.0, 1.0);
}

